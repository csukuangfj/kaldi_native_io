// kaldi_native_io/python/csrc/compressed-matrix.h
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#ifndef KALDI_NATIVE_IO_PYTHON_CSRC_COMPRESSED_MATRIX_H_
#define KALDI_NATIVE_IO_PYTHON_CSRC_COMPRESSED_MATRIX_H_

#include "kaldi_native_io/python/csrc/kaldiio.h"

namespace kaldiio {

void PybindCompressedMatrix(py::module &m);  // NOLINT

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_PYTHON_CSRC_COMPRESSED_MATRIX_H_
