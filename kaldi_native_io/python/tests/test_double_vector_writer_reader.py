#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import numpy as np

import kaldi_native_io

base = "double_vector"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_double_vector_writer():
    with kaldi_native_io.DoubleVectorWriter(wspecifier) as ko:
        ko.write("a", np.array([1, 2], dtype=np.float64))
        ko["b"] = np.array([10.5], dtype=np.float64)


def test_sequential_double_vector_reader():
    with kaldi_native_io.SequentialDoubleVectorReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert np.array_equal(value, np.array([1, 2], dtype=np.float64))
            elif key == "b":
                assert np.array_equal(value, np.array([10.5], dtype=np.float64))
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_double_vector_reader():
    with kaldi_native_io.RandomAccessDoubleVectorReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert np.array_equal(ki["a"], np.array([1, 2], dtype=np.float64))
        assert np.array_equal(ki["b"], np.array([10.5], dtype=np.float64))


def test_read_write_single_vector():
    a = np.array([1, 2], dtype=np.float64)
    v = kaldi_native_io.DoubleVector(a)
    v.write(wxfilename="binary.ark", binary=True)

    b = kaldi_native_io.DoubleVector.read("binary.ark")
    assert np.array_equal(a, b.numpy())

    os.remove("binary.ark")

    a = np.array([1, 2], dtype=np.float64)
    b = np.array([10.5], dtype=np.float64)
    with kaldi_native_io.DoubleVectorWriter("ark,scp:v.ark,v.scp") as ko:
        ko.write("a", a)
        ko["b"] = b

    """
    v.scp contains:
      a v.ark:2
      b v.ark:30
    """
    va = kaldi_native_io.DoubleVector.read("v.ark:2")
    assert np.array_equal(va.numpy(), a)

    vb = kaldi_native_io.DoubleVector.read("v.ark:30")
    assert np.array_equal(vb.numpy(), b)

    os.remove("v.scp")
    os.remove("v.ark")


def main():
    test_double_vector_writer()
    test_sequential_double_vector_reader()
    test_random_access_double_vector_reader()
    test_read_write_single_vector()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
