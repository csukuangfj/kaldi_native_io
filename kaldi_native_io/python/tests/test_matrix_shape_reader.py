#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)
import os
from pathlib import Path

import numpy as np

import kaldi_native_io


def test_matrix_shape_from_uncompressed_matrix():
    for dtype in [np.float32, np.float64]:
        arr = np.array(
            [
                [0, 1, 2, 22, 33],
                [3, 4, 5, -1, -3],
                [6, 7, 8, -9, 0],
                [9, 10, 11, 5, 100],
            ],
            dtype=dtype,
        )
        if dtype == np.float32:
            MatrixType = kaldi_native_io.FloatMatrix
        else:
            MatrixType = kaldi_native_io.DoubleMatrix

        mat = MatrixType(arr)

        mat.write(wxfilename="binary.ark", binary=True)
        mat_shape = kaldi_native_io.MatrixShape.read("binary.ark")
        assert mat_shape.num_rows == arr.shape[0], mat_shape
        assert mat_shape.num_cols == arr.shape[1], mat_shape

        # Now for text format

        mat.write(wxfilename="matrix.txt", binary=False)
        mat_shape = kaldi_native_io.MatrixShape.read("matrix.txt")
        assert mat_shape.num_rows == arr.shape[0], mat_shape
        assert mat_shape.num_cols == arr.shape[1], mat_shape

        os.remove("binary.ark")
        os.remove("matrix.txt")


def test_matrix_shape_from_compressed_matrix():
    wspecifier = "ark,scp:mat.ark,mat.scp"
    with kaldi_native_io.CompressedMatrixWriter(wspecifier) as ko:
        ko.write(
            "a",
            np.array([[1, 2], [3, 4]], dtype=np.float32),
            method=kaldi_native_io.CompressionMethod.kAutomaticMethod,
        )
        ko["b"] = np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float64)

    with open("mat.scp") as f:
        for line in f:
            key, rxfilename = line.split()
            mat_shape = kaldi_native_io.MatrixShape.read(rxfilename)
            if key == "a":
                assert mat_shape.num_rows == 2
                assert mat_shape.num_cols == 2
            else:
                assert mat_shape.num_rows == 2
                assert mat_shape.num_cols == 3
    with kaldi_native_io.SequentialMatrixShapeReader("scp:mat.scp") as ki:
        for key, mat_shape in ki:
            if key == "a":
                assert mat_shape.num_rows == 2
                assert mat_shape.num_cols == 2
            else:
                assert mat_shape.num_rows == 2
                assert mat_shape.num_cols == 3

    with kaldi_native_io.RandomAccessMatrixShapeReader("scp:mat.scp") as ki:
        assert ki["a"].num_rows == 2
        assert ki["a"].num_cols == 2

        assert ki["b"].num_rows == 2
        assert ki["b"].num_cols == 3

    os.remove("mat.scp")
    os.remove("mat.ark")


def main():
    test_matrix_shape_from_uncompressed_matrix()
    test_matrix_shape_from_compressed_matrix()


if __name__ == "__main__":
    main()
