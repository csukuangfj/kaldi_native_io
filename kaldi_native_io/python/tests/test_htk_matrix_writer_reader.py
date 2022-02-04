#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import numpy as np

import kaldi_native_io

base = "htk_matrix"
wspecifier = f"ark,scp:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_htk_matrix_writer():
    with kaldi_native_io.HtkMatrixWriter(wspecifier) as ko:
        a_value = np.array([[1, 2], [3, 4]], dtype=np.float32)
        # Refer to https://labrosa.ee.columbia.edu/doc/HTKBook21/node58.html
        # for the format of the header
        # Assume frame shift is 10ms, which is 10*10^6 ns,
        # which is 100000 x 100 ns
        a_header = kaldi_native_io.HtkHeader(
            num_samples=2,  # there are two frames, i.e., two rows
            sample_period=100000,  # in 100 ns
            sample_size=4 * a_value.shape[1],  # each column is 4 bytes
            # 6 -> MFCC
            # 0o100 -> has energy
            sample_kind=6 | 0o100,
        )
        ko.write("a", (a_value, a_header))

        b_value = np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float32)
        b_header = kaldi_native_io.HtkHeader(
            num_samples=2,  # there are two frames, i.e., two rows
            sample_period=100000,  # in 100 ns
            sample_size=4 * b_value.shape[1],  # each column is 4 bytes
            # 6 -> MFCC
            # 0o100 -> has energy
            sample_kind=6 | 0o100,
        )
        ko["b"] = (b_value, b_header)


def test_sequential_htk_matrix_reader():
    with kaldi_native_io.SequentialHtkMatrixReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert np.array_equal(
                    value[0], np.array([[1, 2], [3, 4]], dtype=np.float32)
                )
                expected_header = kaldi_native_io.HtkHeader(
                    num_samples=2,  # there are two frames, i.e., two rows
                    sample_period=100000,  # in 100 ns
                    sample_size=4 * value[0].shape[1],  # each column is 4 bytes
                    # 6 -> MFCC
                    # 0o100 -> has energy
                    sample_kind=6 | 0o100,
                )
                assert value[1] == expected_header
            elif key == "b":
                assert np.array_equal(
                    value[0],
                    np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float32),
                )
                expected_header = kaldi_native_io.HtkHeader(
                    num_samples=2,  # there are two frames, i.e., two rows
                    sample_period=100000,  # in 100 ns
                    sample_size=4 * value[0].shape[1],  # each column is 4 bytes
                    # 6 -> MFCC
                    # 0o100 -> has energy
                    sample_kind=6 | 0o100,
                )
                assert value[1] == expected_header
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_htk_matrix_reader():
    with kaldi_native_io.RandomAccessHtkMatrixReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert np.array_equal(
            ki["a"][0], np.array([[1, 2], [3, 4]], dtype=np.float32)
        )

        expected_a_header = kaldi_native_io.HtkHeader(
            num_samples=2,  # there are two frames, i.e., two rows
            sample_period=100000,  # in 100 ns
            sample_size=4 * ki["a"][0].shape[1],  # each column is 4 bytes
            # 6 -> MFCC
            # 0o100 -> has energy
            sample_kind=6 | 0o100,
        )
        assert expected_a_header == ki["a"][1]

        assert np.array_equal(
            ki["b"][0], np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float32)
        )
        expected_b_header = kaldi_native_io.HtkHeader(
            num_samples=2,  # there are two frames, i.e., two rows
            sample_period=100000,  # in 100 ns
            sample_size=4 * ki["b"][0].shape[1],  # each column is 4 bytes
            # 6 -> MFCC
            # 0o100 -> has energy
            sample_kind=6 | 0o100,
        )
        assert expected_b_header == ki["b"][1]


def main():
    test_htk_matrix_writer()
    test_sequential_htk_matrix_reader()
    test_random_access_htk_matrix_reader()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
