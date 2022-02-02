// kaldi_native_io/python/csrc/kaldiio.h
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#ifndef KALDI_NATIVE_IO_PYTHON_CSRC_KALDIIO_H_
#define KALDI_NATIVE_IO_PYTHON_CSRC_KALDIIO_H_

#include "pybind11/numpy.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

namespace py = pybind11;

template <typename... Args>
using overload_cast_ = py::detail::overload_cast_impl<Args...>;

#endif  // KALDI_NATIVE_IO_PYTHON_CSRC_KALDIIO_H_
