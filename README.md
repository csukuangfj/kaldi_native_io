

# Introduction

Python wrapper for Kaldi's native I/O. The internal implementation
uses C++ code from [Kaldi](https://github.com/kaldi-asr/kaldi).
A Python wrapper with [pybind11](https://github.com/pybind/pybind11)
is provided to read ark/scp files from Kaldi in Python.

**Note**: This project is self-contained and does not depend on `Kaldi`.


# Features

- Native support for **ALL** types of `rspecifier` and `wspecifier`
  since the C++ code is borrowed from Kaldi.

- Support the following data types (More will be added later on request.)

| C++ Data Type | Writer | Sequential Reader | Random Access Reader |
|------|--------|-------------------|----------------------|
|`int32`  | `Int32Writer` | `SequentialInt32Reader`  | `RandomAccessInt32Reader` |
|`std::vector<int32>` | `Int32VectorWriter`| `SequentialInt32VectorReader`| `RandomAccessInt32VectorReader`|
|`std::vector<std::vector<int32>>`|`Int32VectorVectorWriter`|`SequentialInt32VectorVectorReader`|`RandomAccessInt32VectorVectorReader`|
| `std::vector<std::pair<int32, int32>>` | `Int32PairVectorWriter`   | `SequentialInt32PairVectorReader`   | `RandomAccessInt32PairVectorReader`   |
|`float`| `FloatWriter`| `SequentialFloatReader`| `RandomAccessFloatReader`|
|`std::vector<std::pair<float, float>>`|`FloatPairVectorWriter`|`SequentialFloatPairVectorReader`|`RandomAccessFloatPairVectorReader`|
|`double`|`DoubleWriter`|`SequentialDoubleReader`|`RandomAccessDoubleReader`|
|`bool`|`BoolWriter`|`SequentialBoolReader`|`RandomAccessBoolReader`|
|`bool`|`BoolWriter`|`SequentialBoolReader`|`RandomAccessBoolReader`|
|`std::string`|`TokenWriter`|`SequentialTokenReader`|`RandomAccessTokenReader`|
|`std::vector<std::string>`|`TokenVectorWriter`|`SequentialTokenVectorReader`|`RandomAccessTokenVectorReader`|
|`kaldi::Vector<float>`| `FloatVectorWriter`| `SequentialFloatVectorReader`| `RandomAccessFloatVectorReader`|
|`kaldi::Vector<double>`| `DoubleVectorWriter`| `SequentialDoubleVectorReader`| `RandomAccessDoubleVectorReader`|
|`kaldi::Matrix<float>`| `FloatMatrixWriter`| `SequentialFloatMatrixReader`| `RandomAccessFloatMatrixReader`|
|`kaldi::Matrix<double>`| `DoubleMatrixWriter`| `SequentialDoubleMatrixReader`| `RandomAccessDoubleMatrixReader`|
|`std::pair<kaldi::Matrix<float>, HtkHeader>`| `HtkMatrixWriter`| `SequentialHtkMatrixReader`| `RandomAccessHtkMatrixReader`|
|`kaldi::CompressedMatrix`| `CompressedMatrixWriter`| `SequentialCompressedMatrixReader`| `RandomAccessCompressedMatrixReader`|
|`kaldi::Posterior`|`PosteriorWriter`|`SequentialPosteriorReader`|`RandomAccessPosteriorReader`|
|`kaldi::GausPost`|`GaussPostWriter`|`SequentialGaussPostReader`|`RandomAccessGaussPostReader`|

# Installation

```
pip install --verbose kaldi_native_io
```

# Usage

## Write

Create a `writer` instance with a `wspecifier` and use `writer[key] = value`.

For instance, the following code uses `kaldi_native_io.FloatMatrixWriter` to
write `kaldi::Matrix<float>` to a `wspecifier`.

```python
import numpy as np
import kaldi_native_io

base = "float_matrix"
wspecifier = f"ark,scp,t:{base}.ark,{base}.scp"

def test_float_matrix_writer():
    with kaldi_native_io.FloatMatrixWriter(wspecifier) as ko:
        ko.write("a", np.array([[1, 2], [3, 4]], dtype=np.float32))
        ko["b"] = np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float32)
```

## Read

### Sequential Read
Create a sequential reader class with an `rspecifier` and use `for key, value in reader`
to read the file.

For instance, the following code uses `kaldi_native_io.SequentialFloatMatrixReader` to
read `kaldi::Matrix<float>` from an `rspecifier`.

```python
import numpy as np
import kaldi_native_io

base = "float_matrix"
rspecifier = f"scp:{base}.scp"

def test_sequential_float_matrix_reader():
    with kaldi_native_io.SequentialFloatMatrixReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert np.array_equal(
                    value, np.array([[1, 2], [3, 4]], dtype=np.float32)
                )
            elif key == "b":
                assert np.array_equal(
                    value,
                    np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float32),
                )
            else:
                raise ValueError(f"Unknown key {key} with value {value}")
```


### Random Access Read

Create a random access reader class with an `rspecifier` and use `reader[key]`
to read the file.

For instance, the following code uses `kaldi_native_io.RandomAccessFloatMatrixReader` to
read `kaldi::Matrix<float>` from an `rspecifier`.

```python
import numpy as np
import kaldi_native_io

base = "float_matrix"
rspecifier = f"scp:{base}.scp"

def test_random_access_float_matrix_reader():
    with kaldi_native_io.RandomAccessFloatMatrixReader(rspecifier) as ki:
        assert "b" in ki
        assert "a" in ki
        assert np.array_equal(
            ki["a"], np.array([[1, 2], [3, 4]], dtype=np.float32)
        )
        assert np.array_equal(
            ki["b"], np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float32)
        )
```

There are unit tests for all supported types. Please visit
<https://github.com/csukuangfj/kaldi_native_io/tree/master/kaldi_native_io/python/tests>
for more examples.
