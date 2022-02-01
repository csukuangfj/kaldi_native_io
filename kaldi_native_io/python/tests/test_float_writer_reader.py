#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import kaldi_native_io

base = "float"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_float_writer():
    with kaldi_native_io.FloatWriter(wspecifier) as ko:
        ko.write("a", 10.5)
        ko["b"] = 20.25


def test_sequential_float_reader():
    with kaldi_native_io.SequentialFloatReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert value == 10.5
            elif key == "b":
                assert value == 20.25
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_float_reader():
    with kaldi_native_io.RandomAccessFloatReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert ki["a"] == 10.5
        assert ki["b"] == 20.25


def main():
    test_float_writer()
    test_sequential_float_reader()
    test_random_access_float_reader()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
