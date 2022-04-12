// kaldi_native_io/python/csrc/wave-reader.cc
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/python/csrc/wave-reader.h"

#include "kaldi_native_io/csrc/kaldi-io.h"
#include "kaldi_native_io/csrc/wave-reader.h"

namespace kaldiio {

void PybindWaveReader(py::module &m) {
  {
    using PyClass = WaveData;
    py::class_<PyClass>(m, "WaveData")
        .def(py::init<float, Matrix<float>>())
        .def_property_readonly("sample_freq", &PyClass::SampFreq)
        .def_property_readonly("duration", &PyClass::Duration)  // seconds
        .def_property_readonly("data", &PyClass::Data);
  }

  {
    using PyClass = WaveInfo;
    py::class_<PyClass>(m, "WaveInfo")
        .def(py::init<>())
        .def_property_readonly("sample_freq", &PyClass::SampFreq)
        .def_property_readonly("sample_count", &PyClass::SampleCount)
        .def_property_readonly("duration", &PyClass::Duration)
        .def_property_readonly("num_channels", &PyClass::NumChannels)
        .def_property_readonly("block_align", &PyClass::BlockAlign)
        .def_property_readonly("data_bytes", &PyClass::DataBytes)
        .def_property_readonly("reverse_bytes", &PyClass::ReverseBytes)
        .def("__str__", [](const PyClass &self) -> std::string {
          std::ostringstream os;
          os << "sample_freq: " << self.SampFreq() << "\n";
          os << "sample_count: " << self.SampleCount() << "\n";
          os << "duration: " << self.Duration() << "\n";
          os << "num_channels: " << self.NumChannels() << "\n";
          os << "block_align: " << self.BlockAlign() << "\n";
          os << "data_bytes: " << self.DataBytes() << "\n";
          os << "reverse_bytes: " << self.ReverseBytes() << "\n";
          return os.str();
        });
  }

  m.def(
      "read_wave_info",
      [](const std::string &rxfilename) -> WaveInfo {
        InputType t = ClassifyRxfilename(rxfilename);
        if (t != kFileInput) {
          KALDIIO_ERR << "Expect an rxfilename of type kFileInput. \n"
                      << "Given: " << PrintableRxfilename(rxfilename)
                      << " (of type " << InputTypeToString(t) << ")";
        }
        Input ki(rxfilename);
        if (!ki.IsOpen()) {
          KALDIIO_ERR << "Failed to open " << rxfilename;
        }

        WaveInfo info;
        info.Read(ki.Stream());  // Throws exception on failure.
        return info;
      },
      py::arg("rxfilename"));

  m.def(
      "read_wave",
      [](const std::string &rxfilename) -> WaveData {
        Input ki(rxfilename);
        if (!ki.IsOpen()) {
          KALDIIO_ERR << "Failed to open " << rxfilename;
        }

        WaveData data;
        data.Read(ki.Stream());  // Throws exception on failure.
        return data;
      },
      py::arg("rxfilename"));
}

}  // namespace kaldiio
