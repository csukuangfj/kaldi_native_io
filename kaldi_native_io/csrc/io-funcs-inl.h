// kaldi_native_io/csrc/io-funcs-inl.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/base/io-funcs-inl.h

// Copyright 2009-2011  Microsoft Corporation;  Saarland University;
//                      Jan Silovsky;   Yanmin Qian;
//                      Johns Hopkins University (Author: Daniel Povey)
//                2016  Xiaohui Zhang
#ifndef KALDI_NATIVE_IO_CSRC_IO_FUNCS_INL_H_
#define KALDI_NATIVE_IO_CSRC_IO_FUNCS_INL_H_

#include <iostream>
#include <limits>

#include "kaldi_native_io/csrc/log.h"

namespace kaldiio {

// Initialize an opened stream for writing by writing an optional binary
// header and modifying the floating-point precision.
inline void InitKaldiOutputStream(std::ostream &os, bool binary) {
  // This does not throw exceptions (does not check for errors).
  if (binary) {
    os.put('\0');
    os.put('B');
  }
  // Note, in non-binary mode we may at some point want to mess with
  // the precision a bit.
  // 7 is a bit more than the precision of float..
  if (os.precision() < 7) os.precision(7);
}

/// Initialize an opened stream for reading by detecting the binary header and
// setting the "binary" value appropriately.
inline bool InitKaldiInputStream(std::istream &is, bool *binary) {
  // Sets the 'binary' variable.
  // Throws exception in the very unusual situation that stream
  // starts with '\0' but not then 'B'.

  if (is.peek() == '\0') {  // seems to be binary
    is.get();
    if (is.peek() != 'B') {
      return false;
    }
    is.get();
    *binary = true;
    return true;
  } else {
    *binary = false;
    return true;
  }
}

// Template that covers integers.
template <class T>
void WriteBasicType(std::ostream &os, bool binary, T t) {
  // Compile time assertion that this is not called with a wrong type.
  static_assert(std::is_integral<T>::value, "");
  if (binary) {
    char len_c = (std::numeric_limits<T>::is_signed ? 1 : -1) *
                 static_cast<char>(sizeof(t));
    os.put(len_c);
    os.write(reinterpret_cast<const char *>(&t), sizeof(t));
  } else {
    if (sizeof(t) == 1)
      os << static_cast<int16_t>(t) << " ";
    else
      os << t << " ";
  }
  if (os.fail()) {
    KALDIIO_ERR << "Write failure in WriteBasicType.";
  }
}

// Template that covers integers.
template <class T>
inline void ReadBasicType(std::istream &is, bool binary, T *t) {
  KALDIIO_PARANOID_ASSERT(t != NULL);
  // Compile time assertion that this is not called with a wrong type.
  static_assert(std::is_integral<T>::value, "");
  if (binary) {
    int len_c_in = is.get();
    if (len_c_in == -1)
      KALDIIO_ERR << "ReadBasicType: encountered end of stream.";
    char len_c = static_cast<char>(len_c_in),
         len_c_expected = (std::numeric_limits<T>::is_signed ? 1 : -1) *
                          static_cast<char>(sizeof(*t));
    if (len_c != len_c_expected) {
      KALDIIO_ERR << "ReadBasicType: did not get expected integer type, "
                  << static_cast<int>(len_c) << " vs. "
                  << static_cast<int>(len_c_expected)
                  << ".  You can change this code to successfully"
                  << " read it later, if needed.";
      // insert code here to read "wrong" type.  Might have a switch statement.
    }
    is.read(reinterpret_cast<char *>(t), sizeof(*t));
  } else {
    if (sizeof(*t) == 1) {
      int16_t i;
      is >> i;
      *t = i;
    } else {
      is >> *t;
    }
  }
  if (is.fail()) {
    KALDIIO_ERR << "Read failure in ReadBasicType, file position is "
                << is.tellg() << ", next char is " << is.peek();
  }
}

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_IO_FUNCS_INL_H_
