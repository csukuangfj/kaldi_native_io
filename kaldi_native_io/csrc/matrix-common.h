// kaldi_native_io/csrc/matrix-common.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/matrix/matrix-common.h

// Copyright 2009-2011  Microsoft Corporation

#ifndef KALDI_NATIVE_IO_CSRC_MATRIX_COMMON_H_
#define KALDI_NATIVE_IO_CSRC_MATRIX_COMMON_H_

namespace kaldiio {

typedef enum {
  kTrans = 112,   // = CblasTrans
  kNoTrans = 111  // = CblasNoTrans
} MatrixTransposeType;

typedef enum {
  kSetZero,
  kUndefined,
  kCopyData,
} MatrixResizeType;

typedef enum {
  kDefaultStride,
  kStrideEqualNumCols,
} MatrixStrideType;

template <typename Real>
class VectorBase;
template <typename Real>
class Vector;
template <typename Real>
class SubVector;

typedef int32_t MatrixIndexT;
typedef int32_t SignedMatrixIndexT;
typedef uint32_t UnsignedMatrixIndexT;

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_MATRIX_COMMON_H_
