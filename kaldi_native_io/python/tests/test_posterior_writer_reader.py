#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os
from typing import List, Tuple

import kaldi_native_io

base = "posterior"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def assert_posterior_equal(
    a: List[List[Tuple[int, float]]], b: List[List[Tuple[int, float]]]
):
    assert len(a) == len(b)
    for m, n in zip(a, b):
        assert len(m) == len(n)
        for p, q in zip(m, n):
            assert p[0] == q[0]
            assert abs(p[1] - q[1]) < 1e-5


def test_posterior_writer():
    with kaldi_native_io.PosteriorWriter(wspecifier) as ko:
        value = [
            [(1, 0.5), (2, 0.3)],
            [(9, 0.8)],
            [(10, 0.1), (11, 0.2), (12, 0.9)],
        ]
        ko.write("a", value)

        value = [
            [(3, 0.1), (4, 0.5), (8, 0.2)],
            [(3, 0.1)],
        ]
        ko["b"] = value


def test_sequential_posterior_reader():
    with kaldi_native_io.SequentialPosteriorReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                expected_value = [
                    [(1, 0.5), (2, 0.3)],
                    [(9, 0.8)],
                    [(10, 0.1), (11, 0.2), (12, 0.9)],
                ]
                assert_posterior_equal(value, expected_value)
            elif key == "b":
                expected_value = [
                    [(3, 0.1), (4, 0.5), (8, 0.2)],
                    [(3, 0.1)],
                ]
                assert_posterior_equal(value, expected_value)
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_posterior_reader():
    with kaldi_native_io.RandomAccessPosteriorReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki

        expected_a_value = [
            [(1, 0.5), (2, 0.3)],
            [(9, 0.8)],
            [(10, 0.1), (11, 0.2), (12, 0.9)],
        ]

        expected_b_value = [
            [(3, 0.1), (4, 0.5), (8, 0.2)],
            [(3, 0.1)],
        ]

        assert_posterior_equal(ki["a"], expected_a_value)
        assert_posterior_equal(ki["b"], expected_b_value)


def main():
    test_posterior_writer()
    test_sequential_posterior_reader()
    test_random_access_posterior_reader()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
