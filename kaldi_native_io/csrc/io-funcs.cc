// kaldi_native_io/csrc/io-funcs.cc
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/base/io-funcs.cc

// Copyright 2009-2011  Microsoft Corporation;  Saarland University

#include "kaldi_native_io/csrc/io-funcs.h"

#include "kaldi_native_io/csrc/kaldi-utils.h"
#include "kaldi_native_io/csrc/log.h"

namespace kaldiio {

template <>
void WriteBasicType<bool>(std::ostream &os, bool binary, bool b) {
  os << (b ? "T" : "F");
  if (!binary) os << " ";
  if (os.fail()) KALDIIO_ERR << "Write failure in WriteBasicType<bool>";
}

template <>
void ReadBasicType<bool>(std::istream &is, bool binary, bool *b) {
  KALDIIO_PARANOID_ASSERT(b != NULL);
  if (!binary) is >> std::ws;  // eat up whitespace.
  char c = is.peek();
  if (c == 'T') {
    *b = true;
    is.get();
  } else if (c == 'F') {
    *b = false;
    is.get();
  } else {
    KALDIIO_ERR << "Read failure in ReadBasicType<bool>, file position is "
                << is.tellg() << ", next char is " << CharToString(c);
  }
}

template <>
void WriteBasicType<float>(std::ostream &os, bool binary, float f) {
  if (binary) {
    char c = sizeof(f);
    os.put(c);
    os.write(reinterpret_cast<const char *>(&f), sizeof(f));
  } else {
    os << f << " ";
  }
}

template <>
void WriteBasicType<double>(std::ostream &os, bool binary, double f) {
  if (binary) {
    char c = sizeof(f);
    os.put(c);
    os.write(reinterpret_cast<const char *>(&f), sizeof(f));
  } else {
    os << f << " ";
  }
}

template <>
void ReadBasicType<float>(std::istream &is, bool binary, float *f) {
  KALDIIO_PARANOID_ASSERT(f != NULL);
  if (binary) {
    double d;
    int c = is.peek();
    if (c == sizeof(*f)) {
      is.get();
      is.read(reinterpret_cast<char *>(f), sizeof(*f));
    } else if (c == sizeof(d)) {
      ReadBasicType(is, binary, &d);
      *f = d;
    } else {
      KALDIIO_ERR << "ReadBasicType: expected float, saw " << is.peek()
                  << ", at file position " << is.tellg();
    }
  } else {
    is >> *f;
  }
  if (is.fail()) {
    KALDIIO_ERR << "ReadBasicType: failed to read, at file position "
                << is.tellg();
  }
}

template <>
void ReadBasicType<double>(std::istream &is, bool binary, double *d) {
  KALDIIO_PARANOID_ASSERT(d != NULL);
  if (binary) {
    float f;
    int c = is.peek();
    if (c == sizeof(*d)) {
      is.get();
      is.read(reinterpret_cast<char *>(d), sizeof(*d));
    } else if (c == sizeof(f)) {
      ReadBasicType(is, binary, &f);
      *d = f;
    } else {
      KALDIIO_ERR << "ReadBasicType: expected float, saw " << is.peek()
                  << ", at file position " << is.tellg();
    }
  } else {
    is >> *d;
  }
  if (is.fail()) {
    KALDIIO_ERR << "ReadBasicType: failed to read, at file position "
                << is.tellg();
  }
}

}  // namespace kaldiio
