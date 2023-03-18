include_directories(${CMAKE_SOURCE_DIR})

pybind11_add_module(_kaldi_native_io
  blob.cc
  compressed-matrix.cc
  kaldi-matrix.cc
  kaldi-table.cc
  kaldi-vector.cc
  kaldiio.cc
  matrix-shape.cc
  wave-reader.cc
)

if(APPLE)
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE PYTHON_SITE_PACKAGE_DIR
  )
  message(STATUS "PYTHON_SITE_PACKAGE_DIR: ${PYTHON_SITE_PACKAGE_DIR}")
  target_link_libraries(_kaldi_native_io PRIVATE "-Wl,-rpath,${PYTHON_SITE_PACKAGE_DIR}")
endif()

if(NOT WIN32)
  target_link_libraries(_kaldi_native_io PRIVATE "-Wl,-rpath,${kaldi_native_io_rpath_origin}/kaldi_native_io/${CMAKE_INSTALL_LIBDIR}")
endif()

target_link_libraries(_kaldi_native_io PRIVATE kaldi_native_io_core)

install(TARGETS _kaldi_native_io
  DESTINATION ../
)

add_executable(copy-blob
  copy-blob.cc
  parse-options.cc
)
target_link_libraries(copy-blob kaldi_native_io_core_static)
if(NOT WIN32)
  target_link_libraries(copy-blob -pthread)
endif()

install(
  TARGETS copy-blob
  DESTINATION bin
)
