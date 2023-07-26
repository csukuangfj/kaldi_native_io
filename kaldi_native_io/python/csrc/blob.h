// kaldi_native_io/python/csrc/blob.h
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#ifndef KALDI_NATIVE_IO_PYTHON_CSRC_BLOB_H_
#define KALDI_NATIVE_IO_PYTHON_CSRC_BLOB_H_

#include <string>
#include <utility>
#include <vector>

#include "kaldi_native_io/csrc/kaldi-holder.h"
#include "kaldi_native_io/python/csrc/kaldiio.h"

namespace kaldiio {

class BlobHolder {
 public:
  using T = py::bytes;
  BlobHolder() = default;
  ~BlobHolder() = default;

  static bool Write(std::ostream &os, bool binary, const T &t) {
    if (!binary) {
      PyErr_SetString(PyExc_ValueError, "Only support binary mode for bytes");
      throw py::error_already_set();
    }

    char *bytes_array;
    Py_ssize_t length;
    if (PyBytes_AsStringAndSize(t.ptr(), &bytes_array, &length) != 0) {
      PyErr_SetString(PyExc_ValueError, "Expect a bytes object");
      throw py::error_already_set();
    }

    InitKaldiOutputStream(os, binary);  // Puts binary header if binary mode.

    // format of the data
    // a magic header 4 bytes: 0x20221114
    // size of the data 8 bytes
    // the data

    os.write(reinterpret_cast<const char *>(&kMagicHeader),
             sizeof(kMagicHeader));

    int64_t len = length;
    os.write(reinterpret_cast<const char *>(&len), sizeof(len));

    os.write(reinterpret_cast<const char *>(bytes_array), len);

    return os.good();
  };

  void Clear() { value_ = py::bytes(); }

  bool Read(std::istream &is) {
    bool is_binary;
    if (!InitKaldiInputStream(is, &is_binary)) {
      KALDIIO_WARN << "Reading Table object [blob], failed reading binary"
                      " header\n";
      return false;
    }

    KALDIIO_ASSERT(is_binary) << "Support only binary mode for blob";

    int32_t magic_header;

    is.read(reinterpret_cast<char *>(&magic_header), sizeof(magic_header));
    if (magic_header != kMagicHeader || is.fail()) {
      std::ostringstream os;
      os << "Incorrect magic header. Expected: " << kMagicHeader
         << ". Given: " << magic_header;
      PyErr_SetString(PyExc_ValueError, os.str().c_str());
      throw py::error_already_set();
    }

    int64_t len = -1;
    is.read(reinterpret_cast<char *>(&len), sizeof(len));
    if (len < 0 || is.fail()) {
      PyErr_SetString(PyExc_ValueError, "Failed to read the length");
      throw py::error_already_set();
    }

    std::vector<char> s(len);
    is.read(reinterpret_cast<char *>(s.data()), len);

    if (is.good()) {
      // s is copied to value_
      value_ = py::bytes(s.data(), static_cast<int32_t>(len));
    }

    return is.good();
  }

  bool Read(std::istream &is, int32_t start, int32_t end) {
    bool is_binary;
    if (!InitKaldiInputStream(is, &is_binary)) {
      KALDIIO_WARN << "Reading Table object [blob], failed reading binary"
                      " header\n";
      return false;
    }

    KALDIIO_ASSERT(is_binary) << "Support only binary mode for blob";

    int32_t magic_header;

    is.read(reinterpret_cast<char *>(&magic_header), sizeof(magic_header));
    if (magic_header != kMagicHeader || is.fail()) {
      std::ostringstream os;
      os << "Incorrect magic header. Expected: " << kMagicHeader
         << ". Given: " << magic_header;
      PyErr_SetString(PyExc_ValueError, os.str().c_str());
      throw py::error_already_set();
    }

    int64_t len = -1;
    is.read(reinterpret_cast<char *>(&len), sizeof(len));
    if (len < 0 || is.fail()) {
      PyErr_SetString(PyExc_ValueError, "Failed to read the length");
      throw py::error_already_set();
    }
    if (end == -1) {
      end = len - 1;
    }

    if (end > len - 1) {
      KALDIIO_ERR << "Invalid end: " << end << ".\nlen is: " << len << ".";
    }

    if (start < 0 || start > end) {
      KALDIIO_ERR << "Invalid start: " << start << ".\nend is: " << end << ".";
    }

    // end is inclusive
    std::vector<char> s(end - start + 1);
    is.seekg(start, is.cur);
    is.read(reinterpret_cast<char *>(s.data()), s.size());

    if (is.good()) {
      // s is copied to value_
      value_ = py::bytes(s.data(), static_cast<int32_t>(s.size()));
    }

    return is.good();
  }

  // always in binary
  static bool IsReadInBinary() { return true; }

  T &Value() { return value_; }

  void Swap(BlobHolder *other) { std::swap(value_.ptr(), other->value_.ptr()); }

  bool ExtractRange(const BlobHolder & /*other*/,
                    const std::string & /*range*/) {
    KALDIIO_ERR << "ExtractRange is not defined for this type of holder.";
    return false;
  }

 private:
  py::bytes value_;

  // defained in ./kaldi-table.cc
  static int32_t kMagicHeader /*= 0x20221114*/;
};

py::bytes ReadBlobObject(const std::string &filename);

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_PYTHON_CSRC_BLOB_H_
