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
  matrix-shape.cc
  parse-options.cc
  posterior.cc
  text-utils.cc
  wave-reader.cc
)

add_library(kaldi_native_io_core ${srcs})
if(APPLE)
  set_target_properties(kaldi_native_io_core
    PROPERTIES
      LINK_FLAGS "-undefined dynamic_lookup"
  )
endif()
