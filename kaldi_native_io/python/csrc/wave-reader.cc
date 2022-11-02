// kaldi_native_io/python/csrc/wave-reader.cc
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/python/csrc/wave-reader.h"

#include <memory>
#include <string>

#include "kaldi_native_io/csrc/kaldi-io.h"
#include "kaldi_native_io/csrc/wave-reader.h"

namespace kaldiio {

void PybindWaveReader(py::module &m) {  // NOLINT
  {
    using PyClass = WaveData;
    py::class_<PyClass>(m, "WaveData")
        .def(py::init<float, Matrix<float>>(), py::arg("sample_freq"),
             py::arg("data"))
        .def(
            py::init([](float samp_freq,
                        py::array_t<float> array) -> std::unique_ptr<WaveData> {
              KALDIIO_ASSERT(array.ndim() == 1 || array.ndim() == 2);
              int32_t num_channels;
              int32_t num_cols;
              if (array.ndim() == 1) {
                num_channels = 1;
                num_cols = array.size();
              } else {
                num_channels = array.shape(0);
                num_cols = array.shape(1);
              }

              Matrix<float> data(num_channels, num_cols);

              if (array.ndim() == 1) {
                float *p = data.Data();
                for (int32_t i = 0; i != num_cols; ++i) {
                  p[i] = array.at(i);
                }
              } else {
                for (int32_t r = 0; r != data.NumRows(); ++r) {
                  for (int32_t c = 0; c != data.NumCols(); ++c) {
                    data(r, c) = array.at(r, c);
                  }
                }
              }
              return std::make_unique<WaveData>(samp_freq, data);
            }),
            py::arg("sample_freq"), py::arg("data"))
        .def_property_readonly("sample_freq", &PyClass::SampFreq)
        .def_property_readonly("duration", &PyClass::Duration)  // seconds
        .def_property_readonly("data", &PyClass::Data)
        .def("__str__", [](const PyClass &self) -> std::string {
          std::ostringstream os;
          os << "sample_freq: " << self.SampFreq() << " Hz\n";
          os << "num_channels: " << self.Data().NumRows() << "\n";
          os << "duration: " << self.Duration() << " s\n";
          return os.str();
        });
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
