# Copyright      2021  Xiaomi Corp.        (authors: Fangjun Kuang)
# See ../../../LICENSE for clarification regarding multiple authors


from typing import Any, List, Tuple, Union

import numpy as np
from _kaldi_native_io import (
    CompressionMethod,
    HtkHeader,
    _BlobWriter,
    _BoolWriter,
    _CompressedMatrix,
    _CompressedMatrixWriter,
    _DoubleMatrix,
    _DoubleMatrixWriter,
    _DoubleVector,
    _DoubleVectorWriter,
    _DoubleWriter,
    _FloatMatrix,
    _FloatMatrixWriter,
    _FloatPairVectorWriter,
    _FloatVector,
    _FloatVectorWriter,
    _FloatWriter,
    _GaussPostWriter,
    _HtkMatrixWriter,
    _Int8VectorWriter,
    _Int32PairVectorWriter,
    _Int32VectorVectorWriter,
    _Int32VectorWriter,
    _Int32Writer,
    _PosteriorWriter,
    _RandomAccessBlobReader,
    _RandomAccessBoolReader,
    _RandomAccessDoubleMatrixReader,
    _RandomAccessDoubleReader,
    _RandomAccessDoubleVectorReader,
    _RandomAccessFloatMatrixReader,
    _RandomAccessFloatPairVectorReader,
    _RandomAccessFloatReader,
    _RandomAccessFloatVectorReader,
    _RandomAccessGaussPostReader,
    _RandomAccessHtkMatrixReader,
    _RandomAccessInt8VectorReader,
    _RandomAccessInt32PairVectorReader,
    _RandomAccessInt32Reader,
    _RandomAccessInt32VectorReader,
    _RandomAccessInt32VectorVectorReader,
    _RandomAccessMatrixShapeReader,
    _RandomAccessPosteriorReader,
    _RandomAccessTokenReader,
    _RandomAccessTokenVectorReader,
    _RandomAccessWaveInfoReader,
    _RandomAccessWaveReader,
    _SequentialBlobReader,
    _SequentialBoolReader,
    _SequentialDoubleMatrixReader,
    _SequentialDoubleReader,
    _SequentialDoubleVectorReader,
    _SequentialFloatMatrixReader,
    _SequentialFloatPairVectorReader,
    _SequentialFloatReader,
    _SequentialFloatVectorReader,
    _SequentialGaussPostReader,
    _SequentialHtkMatrixReader,
    _SequentialInt8VectorReader,
    _SequentialInt32PairVectorReader,
    _SequentialInt32Reader,
    _SequentialInt32VectorReader,
    _SequentialInt32VectorVectorReader,
    _SequentialMatrixShapeReader,
    _SequentialPosteriorReader,
    _SequentialTokenReader,
    _SequentialTokenVectorReader,
    _SequentialWaveInfoReader,
    _SequentialWaveReader,
    _TokenVectorWriter,
    _TokenWriter,
    _WaveWriter,
    read_blob,
)


class _TableWriter(object):
    def __init__(self, wspecifier: str) -> None:
        """
        Args:
          wspecifier:
            Kaldi table wspecifier.
        """
        # The subclass should instantiate `_impl`
        self._impl = None
        self.open(wspecifier)

    def open(self, wspecifier: str) -> None:
        """The subclass should implement this function
        and instantiate `self._impl`

        Args:
          wspecifier:
            Kaldi table wspecifier.
        """
        raise NotImplementedError

    @property
    def is_open(self) -> bool:
        """Return ``True`` if it is opened; return ``False`` otherwise."""
        return self._impl.is_open

    def write(self, key: str, value: Any) -> None:
        """Write an item.

        Args:
          key:
            The key for the item.
          value:
            The value for the item. Its type depends on the actual writer class.
        """
        self._impl.write(key, value)

    def flush(self) -> None:
        self._impl.flush()

    def close(self) -> None:
        if self.is_open:
            self._impl.close()

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback) -> None:
        self.close()

    def __setitem__(self, key: str, value: Any) -> None:
        self.write(key, value)

    def __del__(self) -> None:
        self.close()


