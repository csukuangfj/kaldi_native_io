// kaldi_native_io/python/csrc/wave-reader.h
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#ifndef KALDI_NATIVE_IO_PYTHON_CSRC_WAVE_READER_H_
#define KALDI_NATIVE_IO_PYTHON_CSRC_WAVE_READER_H_

#include "kaldi_native_io/python/csrc/kaldiio.h"

namespace kaldiio {

void PybindWaveReader(py::module &m);  // NOLINT

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_PYTHON_CSRC_WAVE_READER_H_
