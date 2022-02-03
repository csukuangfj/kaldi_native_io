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

void PybindKaldiMatrix(py::module &m) {
  PybindKaldiMatrixTpl<float>(m, "_FloatMatrix");
  PybindKaldiMatrixTpl<double>(m, "_DoubleMatrix");
}

}  // namespace kaldiio
