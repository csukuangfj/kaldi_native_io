#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import kaldi_native_io

base = "blob"
wspecifier = f"ark,scp:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_blob_writer():
    with kaldi_native_io.BlobWriter(wspecifier) as ko:
        ko.write("a", bytes([0x30, 0x31]))
        ko["b"] = b"1234"


def test_sequential_blob_reader():
    with kaldi_native_io.SequentialBlobReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert value == bytes([0x30, 0x31])
            elif key == "b":
                assert value == b"1234"
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_blob_reader():
    with kaldi_native_io.RandomAccessBlobReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert ki["a"] == bytes([0x30, 0x31])
        assert ki["b"] == b"1234"


def test_read_single_item():
    a = bytes([10, 20])
    b = b"1234"

    with kaldi_native_io.BlobWriter("ark,scp:b.ark,b.scp") as ko:
        ko.write("a", a)
        ko["b"] = b
    """
    b.scp contains:
      a b.ark:2
      b b.ark:20
    """

    va = kaldi_native_io.read_blob("b.ark:2")
    assert va == a, (va, a)

    vb = kaldi_native_io.read_blob("b.ark:20")
    assert vb == b, (vb, b)

    os.remove("b.scp")
    os.remove("b.ark")


def main():
    test_blob_writer()
    test_sequential_blob_reader()
    test_random_access_blob_reader()
    test_read_single_item()
    return

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
