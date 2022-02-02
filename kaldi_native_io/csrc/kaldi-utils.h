// kaldi_native_io/csrc/kaldi-utils.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/base/kaldi-utils.h

// Copyright 2009-2011  Ondrej Glembek;  Microsoft Corporation;
//                      Saarland University;  Karel Vesely;  Yanmin Qian

#ifndef KALDI_NATIVE_IO_CSRC_KALDI_UTILS_H_
#define KALDI_NATIVE_IO_CSRC_KALDI_UTILS_H_

#include <string>

#if defined(_MSC_VER)
#define KALDIIO_MEMALIGN(align, size, pp_orig) \
  (*(pp_orig) = _aligned_malloc(size, align))
#define KALDIIO_MEMALIGN_FREE(x) _aligned_free(x)
#elif defined(__CYGWIN__)
#define KALDIIO_MEMALIGN(align, size, pp_orig) \
  (*(pp_orig) = aligned_alloc(align, size))
#define KALDIIO_MEMALIGN_FREE(x) free(x)
#else
#define KALDIIO_MEMALIGN(align, size, pp_orig) \
  (!posix_memalign(pp_orig, align, size) ? *(pp_orig) : NULL)
#define KALDIIO_MEMALIGN_FREE(x) free(x)
#endif

#if defined(_MSC_VER)
#define KALDIIO_STRCASECMP _stricmp
#elif defined(__CYGWIN__)
#include <strings.h>
#define KALDIIO_STRCASECMP strcasecmp
#else
#define KALDIIO_STRCASECMP strcasecmp
#endif

namespace kaldiio {

// CharToString prints the character in a human-readable form, for debugging.
std::string CharToString(const char &c);

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_KALDI_UTILS_H_
