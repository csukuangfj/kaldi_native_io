function(kaldi_native_io_add_py_test source)
  get_filename_component(name ${source} NAME_WE)
  set(name "${name}_py")

  add_test(NAME ${name}
    COMMAND
      "${PYTHON_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
  )

  get_filename_component(kaldi_native_io_path ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)

  set_property(TEST ${name}
    PROPERTY ENVIRONMENT "PYTHONPATH=${kaldi_native_io_path}:$<TARGET_FILE_DIR:_kaldi_native_io>:$ENV{PYTHONPATH}"
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
