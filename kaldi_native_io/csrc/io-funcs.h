// kaldi_native_io/csrc/io-funcs.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/base/io-funcs.h

// Copyright 2009-2011  Microsoft Corporation;  Saarland University;
//                      Jan Silovsky;   Yanmin Qian
//                2016  Xiaohui Zhang
#ifndef KALDI_NATIVE_IO_CSRC_IO_FUNCS_H_
#define KALDI_NATIVE_IO_CSRC_IO_FUNCS_H_

#include "kaldi_native_io/csrc/io-funcs-inl.h"

namespace kaldiio {

/// InitKaldiOutputStream initializes an opened stream for writing by writing an
/// optional binary header and modifying the floating-point precision; it will
/// typically not be called by users directly.
inline void InitKaldiOutputStream(std::ostream &os, bool binary);

/// InitKaldiInputStream initializes an opened stream for reading by detecting
/// the binary header and setting the "binary" value appropriately;
/// It will typically not be called by users directly.
inline bool InitKaldiInputStream(std::istream &is, bool *binary);

/// WriteBasicType is the name of the write function for bool, integer types,
/// and floating-point types. They all throw on error.
template <class T>
void WriteBasicType(std::ostream &os, bool binary, T t);

/// ReadBasicType is the name of the read function for bool, integer types,
/// and floating-point types. They all throw on error.
template <class T>
void ReadBasicType(std::istream &is, bool binary, T *t);

// Declare specialization for bool.
template <>
void WriteBasicType<bool>(std::ostream &os, bool binary, bool b);

template <>
void ReadBasicType<bool>(std::istream &is, bool binary, bool *b);

// Declare specializations for float and double.
template <>
void WriteBasicType<float>(std::ostream &os, bool binary, float f);

template <>
void WriteBasicType<double>(std::ostream &os, bool binary, double f);

template <>
void ReadBasicType<float>(std::istream &is, bool binary, float *f);

template <>
void ReadBasicType<double>(std::istream &is, bool binary, double *f);

// Define ReadBasicType that accepts an "add" parameter to add to
// the destination.  Caution: if used in Read functions, be careful
// to initialize the parameters concerned to zero in the default
// constructor.
template <class T>
inline void ReadBasicType(std::istream &is, bool binary, T *t, bool add) {
  if (!add) {
    ReadBasicType(is, binary, t);
  } else {
    T tmp = T(0);
    ReadBasicType(is, binary, &tmp);
    *t += tmp;
  }
}

}  // namespace kaldiio
#endif  // KALDI_NATIVE_IO_CSRC_IO_FUNCS_H_
