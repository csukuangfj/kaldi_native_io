#!/usr/bin/env python3

# Copyright      2022  Xiaomi Corporation (authors: Fangjun Kuang)
from pathlib import Path

import kaldi_native_io

base = "/ceph-fj/fangjun/open-source-2/kaldi_native_io/build-1.10/a.scp"
rspecifier = f"scp:{base}"

# a.scp contains something like the following:
"""
a /path/to/foo.wav
b /path/to/bar.wav
"""


def test_sequential_wave_info_reader():
    if not Path(base).exists():
        return
    with kaldi_native_io.SequentialWaveInfoReader(rspecifier) as ki:
        for key, value in ki:
            print(key, value)


def test_random_access_wave_info_reader():
    if not Path(base).exists():
        return
    with kaldi_native_io.RandomAccessWaveInfoReader(rspecifier) as ki:
        assert "a" in ki
        assert "b" in ki
        print(ki["a"])
        print(ki["b"])


def main():
    if Path(base).exists():
        test_sequential_wave_info_reader()
        test_random_access_wave_info_reader()


if __name__ == "__main__":
    main()
