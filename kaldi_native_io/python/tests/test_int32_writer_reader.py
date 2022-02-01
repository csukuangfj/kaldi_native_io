#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import kaldi_native_io

base = "int32"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_int32_writer():
    with kaldi_native_io.Int32Writer(wspecifier) as ko:
        ko.write("a", 10)
        ko["b"] = 20


def test_sequential_int32_reader():
    with kaldi_native_io.SequentialInt32Reader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert value == 10
            elif key == "b":
                assert value == 20
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_int32_reader():
    with kaldi_native_io.RandomAccessInt32Reader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert ki["a"] == 10
        assert ki["b"] == 20


def main():
    test_int32_writer()
    test_sequential_int32_reader()
    test_random_access_int32_reader()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
