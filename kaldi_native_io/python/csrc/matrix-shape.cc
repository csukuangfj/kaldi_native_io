// kaldi_native_io/python/csrc/matrix-shape.cc
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/python/csrc/matrix-shape.h"

#include <string>

#include "kaldi_native_io/csrc/kaldi-io.h"
#include "kaldi_native_io/csrc/matrix-shape.h"

namespace kaldiio {

void PybindMatrixShape(py::module &m) {  // NOLINT
  using PyClass = MatrixShape;
  py::class_<PyClass>(m, "MatrixShape")
      .def(py::init<int32_t, int32_t>(), py::arg("num_rows") = 0,
           py::arg("num_cols") = 0)
      .def_property_readonly("num_rows", &PyClass::NumRows)
      .def_property_readonly("num_cols", &PyClass::NumCols)
      .def("__str__",
           [](const PyClass &self) -> std::string {
             std::ostringstream os;
             os << "MatrixShape(num_rows=" << self.NumRows() << ","
                << "num_cols=" << self.NumCols() << ")";
             return os.str();
           })
      .def_static("read", [](const std::string &rxfilename) -> PyClass {
        PyClass ans;
        ReadKaldiObject(rxfilename, &ans);
        return ans;
      });
}

}  // namespace kaldiio
