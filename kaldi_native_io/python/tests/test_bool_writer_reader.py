#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import kaldi_native_io

base = "bool"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_bool_writer():
    with kaldi_native_io.BoolWriter(wspecifier) as ko:
        ko.write("a", True)
        ko["b"] = False


def test_sequential_bool_reader():
    with kaldi_native_io.SequentialBoolReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert value is True
            elif key == "b":
                assert value is False
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_bool_reader():
    with kaldi_native_io.RandomAccessBoolReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert ki["a"] is True
        assert ki["b"] is False


def main():
    test_bool_writer()
    test_sequential_bool_reader()
    test_random_access_bool_reader()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
