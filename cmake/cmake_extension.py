# Copyright (c)  2021-2023  Xiaomi Corporation (author: Fangjun Kuang)
# flake8: noqa

import os
import platform
import sys
from pathlib import Path

import setuptools
from setuptools.command.build_ext import build_ext


def is_for_pypi():
    ans = os.environ.get("KALDI_NATIVE_IO_IS_FOR_PYPI", None)
    return ans is not None


def is_macos():
    return platform.system() == "Darwin"


def is_windows():
    return platform.system() == "Windows"


try:
    from wheel.bdist_wheel import bdist_wheel as _bdist_wheel

    class bdist_wheel(_bdist_wheel):
        def finalize_options(self):
            _bdist_wheel.finalize_options(self)
            # In this case, the generated wheel has a name in the form
            # kaldi_native_io-xxx-pyxx-none-any.whl
            if is_for_pypi() and not is_macos():
                self.root_is_pure = True
            else:
                # The generated wheel has a name ending with
                # -linux_x86_64.whl
                self.root_is_pure = False

except ImportError:
    bdist_wheel = None


def cmake_extension(name, *args, **kwargs) -> setuptools.Extension:
    kwargs["language"] = "c++"
    sources = []
    return setuptools.Extension(name, sources, *args, **kwargs)


class BuildExtension(build_ext):
    def build_extension(self, ext: setuptools.extension.Extension):
        # build/temp.linux-x86_64-3.8
        os.makedirs(self.build_temp, exist_ok=True)

        # build/lib.linux-x86_64-3.8
        os.makedirs(self.build_lib, exist_ok=True)

        # CMake installs into lib/ and bin/ below this, so the extension ends
        # up at kaldi_native_io/lib/_kaldi_native_io.<ext> and copy-blob at
        # kaldi_native_io/bin/copy-blob in the wheel -- the same layout
        # kaldifst/kaldilm/kaldi-decoder use. The install rules decide
        # placement; nothing is copied by hand.
        install_dir = Path(self.build_lib).resolve() / "kaldi_native_io"

        kaldi_native_io_dir = Path(__file__).parent.parent.resolve()

        cmake_args = os.environ.get("KALDI_NATIVE_IO_CMAKE_ARGS", "")
        make_args = os.environ.get("KALDI_NATIVE_IO_MAKE_ARGS", "")
        system_make_args = os.environ.get("MAKEFLAGS", "")

        # KALDI_NATIVE_IO_CMAKE_ARGS replaces the default rather than extending
        # it, so only supply CMAKE_BUILD_TYPE when the caller has not. Otherwise
        # any workflow that passes e.g. -A Win32 would silently build without
        # optimisation.
        if "CMAKE_BUILD_TYPE" not in cmake_args:
            cmake_args += " -DCMAKE_BUILD_TYPE=Release "

        extra_cmake_args = " -DKALDI_NATIVE_IO_BUILD_TESTS=OFF "
        extra_cmake_args += " -DKALDI_NATIVE_IO_BUILD_PYTHON=ON "
        extra_cmake_args += f" -DCMAKE_INSTALL_PREFIX={install_dir} "

        if "PYTHON_EXECUTABLE" not in cmake_args:
            print(f"Setting PYTHON_EXECUTABLE to {sys.executable}")
            cmake_args += f" -DPYTHON_EXECUTABLE={sys.executable}"

        cmake_args += extra_cmake_args

        if not is_windows():
            if make_args == "" and system_make_args == "":
                print("for fast compilation, run:")
                print(
                    'export KALDI_NATIVE_IO_MAKE_ARGS="-j"; python setup.py install'
                )
                make_args = " -j1" if is_windows() else " -j4 "
                print(f"Setting make_args to '{make_args}'")

            build_cmd = f"""
                cd {self.build_temp}

                cmake {cmake_args} {kaldi_native_io_dir}

                make {make_args} install/strip
            """
            print(f"build command is:\n{build_cmd}")

            ret = os.system(build_cmd)
            if ret != 0:
                raise Exception(
                    "\nBuild kaldi_native_io failed. Please check the error message.\n"
                    "You can ask for help by creating an issue on GitHub.\n"
                    "\nClick:\n"
                    "   https://github.com/csukuangfj/kaldi_native_io/issues/new\n"
                )
            return

        # for windows
        build_cmd = f"""
            cmake {cmake_args} -B {self.build_temp} -S {kaldi_native_io_dir}
            cmake --build {self.build_temp} --target install --config Release -- -m
        """
        print(f"build command is:\n{build_cmd}")

        ret = os.system(
            f"cmake {cmake_args} -B {self.build_temp} -S {kaldi_native_io_dir}"
        )
        if ret != 0:
            raise Exception("Failed to configure kaldi_native_io")

        ret = os.system(
            f"cmake --build {self.build_temp} --target install --config Release -- -m"
        )
        if ret != 0:
            raise Exception("Failed to build and install kaldi_native_io")
