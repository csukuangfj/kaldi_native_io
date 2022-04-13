#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)
from pathlib import Path

import kaldi_native_io


base = "/ceph-fj/fangjun/open-source-2/kaldi_native_io/build/a.scp"
rspecifier = f"scp:{base}"

# a.scp contains something like the following:
"""
a /path/to/foo.wav
b /path/to/bar.wav
"""


def test_sequential_wave_reader():
    if not Path(base).exists():
        return
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
    if not Path(base).exists():
        return
    file = "/ceph-fj/fangjun/open-source-2/kaldi_native_io/build/BAC009S0002W0123.wav"
    wave = kaldi_native_io.read_wave(file)
    print("sample_freq", wave.sample_freq)  # e.g., 1600
    print("duration in seconds", wave.duration)
    print("data shape", wave.data.numpy().shape)  # (num_channels, num_samples)
    print("data shape", wave.data.shape)  # same as the above one
    print(f"data min", wave.data.numpy().min())
    print(f"data max", wave.data.numpy().max())


def test_read_wave_2():
    if not Path(base).exists():
        return
    # from a pipe
    file = "cat /ceph-fj/fangjun/open-source-2/kaldi_native_io/build/BAC009S0002W0123.wav |"
    wave = kaldi_native_io.read_wave(file)
    print("sample_freq", wave.sample_freq)
    print("duration in seconds", wave.duration)
    print("data shape", wave.data.numpy().shape)
    print("data shape", wave.data.shape)  # same as the above one
    print(f"data min", wave.data.numpy().min())
    print(f"data max", wave.data.numpy().max())


def main():
    if Path(base).exists():
        test_sequential_wave_reader()
        test_random_access_wave_reader()
        test_read_wave_1()
        test_read_wave_2()


if __name__ == "__main__":
    main()
