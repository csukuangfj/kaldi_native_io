// kaldi_native_io/csrc/kaldi-holder.cc
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/util/kaldi-holder.cc

// Copyright 2009-2011     Microsoft Corporation
//                2016     Xiaohui Zhang

#include "kaldi_native_io/csrc/kaldi-holder.h"

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

}  // namespace kaldiio
