#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import kaldi_native_io

base = "float_pair_vector"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_float_pair_vector_writer():
    with kaldi_native_io.FloatPairVectorWriter(wspecifier) as ko:
        ko.write("a", [(10, 20.5), (3, 4.5)])
        ko["b"] = [(1, 2.5)]


def test_sequential_float_pair_vector_reader():
    with kaldi_native_io.SequentialFloatPairVectorReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert value == [(10, 20.5), (3, 4.5)]
            elif key == "b":
                assert value == [(1, 2.5)]
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_float_pair_vector_reader():
    with kaldi_native_io.RandomAccessFloatPairVectorReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert ki["a"] == [(10, 20.5), (3, 4.5)]
        assert ki["b"] == [(1, 2.5)]


def main():
    test_float_pair_vector_writer()
    test_sequential_float_pair_vector_reader()
    test_random_access_float_pair_vector_reader()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
