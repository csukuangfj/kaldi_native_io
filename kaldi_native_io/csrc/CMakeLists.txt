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

add_library(kaldi_native_io_core_static STATIC ${srcs})

add_library(kaldi_native_io_core ${srcs})
if(APPLE)
  set_target_properties(kaldi_native_io_core
    PROPERTIES
      LINK_FLAGS "-undefined dynamic_lookup"
  )
endif()

file(MAKE_DIRECTORY
  DESTINATION
    ${PROJECT_BINARY_DIR}/include/kaldi_native_io/csrc
)

file(GLOB_RECURSE all_headers *.h)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  message(STATUS "All headers: ${all_headers}")
endif()

file(COPY
  ${all_headers}
  DESTINATION
    ${PROJECT_BINARY_DIR}/include/kaldi_native_io/csrc
)

install(TARGETS kaldi_native_io_core
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(KALDI_NATIVE_IO_BUILD_PYTHON AND WIN32)
  install(TARGETS kaldi_native_io_core
    DESTINATION ../
  )
endif()

install(FILES ${all_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kaldi_native_io/csrc
)
