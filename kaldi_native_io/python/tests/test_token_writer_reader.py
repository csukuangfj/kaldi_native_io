#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import kaldi_native_io

base = "token"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_token_writer():
    with kaldi_native_io.TokenWriter(wspecifier) as ko:
        ko.write("a", "hello_world")
        ko["b"] = "hi"


def test_sequential_token_reader():
    with kaldi_native_io.SequentialTokenReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert value == "hello_world"
            elif key == "b":
                assert value == "hi"
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_token_reader():
    with kaldi_native_io.RandomAccessTokenReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert ki["a"] == "hello_world"
        assert ki["b"] == "hi"


def main():
    test_token_writer()
    test_sequential_token_reader()
    test_random_access_token_reader()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
