// kaldi_native_io/python/csrc/kaldi-matrix.h
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#ifndef KALDI_NATIVE_IO_PYTHON_CSRC_KALDI_MATRIX_H_
#define KALDI_NATIVE_IO_PYTHON_CSRC_KALDI_MATRIX_H_

#include "kaldi_native_io/python/csrc/kaldiio.h"

namespace kaldiio {

void PybindKaldiMatrix(py::module &m);  // NOLINT

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_PYTHON_CSRC_KALDI_MATRIX_H_
