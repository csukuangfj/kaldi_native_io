#!/usr/bin/env python3
#
# Copyright (c)  2026  Xiaomi Corporation (author: Fangjun Kuang)
#
# Run every test file in kaldi_native_io/python/tests/ against the *installed*
# kaldi_native_io wheel. Used as a gate before publishing wheels.
#
# Why this exists rather than just calling pytest in the source tree:
#
#   1. The repository root contains a directory named `kaldi_native_io/` (with
#      CMakeLists.txt, csrc/, python/). Python treats that as a namespace
#      package, so `import kaldi_native_io` from inside the checkout can
#      succeed against the source tree instead of the installed wheel. We copy
#      the tests out to a temporary directory and run from there so the
#      installed wheel is what gets imported.
#
#   2. Each test file is written as a script with `if __name__ == "__main__":
#      main()`, and `main()` calls the test functions in a specific order --
#      several write an ark/scp first and read it in later functions. Running
#      each file as a script preserves that ordering; pytest's collection
#      would not necessarily.
#
# Usage:
#     python3 scripts/run_python_tests.py
#
# Run it from outside the source tree (e.g. cd "$RUNNER_TEMP" first).

import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

# Every file currently in kaldi_native_io/python/tests/. Asserted against so a
# future test file cannot be silently skipped by this runner.
EXPECTED_TESTS = [
    "test_blob_writer_reader.py",
    "test_bool_writer_reader.py",
    "test_compressed_matrix_writer_reader.py",
    "test_double_matrix_writer_reader.py",
    "test_double_vector_writer_reader.py",
    "test_double_writer_reader.py",
    "test_float_matrix_writer_reader.py",
    "test_float_pair_vector_writer_reader.py",
    "test_float_vector_writer_reader.py",
    "test_float_writer_reader.py",
    "test_gauss_post_writer_reader.py",
    "test_htk_matrix_writer_reader.py",
    "test_int32_pair_vector_writer_reader.py",
    "test_int32_vector_vector_writer_reader.py",
    "test_int32_vector_writer_reader.py",
    "test_int32_writer_reader.py",
    "test_int8_vector_writer_reader.py",
    "test_matrix_shape_reader.py",
    "test_posterior_writer_reader.py",
    "test_token_vector_writer_reader.py",
    "test_token_writer_reader.py",
    "test_wave_data.py",
    "test_wave_info_reader.py",
    "test_wave_reader.py",
]


def find_tests_dir() -> Path:
    """Locate kaldi_native_io/python/tests relative to this script."""
    here = Path(__file__).resolve().parent
    tests_dir = here.parent / "kaldi_native_io" / "python" / "tests"
    if not tests_dir.is_dir():
        raise SystemExit(f"cannot find the tests directory: {tests_dir}")
    return tests_dir


def main() -> int:
    tests_dir = find_tests_dir()
    on_disk = sorted(p.name for p in tests_dir.glob("test_*.py"))
    expected = sorted(EXPECTED_TESTS)

    print(f"tests dir : {tests_dir}")
    print(f"python    : {sys.executable}")
    print(f"version   : {sys.version.replace(chr(10), ' ')}")

    import kaldi_native_io

    print(f"module    : {kaldi_native_io.__file__}")
    print(f"version   : {getattr(kaldi_native_io, '__version__', '<none>')}")
    ext = kaldi_native_io._kaldi_native_io.__file__
    print(f"extension : {ext}")

    # A namespace package has no __file__; that means nothing installed a real
    # `kaldi_native_io` package and `import kaldi_native_io` picked up a bare
    # directory (e.g. the one at the repository root).
    if getattr(kaldi_native_io, "__file__", None) is None:
        print(
            "\nERROR: kaldi_native_io resolved to a namespace package "
            f"({list(kaldi_native_io.__path__)}), not to an installed package.\n"
            "Run this from outside the checkout after installing the wheel.\n"
        )
        return 1

    # Guard against the shadowing described above: if the resolved package
    # lives inside this checkout, we are testing the source tree rather than
    # the installed wheel.
    pkg_dir = Path(kaldi_native_io.__file__).resolve().parent
    repo_root = Path(__file__).resolve().parent.parent
    if repo_root in pkg_dir.parents:
        print(
            "\nERROR: kaldi_native_io resolved to the source tree "
            f"({pkg_dir}), not to an installed package.\n"
            "Run this from outside the checkout.\n"
        )
        return 1

    if on_disk != expected:
        missing = sorted(set(expected) - set(on_disk))
        extra = sorted(set(on_disk) - set(expected))
        print("\nERROR: the test list is out of date.")
        if missing:
            print(f"  missing from disk: {missing}")
        if extra:
            print(f"  new on disk (add to EXPECTED_TESTS): {extra}")
        return 1

    print(f"\nrunning {len(expected)} test files...\n")

    # Copy the tests into a scratch directory and run there, so the tests'
    # ark/scp scratch files land somewhere writable and so no source-tree
    # package can shadow the installed one.
    failures = []
    with tempfile.TemporaryDirectory() as tmp:
        tmpdir = Path(tmp)
        for name in expected:
            src = tests_dir / name
            dst = tmpdir / name
            shutil.copy(src, dst)

            print(f"--- {name} ---", flush=True)
            ret = subprocess.run(
                [sys.executable, str(dst)],
                cwd=tmpdir,
            )
            if ret.returncode != 0:
                failures.append(name)
                print(f"*** FAILED ({ret.returncode}): {name}", flush=True)
            else:
                print(f"--- passed: {name} ---", flush=True)

    print("\n" + "=" * 60)
    if failures:
        print(f"{len(failures)}/{len(expected)} test files FAILED:")
        for name in failures:
            print(f"  {name}")
        return 1

    print(f"all {len(expected)} test files passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
