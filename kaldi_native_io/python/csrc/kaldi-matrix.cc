// kaldi_native_io/python/csrc/kaldi-matrix.cc
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/python/csrc/kaldi-matrix.h"

#include <memory>

#include "kaldi_native_io/csrc/kaldi-matrix.h"

namespace kaldiio {

template <typename Real>
void PybindKaldiMatrixTpl(py::module &m, const std::string &class_name,
                          const std::string &class_help_doc = "") {
  using PyClass = Matrix<Real>;
  py::class_<PyClass>(m, class_name.c_str(), class_help_doc.c_str(),
                      pybind11::buffer_protocol())
      .def(py::init<>())
      .def(py::init(
          [](py::array_t<Real> array) -> std::unique_ptr<Matrix<Real>> {
            // TODO(fangjun): Avoid extra copy here!
            KALDIIO_ASSERT(array.ndim() == 2);
            auto ans = std::make_unique<Matrix<Real>>(
                array.shape(0), array.shape(1), kUndefined);
            for (int32_t r = 0; r != ans->NumRows(); ++r) {
              for (int32_t c = 0; c != ans->NumCols(); ++c) {
                ans->operator()(r, c) = array.at(r, c);
              }
            }
            return std::move(ans);
          }))
      .def("numpy",
           [](py::object obj) {
             auto *m = obj.cast<Matrix<Real> *>();
             return py::array_t<Real>(
                 {m->NumRows(), m->NumCols()},                // shape
                 {sizeof(Real) * m->Stride(), sizeof(Real)},  // stride in bytes
                 m->Data(),                                   // ptr
                 obj);  // it will increase the reference
                        // count of **this** matrix
           })
      .def_buffer([](const PyClass &m) -> py::buffer_info {
        return pybind11::buffer_info(
            (void *)m.Data(),  // pointer to buffer
            sizeof(Real),      // size of one scalar
            pybind11::format_descriptor<Real>::format(),
            2,                           // num-axes
            {m.NumRows(), m.NumCols()},  // buffer dimensions
            {sizeof(Real) * m.Stride(),
             sizeof(Real)});  // stride for each index (in chars)
      });
}

static void PybindHtkHeader(py::module &m) {
  using PyClass = HtkHeader;
  py::class_<PyClass>(
      m, "HtkHeader",
      "See https://labrosa.ee.columbia.edu/doc/HTKBook21/node58.html for help.")
      .def(py::init<>())
      .def(py::init<int32_t, int32_t, int16_t, uint16_t>(),
           py::arg("num_samples"), py::arg("sample_period"),
           py::arg("sample_size"), py::arg("sample_kind"))
      .def_readwrite("num_samples", &PyClass::mNSamples)
      .def_readwrite("sample_period", &PyClass::mSamplePeriod)
      .def_readwrite("sample_size", &PyClass::mSampleSize)
      .def_readwrite("sample_kind", &PyClass::mSampleKind)
      .def("__eq__",
           [](const PyClass &self, const PyClass &other) {
             return self.mNSamples == other.mNSamples &&
                    self.mSamplePeriod == other.mSamplePeriod &&
                    self.mSampleSize == other.mSampleSize &&
                    self.mSampleKind == other.mSampleKind;
           })
      .def("__str__",
           [](const PyClass &self) -> std::string {
             std::ostringstream os;
             os << "HtkHeader(num_samples=" << self.mNSamples << ",\n"
                << "          sample_period=" << self.mSamplePeriod << ",\n"
                << "          sample_size=" << self.mSampleSize << ",\n"
                << "          sample_kind=" << self.mSampleKind << ")\n";
             return os.str();
           })
      .def("__repr__", [](const PyClass &self) -> std::string {
        std::ostringstream os;
        os << "HtkHeader(num_samples=" << self.mNSamples << ",\n"
           << "          sample_period=" << self.mSamplePeriod << ",\n"
           << "          sample_size=" << self.mSampleSize << ",\n"
           << "          sample_kind=" << self.mSampleKind << ")\n";
        return os.str();
      });
}

void PybindKaldiMatrix(py::module &m) {
  PybindKaldiMatrixTpl<float>(m, "_FloatMatrix");
  PybindKaldiMatrixTpl<double>(m, "_DoubleMatrix");
  PybindHtkHeader(m);
}

}  // namespace kaldiio
