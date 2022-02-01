// kaldi_native_io/python/csrc/kaldiio.cc
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/python/csrc/kaldiio.h"

#include "kaldi_native_io/python/csrc/kaldi-table.h"

namespace kaldiio {

PYBIND11_MODULE(_kaldi_native_io, m) {
  m.doc() = "Python wrapper for kaldi native I/O";
  PybindKaldiTable(m);
}

}  // namespace kaldiio
