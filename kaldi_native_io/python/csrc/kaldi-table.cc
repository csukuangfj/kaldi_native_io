// kaldi_native_io/python/csrc/kaldi-table.cc
//
// Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)

#include "kaldi_native_io/csrc/kaldi-table.h"

#include <string>

#include "kaldi_native_io/csrc/compressed-matrix.h"
#include "kaldi_native_io/csrc/kaldi-holder.h"
#include "kaldi_native_io/csrc/kaldi-io.h"
#include "kaldi_native_io/csrc/kaldi-matrix.h"
#include "kaldi_native_io/csrc/kaldi-vector.h"
#include "kaldi_native_io/csrc/matrix-shape.h"
#include "kaldi_native_io/csrc/posterior.h"
#include "kaldi_native_io/csrc/wave-reader.h"
#include "kaldi_native_io/python/csrc/blob.h"
#include "kaldi_native_io/python/csrc/kaldi-table.h"

namespace kaldiio {

template <class Holder>
void PybindTableWriter(py::module &m,  // NOLINT
                       const std::string &class_name,
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
void PybindSequentialTableReader(py::module &m,  // NOLINT
                                 const std::string &class_name,
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
void PybindRandomAccessTableReader(py::module &m,  // NOLINT
                                   const std::string &class_name,
                                   const std::string &class_help_doc = "") {
  using PyClass = RandomAccessTableReader<Holder>;
  py::class_<PyClass>(m, class_name.c_str(), class_help_doc.c_str())
      .def(py::init<>())
      .def(py::init<const std::string &>(), py::arg("rspecifier"))
      .def("open", &PyClass::Open, py::arg("rspecifier"))
      .def_property_readonly("is_open", &PyClass::IsOpen)
      .def("close", &PyClass::Close)
      .def("__contains__", &PyClass::HasKey)
      .def("__getitem__", &PyClass::Value, py::arg("key"),
           py::return_value_policy::reference);
}

template <typename Holder>
void PybindReadSingleItem(py::module &m,  // NOLINT
                          const std::string &name,
                          const std::string &help_doc = "") {
  m.def(
      name.c_str(),
      [](const std::string &rxfilename) {
        Input ki(rxfilename);
        Holder holder;
        holder.Read(ki.Stream());
        return holder.Value();  // Return a copy
      },
      py::arg("rxfilename"), help_doc.c_str());
}

template <>
void PybindReadSingleItem<BlobHolder>(py::module &m,  // NOLINT
                                      const std::string &name,
                                      const std::string &help_doc /*= ""*/) {
  m.def(
      name.c_str(),
      [](const std::string &rxfilename) { return ReadBlobObject(rxfilename); },
      py::arg("rxfilename"), help_doc.c_str());
}

void PybindKaldiTable(py::module &m) {  // NOLINT
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
  PybindReadSingleItem<BasicVectorHolder<int32_t>>(
      m, "read_int32_vector", "Read std::vector<int32_t> for an rxfilename");

  PybindTableWriter<BlobHolder>(m, "_BlobWriter");
  PybindSequentialTableReader<BlobHolder>(m, "_SequentialBlobReader");
  PybindRandomAccessTableReader<BlobHolder>(m, "_RandomAccessBlobReader");
  PybindReadSingleItem<BlobHolder>(m, "read_blob",
                                   "Read a bytes object from an rxfilename");

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

  PybindTableWriter<BasicVectorHolder<int8_t>>(m, "_Int8VectorWriter");
  PybindSequentialTableReader<BasicVectorHolder<int8_t>>(
      m, "_SequentialInt8VectorReader");
  PybindRandomAccessTableReader<BasicVectorHolder<int8_t>>(
      m, "_RandomAccessInt8VectorReader");
  PybindReadSingleItem<BasicVectorHolder<int8_t>>(
      m, "read_int8_vector", "Read std::vector<int8_t> for an rxfilename");

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

  {
    using PyClass = KaldiObjectHolder<Vector<float>>;
    PybindTableWriter<PyClass>(m, "_FloatVectorWriter");
    PybindSequentialTableReader<PyClass>(m, "_SequentialFloatVectorReader");
    PybindRandomAccessTableReader<PyClass>(m, "_RandomAccessFloatVectorReader");
  }

  {
    using PyClass = KaldiObjectHolder<Vector<double>>;
    PybindTableWriter<PyClass>(m, "_DoubleVectorWriter");
    PybindSequentialTableReader<PyClass>(m, "_SequentialDoubleVectorReader");
    PybindRandomAccessTableReader<PyClass>(m,
                                           "_RandomAccessDoubleVectorReader");
  }

  {
    using PyClass = KaldiObjectHolder<Matrix<float>>;
    PybindTableWriter<PyClass>(m, "_FloatMatrixWriter");
    PybindSequentialTableReader<PyClass>(m, "_SequentialFloatMatrixReader");
    PybindRandomAccessTableReader<PyClass>(m, "_RandomAccessFloatMatrixReader");
  }

  {
    using PyClass = KaldiObjectHolder<Matrix<double>>;
    PybindTableWriter<PyClass>(m, "_DoubleMatrixWriter");
    PybindSequentialTableReader<PyClass>(m, "_SequentialDoubleMatrixReader");
    PybindRandomAccessTableReader<PyClass>(m,
                                           "_RandomAccessDoubleMatrixReader");
  }

  {
    using PyClass = HtkMatrixHolder;
    PybindTableWriter<PyClass>(m, "_HtkMatrixWriter");
    PybindSequentialTableReader<PyClass>(m, "_SequentialHtkMatrixReader");
    PybindRandomAccessTableReader<PyClass>(m, "_RandomAccessHtkMatrixReader");
  }

  PybindTableWriter<KaldiObjectHolder<CompressedMatrix>>(
      m, "_CompressedMatrixWriter");

  {
    using PyClass = PosteriorHolder;
    PybindTableWriter<PyClass>(m, "_PosteriorWriter");
    PybindSequentialTableReader<PyClass>(m, "_SequentialPosteriorReader");
    PybindRandomAccessTableReader<PyClass>(m, "_RandomAccessPosteriorReader");
  }

  {
    using PyClass = KaldiObjectHolder<MatrixShape>;
    PybindSequentialTableReader<PyClass>(m, "_SequentialMatrixShapeReader");
    PybindRandomAccessTableReader<PyClass>(m, "_RandomAccessMatrixShapeReader");
  }

  {
    using PyClass = GaussPostHolder;
    PybindTableWriter<PyClass>(m, "_GaussPostWriter");
    PybindSequentialTableReader<PyClass>(m, "_SequentialGaussPostReader");
    PybindRandomAccessTableReader<PyClass>(m, "_RandomAccessGaussPostReader");
  }

  {
    using PyClass = WaveHolder;
    PybindTableWriter<PyClass>(m, "_WaveWriter");
    PybindSequentialTableReader<PyClass>(m, "_SequentialWaveReader");
    PybindRandomAccessTableReader<PyClass>(m, "_RandomAccessWaveReader");
  }

  {
    using PyClass = WaveInfoHolder;
    PybindSequentialTableReader<PyClass>(m, "_SequentialWaveInfoReader");
    PybindRandomAccessTableReader<PyClass>(m, "_RandomAccessWaveInfoReader");
  }
}

}  // namespace kaldiio
