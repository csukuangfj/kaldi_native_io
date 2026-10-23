if (CMAKE_VERSION VERSION_GREATER_EQUAL "4.0.0")
  set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
endif()

if("x${CMAKE_SOURCE_DIR}" STREQUAL "x${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "\
In-source build is not a good practice.
Please use:
  mkdir build
  cd build
  cmake ..
to build this project"
  )
endif()

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(kaldi_native_io CXX)

# Disable warning about
#
# "The DOWNLOAD_EXTRACT_TIMESTAMP option was not given and policy CMP0135 is
#  not set.
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

set(KALDI_NATIVE_IO_VERSION "1.23.0")

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No CMAKE_BUILD_TYPE given, default to Release")
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# A toolchain file means we are cross-compiling (iOS, Android NDK, ...) and
# there is no host Python to build bindings against, so default both options
# off. Callers may still turn them on explicitly.
if(CMAKE_TOOLCHAIN_FILE)
  set(_BUILD_PYTHON_DEFAULT OFF)
  set(_BUILD_TESTS_DEFAULT OFF)
else()
  set(_BUILD_PYTHON_DEFAULT ON)
  set(_BUILD_TESTS_DEFAULT OFF)
endif()

option(KALDI_NATIVE_IO_BUILD_TESTS "Whether to build tests or not" ${_BUILD_TESTS_DEFAULT})
option(KALDI_NATIVE_IO_BUILD_PYTHON "Whether to build Python" ${_BUILD_PYTHON_DEFAULT})

message(STATUS "KALDI_NATIVE_IO_BUILD_TESTS: ${KALDI_NATIVE_IO_BUILD_TESTS}")
message(STATUS "KALDI_NATIVE_IO_BUILD_PYTHON: ${KALDI_NATIVE_IO_BUILD_PYTHON}")
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

if(DEFINED _GLIBCXX_USE_CXX11_ABI)
  set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=${_GLIBCXX_USE_CXX11_ABI}")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(NOT APPLE)
  set(KALDI_NATIVE_IO_RPATH_ORIGIN "$ORIGIN")
else()
  set(CMAKE_MACOSX_RPATH ON)
  set(KALDI_NATIVE_IO_RPATH_ORIGIN "@loader_path")
endif()

set(CMAKE_INSTALL_RPATH ${KALDI_NATIVE_IO_RPATH_ORIGIN})
set(CMAKE_BUILD_RPATH ${KALDI_NATIVE_IO_RPATH_ORIGIN})

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ version to be used.")
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

message(STATUS "C++ Standard version: ${CMAKE_CXX_STANDARD}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if(WIN32)
  add_definitions(-DNOMINMAX) # Otherwise, std::max() and std::min() won't work

  set(disabled_warnings
    /wd4018
    /wd4061
    /wd4127
    /wd4191
    /wd4242
    /wd4244
    /wd4267
    /wd4291
    /wd4305
    /wd4355
    /wd4365
    /wd4371
    /wd4456
    /wd4458
    /wd4464
    /wd4514
    /wd4623
    /wd4625
    /wd4626
    /wd4668
    /wd4686
    /wd4710
    /wd4711
    /wd4820
    /wd4868
    /wd4996
    /wd5027
    /wd5039
    /wd5045
    /wd5219
  )

  message(STATUS "Disabled warnings: ${disabled_warnings}")
  foreach(w IN LISTS disabled_warnings)
    string(APPEND CMAKE_CXX_FLAGS " ${w} ")
  endforeach()

  add_compile_options(/bigobj)
endif()

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

if(KALDI_NATIVE_IO_BUILD_PYTHON)
  include(pybind11)
endif()

if(KALDI_NATIVE_IO_BUILD_TESTS)
  enable_testing()
  include(googletest)
endif()

# PYTHON_EXECUTABLE is used to run the Python tests and to query the Python
# site-packages directory (for the macOS rpath). pybind11 resolves the
# interpreter into Python_EXECUTABLE, but only within its own add_subdirectory()
# scope, so it is not visible here; fall back to FindPython3 when the Python
# bindings are off.
if(NOT PYTHON_EXECUTABLE)
  if(Python_EXECUTABLE)
    set(PYTHON_EXECUTABLE "${Python_EXECUTABLE}")
  elseif(Python3_EXECUTABLE)
    set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
  else()
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
  endif()
endif()
message(STATUS "PYTHON_EXECUTABLE: ${PYTHON_EXECUTABLE}")

add_subdirectory(kaldi_native_io)