class _SequentialTableReader(object):
    def __init__(self, rspecifier: str) -> None:
        """
        Args:
          rspecifier:
            Kaldi table rspecifier.
        """
        # The subclass should instantiate `_impl`
        self._impl = None
        self.open(rspecifier)

    def open(self, rspecifier: str) -> None:
        """The subclass should implement this function
        and instantiate `self._impl`

        Args:
          rspecifier:
            Kaldi table rspecifier.
        """
        raise NotImplementedError

    @property
    def is_open(self) -> bool:
        """Return ``True`` if it is opened; return ``False`` otherwise."""
        return self._impl.is_open

    @property
    def done(self) -> bool:
        return self._impl.done

    @property
    def key(self) -> str:
        return self._impl.key

    @property
    def value(self) -> Any:
        return self._impl.value

    def free_current(self) -> None:
        self._impl.free_current()

    def next(self) -> None:
        self._impl.next()

    def close(self) -> None:
        if self.is_open:
            self._impl.close()

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback) -> None:
        if self.is_open:
            self.close()

    def __iter__(self) -> Tuple[str, Any]:
        while not self.done:
            key = self.key
            value = self.value
            yield key, value
            self.next()

    def __del__(self) -> None:
        self.close()


class _RandomAccessTableReader(object):
    def __init__(self, rspecifier: str) -> None:
        """
        Args:
          rspecifier:
            Kaldi table rspecifier.
        """
        # The subclass should instantiate `_impl`
        self._impl = None
        self.open(rspecifier)

    def open(self, rspecifier: str) -> None:
        """The subclass should implement this function
        and instantiate `self._impl`

        Args:
          rspecifier:
            Kaldi table rspecifier.
        """
        raise NotImplementedError

    @property
    def is_open(self) -> bool:
        """Return ``True`` if it is opened; return ``False`` otherwise."""
        return self._impl.is_open

    def close(self) -> None:
        if self.is_open:
            self._impl.close()

    def __contains__(self, key: str) -> bool:
        return key in self._impl

    def __getitem__(self, key) -> Any:
        """The actual return type depends on the type of `self._impl`."""
        return self._impl[key]

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback) -> None:
        if self.is_open:
            self.close()

    def __del__(self) -> None:
        self.close()


