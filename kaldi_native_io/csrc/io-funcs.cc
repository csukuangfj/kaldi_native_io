// kaldi_native_io/csrc/io-funcs.cc
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/base/io-funcs.cc

// Copyright 2009-2011  Microsoft Corporation;  Saarland University

#include "kaldi_native_io/csrc/io-funcs.h"

#include <string.h>

#include <string>

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

void CheckToken(const char *token) {
  if (*token == '\0') KALDIIO_ERR << "Token is empty (not a valid token)";
  const char *orig_token = token;
  while (*token != '\0') {
    if (::isspace(*token))
      KALDIIO_ERR << "Token is not a valid token (contains space): '"
                  << orig_token << "'";
    token++;
  }
}

void WriteToken(std::ostream &os, bool /*binary*/, const char *token) {
  // binary mode is ignored;
  // we use space as termination character in either case.
  KALDIIO_ASSERT(token != NULL);
  CheckToken(token);  // make sure it's valid (can be read back)
  os << token << " ";
  if (os.fail()) {
    KALDIIO_ERR << "Write failure in WriteToken.";
  }
}

void WriteToken(std::ostream &os, bool binary, const std::string &token) {
  WriteToken(os, binary, token.c_str());
}

void ReadToken(std::istream &is, bool binary, std::string *str) {
  KALDIIO_ASSERT(str != NULL);
  if (!binary) is >> std::ws;  // consume whitespace.
  is >> *str;
  if (is.fail()) {
    KALDIIO_ERR << "ReadToken, failed to read token at file position "
                << is.tellg();
  }
  if (!isspace(is.peek())) {
    KALDIIO_ERR << "ReadToken, expected space after token, saw instead "
                << CharToString(static_cast<char>(is.peek()))
                << ", at file position " << is.tellg();
  }
  is.get();  // consume the space.
}

int PeekToken(std::istream &is, bool binary) {
  if (!binary) is >> std::ws;  // consume whitespace.
  bool read_bracket;
  if (static_cast<char>(is.peek()) == '<') {
    read_bracket = true;
    is.get();
  } else {
    read_bracket = false;
  }
  int ans = is.peek();
  if (read_bracket) {
    if (!is.unget()) {
      // Clear the bad bit. This code can be (and is in fact) reached, since the
      // C++ standard does not guarantee that a call to unget() must succeed.
      is.clear();
    }
  }
  return ans;
}

void ExpectToken(std::istream &is, bool binary, const char *token) {
  int pos_at_start = is.tellg();
  KALDIIO_ASSERT(token != NULL);
  CheckToken(token);           // make sure it's valid (can be read back)
  if (!binary) is >> std::ws;  // consume whitespace.
  std::string str;
  is >> str;
  is.get();  // consume the space.
  if (is.fail()) {
    KALDIIO_ERR << "Failed to read token [started at file position "
                << pos_at_start << "], expected " << token;
  }
  // The second half of the '&&' expression below is so that if we're expecting
  // "<Foo>", we will accept "Foo>" instead.  This is so that the model-reading
  // code will tolerate errors in PeekToken where is.unget() failed; search for
  // is.clear() in PeekToken() for an explanation.
  if (strcmp(str.c_str(), token) != 0 &&
      !(token[0] == '<' && strcmp(str.c_str(), token + 1) == 0)) {
    KALDIIO_ERR << "Expected token \"" << token << "\", got instead \"" << str
                << "\".";
  }
}

void ExpectToken(std::istream &is, bool binary, const std::string &token) {
  ExpectToken(is, binary, token.c_str());
}

int Peek(std::istream &is, bool binary) {
  if (!binary) is >> std::ws;  // eat up whitespace.
  return is.peek();
}

}  // namespace kaldiio
