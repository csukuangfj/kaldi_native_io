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
target_link_libraries(copy-blob kaldi_native_io_core)

set(exe_list copy-blob)

if(NOT WIN32)
  if(NOT DEFINED ENV{VIRTUAL_ENV})
    message(STATUS "Outside a virtual environment")
    execute_process(
      COMMAND "${PYTHON_EXECUTABLE}" -c "import site; print(';'.join(site.getsitepackages()))"
      OUTPUT_STRIP_TRAILING_WHITESPACE
      OUTPUT_VARIABLE path_list
    )
  else()
    message(STATUS "Inside a virtual environment")
    execute_process(
      COMMAND "${PYTHON_EXECUTABLE}" -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())"
      OUTPUT_STRIP_TRAILING_WHITESPACE
      OUTPUT_VARIABLE PYTHON_SITE_PACKAGE_DIR
    )
    set(path_list ${PYTHON_SITE_PACKAGE_DIR})
  endif()

  message(STATUS "path list: ${path_list}")
  foreach(p IN LISTS path_list)
    foreach(exe IN LISTS exe_list)
      target_link_libraries(${exe} "-Wl,-rpath,${p}/kaldi_native_io/lib")
      target_link_libraries(${exe} "-Wl,-rpath,${p}/../lib")
    endforeach()
  endforeach()

  foreach(exe IN LISTS exe_list)
    target_link_libraries(${exe} "-Wl,-rpath,${kaldi_native_io_rpath_origin}/../lib")
  endforeach()
endif()

install(
  TARGETS ${exe_list}
  DESTINATION bin
)