class Int32Writer(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _Int32Writer(wspecifier)


class SequentialInt32Reader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialInt32Reader(rspecifier)


class RandomAccessInt32Reader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessInt32Reader(rspecifier)


class Int32VectorWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _Int32VectorWriter(wspecifier)


class SequentialInt32VectorReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialInt32VectorReader(rspecifier)


class RandomAccessInt32VectorReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessInt32VectorReader(rspecifier)


class BlobWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _BlobWriter(wspecifier)


class SequentialBlobReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialBlobReader(rspecifier)


class RandomAccessBlobReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessBlobReader(rspecifier)


class Int8VectorWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _Int8VectorWriter(wspecifier)


class SequentialInt8VectorReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialInt8VectorReader(rspecifier)


class RandomAccessInt8VectorReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessInt8VectorReader(rspecifier)


class Int32VectorVectorWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _Int32VectorVectorWriter(wspecifier)


class SequentialInt32VectorVectorReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialInt32VectorVectorReader(rspecifier)


class RandomAccessInt32VectorVectorReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessInt32VectorVectorReader(rspecifier)


class Int32PairVectorWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _Int32PairVectorWriter(wspecifier)


class SequentialInt32PairVectorReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialInt32PairVectorReader(rspecifier)


class RandomAccessInt32PairVectorReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessInt32PairVectorReader(rspecifier)


class FloatWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _FloatWriter(wspecifier)


class SequentialFloatReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialFloatReader(rspecifier)


class RandomAccessFloatReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessFloatReader(rspecifier)


class FloatPairVectorWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _FloatPairVectorWriter(wspecifier)


class SequentialFloatPairVectorReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialFloatPairVectorReader(rspecifier)


class RandomAccessFloatPairVectorReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessFloatPairVectorReader(rspecifier)


class TokenWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _TokenWriter(wspecifier)


class SequentialTokenReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialTokenReader(rspecifier)


class RandomAccessTokenReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessTokenReader(rspecifier)


class TokenVectorWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _TokenVectorWriter(wspecifier)


class SequentialTokenVectorReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialTokenVectorReader(rspecifier)


class RandomAccessTokenVectorReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessTokenVectorReader(rspecifier)


class BoolWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _BoolWriter(wspecifier)


class SequentialBoolReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialBoolReader(rspecifier)


class RandomAccessBoolReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessBoolReader(rspecifier)


class DoubleWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _DoubleWriter(wspecifier)


class SequentialDoubleReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialDoubleReader(rspecifier)


class RandomAccessDoubleReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessDoubleReader(rspecifier)


class FloatVectorWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _FloatVectorWriter(wspecifier)

    def write(self, key: str, value: np.ndarray) -> None:
        """
        Args:
          key:
            Key of the value.
          value:
            A 1-D array with dtype torch.float32.
        """
        assert value.dtype == np.float32
        assert value.ndim == 1
        super().write(key, _FloatVector(value))


class SequentialFloatVectorReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialFloatVectorReader(rspecifier)

    @property
    def value(self) -> np.ndarray:
        """Return a 1-D array with dtype np.float32."""
        return self._impl.value.numpy()


class RandomAccessFloatVectorReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessFloatVectorReader(rspecifier)

    def __getitem__(self, key) -> np.ndarray:
        """Return a 1-D array of type np.float32."""
        return self._impl[key].numpy()


class DoubleVectorWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _DoubleVectorWriter(wspecifier)

    def write(self, key: str, value: np.ndarray) -> None:
        """
        Args:
          key:
            Key of the value.
          value:
            A 1-D array with dtype torch.float64.
        """
        assert value.dtype == np.float64
        assert value.ndim == 1
        super().write(key, _DoubleVector(value))


class SequentialDoubleVectorReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialDoubleVectorReader(rspecifier)

    @property
    def value(self) -> np.ndarray:
        """Return a 1-D array with dtype np.float64."""
        return self._impl.value.numpy()


class RandomAccessDoubleVectorReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessDoubleVectorReader(rspecifier)

    def __getitem__(self, key) -> np.ndarray:
        """Return a 1-D array of type np.float64."""
        return self._impl[key].numpy()


class FloatMatrixWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _FloatMatrixWriter(wspecifier)

    def write(self, key: str, value: np.ndarray) -> None:
        """
        Args:
          key:
            Key of the value.
          value:
            A 2-D array with dtype torch.float32.
        """
        assert value.dtype == np.float32
        assert value.ndim == 2
        super().write(key, _FloatMatrix(value))


class SequentialFloatMatrixReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialFloatMatrixReader(rspecifier)

    @property
    def value(self) -> np.ndarray:
        """Return a 2-D array with dtype np.float32."""
        return self._impl.value.numpy()


class RandomAccessFloatMatrixReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessFloatMatrixReader(rspecifier)

    def __getitem__(self, key) -> np.ndarray:
        """Return a 2-D array of type np.float32."""
        return self._impl[key].numpy()


class DoubleMatrixWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _DoubleMatrixWriter(wspecifier)

    def write(self, key: str, value: np.ndarray) -> None:
        """
        Args:
          key:
            Key of the value.
          value:
            A 2-D array with dtype torch.float64.
        """
        assert value.dtype == np.float64
        assert value.ndim == 2
        super().write(key, _DoubleMatrix(value))


class SequentialDoubleMatrixReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialDoubleMatrixReader(rspecifier)

    @property
    def value(self) -> np.ndarray:
        """Return a 2-D array with dtype np.float64."""
        return self._impl.value.numpy()


class RandomAccessDoubleMatrixReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessDoubleMatrixReader(rspecifier)

    def __getitem__(self, key) -> np.ndarray:
        """Return a 2-D array of type np.float64."""
        return self._impl[key].numpy()


class HtkMatrixWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _HtkMatrixWriter(wspecifier)

    def write(self, key: str, value: Tuple[np.ndarray, HtkHeader]) -> None:
        """
        Args:
          key:
            Key of the value.
          value:
            A tuple containing a 2-D array with dtype torch.float32 and
            a htk header. See
            https://labrosa.ee.columbia.edu/doc/HTKBook21/node58.html
            for the format of the header.
        """
        assert isinstance(value, tuple)
        assert value[0].dtype == np.float32
        assert value[0].ndim == 2

        super().write(key, (_FloatMatrix(value[0]), value[1]))


class SequentialHtkMatrixReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialHtkMatrixReader(rspecifier)

    @property
    def value(self) -> Tuple[np.ndarray, HtkHeader]:
        """Return a tuple containing a 2-D array of type np.float32 and a
        header."""
        value = self._impl.value
        return (value[0].numpy(), value[1])


class RandomAccessHtkMatrixReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessHtkMatrixReader(rspecifier)

    def __getitem__(self, key) -> Tuple[np.ndarray, HtkHeader]:
        """Return a tuple containing a 2-D array of type np.float32 and a
        header."""
        value = self._impl[key]
        return (value[0].numpy(), value[1])


class CompressedMatrixWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _CompressedMatrixWriter(wspecifier)

    def write(
        self,
        key: str,
        value: np.ndarray,
        method: CompressionMethod.kAutomaticMethod,
    ) -> None:
        """
        Args:
          key:
            Key of the value.
          value:
            A 2-D array with dtype torch.float32 or torch.float64.
          method:
            See the documentation for :enum:`CompressionMethod`.
        """
        assert value.ndim == 2
        assert value.dtype in (np.float32, np.float64)

        if value.dtype == np.float32:
            m = _CompressedMatrix(_FloatMatrix(value), method)
        else:
            m = _CompressedMatrix(_DoubleMatrix(value), method)

        super().write(key, m)

    def __setitem__(
        self,
        key: str,
        value: Union[np.ndarray, Tuple[np.ndarray, CompressionMethod]],
    ) -> None:
        """
        Args:
          key:
            Key of the value.
          value:
            A tuple containing:
              - A 2-D array with dtype np.float32 or np.float64
              - compression method.
        """
        if isinstance(value, np.ndarray):
            value = (value, CompressionMethod.kAutomaticMethod)
        else:
            assert isinstance(value, tuple)
        self.write(key, *value)


class PosteriorWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _PosteriorWriter(wspecifier)

    def write(self, key: str, value: List[List[Tuple[int, float]]]) -> None:
        """
        Args:
          key:
            Key of the value.
          value:
            A list-of-list of tuples. Each tuple contains a pair of int
            and float, where the `int` is the transition ID and the `float`
            is a probability (typically between zero and one).
        """
        super().write(key, value)


class SequentialPosteriorReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialPosteriorReader(rspecifier)


class RandomAccessPosteriorReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessPosteriorReader(rspecifier)


class GaussPostWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _GaussPostWriter(wspecifier)

    def write(
        self, key: str, value: List[List[Tuple[int, np.ndarray]]]
    ) -> None:
        """
        Args:
          key:
            Key of the value.
          value:
            A list-of-list of tuples. Each tuple contains a pair of int
            and float, where the `int` is the pdf-id and the `np.ndarray`
            is a 1-D array with dtype np.float32
        """
        for v in value:
            for i, k in enumerate(v):
                v[i] = (k[0], _FloatVector(k[1]))
        super().write(key, value)

    __setitem__ = write


class SequentialGaussPostReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialGaussPostReader(rspecifier)

    @property
    def value(self) -> List[List[Tuple[int, np.ndarray]]]:
        """Return a list-of-list of tuples. Each tuple contains a pair of int
        and float, where the `int` is the pdf-id and the `np.ndarray` is a 1-D
        array with dtype np.float32
        """
        value = self._impl.value

        for v in value:
            for i, k in enumerate(v):
                v[i] = (k[0], k[1].numpy())

        return value


class RandomAccessGaussPostReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessGaussPostReader(rspecifier)

    def __getitem__(self, key) -> List[List[Tuple[int, np.ndarray]]]:
        """Return a list-of-list of tuples. Each tuple contains a pair of int
        and float, where the `int` is the pdf-id and the `np.ndarray` is a 1-D
        array with dtype np.float32
        """
        value = self._impl[key]

        for v in value:
            for i, k in enumerate(v):
                v[i] = (k[0], k[1].numpy())

        return value


class SequentialWaveInfoReader(_SequentialTableReader):
    """Caution: It does not support pipe input yet since it
    closes the pipe as soon as it reads the header, which
    causes pipe write error as the producer is still generating data.
    """

    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialWaveInfoReader(rspecifier)


class RandomAccessWaveInfoReader(_RandomAccessTableReader):
    """Caution: It does not support pipe input yet since it
    closes the pipe as soon as it reads the header, which
    causes pipe write error as the producer is still generating data.
    """

    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessWaveInfoReader(rspecifier)


class WaveWriter(_TableWriter):
    def open(self, wspecifier: str) -> None:
        self._impl = _WaveWriter(wspecifier)


class SequentialWaveReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialWaveReader(rspecifier)


class RandomAccessWaveReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessWaveReader(rspecifier)


class SequentialMatrixShapeReader(_SequentialTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _SequentialMatrixShapeReader(rspecifier)


class RandomAccessMatrixShapeReader(_RandomAccessTableReader):
    def open(self, rspecifier: str) -> None:
        self._impl = _RandomAccessMatrixShapeReader(rspecifier)
