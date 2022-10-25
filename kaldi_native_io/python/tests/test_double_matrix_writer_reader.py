#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import numpy as np

import kaldi_native_io

base = "double_matrix"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_double_matrix_writer():
    with kaldi_native_io.DoubleMatrixWriter(wspecifier) as ko:
        ko.write("a", np.array([[1, 2], [3, 4]], dtype=np.float64))
        ko["b"] = np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float64)


def test_sequential_double_matrix_reader():
    with kaldi_native_io.SequentialDoubleMatrixReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert np.array_equal(
                    value, np.array([[1, 2], [3, 4]], dtype=np.float64)
                )
            elif key == "b":
                assert np.array_equal(
                    value,
                    np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float64),
                )
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_double_matrix_reader():
    with kaldi_native_io.RandomAccessDoubleMatrixReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert np.array_equal(
            ki["a"], np.array([[1, 2], [3, 4]], dtype=np.float64)
        )
        assert np.array_equal(
            ki["b"], np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float64)
        )


def test_read_write_single_mat():
    arr = np.array(
        [
            [0, 1, 2, 22, 33],
            [3, 4, 5, -1, -3],
            [6, 7, 8, -9, 0],
            [9, 10, 11, 5, 100],
        ],
        dtype=np.float64,
    )
    mat = kaldi_native_io.DoubleMatrix(arr)
    mat.write(wxfilename="binary.ark", binary=True)
    mat.write(wxfilename="matrix.txt", binary=False)

    m1 = kaldi_native_io.FloatMatrix.read("binary.ark")
    m2 = kaldi_native_io.FloatMatrix.read("matrix.txt")

    assert np.array_equal(mat, m1)
    assert np.array_equal(mat, m2)

    # read range
    # Note: the upper bound is inclusive!
    m3 = kaldi_native_io.FloatMatrix.read("binary.ark[0:1]")  # row 0 and row 1
    assert np.array_equal(mat.numpy()[0:2], m3.numpy())

    m4 = kaldi_native_io.FloatMatrix.read(
        "matrix.txt[:,3:4]"
    )  # column 3 and column 4
    assert np.array_equal(mat.numpy()[:, 3:5], m4.numpy())

    os.remove("binary.ark")
    os.remove("matrix.txt")

    a = np.array([[1, 2], [3, 4]], dtype=np.float64)
    b = np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float64)
    with kaldi_native_io.DoubleMatrixWriter("ark,scp:m.ark,m.scp") as ko:
        ko.write("a", a)
        ko["b"] = b

    """
    m.scp contains:
      a m.ark:2
      b m.ark:51
    """

    m5 = kaldi_native_io.FloatMatrix.read("m.ark:2")
    assert np.array_equal(m5.numpy(), a)

    m6 = kaldi_native_io.FloatMatrix.read("m.ark:51")
    assert np.array_equal(m6.numpy(), b)

    os.remove("m.scp")
    os.remove("m.ark")


def main():
    test_double_matrix_writer()
    test_sequential_double_matrix_reader()
    test_random_access_double_matrix_reader()

    test_read_write_single_mat()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
