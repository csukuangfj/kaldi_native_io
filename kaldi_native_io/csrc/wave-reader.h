// kaldi_native_io/csrc/wave-reader.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/feat/wave-reader.h

// Copyright 2009-2011  Karel Vesely;  Microsoft Corporation
//                2013  Florent Masson
//                2013  Johns Hopkins University (author: Daniel Povey)
//
#ifndef KALDI_NATIVE_IO_CSRC_WAVE_READER_H_
#define KALDI_NATIVE_IO_CSRC_WAVE_READER_H_

#include <istream>
#include <string>
#include <utility>

#include "kaldi_native_io/csrc/kaldi-matrix.h"
#include "kaldi_native_io/csrc/log.h"

/*
// THE WAVE FORMAT IS SPECIFIED IN:
// https://en.wikipedia.org/wiki/WAV
//
//
//
//  RIFF
//  |
//  WAVE
//  |    \    \   \
//  fmt_ data ... data
//
//
//  Riff is a general container, which usually contains one WAVE chunk
//  each WAVE chunk has header sub-chunk 'fmt_'
//  and one or more data sub-chunks 'data'
//
//  [Note from Dan: to say that the wave format was ever "specified" anywhere is
//   not quite right.  The guy who invented the wave format attempted to create
//   a formal specification but it did not completely make sense.  And there
//   doesn't seem to be a consensus on what makes a valid wave file,
//   particularly where the accuracy of header information is concerned.]
*/

namespace kaldiio {

/// For historical reasons, we scale waveforms to the range
/// (2^15-1)*[-1, 1], not the usual default DSP range [-1, 1].
const float kWaveSampleMax = 32768.0;

/// This class reads and hold wave file header information.
class WaveInfo {
 public:
  WaveInfo()
      : samp_freq_(0), samp_count_(0), num_channels_(0), reverse_bytes_(0) {}

  /// Is stream size unknown? Duration and SampleCount not valid if true.
  bool IsStreamed() const { return samp_count_ < 0; }

  /// Sample frequency, Hz.
  float SampFreq() const { return samp_freq_; }

  /// Number of samples in stream. Invalid if IsStreamed() is true.
  uint32_t SampleCount() const { return samp_count_; }

  /// Approximate duration, seconds. Invalid if IsStreamed() is true.
  float Duration() const { return samp_count_ / samp_freq_; }

  /// Number of channels, 1 to 16.
  int32_t NumChannels() const { return num_channels_; }

  /// Bytes per sample.
  size_t BlockAlign() const { return 2 * num_channels_; }

  /// Wave data bytes. Invalid if IsStreamed() is true.
  size_t DataBytes() const { return samp_count_ * BlockAlign(); }

  /// Is data file byte order different from machine byte order?
  bool ReverseBytes() const { return reverse_bytes_; }

  /// 'is' should be opened in binary mode. Read() will throw on error.
  /// On success 'is' will be positioned at the beginning of wave data.
  void Read(std::istream &is);

 private:
  float samp_freq_;
  int32_t samp_count_;  // 0 if empty, -1 if undefined length.
  uint8_t num_channels_;
  bool reverse_bytes_;  // File endianness differs from host.
};

/// This class's purpose is to read in Wave files.
class WaveData {
 public:
  WaveData(float samp_freq, const Matrix<float> &data)
      : data_(data), samp_freq_(samp_freq) {}

  WaveData() : samp_freq_(0.0) {}

  /// Read() will throw on error.  It's valid to call Read() more than once--
  /// in this case it will destroy what was there before.
  /// "is" should be opened in binary mode.
  void Read(std::istream &is);

  /// Write() will throw on error.   os should be opened in binary mode.
  void Write(std::ostream &os) const;

  // This function returns the wave data-- it's in a matrix
  // because there may be multiple channels.  In the normal case
  // there's just one channel so Data() will have one row.
  const Matrix<float> &Data() const { return data_; }

  float SampFreq() const { return samp_freq_; }

  // Returns the duration in seconds
  float Duration() const { return data_.NumCols() / samp_freq_; }

  void CopyFrom(const WaveData &other) {
    samp_freq_ = other.samp_freq_;
    data_.CopyFromMat(other.data_);
  }

  void Clear() {
    data_.Resize(0, 0);
    samp_freq_ = 0.0;
  }

  void Swap(WaveData *other) {
    data_.Swap(&(other->data_));
    std::swap(samp_freq_, other->samp_freq_);
  }

 private:
  static const uint32_t kBlockSize = 1024 * 1024;  // Use 1M bytes.
  Matrix<float> data_;
  float samp_freq_;
};

// Holder class for .wav files that enables us to read (but not write) .wav
// files. c.f. util/kaldi-holder.h we don't use the KaldiObjectHolder template
// because we don't want to check for the \0B binary header. We could have faked
// it by pretending to read in the wave data in text mode after failing to find
// the \0B header, but that would have been a little ugly.
class WaveHolder {
 public:
  typedef WaveData T;

  static bool Write(std::ostream &os, bool binary, const T &t) {
    // We don't write the binary-mode header here [always binary].
    if (!binary) KALDIIO_ERR << "Wave data can only be written in binary mode.";
    try {
      t.Write(os);  // throws exception on failure.
      return true;
    } catch (const std::exception &e) {
      KALDIIO_WARN << "Exception caught in WaveHolder object (writing). "
                   << e.what();
      return false;  // write failure.
    }
  }
  void Copy(const T &t) { t_.CopyFrom(t); }

  static bool IsReadInBinary() { return true; }

  void Clear() { t_.Clear(); }

  T &Value() { return t_; }

  WaveHolder &operator=(const WaveHolder &other) {
    t_.CopyFrom(other.t_);
    return *this;
  }
  WaveHolder(const WaveHolder &other) : t_(other.t_) {}

  WaveHolder() {}

  bool Read(std::istream &is) {
    // We don't look for the binary-mode header here [always binary]
    try {
      t_.Read(is);  // Throws exception on failure.
      return true;
    } catch (const std::exception &e) {
      KALDIIO_WARN << "Exception caught in WaveHolder::Read(). " << e.what();
      return false;
    }
  }

  void Swap(WaveHolder *other) { t_.Swap(&(other->t_)); }

  bool ExtractRange(const WaveHolder & /*other*/,
                    const std::string & /*range*/) {
    KALDIIO_ERR << "ExtractRange is not defined for this type of holder.";
    return false;
  }

 private:
  T t_;
};

// This is like WaveHolder but when you just want the metadata-
// it leaves the actual data undefined, it doesn't read it.
class WaveInfoHolder {
 public:
  typedef WaveInfo T;

  void Clear() { info_ = WaveInfo(); }
  void Swap(WaveInfoHolder *other) { std::swap(info_, other->info_); }
  T &Value() { return info_; }
  static bool IsReadInBinary() { return true; }

  bool Read(std::istream &is) {
    try {
      info_.Read(is);  // Throws exception on failure.
      return true;
    } catch (const std::exception &e) {
      KALDIIO_WARN << "Exception caught in WaveInfoHolder::Read(). "
                   << e.what();
      return false;
    }
  }

  bool ExtractRange(const WaveInfoHolder & /*other*/,
                    const std::string & /*range*/) {
    KALDIIO_ERR << "ExtractRange is not defined for this type of holder.";
    return false;
  }

 private:
  WaveInfo info_;
};

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_WAVE_READER_H_
