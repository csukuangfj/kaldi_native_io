// kaldi_native_io/csrc/kaldi-matrix-inl.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/matrix/kaldi-matrix-inl.h

// Copyright 2009-2011  Microsoft Corporation;  Haihua Xu

#ifndef KALDI_NATIVE_IO_CSRC_KALDI_MATRIX_INL_H_
#define KALDI_NATIVE_IO_CSRC_KALDI_MATRIX_INL_H_

// Included by kaldi-matrix.h; do not include this file directly.

namespace kaldiio {

/// Empty constructor
template <typename Real>
Matrix<Real>::Matrix() : MatrixBase<Real>(nullptr, 0, 0, 0) {}

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_KALDI_MATRIX_INL_H_
