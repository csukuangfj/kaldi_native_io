add_subdirectory(csrc)

if(KALDI_NATIVE_IO_BUILD_PYTHON)
  add_subdirectory(python)
endif()
