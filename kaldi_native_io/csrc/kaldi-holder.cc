// kaldi_native_io/csrc/kaldi-holder.cc
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/util/kaldi-holder.cc

// Copyright 2009-2011     Microsoft Corporation
//                2016     Xiaohui Zhang

#include "kaldi_native_io/csrc/kaldi-holder.h"

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

}  // namespace kaldiio
