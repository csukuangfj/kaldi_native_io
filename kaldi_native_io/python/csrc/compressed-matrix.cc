// kaldi_native_io/python/csrc/kaldi-matrix.cc
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/csrc/compressed-matrix.h"

#include "kaldi_native_io/python/csrc/compressed-matrix.h"
#include "kaldi_native_io/python/csrc/kaldi-matrix.h"

namespace kaldiio {

static void PybindCompressionMethod(py::module &m) {  // NOLINT
  py::enum_<CompressionMethod>(
      m, "CompressionMethod", py::arithmetic(),
      "The enum CompressionMethod is used when creating a CompressedMatrix (a "
      "lossily compressed matrix) from a regular Matrix. It dictates how we "
      "choose the compressed format and how we choose the ranges of floats "
      "that are represented by particular integers.")
      .value(
          "kAutomaticMethod", kAutomaticMethod,
          "This is the default when you don't specify the compression method. "
          " It is a shorthand for using kSpeechFeature if the num-rows is "
          "more than 8, and kTwoByteAuto otherwise.")
      .value(
          "kSpeechFeature", kSpeechFeature,
          "This is the most complicated of the compression methods, and was "
          "designed for speech features which have a roughly Gaussian "
          "distribution with different ranges for each dimension.  Each "
          "element is stored in one byte, but there is an 8-byte header per "
          "column; the spacing of the integer values is not uniform but is in "
          "3 ranges.")
      .value("kTwoByteAuto", kTwoByteAuto,
             "Each element is stored in two bytes as a uint16, with the "
             "representable range of values chosen automatically with the "
             "minimum and maximum elements of the matrix as its edges.")
      .value("kTwoByteSignedInteger", kTwoByteSignedInteger,
             "Each element is stored in two bytes as a uint16, with the "
             "representable range of value chosen to coincide with what you'd "
             "get if you stored signed integers, i.e. [-32768.0, 32767.0].  "
             "Suitable for waveform data that was previously stored as 16-bit "
             "PCM.")
      .value("kOneByteAuto", kOneByteAuto,
             "Each element is stored in one byte as a uint8, with the "
             "representable range of values chosen automatically with the "
             "minimum and maximum elements of the matrix as its edges.")
      .value("kOneByteUnsignedInteger", kOneByteUnsignedInteger,
             "Each element is stored in one byte as a uint8, with the "
             "representable range of values equal to [0.0, 255.0].")
      .value("kOneByteZeroOne", kOneByteZeroOne,
             "Each element is stored in one byte as a uint8, with the "
             "representable range of values equal to [0.0, 1.0].  Suitable for "
             "image data that has previously been compressed as int8.")
      .export_values();
}

void PybindCompressedMatrix(py::module &m) {  // NOLINT
  PybindCompressionMethod(m);
  {
    using PyClass = CompressedMatrix;
    py::class_<PyClass>(m, "_CompressedMatrix")
        .def(py::init<>())
        .def(py::init<const Matrix<float> &, CompressionMethod>(),
             py::arg("mat"), py::arg("method") = kAutomaticMethod)
        .def(py::init<const Matrix<double> &, CompressionMethod>(),
             py::arg("mat"), py::arg("method") = kAutomaticMethod);
  }
}

}  // namespace kaldiio
