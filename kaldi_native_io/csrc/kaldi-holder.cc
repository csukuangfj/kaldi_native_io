// kaldi_native_io/csrc/kaldi-holder.cc
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/util/kaldi-holder.cc

// Copyright 2009-2011     Microsoft Corporation
//                2016     Xiaohui Zhang

#include "kaldi_native_io/csrc/kaldi-holder.h"

#include <algorithm>
#include <vector>

#include "kaldi_native_io/csrc/kaldi-matrix.h"
#include "kaldi_native_io/csrc/kaldi-vector.h"
#include "kaldi_native_io/csrc/log.h"
#include "kaldi_native_io/csrc/text-utils.h"

namespace kaldiio {

bool ExtractRangeSpecifier(const std::string &rxfilename_with_range,
                           std::string *data_rxfilename, std::string *range) {
  if (rxfilename_with_range.empty() ||
      rxfilename_with_range[rxfilename_with_range.size() - 1] != ']')
    KALDIIO_ERR << "ExtractRangeRspecifier called wrongly.";
  std::vector<std::string> splits;
  SplitStringToVector(rxfilename_with_range, "[", false, &splits);
  if (splits.size() == 2 && !splits[0].empty() && splits[1].size() > 1) {
    *data_rxfilename = splits[0];
    range->assign(splits[1], 0, splits[1].size() - 1);
    return true;
  }
  return false;
}

template <class Real>
bool ExtractObjectRange(const Vector<Real> &input, const std::string &range,
                        Vector<Real> *output) {
  if (range.empty()) {
    KALDIIO_ERR << "Empty range specifier.";
    return false;
  }
  std::vector<std::string> splits;
  SplitStringToVector(range, ",", false, &splits);
  if (!((splits.size() == 1 && !splits[0].empty()))) {
    KALDIIO_ERR << "Invalid range specifier for vector: " << range;
    return false;
  }
  std::vector<int32_t> index_range;
  bool status = true;
  if (splits[0] != ":")
    status = SplitStringToIntegers(splits[0], ":", false, &index_range);

  if (index_range.size() == 0) {
    index_range.push_back(0);
    index_range.push_back(input.Dim() - 1);
  }

  // Length tolerance of 3 -- 2 to account for edge effects when
  // frame-length is 25ms and frame-shift is 10ms, and 1 for rounding effects
  // since segments are usually retained up to 2 decimal places.
  int32_t length_tolerance = 3;
  if (!(status && index_range.size() == 2 && index_range[0] >= 0 &&
        index_range[0] <= index_range[1] &&
        index_range[1] < input.Dim() + length_tolerance)) {
    KALDIIO_ERR << "Invalid range specifier: " << range
                << " for vector of size " << input.Dim();
    return false;
  }

  if (index_range[1] >= input.Dim())
    KALDIIO_WARN << "Range " << index_range[0] << ":" << index_range[1]
                 << " goes beyond the vector dimension " << input.Dim();
  int32_t size = std::min(index_range[1], input.Dim() - 1) - index_range[0] + 1;
  output->Resize(size, kUndefined);
  output->CopyFromVec(input.Range(index_range[0], size));
  return true;
}

// template instantiation
template bool ExtractObjectRange(const Vector<double> &, const std::string &,
                                 Vector<double> *);
template bool ExtractObjectRange(const Vector<float> &, const std::string &,
                                 Vector<float> *);

// Parse matrix range specifier in form r1:r2,c1:c2
// where any of those four numbers can be missing. In those
// cases, the missing number is set either to 0 (for r1 or c1)
// or the value of parameter rows -1 or columns -1 (which
// represent the dimensions of the original matrix) for missing
// r2 or c2, respectively.
// Examples of valid ranges: 0:39,: or :,:3 or :,5:10
bool ParseMatrixRangeSpecifier(const std::string &range, const int rows,
                               const int cols, std::vector<int32> *row_range,
                               std::vector<int32> *col_range) {
  if (range.empty()) {
    KALDIIO_ERR << "Empty range specifier.";
    return false;
  }
  std::vector<std::string> splits;
  SplitStringToVector(range, ",", false, &splits);
  if (!((splits.size() == 1 && !splits[0].empty()) ||
        (splits.size() == 2 && !splits[0].empty() && !splits[1].empty()))) {
    KALDIIO_ERR << "Invalid range specifier for matrix: " << range;
    return false;
  }

  bool status = true;

  if (splits[0] != ":")
    status = SplitStringToIntegers(splits[0], ":", false, row_range);

  if (splits.size() == 2 && splits[1] != ":") {
    status = status && SplitStringToIntegers(splits[1], ":", false, col_range);
  }
  if (row_range->size() == 0) {
    row_range->push_back(0);
    row_range->push_back(rows - 1);
  }
  if (col_range->size() == 0) {
    col_range->push_back(0);
    col_range->push_back(cols - 1);
  }

  // Length tolerance of 3 -- 2 to account for edge effects when
  // frame-length is 25ms and frame-shift is 10ms, and 1 for rounding effects
  // since segments are usually retained up to 2 decimal places.
  int32 length_tolerance = 3;
  if (!(status && row_range->size() == 2 && col_range->size() == 2 &&
        row_range->at(0) >= 0 && row_range->at(0) <= row_range->at(1) &&
        row_range->at(1) < rows + length_tolerance && col_range->at(0) >= 0 &&
        col_range->at(0) <= col_range->at(1) && col_range->at(1) < cols)) {
    KALDIIO_ERR << "Invalid range specifier: " << range
                << " for matrix of size " << rows << "x" << cols;
    return false;
  }

  if (row_range->at(1) >= rows)
    KALDIIO_WARN << "Row range " << row_range->at(0) << ":" << row_range->at(1)
                 << " goes beyond the number of rows of the "
                 << "matrix " << rows;
  return status;
}

template <class Real>
bool ExtractObjectRange(const CompressedMatrix &input, const std::string &range,
                        Matrix<Real> *output) {
  std::vector<int32> row_range, col_range;

  if (!ParseMatrixRangeSpecifier(range, input.NumRows(), input.NumCols(),
                                 &row_range, &col_range)) {
    KALDIIO_ERR << "Could not parse range specifier \"" << range << "\".";
  }

  int32 row_size =
            std::min(row_range[1], input.NumRows() - 1) - row_range[0] + 1,
        col_size = col_range[1] - col_range[0] + 1;

  output->Resize(row_size, col_size, kUndefined);
  input.CopyToMat(row_range[0], col_range[0], output);
  return true;
}

// template instantiation
template bool ExtractObjectRange(const CompressedMatrix &, const std::string &,
                                 Matrix<float> *);
template bool ExtractObjectRange(const CompressedMatrix &, const std::string &,
                                 Matrix<double> *);

template <class Real>
bool ExtractObjectRange(const Matrix<Real> &input, const std::string &range,
                        Matrix<Real> *output) {
  std::vector<int32> row_range, col_range;

  if (!ParseMatrixRangeSpecifier(range, input.NumRows(), input.NumCols(),
                                 &row_range, &col_range)) {
    KALDIIO_ERR << "Could not parse range specifier \"" << range << "\".";
  }

  int32 row_size =
            std::min(row_range[1], input.NumRows() - 1) - row_range[0] + 1,
        col_size = col_range[1] - col_range[0] + 1;
  output->Resize(row_size, col_size, kUndefined);
  output->CopyFromMat(
      input.Range(row_range[0], row_size, col_range[0], col_size));
  return true;
}

// template instantiation
template bool ExtractObjectRange(const Matrix<double> &, const std::string &,
                                 Matrix<double> *);
template bool ExtractObjectRange(const Matrix<float> &, const std::string &,
                                 Matrix<float> *);

}  // namespace kaldiio
