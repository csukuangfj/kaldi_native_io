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

# Explicitly static, like kaldifst_core: the _kaldi_native_io extension then
# links the code in and becomes self-contained, so the wheel needs no extra
# libraries and no RPATH fixups.
add_library(kaldi_native_io_core STATIC ${srcs})

if(NOT WIN32)
  set_target_properties(kaldi_native_io_core
    PROPERTIES
      POSITION_INDEPENDENT_CODE ON
      C_VISIBILITY_PRESET hidden
      CXX_VISIBILITY_PRESET hidden
  )
endif()

function(kaldi_native_io_add_test source)
  get_filename_component(name ${source} NAME_WE)
  add_executable(${name} "${source}")
  target_link_libraries(${name}
    PRIVATE
      kaldi_native_io_core
      gtest
      gtest_main
  )

  add_test(NAME "Test.${name}"
    COMMAND
    $<TARGET_FILE:${name}>
  )
endfunction()

if(KALDI_NATIVE_IO_BUILD_TESTS)
  # please sort the source files alphabetically
  set(test_srcs
  )

  foreach(source IN LISTS test_srcs)
    kaldi_native_io_add_test(${source})
  endforeach()
endif()
