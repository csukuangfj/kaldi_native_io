// kaldi_native_io/csrc/matrix-shape.h
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#ifndef KALDI_NATIVE_IO_CSRC_MATRIX_SHAPE_H_
#define KALDI_NATIVE_IO_CSRC_MATRIX_SHAPE_H_

#include "kaldi_native_io/csrc/log.h"

namespace kaldiio {
class MatrixShape {
 public:
  explicit MatrixShape(int32_t num_rows = 0, int32_t num_cols = 0)
      : num_rows_(num_rows), num_cols_(num_cols) {}

  // Read the matrix header from a stream that contains a matrix
  void Read(std::istream &is, bool binary);

  int32_t NumRows() const { return num_rows_; }
  int32_t NumCols() const { return num_cols_; }

 private:
  void ReadCompressedBinary(std::istream &is);
  void ReadNonCompressedBinary(std::istream &is);

 private:
  int32_t num_rows_ = 0;
  int32_t num_cols_ = 0;
};

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_MATRIX_SHAPE_H_
