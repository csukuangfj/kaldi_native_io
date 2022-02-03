// kaldi_native_io/csrc/kaldi-matrix-inl.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/matrix/kaldi-matrix-inl.h

// Copyright 2009-2011  Microsoft Corporation;  Haihua Xu

#ifndef KALDI_NATIVE_IO_CSRC_KALDI_MATRIX_INL_H_
#define KALDI_NATIVE_IO_CSRC_KALDI_MATRIX_INL_H_

#include "kaldi_native_io/csrc/kaldi-matrix.h"

namespace kaldiio {

/// Empty constructor
template <typename Real>
Matrix<Real>::Matrix() : MatrixBase<Real>(NULL, 0, 0, 0) {}

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_KALDI_MATRIX_INL_H_
