# Copyright      2021  Xiaomi Corp.        (authors: Fangjun Kuang)
# See ../../../LICENSE for clarification regarding multiple authors


from typing import Any, Tuple

from _kaldi_native_io import (
    _BoolWriter,
    _DoubleWriter,
    _FloatPairVectorWriter,
    _FloatWriter,
    _Int32PairVectorWriter,
    _Int32VectorVectorWriter,
    _Int32VectorWriter,
    _Int32Writer,
    _RandomAccessBoolReader,
    _RandomAccessDoubleReader,
    _RandomAccessFloatPairVectorReader,
    _RandomAccessFloatReader,
    _RandomAccessInt32PairVectorReader,
    _RandomAccessInt32Reader,
    _RandomAccessInt32VectorReader,
    _RandomAccessInt32VectorVectorReader,
    _RandomAccessTokenReader,
    _RandomAccessTokenVectorReader,
    _SequentialBoolReader,
    _SequentialDoubleReader,
    _SequentialFloatPairVectorReader,
    _SequentialFloatReader,
    _SequentialInt32PairVectorReader,
    _SequentialInt32Reader,
    _SequentialInt32VectorReader,
    _SequentialInt32VectorVectorReader,
    _SequentialTokenReader,
    _SequentialTokenVectorReader,
    _TokenVectorWriter,
    _TokenWriter,
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
        self._impl.close()

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback) -> None:
        if self.is_open:
            self.close()

    def __setitem__(self, key: str, value: Any) -> None:
        self.write(key, value)


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
