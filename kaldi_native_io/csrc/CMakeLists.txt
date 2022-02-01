include_directories(${CMAKE_SOURCE_DIR})

set(srcs
  io-funcs.cc
  kaldi-holder.cc
  kaldi-io.cc
  kaldi-semaphore.cc
  kaldi-table.cc
  kaldi-utils.cc
  parse-options.cc
  text-utils.cc
  )

add_library(kaldi_native_io_core SHARED ${srcs})
