#!/usr/bin/env python3
#
# Copyright (c)  2026  Xiaomi Corporation (author: Fangjun Kuang)
#
# Smoke test for the built wheels. Run by build-wheels-*.yaml after installing
# a freshly built wheel, and by test-pip-install.yaml / test-uv-install.yaml
# after installing from PyPI.
#
# It exercises a real write->read round trip through the table classes so that
# a wheel which cannot load its extension module (missing shared library, wrong
# architecture, wrong platform tag) fails here rather than on a user's machine.
#
# Run it from outside the source tree so `import kaldi_native_io` resolves to
# the installed wheel, not to the checkout:
#
#     cd "$RUNNER_TEMP" && python3 "$GITHUB_WORKSPACE/scripts/smoke_test.py"

import platform
import sys
import tempfile
from pathlib import Path

import numpy as np

import kaldi_native_io


def check_table_io(tmpdir: Path) -> None:
    """Round trip a few table types through the real C++ implementation."""
    base = tmpdir / "smoke"
    wspecifier = f"ark,scp:{base}.ark,{base}.scp"
    rspecifier = f"scp:{base}.scp"

    # Token (std::string) -- no numpy involved.
    with kaldi_native_io.TokenWriter(wspecifier) as ko:
        ko.write("a", "hello")
        ko["b"] = "world"

    with kaldi_native_io.SequentialTokenReader(rspecifier) as ki:
        got = {key: value for key, value in ki}
    assert got == {"a": "hello", "b": "world"}, f"token round trip: {got}"

    with kaldi_native_io.RandomAccessTokenReader(rspecifier) as ki:
        assert "a" in ki and "b" in ki
        assert ki["a"] == "hello" and ki["b"] == "world"

    # int32
    base2 = tmpdir / "smoke_int32"
    ws2 = f"ark,scp:{base2}.ark,{base2}.scp"
    rs2 = f"scp:{base2}.scp"
    with kaldi_native_io.Int32Writer(ws2) as ko:
        ko.write("a", 1)
        ko["b"] = -2
    with kaldi_native_io.SequentialInt32Reader(rs2) as ki:
        got = {key: value for key, value in ki}
    assert got == {"a": 1, "b": -2}, f"int32 round trip: {got}"

    # bytes / blob
    base3 = tmpdir / "smoke_blob"
    ws3 = f"ark,scp:{base3}.ark,{base3}.scp"
    rs3 = f"scp:{base3}.scp"
    with kaldi_native_io.BlobWriter(ws3) as ko:
        ko.write("a", bytes([0x30, 0x31]))
        ko["b"] = b"1234"
    with kaldi_native_io.RandomAccessBlobReader(rs3) as ki:
        assert ki["a"] == bytes([0x30, 0x31])
        assert ki["b"] == b"1234"

    # float vector -- exercises the numpy interop path.
    base4 = tmpdir / "smoke_fv"
    ws4 = f"ark,scp,t:{base4}.ark,{base4}.scp"
    rs4 = f"scp:{base4}.scp"
    with kaldi_native_io.FloatVectorWriter(ws4) as ko:
        ko.write("a", np.array([1, 2], dtype=np.float32))
        ko["b"] = np.array([10.5], dtype=np.float32)
    with kaldi_native_io.RandomAccessFloatVectorReader(rs4) as ki:
        assert np.array_equal(ki["a"], np.array([1, 2], dtype=np.float32))
        assert np.array_equal(ki["b"], np.array([10.5], dtype=np.float32))

    # float matrix
    base5 = tmpdir / "smoke_fm"
    ws5 = f"ark,scp:{base5}.ark,{base5}.scp"
    rs5 = f"scp:{base5}.scp"
    m = np.arange(6, dtype=np.float32).reshape(2, 3)
    with kaldi_native_io.FloatMatrixWriter(ws5) as ko:
        ko.write("a", m)
    with kaldi_native_io.RandomAccessFloatMatrixReader(rs5) as ki:
        assert np.array_equal(ki["a"], m)

    # single-item readers: read_int32_vector takes an "ark:offset" rxfilename
    # into a table of int32 *vectors*, so write those and use the scp entry for
    # the offset.
    base6 = tmpdir / "smoke_iv"
    ws6 = f"ark,scp:{base6}.ark,{base6}.scp"
    with kaldi_native_io.Int32VectorWriter(ws6) as ko:
        ko.write("a", [10, 20])
        ko["b"] = [100, 200, 300]

    scp_entries = {}
    for line in (tmpdir / "smoke_iv.scp").read_text().splitlines():
        key, rxfilename = line.split(maxsplit=1)
        scp_entries[key] = rxfilename

    assert kaldi_native_io.read_int32_vector(scp_entries["a"]) == [10, 20]
    assert kaldi_native_io.read_int32_vector(scp_entries["b"]) == [100, 200, 300]


def check_wave_data() -> None:
    sample_freq = 16000
    a = np.arange(8000, dtype=np.float32)
    wave = kaldi_native_io.WaveData(sample_freq=sample_freq, data=a)
    d = wave.data.numpy()
    assert d.ndim == 2, f"expected 2-d wave data, got {d.ndim}"
    assert np.array_equal(d[0], a)
    assert wave.sample_freq == sample_freq
    assert wave.duration == a.shape[0] / sample_freq


def main() -> int:
    print("kaldi_native_io:", getattr(kaldi_native_io, "__version__", "<none>"),
          kaldi_native_io.__file__)
    print("extension:", kaldi_native_io._kaldi_native_io.__file__)
    print("python:", sys.version.replace("\n", " "))
    print("platform:", platform.platform())
    print("machine:", platform.machine())
    print("numpy:", np.__version__, np.__file__)

    # The wheel must load the extension from inside the package, not from a
    # top-level module and not from a source tree.
    ext_file = kaldi_native_io._kaldi_native_io.__file__
    pkg_dir = Path(kaldi_native_io.__file__).resolve().parent
    assert Path(ext_file).resolve().parent == pkg_dir / "lib", (
        f"extension is not in the package's lib/ dir: {ext_file}"
    )

    with tempfile.TemporaryDirectory() as tmp:
        check_table_io(Path(tmp))
    check_wave_data()

    print("SMOKE TEST PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
