// kaldi_native_io/python/csrc/copy-blob.cc
//
// Copyright (c)  2023  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/csrc/kaldi-io.h"
#include "kaldi_native_io/csrc/kaldi-table.h"
#include "kaldi_native_io/python/csrc/parse-options.h"

namespace kaldiio {
class Blob {
 public:
  using T = std::vector<uint8_t>;
  Blob() = default;

  static bool Write(std::ostream &os, bool binary, const T &t) {
    if (!binary) {
      KALDIIO_ERR << "Only support binary mode for bytes";
      return false;
    }

    InitKaldiOutputStream(os, binary);  // Puts binary header if binary mode.
    return Write(os, t);
  }

  // raw: true to not write magic header and length
  static bool Write(std::ostream &os, const T &t, bool raw = false) {
    // format of the data
    // a magic header 4 bytes: 0x20221114
    // size of the data 8 bytes
    // the data

    if (!raw) {
      os.write(reinterpret_cast<const char *>(&kMagicHeader),
               sizeof(kMagicHeader));

      int64_t len = t.size();
      os.write(reinterpret_cast<const char *>(&len), sizeof(len));
    }

    os.write(reinterpret_cast<const char *>(t.data()), t.size());

    return os.good();
  };

  bool Read(std::istream &is) {
    bool is_binary;
    if (!InitKaldiInputStream(is, &is_binary)) {
      KALDIIO_WARN << "Reading Table object [blob], failed reading binary"
                      " header\n";
      return false;
    }

    KALDIIO_ASSERT(is_binary) << "Support only binary mode for blob";
    ReadImpl(is, is_binary);
    return true;
  }

  void Read(std::istream &is, bool is_binary) { ReadImpl(is, is_binary); }

  void ReadImpl(std::istream &is, bool is_binary) {
    KALDIIO_ASSERT(is_binary) << "Support only binary mode for blob";

    int32_t magic_header;

    is.read(reinterpret_cast<char *>(&magic_header), sizeof(magic_header));
    if (magic_header != kMagicHeader || is.fail()) {
      std::ostringstream os;
      os << "Incorrect magic header. Expected: " << kMagicHeader
         << ". Given: " << magic_header;
      KALDIIO_ERR << os.str();
    }

    int64_t len = -1;
    is.read(reinterpret_cast<char *>(&len), sizeof(len));
    if (len < 0 || is.fail()) {
      KALDIIO_ERR << "Failed to read the length";
    }

    data_.resize(len);
    is.read(reinterpret_cast<char *>(data_.data()), len);

    if (!is.good()) {
      // s is copied to value_
      KALDIIO_ERR << "Failed to read in Blob::Read";
    }
  }

  void Clear() { data_.clear(); }

  // always in binary
  static bool IsReadInBinary() { return true; }

  T &Value() { return data_; }

  void Swap(Blob *other) { std::swap(data_, other->data_); }

  bool ExtractRange(const Blob & /*other*/, const std::string & /*range*/) {
    KALDIIO_ERR << "ExtractRange is not defined for Blob.";
    return false;
  }

 private:
  // must be the same as BlobHolder::kMagicHeader
  static int32_t kMagicHeader /*= 0x20221114*/;
  std::vector<uint8_t> data_;
};

int32_t Blob::kMagicHeader = 0x20221114;
}  // namespace kaldiio

int main(int argc, char *argv[]) {
  const char *usage =
      "Copy blob, or archives of blob. "
      "\n"
      "Usage: copy-blob [options] "
      "(<blob-in-rspecifier>|<blob-in-rxfilename>) "
      "(<blob-out-wspecifier>|<blob-out-wxfilename>)\n"
      " e.g.: copy-blob 1.ark - | soxi -\n"
      "   copy-blob ark:2.ark ark,scp:out.ark,out.scp\n";
  bool binary = true;  // must be true
                       //
  kaldiio::ParseOptions po(usage);
  po.Read(argc, argv);
  if (po.NumArgs() != 2) {
    po.PrintUsage();
    exit(1);
  }

  std::string in_fn = po.GetArg(1);
  std::string out_fn = po.GetArg(2);

  bool in_is_rspecifier = (kaldiio::ClassifyRspecifier(in_fn, NULL, NULL) !=
                           kaldiio::kNoRspecifier);
  bool out_is_wspecifier =
      (kaldiio::ClassifyWspecifier(out_fn, NULL, NULL, NULL) !=
       kaldiio::kNoWspecifier);

  if (in_is_rspecifier != out_is_wspecifier)
    KALDIIO_ERR << "Cannot mix archives with regular files (copying blobs)";

  if (!in_is_rspecifier) {
    kaldiio::Blob blob;
    kaldiio::ReadKaldiObject(in_fn, &blob);
    kaldiio::Output ko(out_fn, binary, false /*write_header*/);
    blob.Write(ko.Stream(), blob.Value(), true /*raw*/);
    KALDIIO_LOG << "Copied " << in_fn << " to " << out_fn;
    return 0;
  } else {
    int32_t num_done = 0;
    kaldiio::TableWriter<kaldiio::Blob> writer(out_fn);
    kaldiio::SequentialTableReader<kaldiio::Blob> reader(in_fn);

    for (; !reader.Done(); reader.Next(), ++num_done) {
      writer.Write(reader.Key(), reader.Value());
    }
    KALDIIO_LOG << "Copied " << num_done << " blobs.";
  }
  return 0;
}
