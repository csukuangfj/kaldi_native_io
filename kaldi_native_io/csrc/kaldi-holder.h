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

/// We define a Token (not a typedef, just a word) as a nonempty, printable,
/// whitespace-free std::string.  The binary and text formats here are the same
/// (newline-terminated) and as such we don't bother with the binary-mode
/// headers.
class TokenHolder;

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

}  // namespace kaldiio

#include "kaldi_native_io/csrc/kaldi-holder-inl.h"

#endif  // KALDI_NATIVE_IO_CSRC_KALDI_HOLDER_H_
