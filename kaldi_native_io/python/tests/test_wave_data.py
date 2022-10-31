#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)

import os

import numpy as np

import kaldi_native_io


def test_from_1d_array():
    sample_freq = 16000
    a = np.arange(8000, dtype=np.float32)
    wave_data = kaldi_native_io.WaveData(sample_freq=sample_freq, data=a)
    b = wave_data.data.numpy()

    # Always 2-d. Number of rows == number of channels
    assert b.ndim == 2

    assert np.array_equal(b[0], a)

    assert wave_data.sample_freq == sample_freq
    assert wave_data.duration == a.shape[0] / sample_freq


def test_from_2d_array():
    sample_freq = 16000
    a = np.arange(8000, dtype=np.float32)
    b = a + 10
    c = a + 10
    samples = np.stack([a, b, c], axis=0)
    assert samples.shape == (3, 8000)

    wave_data = kaldi_native_io.WaveData(sample_freq=sample_freq, data=samples)
    d = wave_data.data.numpy()

    # Always 2-d. Number of rows == number of channels
    assert d.ndim == 2

    assert np.array_equal(d[0], a)
    assert np.array_equal(d[1], b)
    assert np.array_equal(d[2], c)

    assert wave_data.sample_freq == sample_freq
    assert wave_data.duration == a.shape[0] / sample_freq


def main():
    test_from_1d_array()
    test_from_2d_array()


if __name__ == "__main__":
    main()
