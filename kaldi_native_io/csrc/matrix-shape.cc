// kaldi_native_io/csrc/matrix-shape.cc
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/csrc/matrix-shape.h"

#include <string>

#include "kaldi_native_io/csrc/compressed-matrix.h"
#include "kaldi_native_io/csrc/io-funcs.h"

namespace kaldiio {

void MatrixShape::Read(std::istream &is, bool binary) {
  if (binary) {  // Read in binary mode.
    int32_t peekval = Peek(is, binary);

    if (peekval == 'C') {
      ReadCompressedBinary(is);
    } else {
      ReadNonCompressedBinary(is);
    }
  } else {
    // for non-binary read
    Matrix<float> m;
    m.Read(is, binary);
    num_rows_ = m.NumRows();
    num_cols_ = m.NumCols();
  }
}

void MatrixShape::ReadCompressedBinary(std::istream &is) {
  bool binary = true;
  int peekval = Peek(is, binary);

  if (peekval != 'C') {
    KALDIIO_ERR << "Expect token 'C'. Given: " << static_cast<char>(peekval);
  }

  std::string tok;  // Should be CM (format 1) or CM2 (format 2)
  ReadToken(is, binary, &tok);

  CompressedMatrix::GlobalHeader h;
  if (tok == "CM") {
    h.format = 1;  //  kOneByteWithColHeaders
  } else if (tok == "CM2") {
    h.format = 2;  // kTwoByte
  } else if (tok == "CM3") {
    h.format = 3;  // kOneByte
  } else {
    KALDIIO_ERR << "Unexpected token " << tok << ", expecting CM, CM2 or CM3";
  }
  // don't read the "format" -> hence + 4, - 4.
  is.read(reinterpret_cast<char *>(&h) + 4, sizeof(h) - 4);
  if (is.fail()) KALDIIO_ERR << "Failed to read header";

  num_rows_ = h.num_rows;
  num_cols_ = h.num_cols;
}

void MatrixShape::ReadNonCompressedBinary(std::istream &is) {
  bool binary = true;
  std::string token;
  ReadToken(is, binary, &token);
  if (token != "FM" && token != "DM") {
    KALDIIO_ERR << "Expect token FM or DM. Given: " << token;
  }

  ReadBasicType(is, binary, &num_rows_);  // throws on error.
  ReadBasicType(is, binary, &num_cols_);  // throws on error.
}

}  // namespace kaldiio
