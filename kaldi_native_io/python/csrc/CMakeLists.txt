include_directories(${CMAKE_SOURCE_DIR})

pybind11_add_module(_kaldi_native_io
  compressed-matrix.cc
  kaldi-matrix.cc
  kaldi-table.cc
  kaldi-vector.cc
  kaldiio.cc
)

target_link_libraries(_kaldi_native_io PRIVATE kaldi_native_io_core)
if(UNIX AND NOT APPLE)
  target_link_libraries(_kaldi_native_io PUBLIC ${PYTHON_LIBRARY})
endif()
