#!/usr/bin/env python3
#
# Copyright (c)  2022  Xiaomi Corporation (author: Fangjun Kuang)

import glob
import os
import re
import shutil
import sys

import setuptools
from setuptools.command.build_ext import build_ext

cur_dir = os.path.dirname(os.path.abspath(__file__))


def cmake_extension(name, *args, **kwargs) -> setuptools.Extension:
    kwargs["language"] = "c++"
    sources = []
    return setuptools.Extension(name, sources, *args, **kwargs)


class BuildExtension(build_ext):
    def build_extension(self, ext: setuptools.extension.Extension):
        build_dir = self.build_temp
        os.makedirs(build_dir, exist_ok=True)

        # build/lib.linux-x86_64-3.8
        os.makedirs(self.build_lib, exist_ok=True)

        kaldi_native_io_dir = os.path.dirname(os.path.abspath(__file__))

        cmake_args = os.environ.get("KALDI_NATIVE_IO_CMAKE_ARGS", "")
        make_args = os.environ.get("KALDI_NATIVE_IO_MAKE_ARGS", "")
        system_make_args = os.environ.get("MAKEFLAGS", "")

        if cmake_args == "":
            cmake_args = "-DCMAKE_BUILD_TYPE=Release"

        if make_args == "" and system_make_args == "":
            print("For fast compilation, run:")
            print(
                'export KALDI_NATIVE_IO_MAKE_ARGS="-j"; '
                "python setup.py install"
            )

        if "PYTHON_EXECUTABLE" not in cmake_args:
            print(f"Setting PYTHON_EXECUTABLE to {sys.executable}")
            cmake_args += f" -DPYTHON_EXECUTABLE={sys.executable}"

        build_cmd = f"""
            cd {self.build_temp}

            cmake {cmake_args} {kaldi_native_io_dir}

            make {make_args} _kaldi_native_io
        """
        print(f"build command is:\n{build_cmd}")

        ret = os.system(build_cmd)
        if ret != 0:
            raise Exception(
                "\nBuild kaldi_native_io failed. Please check the error "
                "message.\n"
                "You can ask for help by creating an issue on GitHub.\n"
                "\nClick:\n"
                "   https://github.com/csukuangfj/kaldi_native_io/issues/new\n"
            )
        lib_so = glob.glob(f"{build_dir}/lib/*.so*")
        for so in lib_so:
            print(f"Copying {so} to {self.build_lib}/")
            shutil.copy(f"{so}", f"{self.build_lib}/")

        # macos
        lib_so = glob.glob(f"{build_dir}/lib/*.dylib*")
        for so in lib_so:
            print(f"Copying {so} to {self.build_lib}/")
            shutil.copy(f"{so}", f"{self.build_lib}/")


def read_long_description():
    with open("README.md", encoding="utf8") as f:
        readme = f.read()
    return readme


def get_package_version():
    with open("CMakeLists.txt") as f:
        content = f.read()

    latest_version = re.search(
        r"set\(KALDI_NATIVE_IO_VERSION (.*)\)", content
    ).group(1)
    latest_version = latest_version.strip('"')
    return latest_version


with open("kaldi_native_io/python/kaldi_native_io/__init__.py", "a") as f:
    f.write(f"__version__ = '{get_package_version()}'\n")


package_name = "kaldi_native_io"

setuptools.setup(
    name=package_name,
    version=get_package_version(),
    author="Fangjun Kuang",
    author_email="csukuangfj@gmail.com",
    package_dir={
        package_name: "kaldi_native_io/python/kaldi_native_io",
    },
    packages=[package_name],
    install_requires=["numpy"],
    url="https://github.com/csukuangfj/kaldi_native_io",
    long_description=read_long_description(),
    long_description_content_type="text/markdown",
    ext_modules=[cmake_extension("_kaldi_native_io")],
    cmdclass={"build_ext": BuildExtension},
    zip_safe=False,
    classifiers=[
        "Programming Language :: C++",
        "Programming Language :: Python",
        "Topic :: Scientific/Engineering :: Artificial Intelligence",
    ],
    license="Apache licensed, as found in the LICENSE file",
)

# remove the line __version__ from
# kaldi_native_io/python/kaldi_native_io/__init__.py
with open("kaldi_native_io/python/kaldi_native_io/__init__.py", "r") as f:
    lines = f.readlines()

with open("kaldi_native_io/python/kaldi_native_io/__init__.py", "w") as f:
    for line in lines:
        if "__version__" not in line:
            f.write(line)
