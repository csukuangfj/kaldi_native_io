// kaldi_native_io/python/csrc/kaldi-vector.cc
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/python/csrc/kaldi-vector.h"

#include <memory>
#include <string>

#include "kaldi_native_io/csrc/kaldi-io.h"
#include "kaldi_native_io/csrc/kaldi-vector.h"

namespace kaldiio {

template <typename Real>
void PybindKaldiVectorTpl(py::module &m,  // NOLINT
                          const std::string &class_name,
                          const std::string &class_help_doc = "") {
  using PyClass = Vector<Real>;
  py::class_<PyClass>(m, class_name.c_str(), class_help_doc.c_str(),
                      pybind11::buffer_protocol())
      .def(py::init<>())
      .def(py::init(
          [](py::array_t<Real> array) -> std::unique_ptr<Vector<Real>> {
            // TODO(fangjun): Avoid extra copy here!
            KALDIIO_ASSERT(array.ndim() == 1);
            auto ans = std::make_unique<Vector<Real>>(array.size(), kUndefined);
            for (int32_t i = 0; i != ans->Dim(); ++i) {
              ans->Data()[i] = array.at(i);
            }
            return ans;
          }))
      .def("numpy",
           [](py::object obj) {
             auto *v = obj.cast<Vector<Real> *>();
             return py::array_t<Real>({v->Dim()},      // shape
                                      {sizeof(Real)},  // stride in bytes
                                      v->Data(),       // ptr
                                      obj);  // it will increase the reference
                                             // count of **this** vector
           })
      .def_buffer([](PyClass &v) -> py::buffer_info {
        return py::buffer_info(reinterpret_cast<void *>(v.Data()), sizeof(Real),
                               py::format_descriptor<Real>::format(),
                               1,  // num-axes
                               {v.Dim()},
                               {sizeof(Real)});  // strides (in chars)
      })
      .def_static(
          "read",
          [](const std::string &rxfilename) -> PyClass {
            PyClass ans;
            ReadKaldiObject(rxfilename, &ans);
            return ans;
          },
          py::arg("rxfilename"))
      .def(
          "write",
          [](const PyClass &self, const std::string &wxfilename, bool binary) {
            WriteKaldiObject(self, wxfilename, binary);
          },
          py::arg("wxfilename"), py::arg("binary"));
}

void PybindKaldiVector(py::module &m) {  // NOLINT
  PybindKaldiVectorTpl<float>(m, "_FloatVector");
  PybindKaldiVectorTpl<double>(m, "_DoubleVector");
}

}  // namespace kaldiio
