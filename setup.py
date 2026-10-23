#!/usr/bin/env python3
#
# Copyright (c)  2022  Xiaomi Corporation (author: Fangjun Kuang)

import os
import re
import sys
from pathlib import Path

import setuptools

from cmake.cmake_extension import (
    BuildExtension,
    bdist_wheel,
    cmake_extension,
    is_windows,
)


def read_long_description():
    with open("README.md", encoding="utf8") as f:
        readme = f.read()
    return readme


def get_package_version():
    with open("CMakeLists.txt") as f:
        content = f.read()

    match = re.search(r"set\(KALDI_NATIVE_IO_VERSION (.*)\)", content)
    latest_version = match.group(1).strip('"')
    return latest_version


package_name = "kaldi_native_io"

with open("kaldi_native_io/python/kaldi_native_io/__init__.py", "a") as f:
    f.write(f"__version__ = '{get_package_version()}'\n")

setuptools.setup(
    name=package_name,
    python_requires=">=3.8",
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
    cmdclass={"build_ext": BuildExtension, "bdist_wheel": bdist_wheel},
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
        if "__version__" in line:
            # skip __version__ = "x.x.x"
            continue
        f.write(line)
