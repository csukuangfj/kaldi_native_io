#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import numpy as np

import kaldi_native_io

base = "float_vector"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_float_vector_writer():
    with kaldi_native_io.FloatVectorWriter(wspecifier) as ko:
        ko.write("a", np.array([1, 2], dtype=np.float32))
        ko["b"] = np.array([10.5], dtype=np.float32)


def test_sequential_float_vector_reader():
    with kaldi_native_io.SequentialFloatVectorReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert np.array_equal(value, np.array([1, 2], dtype=np.float32))
            elif key == "b":
                assert np.array_equal(value, np.array([10.5], dtype=np.float32))
            else:
                raise ValueError(f"Unknown key {key} with value {value}")


def test_random_access_float_vector_reader():
    with kaldi_native_io.RandomAccessFloatVectorReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert np.array_equal(ki["a"], np.array([1, 2], dtype=np.float32))
        assert np.array_equal(ki["b"], np.array([10.5], dtype=np.float32))


def test_read_write_single_vector():
    a = np.array([1, 2], dtype=np.float32)
    v = kaldi_native_io.FloatVector(a)
    v.write(wxfilename="binary.ark", binary=True)

    b = kaldi_native_io.FloatVector.read("binary.ark")
    assert np.array_equal(a, b.numpy())

    os.remove("binary.ark")

    a = np.array([1, 2], dtype=np.float32)
    b = np.array([10.5], dtype=np.float32)
    with kaldi_native_io.FloatVectorWriter("ark,scp:v.ark,v.scp") as ko:
        ko.write("a", a)
        ko["b"] = b

    """
    v.scp contains:
      a v.ark:2
      b v.ark:22
    """
    va = kaldi_native_io.FloatVector.read("v.ark:2")
    assert np.array_equal(va.numpy(), a)

    vb = kaldi_native_io.FloatVector.read("v.ark:22")
    assert np.array_equal(vb.numpy(), b)

    os.remove("v.scp")
    os.remove("v.ark")


def main():
    test_float_vector_writer()
    test_sequential_float_vector_reader()
    test_random_access_float_vector_reader()
    test_read_write_single_vector()

    os.remove(f"{base}.scp")
    os.remove(f"{base}.ark")


if __name__ == "__main__":
    main()
