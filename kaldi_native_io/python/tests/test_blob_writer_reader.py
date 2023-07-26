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

    # test range read
    # [start:end], both ends are inclusive
    # Must satisfy 0 <= start <= end < length of the data

    # start 0, end 2
    vc = kaldi_native_io.read_blob("b.ark:20[0:2]")
    assert vc == b"123", (vc, b"123")

    # start 1, end 2
    vd = kaldi_native_io.read_blob("b.ark:20[1:2]")
    assert vd == b"23", (vd, b"23")

    # start 2, end 2
    ve = kaldi_native_io.read_blob("b.ark:20[2:2]")
    assert ve == b"3", (ve, b"3")

    # start 2, end -1
    # -1 means the end of the data
    vf = kaldi_native_io.read_blob("b.ark:20[2:-1]")
    assert vf == b"34", (vf, b"34")

    # [:] means all the data
    vg = kaldi_native_io.read_blob("b.ark:20[:]")
    assert vg == b"1234", (vg, b"1234")

    os.remove("b.scp")
    os.remove("b.ark")


def main():
    test_blob_writer()
    test_sequential_blob_reader()
    test_random_access_blob_reader()
    test_read_single_item()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
