// kaldi_native_io/python/csrc/blob.cc
//
// Copyright (c)  2023  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/python/csrc/blob.h"

#include <string>
#include <vector>

#include "kaldi_native_io/csrc/kaldi-io.h"

namespace kaldiio {

int32_t BlobHolder::kMagicHeader = 0x20221114;

py::bytes ReadBlobObject(const std::string &filename) {
  if (!filename.empty() && filename[filename.size() - 1] == ']') {
    // This filename seems to have a 'range'... like foo.ark:4312423[20:30].
    // (the bit in square brackets is the range).
    std::string rxfilename, range;
    if (!ExtractRangeSpecifier(filename, &rxfilename, &range)) {
      KALDIIO_ERR
          << "Could not make sense of possible range specifier in filename "
          << "while reading matrix: " << filename;
    }
    std::vector<std::string> splits;
    SplitStringToVector(range, ",", false, &splits);
    if (!((splits.size() == 1 && !splits[0].empty()))) {
      KALDIIO_ERR << "Invalid range specifier for vector: " << range;
    }

    std::vector<int32_t> index_range;
    bool status = true;
    if (splits[0] != ":")
      status = SplitStringToIntegers(splits[0], ":", false, &index_range);

    if (index_range.size() == 0) {
      index_range.push_back(0);
      index_range.push_back(-1);  // -1 means the end
    } else if (static_cast<int32_t>(index_range.size()) != 2) {
      KALDIIO_ERR << "Invalid range specifier for vector: " << range;
    }

    Input ki(rxfilename);
    BlobHolder holder;
    holder.Read(ki.Stream(), index_range[0], index_range[1]);
    return holder.Value();  // Return a copy

  } else {
    Input ki(filename);
    BlobHolder holder;
    holder.Read(ki.Stream());
    return holder.Value();  // Return a copy
  }
}

}  // namespace kaldiio
