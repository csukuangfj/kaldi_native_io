// kaldi_native_io/csrc/kaldi-utils.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/base/kaldi-utils.h

// Copyright 2009-2011  Ondrej Glembek;  Microsoft Corporation;
//                      Saarland University;  Karel Vesely;  Yanmin Qian

#ifndef KALDI_NATIVE_IO_CSRC_KALDI_UTILS_H_
#define KALDI_NATIVE_IO_CSRC_KALDI_UTILS_H_

#include <string>

namespace kaldiio {

// CharToString prints the character in a human-readable form, for debugging.
std::string CharToString(const char &c);

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_KALDI_UTILS_H_
