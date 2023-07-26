

# Introduction

Python wrapper for Kaldi's native I/O. The internal implementation
uses C++ code from [Kaldi](https://github.com/kaldi-asr/kaldi).
A Python wrapper with [pybind11](https://github.com/pybind/pybind11)
is provided to read ark/scp files from Kaldi in Python.

**Note**: This project is self-contained and does not depend on `Kaldi`.

# Installation

```bash
pip install --verbose kaldi_native_io
```

or

```bash
git clone https://github.com/csukuangfj/kaldi_native_io
cd kaldi_native_io
python3 setup.py install
```

or

```bash
conda install -c kaldi_native_io kaldi_native_io
```


# Features

- Native support for **ALL** types of `rspecifier` and `wspecifier`
  since the C++ code is borrowed from Kaldi.

- Support the following data types (More will be added later on request.)

  * Note: We also support Python `bytes` class

| C++ Data Type | Writer | Sequential Reader | Random Access Reader |
|------|--------|-------------------|----------------------|
| Python's `bytes`  | `BlobWriter` | `SequentialBlobReader`  | `RandomAccessBlobReader` |
|`int32`  | `Int32Writer` | `SequentialInt32Reader`  | `RandomAccessInt32Reader` |
|`std::vector<int32>` | `Int32VectorWriter`| `SequentialInt32VectorReader`| `RandomAccessInt32VectorReader`|
|`std::vector<int8>` | `Int8VectorWriter`| `SequentialInt8VectorReader`| `RandomAccessInt8VectorReader`|
|`std::vector<std::vector<int32>>`|`Int32VectorVectorWriter`|`SequentialInt32VectorVectorReader`|`RandomAccessInt32VectorVectorReader`|
| `std::vector<std::pair<int32, int32>>` | `Int32PairVectorWriter`   | `SequentialInt32PairVectorReader`   | `RandomAccessInt32PairVectorReader`   |
|`float`| `FloatWriter`| `SequentialFloatReader`| `RandomAccessFloatReader`|
|`std::vector<std::pair<float, float>>`|`FloatPairVectorWriter`|`SequentialFloatPairVectorReader`|`RandomAccessFloatPairVectorReader`|
|`double`|`DoubleWriter`|`SequentialDoubleReader`|`RandomAccessDoubleReader`|
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
|`kaldi::WaveInfo`|-|`SequentialWaveInfoReader`|`RandomAccessWaveInfoReader`|
|`kaldi::WaveData`|-|`SequentialWaveReader`|`RandomAccessWaveReader`|
|`MatrixShape`|-|`SequentialMatrixShapeReader`|`RandomAccessMatrixShapeReader`|

**Note**:

- `MatrixShape` does not exist in Kaldi. Its purpose is to get the shape information
  of a matrix without reading all the data.


# Usage

## Table readers and writers

### Write

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

### Read

#### Sequential Read

Create a sequential reader instance with an `rspecifier` and use `for key, value in reader`
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


#### Random Access Read

Create a random access reader instance with an `rspecifier` and use `reader[key]`
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

## Read and write a single matrix

See

- <https://github.com/csukuangfj/kaldi_native_io/blob/master/kaldi_native_io/python/tests/test_float_matrix_writer_reader.py>
- <https://github.com/csukuangfj/kaldi_native_io/blob/master/kaldi_native_io/python/tests/test_double_matrix_writer_reader.py>

```python3
def test_read_write_single_mat():
    arr = np.array(
        [
            [0, 1, 2, 22, 33],
            [3, 4, 5, -1, -3],
            [6, 7, 8, -9, 0],
            [9, 10, 11, 5, 100],
        ],
        dtype=np.float32,
    )
    mat = kaldi_native_io.FloatMatrix(arr)
    mat.write(wxfilename="binary.ark", binary=True)
    mat.write(wxfilename="matrix.txt", binary=False)

    m1 = kaldi_native_io.FloatMatrix.read("binary.ark")
    m2 = kaldi_native_io.FloatMatrix.read("matrix.txt")

    assert np.array_equal(mat, m1)
    assert np.array_equal(mat, m2)

    # read range
    # Note: the upper bound is inclusive!
    m3 = kaldi_native_io.FloatMatrix.read("binary.ark[0:1]")  # row 0 and row 1
    assert np.array_equal(mat.numpy()[0:2], m3.numpy())

    m4 = kaldi_native_io.FloatMatrix.read(
        "matrix.txt[:,3:4]"
    )  # column 3 and column 4
    assert np.array_equal(mat.numpy()[:, 3:5], m4.numpy())

    os.remove("binary.ark")
    os.remove("matrix.txt")

    a = np.array([[1, 2], [3, 4]], dtype=np.float32)
    b = np.array([[10, 20, 30], [40, 50, 60]], dtype=np.float32)
    with kaldi_native_io.FloatMatrixWriter("ark,scp:m.ark,m.scp") as ko:
        ko.write("a", a)
        ko["b"] = b

    """
    m.scp contains:
      a m.ark:2
      b m.ark:35
    """

    m5 = kaldi_native_io.FloatMatrix.read("m.ark:2")
    assert np.array_equal(m5.numpy(), a)

    m6 = kaldi_native_io.FloatMatrix.read("m.ark:35")
    assert np.array_equal(m6.numpy(), b)

    os.remove("m.scp")
    os.remove("m.ark")
```

## Read and write a single vector

See

- <https://github.com/csukuangfj/kaldi_native_io/blob/master/kaldi_native_io/python/tests/test_float_vector_writer_reader.py>
- <https://github.com/csukuangfj/kaldi_native_io/blob/master/kaldi_native_io/python/tests/test_double_vector_writer_reader.py>

```python3
def test_read_write_single_vector():
    a = np.array([1, 2], dtype=np.float32)
    v = kaldi_native_io.FloatVector(a)
    v.write(wxfilename="binary.ark", binary=True)

    b = kaldi_native_io.FloatVector.read("binary.ark")
    assert np.array_equal(a, b.numpy())

    a = np.array([1, 2], dtype=np.float32)
    b = np.array([10.5], dtype=np.float32)
    with kaldi_native_io.FloatVectorWriter("ark,scp:v.ark,v.scp") as ko:
        ko.write("a", a)
        ko["b"] = b

    """
    v.scp contains:
      a v.ark:2
      b v.ark:22
    """
    va = kaldi_native_io.FloatVector.read("v.ark:2")
    assert np.array_equal(va.numpy(), a)

    vb = kaldi_native_io.FloatVector.read("v.ark:22")
    assert np.array_equal(vb.numpy(), b)

    os.remove("v.scp")
    os.remove("v.ark")
```

```python3
def test_read_write_single_vector():
    a = np.array([1, 2], dtype=np.float64)
    v = kaldi_native_io.DoubleVector(a)
    v.write(wxfilename="binary.ark", binary=True)

    b = kaldi_native_io.DoubleVector.read("binary.ark")
    assert np.array_equal(a, b.numpy())

    os.remove("binary.ark")

    a = np.array([1, 2], dtype=np.float64)
    b = np.array([10.5], dtype=np.float64)
    with kaldi_native_io.DoubleVectorWriter("ark,scp:v.ark,v.scp") as ko:
        ko.write("a", a)
        ko["b"] = b

    """
    v.scp contains:
      a v.ark:2
      b v.ark:30
    """
    va = kaldi_native_io.DoubleVector.read("v.ark:2")
    assert np.array_equal(va.numpy(), a)

    vb = kaldi_native_io.DoubleVector.read("v.ark:30")
    assert np.array_equal(vb.numpy(), b)

    os.remove("v.scp")
    os.remove("v.ark")
```

## Read a single int32 vector

See <https://github.com/csukuangfj/kaldi_native_io/blob/master/kaldi_native_io/python/tests/test_int32_vector_writer_reader.py>

```python3
def test_read_single_item():
    a = [10, 20]
    b = [100, 200, 300]

    # You can also generate a text format by adding ",t" if you like
    #  with kaldi_native_io.Int32VectorWriter("ark,scp,t:v.ark,v.scp") as ko:
    with kaldi_native_io.Int32VectorWriter("ark,scp:v.ark,v.scp") as ko:
        ko.write("a", a)
        ko["b"] = b
    """
    v.scp contains:
      a v.ark:2
      b v.ark:21
    """

    va = kaldi_native_io.read_int32_vector("v.ark:2")
    assert va == a

    vb = kaldi_native_io.read_int32_vector("v.ark:21")
    assert va == b
```

## Read/Write Waves


See
- <https://github.com/csukuangfj/kaldi_native_io/blob/master/kaldi_native_io/python/tests/test_wave_reader.py>
- <https://github.com/csukuangfj/kaldi_native_io/blob/master/kaldi_native_io/python/tests/test_wave_data.py>

```python3
def test_wave_writer():
    file1 = "/ceph-fj/fangjun/open-source-2/kaldi_native_io/build/BAC009S0002W0123.wav"
    if not Path(file1).is_file():
        return

    file2 = "/ceph-fj/fangjun/open-source-2/kaldi_native_io/build/BAC009S0002W0124.wav"
    if not Path(file2).is_file():
        return

    print("-----test_wave_writer------")

    file2 = f"cat {file2} |"

    wave1 = kaldi_native_io.read_wave(file1)
    wave2 = kaldi_native_io.read_wave(file2)

    wspecifier = "ark,scp:wave.ark,wave.scp"
    with kaldi_native_io.WaveWriter(wspecifier) as ko:
        ko.write("a", wave1)
        ko["b"] = wave2
    """
    wave.scp has the following content:
      a wave.ark:2
      b wave.ark:123728
    """
    wave3 = kaldi_native_io.read_wave("wave.ark:2")
    wave4 = kaldi_native_io.read_wave("wave.ark:123728")

    assert wave1.sample_freq == wave3.sample_freq
    assert wave2.sample_freq == wave4.sample_freq

    assert np.array_equal(wave1.data.numpy(), wave3.data.numpy())
    assert np.array_equal(wave2.data.numpy(), wave4.data.numpy())
```

## Read/Write Python's bytes


See
- <https://github.com/csukuangfj/kaldi_native_io/blob/master/kaldi_native_io/python/tests/test_blob_writer_reader.py>

```python3
base = "blob"
wspecifier = f"ark,scp:{base}.ark,{base}.scp"
rspecifier = f"scp:{base}.scp"


def test_blob_writer():
    with kaldi_native_io.BlobWriter(wspecifier) as ko:
        ko.write("a", bytes([0x30, 0x31]))
        ko["b"] = b"1234"


def test_sequential_blob_reader():
    with kaldi_native_io.SequentialBlobReader(rspecifier) as ki:
        for key, value in ki:
            if key == "a":
                assert value == bytes([0x30, 0x31])
            elif key == "b":
                assert value == b"1234"
            else:
                raise ValueError(f"Unknown key {key} with value {value}")

def test_read_single_item():
    a = bytes([10, 20])
    b = b"1234"

    with kaldi_native_io.BlobWriter("ark,scp:b.ark,b.scp") as ko:
        ko.write("a", a)
        ko["b"] = b
    """
    b.scp contains:
      a b.ark:2
      b b.ark:20
    """

    va = kaldi_native_io.read_blob("b.ark:2")
    assert va == a, (va, a)

    vb = kaldi_native_io.read_blob("b.ark:20")
    assert vb == b, (vb, b)

    # test range read
    # [start:end], both ends are inclusive
    # Must satisfy 0 <= start <= end < length of the data

    # start 0, end 2
    vc = kaldi_native_io.read_blob("b.ark:20[0:2]")
    assert vc == b"123", (vc, b"123")

    # start 1, end 2
    vd = kaldi_native_io.read_blob("b.ark:20[1:2]")
    assert vd == b"23", (vd, b"23")

    # start 2, end 2
    ve = kaldi_native_io.read_blob("b.ark:20[2:2]")
    assert ve == b"3", (ve, b"3")

    # start 2, end -1
    # -1 means the end of the data
    vf = kaldi_native_io.read_blob("b.ark:20[2:-1]")
    assert vf == b"34", (vf, b"34")

    # [:] means all the data
    vg = kaldi_native_io.read_blob("b.ark:20[:]")
    assert vg == b"1234", (vg, b"1234")

    os.remove("b.scp")
    os.remove("b.ark")
```
