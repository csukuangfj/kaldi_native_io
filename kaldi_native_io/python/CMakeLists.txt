add_subdirectory(csrc)

if(KALDI_NATIVE_IO_BUILD_TESTS)
  add_subdirectory(tests)
endif()
