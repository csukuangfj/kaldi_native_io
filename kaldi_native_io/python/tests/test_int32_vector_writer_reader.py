#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import kaldi_native_io

base = "int32_vector"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_int32_vector_writer():
    with kaldi_native_io.Int32VectorWriter(wspecifier) as ko:
        ko.write("a", [10, 20])
        ko["b"] = [100, 200, 300]


def test_sequential_int32_vector_reader():
    with kaldi_native_io.SequentialInt32VectorReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert value == [10, 20]
            elif key == "b":
                assert value == [100, 200, 300]
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_int32_vector_reader():
    with kaldi_native_io.RandomAccessInt32VectorReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert ki["a"] == [10, 20]
        assert ki["b"] == [100, 200, 300]


def main():
    test_int32_vector_writer()
    test_sequential_int32_vector_reader()
    test_random_access_int32_vector_reader()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
