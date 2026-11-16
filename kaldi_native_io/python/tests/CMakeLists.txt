function(kaldi_native_io_add_py_test source)
  get_filename_component(name ${source} NAME_WE)
  set(name "${name}_py")

  # Fall back to the interpreter found by FindPython when the legacy
  # PYTHON_EXECUTABLE cache variable was not provided (ctest would otherwise
  # fail with "no such file or directory").
  if(NOT PYTHON_EXECUTABLE)
    if(Python_EXECUTABLE)
      set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
    elseif(Python3_EXECUTABLE)
      set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
    endif()
  endif()

  get_filename_component(kaldi_native_io_path ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)

  # PYTHONPATH entries are separated by ":" on posix and ";" on Windows.
  # The ENVIRONMENT test property is a CMake list: a literal ";" in a value
  # splits it into bogus entries, and $<SEMICOLON> does not help because
  # ctest re-splits the property after evaluating the genex (so only the
  # first entry would survive). Run the test through "cmake -E env" instead:
  # it keeps the whole assignment as a single argument on all platforms.
  if(WIN32)
    set(_pathsep ";")
  else()
    set(_pathsep ":")
  endif()
  # The extension module is copied to kaldi_native_io/lib/ by the
  # _kaldi_native_io target; the tests import the package from
  # kaldi_native_io/python/.
  set(_pythonpath
    "${kaldi_native_io_path}${_pathsep}$<TARGET_FILE_DIR:_kaldi_native_io>${_pathsep}$ENV{PYTHONPATH}"
  )

  add_test(NAME ${name}
    COMMAND
      "${CMAKE_COMMAND}" -E env "PYTHONPATH=${_pythonpath}"
      "${PYTHON_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
  )
endfunction()

# please sort the files in alphabetic order
set(py_test_files
  test_blob_writer_reader.py
  test_bool_writer_reader.py
  test_compressed_matrix_writer_reader.py
  test_double_matrix_writer_reader.py
  test_double_vector_writer_reader.py
  test_double_writer_reader.py
  test_float_matrix_writer_reader.py
  test_float_pair_vector_writer_reader.py
  test_float_vector_writer_reader.py
  test_float_writer_reader.py
  test_gauss_post_writer_reader.py
  test_htk_matrix_writer_reader.py
  test_int32_pair_vector_writer_reader.py
  test_int32_vector_vector_writer_reader.py
  test_int32_vector_writer_reader.py
  test_int32_writer_reader.py
  test_int8_vector_writer_reader.py
  test_matrix_shape_reader.py
  test_posterior_writer_reader.py
  test_token_vector_writer_reader.py
  test_token_writer_reader.py
  test_wave_data.py
  test_wave_info_reader.py
  test_wave_reader.py
)

foreach(source IN LISTS py_test_files)
  kaldi_native_io_add_py_test(${source})
endforeach()
