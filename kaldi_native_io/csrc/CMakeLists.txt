include_directories(${CMAKE_SOURCE_DIR})

set(srcs
  compressed-matrix.cc
  io-funcs.cc
  kaldi-holder.cc
  kaldi-io.cc
  kaldi-matrix.cc
  kaldi-semaphore.cc
  kaldi-table.cc
  kaldi-utils.cc
  kaldi-vector.cc
  parse-options.cc
  posterior.cc
  text-utils.cc
)

add_library(kaldi_native_io_core SHARED ${srcs})
