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
target_link_libraries(_kaldi_native_io PRIVATE kaldi_native_io_core)

if(MSVC)
  target_compile_options(
    _kaldi_native_io
    PRIVATE
      $<$<CONFIG:Release>:/GL->
      $<$<CONFIG:Release>:/Od /Ob0>
      $<$<CONFIG:RelWithDebInfo>:/GL->
      $<$<CONFIG:RelWithDebInfo>:/Od /Ob0>
      $<$<CONFIG:MinSizeRel>:/GL->
      $<$<CONFIG:MinSizeRel>:/Od /Ob0>
  )
  target_link_options(
    _kaldi_native_io
    PRIVATE
      $<$<CONFIG:Release>:/LTCG:OFF>
      $<$<CONFIG:RelWithDebInfo>:/LTCG:OFF>
      $<$<CONFIG:MinSizeRel>:/LTCG:OFF>
  )
endif()

if(APPLE)
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE PYTHON_SITE_PACKAGE_DIR
  )
  message(STATUS "PYTHON_SITE_PACKAGE_DIR: ${PYTHON_SITE_PACKAGE_DIR}")
  if(PYTHON_SITE_PACKAGE_DIR STREQUAL "")
    message(WARNING "PYTHON_SITE_PACKAGE_DIR is empty!")
  else()
    target_link_libraries(_kaldi_native_io PRIVATE "-Wl,-rpath,${PYTHON_SITE_PACKAGE_DIR}/kaldi_native_io/lib")
  endif()
endif()

if(NOT WIN32)
  target_link_libraries(_kaldi_native_io PRIVATE "-Wl,-rpath,${KALDI_NATIVE_IO_RPATH_ORIGIN}/kaldi_native_io/lib")
endif()

install(TARGETS _kaldi_native_io DESTINATION lib)

# `kaldi_native_io/__init__.py` does `from kaldi_native_io.lib._kaldi_native_io
# import ...`, so the extension has to sit next to the Python package it
# belongs to. The tests put kaldi_native_io/python on PYTHONPATH and expect to
# find it under kaldi_native_io/lib/; `make install` puts it there for a real
# install, this covers the build tree.
add_custom_command(TARGET _kaldi_native_io POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory
    "${CMAKE_SOURCE_DIR}/kaldi_native_io/python/kaldi_native_io/lib"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:_kaldi_native_io>"
    "${CMAKE_SOURCE_DIR}/kaldi_native_io/python/kaldi_native_io/lib/"
  COMMENT "Copying $<TARGET_FILE_NAME:_kaldi_native_io> to kaldi_native_io/python/kaldi_native_io/lib"
)

add_executable(copy-blob
  copy-blob.cc
  parse-options.cc
)
target_link_libraries(copy-blob kaldi_native_io_core)
if(NOT WIN32)
  target_link_libraries(copy-blob -pthread)
endif()

# Installed into the package (kaldi_native_io/bin/copy-blob) so the wheel ships
# it via the normal install rules; setup.py copies nothing by hand.
install(TARGETS copy-blob DESTINATION bin)
