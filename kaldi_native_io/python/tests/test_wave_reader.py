#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)
from pathlib import Path

import kaldi_native_io
import numpy as np


base = "/ceph-fj/fangjun/open-source/kaldi_native_io/build/wave.scp"
rspecifier = f"scp:{base}"

# a.scp contains something like the following:
"""
a /path/to/foo.wav
b /path/to/bar.wav
"""


def test_wave_writer():
    file1 = "/ceph-fj/fangjun/open-source-2/kaldi_native_io/build/BAC009S0002W0123.wav"
    if not Path(file1).is_file():
        return

    file2 = "/ceph-fj/fangjun/open-source-2/kaldi_native_io/build/BAC009S0002W0124.wav"
    if not Path(file2).is_file():
        return

    print("-----test_wave_writer------")

    file2 = f"cat {file2} |"

    wave1 = kaldi_native_io.read_wave(file1)
    wave2 = kaldi_native_io.read_wave(file2)

    wspecifier = "ark,scp:wave.ark,wave.scp"
    with kaldi_native_io.WaveWriter(wspecifier) as ko:
        ko.write("a", wave1)
        ko["b"] = wave2
    """
    wave.scp has the following content:
      a wave.ark:2
      b wave.ark:123728
    """
    wave3 = kaldi_native_io.read_wave("wave.ark:2")
    wave4 = kaldi_native_io.read_wave("wave.ark:123728")

    assert wave1.sample_freq == wave3.sample_freq
    assert wave2.sample_freq == wave4.sample_freq

    assert np.array_equal(wave1.data.numpy(), wave3.data.numpy())
    assert np.array_equal(wave2.data.numpy(), wave4.data.numpy())


def test_sequential_wave_reader():
    if not Path(base).exists():
        return
    print("-----test_sequential_wave_reader------")
    with kaldi_native_io.SequentialWaveReader(rspecifier) as ki:
        for key, value in ki:
            print(
                key,
                f"sample_freq: {value.sample_freq}, "
                f"duration: {value.duration} (seconds)",
                f"data shape: {value.data.numpy().shape}",
                f"data shape: {value.data.shape}",  # same as the above one
            )


def test_random_access_wave_reader():
    if not Path(base).exists():
        return
    print("-----test_random_access_wave_reader------")
    with kaldi_native_io.RandomAccessWaveReader(rspecifier) as ki:
        assert "a" in ki
        assert "b" in ki
        # You can access the following fields of ki['a'] and ki['b']
        # - sample_freq
        # - duration (in seconds)
        # - data.numpy() (a 2-D array of shape (num_channels, num_samples)
        # Note: the data is not normalized, i.e., it is in the range
        # [-32768, 32767]
        print(ki["a"])
        print(ki["b"])


def test_read_wave_1():
    file = "/ceph-fj/fangjun/open-source-2/kaldi_native_io/build/BAC009S0002W0123.wav"
    if not Path(file).is_file():
        return
    print("-----test_read_wave_1------")
    wave = kaldi_native_io.read_wave(file)
    print("sample_freq", wave.sample_freq)  # e.g., 1600
    print("duration in seconds", wave.duration)
    print("data shape", wave.data.numpy().shape)  # (num_channels, num_samples)
    print("data shape", wave.data.shape)  # same as the above one
    print(f"data min", wave.data.numpy().min())
    print(f"data max", wave.data.numpy().max())


def test_read_wave_2():
    file = "/ceph-fj/fangjun/open-source-2/kaldi_native_io/build/BAC009S0002W0123.wav"
    if not Path(file).is_file():
        return
    print("-----test_read_wave_2------")
    # from a pipe
    file = f"cat {file} |"
    wave = kaldi_native_io.read_wave(file)
    print("sample_freq", wave.sample_freq)
    print("duration in seconds", wave.duration)
    print("data shape", wave.data.numpy().shape)
    print("data shape", wave.data.shape)  # same as the above one
    print(f"data min", wave.data.numpy().min())
    print(f"data max", wave.data.numpy().max())


def main():
    test_wave_writer()
    test_sequential_wave_reader()
    test_random_access_wave_reader()
    test_read_wave_1()
    test_read_wave_2()


if __name__ == "__main__":
    main()
