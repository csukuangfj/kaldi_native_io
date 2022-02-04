#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os
from typing import List, Tuple

import numpy as np

import kaldi_native_io

base = "gauss_post"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def assert_gauss_post_equal(
    a: List[List[Tuple[int, np.ndarray]]], b: List[List[Tuple[int, np.ndarray]]]
):
    assert len(a) == len(b)
    for m, n in zip(a, b):
        assert len(m) == len(n)
        for p, q in zip(m, n):
            assert p[0] == q[0]
            assert np.array_equal(p[1], q[1])


def test_gauss_post_writer():
    with kaldi_native_io.GaussPostWriter(wspecifier) as ko:
        value = [
            [
                (1, np.array([0.5, 0.2], dtype=np.float32)),
                (2, np.array([0.3], dtype=np.float32)),
            ],
            [(9, np.array([0.8, 0.1, 0.3], dtype=np.float32))],
            [
                (10, np.array([0.1, 0.2], dtype=np.float32)),
                (11, np.array([0.2, 0.3], dtype=np.float32)),
                (12, np.array([0.9], dtype=np.float32)),
            ],
        ]
        ko.write("a", value)

        value = [
            [
                (3, np.array([0.1, 0.3, 0.4], dtype=np.float32)),
                (4, np.array([0.5, 0.3, 0.2], dtype=np.float32)),
                (8, np.array([0.2], dtype=np.float32)),
            ],
            [(3, np.array([0.1, 0.5], dtype=np.float32))],
        ]
        ko["b"] = value


def test_sequential_gauss_post_reader():
    with kaldi_native_io.SequentialGaussPostReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                expected_value = [
                    [
                        (1, np.array([0.5, 0.2], dtype=np.float32)),
                        (2, np.array([0.3], dtype=np.float32)),
                    ],
                    [(9, np.array([0.8, 0.1, 0.3], dtype=np.float32))],
                    [
                        (10, np.array([0.1, 0.2], dtype=np.float32)),
                        (11, np.array([0.2, 0.3], dtype=np.float32)),
                        (12, np.array([0.9], dtype=np.float32)),
                    ],
                ]

                assert_gauss_post_equal(value, expected_value)
            elif key == "b":
                expected_value = [
                    [
                        (3, np.array([0.1, 0.3, 0.4], dtype=np.float32)),
                        (4, np.array([0.5, 0.3, 0.2], dtype=np.float32)),
                        (8, np.array([0.2], dtype=np.float32)),
                    ],
                    [(3, np.array([0.1, 0.5], dtype=np.float32))],
                ]

                assert_gauss_post_equal(value, expected_value)
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_gauss_post_reader():
    with kaldi_native_io.RandomAccessGaussPostReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki

        expected_a_value = [
            [
                (1, np.array([0.5, 0.2], dtype=np.float32)),
                (2, np.array([0.3], dtype=np.float32)),
            ],
            [(9, np.array([0.8, 0.1, 0.3], dtype=np.float32))],
            [
                (10, np.array([0.1, 0.2], dtype=np.float32)),
                (11, np.array([0.2, 0.3], dtype=np.float32)),
                (12, np.array([0.9], dtype=np.float32)),
            ],
        ]

        expected_b_value = [
            [
                (3, np.array([0.1, 0.3, 0.4], dtype=np.float32)),
                (4, np.array([0.5, 0.3, 0.2], dtype=np.float32)),
                (8, np.array([0.2], dtype=np.float32)),
            ],
            [(3, np.array([0.1, 0.5], dtype=np.float32))],
        ]

        assert_gauss_post_equal(ki["a"], expected_a_value)
        assert_gauss_post_equal(ki["b"], expected_b_value)


def main():
    test_gauss_post_writer()
    test_sequential_gauss_post_reader()
    test_random_access_gauss_post_reader()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
