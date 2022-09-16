// kaldi_native_io/csrc/kaldi-holder.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/util/kaldi-holder.h

// Copyright 2009-2011     Microsoft Corporation
//                2016     Johns Hopkins University (author: Daniel Povey)
//                2016     Xiaohui Zhang
//
#ifndef KALDI_NATIVE_IO_CSRC_KALDI_HOLDER_H_
#define KALDI_NATIVE_IO_CSRC_KALDI_HOLDER_H_

#include <string>

#include "kaldi_native_io/csrc/compressed-matrix.h"
#include "kaldi_native_io/csrc/kaldi-matrix.h"
#include "kaldi_native_io/csrc/kaldi-vector.h"

namespace kaldiio {

/// BasicHolder is valid for float, double, bool, and integer
/// types.  There will be a compile time error otherwise, because
/// we make sure that the {Write, Read}BasicType functions do not
/// get instantiated for other types.
template <class BasicType>
class BasicHolder;

/// A Holder for a vector of basic types, e.g.
/// std::vector<int32>, std::vector<float>, and so on.
/// Note: a basic type is defined as a type for which ReadBasicType
/// and WriteBasicType are implemented, i.e. integer and floating
/// types, and bool.
template <class BasicType>
class BasicVectorHolder;

// A holder for vectors of vectors of basic types, e.g.
// std::vector<std::vector<int32> >, and so on.
// Note: a basic type is defined as a type for which ReadBasicType
// and WriteBasicType are implemented, i.e. integer and floating
// types, and bool.
template <class BasicType>
class BasicVectorVectorHolder;

// A holder for vectors of pairs of basic types, e.g.
// std::vector<std::pair<int32, int32> >, and so on.
// Note: a basic type is defined as a type for which ReadBasicType
// and WriteBasicType are implemented, i.e. integer and floating
// types, and bool.  Text format is (e.g. for integers),
// "1 12 ; 43 61 ; 17 8 \n"
template <class BasicType>
class BasicPairVectorHolder;

/// We define a Token (not a typedef, just a word) as a nonempty, printable,
/// whitespace-free std::string.  The binary and text formats here are the same
/// (newline-terminated) and as such we don't bother with the binary-mode
/// headers.
class TokenHolder;

/// Class TokenVectorHolder is a Holder class for vectors of Tokens
/// (T == std::string).
class TokenVectorHolder;

/// KaldiObjectHolder works for Kaldi objects that have the "standard" Read
/// and Write functions, and a copy constructor.
template <class KaldiType>
class KaldiObjectHolder;

/// A class for reading/writing HTK-format matrices.
/// T == std::pair<Matrix<BaseFloat>, HtkHeader>
class HtkMatrixHolder;

// In SequentialTableReaderScriptImpl and RandomAccessTableReaderScriptImpl, for
// cases where the scp contained 'range specifiers' (things in square brackets
// identifying parts of objects like matrices), use this function to separate
// the input string 'rxfilename_with_range' (e.g "1.ark:100[1:2,2:10]") into the
// data_rxfilename (e.g. "1.ark:100") and the optional range specifier which
// will be everything inside the square brackets.  It returns true if everything
// seems OK, and false if for example the string contained more than one '['.
// This function should only be called if 'line' ends in ']', otherwise it is an
// error.
bool ExtractRangeSpecifier(const std::string &rxfilename_with_range,
                           std::string *data_rxfilename, std::string *range);

/// This templated function exists so that we can write .scp files with
/// 'object ranges' specified: the canonical example is a [first:last] range
/// of rows of a matrix, or [first-row:last-row,first-column,last-column]
/// of a matrix.  We can also support [begin-time:end-time] of a wave
/// file.  The string 'range' is whatever is in the square brackets; it is
/// parsed inside this function.
/// This function returns true if the partial object was successfully extracted,
/// and false if there was an error such as an invalid range.
/// The generic version of this function just fails; we overload the template
/// whenever we need it for a specific class.
template <class T>
bool ExtractObjectRange(const T & /*input*/, const std::string & /*range*/,
                        T * /*output*/) {
  KALDIIO_ERR << "Ranges not supported for objects of this type.";
  return false;
}

/// The template is specialized types Vector<float> and Vector<double>.
template <class Real>
bool ExtractObjectRange(const Vector<Real> &input, const std::string &range,
                        Vector<Real> *output);

/// The template is specialized with a version that actually does something,
/// for types Matrix<float> and Matrix<double>.  We can later add versions of
/// this template for other types, such as Vector, which can meaningfully
/// have ranges extracted.
template <class Real>
bool ExtractObjectRange(const Matrix<Real> &input, const std::string &range,
                        Matrix<Real> *output);

/// CompressedMatrix is always of the type BaseFloat but it is more
/// efficient to provide template as it uses CompressedMatrix's own
/// conversion to Matrix<Real>
template <class Real>
bool ExtractObjectRange(const CompressedMatrix &input, const std::string &range,
                        Matrix<Real> *output);

}  // namespace kaldiio

#include "kaldi_native_io/csrc/kaldi-holder-inl.h"

#endif  // KALDI_NATIVE_IO_CSRC_KALDI_HOLDER_H_
