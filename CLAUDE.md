# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

kaldi_native_io: a standalone C++ library for Kaldi-style IO — Matrix/Vector
(including CompressedMatrix), table readers/writers (ark/scp), wave reader,
ParseOptions — copied/modified from Kaldi. Downstream users include
sherpa-onnx. The core is C++; the Python bindings wrap it.

## Build

CMake project (>= 3.13). Root `CMakeLists.txt` options:
`KALDI_NATIVE_IO_BUILD_TESTS` (GoogleTest), `KALDI_NATIVE_IO_BUILD_PYTHON`
(pybind11). Dependencies are fetched via the scripts in `cmake/`.

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

- The library target is `kaldi_native_io_core` (STATIC + PIC), defined in
  `kaldi_native_io/csrc/CMakeLists.txt` from its `srcs` list.
- Python: `python setup.py install`; wheels are built by the
  `build-wheels-*.yaml` workflows (cibuildwheel).
- Tests: `KALDI_NATIVE_IO_BUILD_TESTS=ON` registers each file in `test_srcs`
  (currently empty); Python checks are `scripts/run_python_tests.py` and
  `scripts/smoke_test.py`.

## Lint

- cpplint 1.5.4 via `./scripts/check_style_cpplint.sh` (no arg = last commit,
  `1` = uncommitted changes). Config in `CPPLINT.cfg`.
- clang-tidy via the root `.clang-tidy`; run `make check` (target
  `clang-tidy-check` in `kaldi_native_io/csrc/CMakeLists.txt`); CI is
  `.github/workflows/clang-tidy.yaml`. On macOS, run clang-tidy with
  `--extra-arg=-isysroot$(xcrun --show-sdk-path) --extra-arg=-stdlib=libc++`
  on top of the compile database, otherwise the C++ headers are not found.
- Suppressions must be bare `// NOLINT` (no check name): cpplint 1.5.4 rejects
  clang-tidy category names in `NOLINT(...)`.
- `.clang-tidy` is sherpa-onnx's config minus three checks (documented in the
  file): the unscoped enums are public API/ABI and the goto-based cleanup in
  the matrix/vector Read/Write paths mirrors upstream Kaldi.

## Conventions

- The code is copied/modified from Kaldi (see each file's header comment);
  prefer upstream-style minimal diffs over rewrites.
- Upstream idioms that are deliberate, not oversights: `goto`-based cleanup in
  Read/Write, KALDIIO_ERR throwing from the `Logger` destructor
  (`noexcept(false)`), KALDIIO_MEMALIGN buffer management, `popen`-based pipe
  rspecifiers/wspecifiers.
- Style: initializer lists use `= 0` / `= false` form, never `T x{0};`.
