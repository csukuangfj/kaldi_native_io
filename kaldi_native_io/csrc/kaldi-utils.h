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

// clang-format off
#define KALDIIO_SWAP8(a) do { \
  int t = (reinterpret_cast<char*>(&a))[0];\
          (reinterpret_cast<char*>(&a))[0]=(reinterpret_cast<char*>(&a))[7];\
          (reinterpret_cast<char*>(&a))[7]=t;\
      t = (reinterpret_cast<char*>(&a))[1];\
          (reinterpret_cast<char*>(&a))[1]=(reinterpret_cast<char*>(&a))[6];\
          (reinterpret_cast<char*>(&a))[6]=t;\
      t = (reinterpret_cast<char*>(&a))[2];\
          (reinterpret_cast<char*>(&a))[2]=(reinterpret_cast<char*>(&a))[5];\
          (reinterpret_cast<char*>(&a))[5]=t;\
      t = (reinterpret_cast<char*>(&a))[3];\
          (reinterpret_cast<char*>(&a))[3]=(reinterpret_cast<char*>(&a))[4];\
          (reinterpret_cast<char*>(&a))[4]=t;} while (0)
#define KALDIIO_SWAP4(a) do { \
  int t = (reinterpret_cast<char*>(&a))[0];\
          (reinterpret_cast<char*>(&a))[0]=(reinterpret_cast<char*>(&a))[3];\
          (reinterpret_cast<char*>(&a))[3]=t;\
      t = (reinterpret_cast<char*>(&a))[1];\
          (reinterpret_cast<char*>(&a))[1]=(reinterpret_cast<char*>(&a))[2];\
          (reinterpret_cast<char*>(&a))[2]=t;} while (0)
#define KALDIIO_SWAP2(a) do { \
  int t = (reinterpret_cast<char*>(&a))[0];\
          (reinterpret_cast<char*>(&a))[0]=(reinterpret_cast<char*>(&a))[1];\
          (reinterpret_cast<char*>(&a))[1]=t;} while (0)
// clang-format on

namespace kaldiio {

// CharToString prints the character in a human-readable form, for debugging.
std::string CharToString(const char &c);

inline int MachineIsLittleEndian() {
  int check = 1;
  return (*reinterpret_cast<char *>(&check) != 0);
}

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_KALDI_UTILS_H_
