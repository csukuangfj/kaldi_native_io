// kaldi_native_io/python/csrc/kaldi-table.cc
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/csrc/kaldi-table.h"

#include "kaldi_native_io/csrc/kaldi-holder.h"
#include "kaldi_native_io/python/csrc/kaldi-table.h"

namespace kaldiio {

template <class Holder>
void PybindTableWriter(py::module &m, const std::string &class_name,
                       const std::string &class_help_doc = "") {
  using PyClass = TableWriter<Holder>;

  py::class_<PyClass>(m, class_name.c_str(), class_help_doc.c_str())
      .def(py::init<>())
      .def(py::init<const std::string &>(), py::arg("wspecifier"))
      .def("open", &PyClass::Open, py::arg("wspecifier"))
      .def_property_readonly("is_open", &PyClass::IsOpen)
      .def("write", &PyClass::Write, py::arg("key"), py::arg("value"))
      .def("flush", &PyClass::Flush)
      .def("close", &PyClass::Close);
}

template <class Holder>
void PybindSequentialTableReader(py::module &m, const std::string &class_name,
                                 const std::string &class_help_doc = "") {
  using PyClass = SequentialTableReader<Holder>;
  py::class_<PyClass>(m, class_name.c_str(), class_help_doc.c_str())
      .def(py::init<>())
      .def(py::init<const std::string &>(), py::arg("rspecifier"))
      .def("open", &PyClass::Open, py::arg("rspecifier"))
      .def_property_readonly("done", &PyClass::Done)
      .def_property_readonly("key", &PyClass::Key)
      .def("free_current", &PyClass::FreeCurrent)
      .def_property_readonly("value", &PyClass::Value)
      .def("next", &PyClass::Next)
      .def_property_readonly("is_open", &PyClass::IsOpen)
      .def("close", &PyClass::Close);
}

template <class Holder>
void PybindRandomAccessTableReader(py::module &m, const std::string &class_name,
                                   const std::string &class_help_doc = "") {
  using PyClass = RandomAccessTableReader<Holder>;
  py::class_<PyClass>(m, class_name.c_str(), class_help_doc.c_str())
      .def(py::init<>())
      .def(py::init<const std::string &>(), py::arg("rspecifier"))
      .def("open", &PyClass::Open, py::arg("rspecifier"))
      .def_property_readonly("is_open", &PyClass::IsOpen)
      .def("close", &PyClass::Close)
      .def("__contains__", &PyClass::HasKey)
      .def("__getitem__", &PyClass::Value, py::arg("key"));
}

void PybindKaldiTable(py::module &m) {
  PybindTableWriter<BasicHolder<int32_t>>(m, "_Int32Writer");
  PybindSequentialTableReader<BasicHolder<int32_t>>(m,
                                                    "_SequentialInt32Reader");
  PybindRandomAccessTableReader<BasicHolder<int32_t>>(
      m, "_RandomAccessInt32Reader");

  PybindTableWriter<BasicVectorHolder<int32_t>>(m, "_Int32VectorWriter");
  PybindSequentialTableReader<BasicVectorHolder<int32_t>>(
      m, "_SequentialInt32VectorReader");
  PybindRandomAccessTableReader<BasicVectorHolder<int32_t>>(
      m, "_RandomAccessInt32VectorReader");

  PybindTableWriter<BasicVectorVectorHolder<int32_t>>(
      m, "_Int32VectorVectorWriter");
  PybindSequentialTableReader<BasicVectorVectorHolder<int32_t>>(
      m, "_SequentialInt32VectorVectorReader");
  PybindRandomAccessTableReader<BasicVectorVectorHolder<int32_t>>(
      m, "_RandomAccessInt32VectorVectorReader");

  PybindTableWriter<BasicPairVectorHolder<int32_t>>(m,
                                                    "_Int32PairVectorWriter");
  PybindSequentialTableReader<BasicPairVectorHolder<int32_t>>(
      m, "_SequentialInt32PairVectorReader");
  PybindRandomAccessTableReader<BasicPairVectorHolder<int32_t>>(
      m, "_RandomAccessInt32PairVectorReader");

  PybindTableWriter<BasicHolder<float>>(m, "_FloatWriter");
  PybindSequentialTableReader<BasicHolder<float>>(m, "_SequentialFloatReader");
  PybindRandomAccessTableReader<BasicHolder<float>>(m,
                                                    "_RandomAccessFloatReader");

  PybindTableWriter<BasicPairVectorHolder<float>>(m, "_FloatPairVectorWriter");
  PybindSequentialTableReader<BasicPairVectorHolder<float>>(
      m, "_SequentialFloatPairVectorReader");
  PybindRandomAccessTableReader<BasicPairVectorHolder<float>>(
      m, "_RandomAccessFloatPairVectorReader");

  PybindTableWriter<BasicHolder<double>>(m, "_DoubleWriter");
  PybindSequentialTableReader<BasicHolder<double>>(m,
                                                   "_SequentialDoubleReader");
  PybindRandomAccessTableReader<BasicHolder<double>>(
      m, "_RandomAccessDoubleReader");

  PybindTableWriter<BasicHolder<bool>>(m, "_BoolWriter");
  PybindSequentialTableReader<BasicHolder<bool>>(m, "_SequentialBoolReader");
  PybindRandomAccessTableReader<BasicHolder<bool>>(m,
                                                   "_RandomAccessBoolReader");

  PybindTableWriter<TokenHolder>(m, "_TokenWriter");
  PybindSequentialTableReader<TokenHolder>(m, "_SequentialTokenReader");
  PybindRandomAccessTableReader<TokenHolder>(m, "_RandomAccessTokenReader");

  PybindTableWriter<TokenVectorHolder>(m, "_TokenVectorWriter");
  PybindSequentialTableReader<TokenVectorHolder>(
      m, "_SequentialTokenVectorReader");
  PybindRandomAccessTableReader<TokenVectorHolder>(
      m, "_RandomAccessTokenVectorReader");
}

}  // namespace kaldiio
