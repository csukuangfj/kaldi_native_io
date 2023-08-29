// kaldi_native_io/csrc/wave-reader.cc
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/feat/wave-reader.cc

// Copyright 2009-2011  Karel Vesely;  Microsoft Corporation
//                2013  Florent Masson
//                2013  Johns Hopkins University (author: Daniel Povey)
//

#include "kaldi_native_io/csrc/wave-reader.h"

#include <algorithm>
#include <cmath>  // for trunc()
#include <cstring>
#include <limits>
#include <vector>

#include "kaldi_native_io/csrc/kaldi-utils.h"

namespace kaldiio {

// A utility class for reading wave header.
struct WaveHeaderReadGofer {
  std::istream &is;
  bool swap;
  char tag[5];

  explicit WaveHeaderReadGofer(std::istream &is) : is(is), swap(false) {
    memset(tag, '\0', sizeof tag);
  }

  void Expect4ByteTag(const char *expected) {
    is.read(tag, 4);
    if (is.fail())
      KALDIIO_ERR << "WaveData: expected " << expected
                  << ", failed to read anything";
    if (strcmp(tag, expected))
      KALDIIO_ERR << "WaveData: expected " << expected << ", got " << tag;
  }

  void Read4ByteTag() {
    is.read(tag, 4);
    if (is.fail())
      KALDIIO_ERR << "WaveData: expected 4-byte chunk-name, got read error";
  }

  uint32 ReadUint32() {
    union {
      char result[4];
      uint32 ans;
    } u;
    is.read(u.result, 4);
    if (swap) KALDIIO_SWAP4(u.result);
    if (is.fail())
      KALDIIO_ERR << "WaveData: unexpected end of file or read error";
    return u.ans;
  }

  uint16 ReadUint16() {
    union {
      char result[2];
      int16 ans;
    } u;
    is.read(u.result, 2);
    if (swap) KALDIIO_SWAP2(u.result);
    if (is.fail())
      KALDIIO_ERR << "WaveData: unexpected end of file or read error";
    return u.ans;
  }

  void SkipBytes(int32 num_bytes) {
    KALDIIO_ASSERT(num_bytes >= 0);
    for (uint32 i = 0; i < num_bytes; i++) {
      is.get();
    }
  }

};



static void WriteUint32(std::ostream &os, int32_t i) {
  union {
    char buf[4];
    int i;
  } u;
  u.i = i;
#ifdef __BIG_ENDIAN__
  KALDIIO_SWAP4(u.buf);
#endif
  os.write(u.buf, 4);
  if (os.fail()) KALDIIO_ERR << "WaveData: error writing to stream.";
}

static void WriteUint16(std::ostream &os, int16 i) {
  union {
    char buf[2];
    int16 i;
  } u;
  u.i = i;
#ifdef __BIG_ENDIAN__
  KALDIIO_SWAP2(u.buf);
#endif
  os.write(u.buf, 2);
  if (os.fail()) KALDIIO_ERR << "WaveData: error writing to stream.";
}

void WaveInfo::Read(std::istream &is) {
  WaveHeaderReadGofer reader(is);

  reader.Read4ByteTag();
  if (strcmp(reader.tag, "RIFF") == 0)
    reverse_bytes_ = false;
  else if (strcmp(reader.tag, "RIFX") == 0)
    reverse_bytes_ = true;
  else
    KALDIIO_ERR << "WaveData: expected RIFF or RIFX, got " << reader.tag;

#ifdef __BIG_ENDIAN__
  reverse_bytes_ = !reverse_bytes_;
#endif
  reader.swap = reverse_bytes_;

  uint32 riff_chunk_size = reader.ReadUint32();
  uint32 riff_chunk_read = 0;

  reader.Expect4ByteTag("WAVE");
  riff_chunk_read += 4;  // WAVE included in riff_chunk_size.

  // Possibly skip any RIFF tags between 'WAVE' and 'fmt '.
  // Apple devices produce a filler tag 'JUNK' for memory alignment.
  reader.Read4ByteTag();
  riff_chunk_read += 4;
  while (strcmp(reader.tag, "fmt ") != 0) {
    uint32 filler_size = reader.ReadUint32();
    riff_chunk_read += 4;

    reader.SkipBytes(filler_size);
    riff_chunk_read += filler_size;

    // get next RIFF tag,
    reader.Read4ByteTag();
    riff_chunk_read += 4;
  }
  KALDIIO_ASSERT(strcmp(reader.tag, "fmt ") == 0);

  uint32 fmt_chunk_size = reader.ReadUint32();
  uint32 fmt_chunk_read = 0;
  if (fmt_chunk_size < 16) {
    KALDIIO_ERR << "WaveData: expect fmt chunk of at least size 16.";
  }
  riff_chunk_read += 4;

  uint16 audio_format = reader.ReadUint16();
  num_channels_ = reader.ReadUint16();
  uint32 sample_rate = reader.ReadUint32(),
         byte_rate = reader.ReadUint32(),
         block_align = reader.ReadUint16(),
         bits_per_sample = reader.ReadUint16();
  samp_freq_ = static_cast<float>(sample_rate);
  fmt_chunk_read += 16;

  if (audio_format == 1) {
    ;
  } else if (audio_format == 0xFFFE) {  // WAVE_FORMAT_EXTENSIBLE
    uint16 extra_size = reader.ReadUint16();
    if (fmt_chunk_size < 40 || extra_size < 22) {
      KALDIIO_ERR << "WaveData: malformed WAVE_FORMAT_EXTENSIBLE format data.";
    }
    reader.ReadUint16();  // Unused for PCM.
    reader.ReadUint32();  // Channel map: we do not care.
    uint32 guid1 = reader.ReadUint32(),
           guid2 = reader.ReadUint32(),
           guid3 = reader.ReadUint32(),
           guid4 = reader.ReadUint32();
    fmt_chunk_read += 24;

    // Support only KSDATAFORMAT_SUBTYPE_PCM for now. Interesting formats:
    // ("00000001-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_PCM)
    // ("00000003-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
    // ("00000006-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_ALAW)
    // ("00000007-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_MULAW)
    if (guid1 != 0x00000001 || guid2 != 0x00100000 || guid3 != 0xAA000080 ||
        guid4 != 0x719B3800) {
      KALDIIO_ERR << "WaveData: unsupported WAVE_FORMAT_EXTENSIBLE format.";
    }
  } else {
    KALDIIO_ERR << "WaveData: can read only PCM data, format id in file is: "
                << audio_format;
  }

  if (num_channels_ == 0) KALDIIO_ERR << "WaveData: no channels present";
  if (bits_per_sample != 16)
    KALDIIO_ERR << "WaveData: unsupported bits_per_sample = "
                << bits_per_sample;
  uint32 bytes_per_sample = bits_per_sample / 8;

  if (byte_rate != sample_rate * bytes_per_sample * num_channels_)
    KALDIIO_ERR << "Unexpected byte rate " << byte_rate << " vs. "
                << sample_rate << " * " << bytes_per_sample << " * "
                << num_channels_;
  if (block_align != num_channels_ * bytes_per_sample)
    KALDIIO_ERR << "Unexpected block_align: " << block_align << " vs. "
                << num_channels_ << " * " << bytes_per_sample;

  // consume any remaining extra data in the "fmt " subchunk
  reader.SkipBytes(fmt_chunk_size - fmt_chunk_read);
  riff_chunk_read += fmt_chunk_size;

  // Skip any subchunks between "fmt" and "data".  Usually there will
  // be a single "fact" subchunk, but on Windows there can also be a
  // "list" subchunk.
  reader.Read4ByteTag();
  riff_chunk_read += 4;
  while (strcmp(reader.tag, "data") != 0) {
    // We will just ignore the data in these chunks.
    uint32 chunk_sz = reader.ReadUint32();
    riff_chunk_read += 4;
    if (chunk_sz != 4 && strcmp(reader.tag, "fact") == 0)
      KALDIIO_WARN << "Expected fact chunk to be 4 bytes long.";

    reader.SkipBytes(chunk_sz);
    riff_chunk_read += chunk_sz;

    // Now read the next chunk name.
    reader.Read4ByteTag();
    riff_chunk_read += 4;
  }
  KALDIIO_ASSERT(strcmp(reader.tag, "data") == 0);

  uint32 data_chunk_size = reader.ReadUint32();
  riff_chunk_read += 4;

  // Figure out if the file is going to be read to the end. Values as
  // observed in the wild:
  bool is_stream_mode =
      riff_chunk_size == 0 || riff_chunk_size == 0xFFFFFFFF ||
      data_chunk_size == 0 || data_chunk_size == 0xFFFFFFFF ||
      data_chunk_size == 0x7FFFF000 || // This value is used by SoX
      data_chunk_size == 0xFFFFFFFE; // This value was seen from ffmpeg -> SoX pipeline

#if 0
  if (is_stream_mode)
    KALDIIO_VLOG(1) << "Read in RIFF chunk size: " << riff_chunk_size
                    << ", data chunk size: " << data_chunk_size
                    << ". Assume 'stream mode' (reading data to EOF).";
#endif

  if (!is_stream_mode && std::abs(static_cast<int64_t>(riff_chunk_read) +
                                  static_cast<int64_t>(data_chunk_size) -
                                  static_cast<int64_t>(riff_chunk_size)) > 1) {
    // We allow the size to be off by one without warning, because there is a
    // weirdness in the format of RIFF files that means that the input may
    // sometimes be padded with 1 unused byte to make the total size even.
    KALDIIO_WARN << "Expected " << riff_chunk_size
                 << " bytes in RIFF chunk, but "
                 << "after first data block there will be " << riff_chunk_read
                 << " + " << data_chunk_size << " bytes "
                 << "(we do not support reading multiple data chunks).";
  }

  if (is_stream_mode)
    samp_count_ = -1;
  else
    samp_count_ = data_chunk_size / block_align;
}

void WaveData::Read(std::istream &is) {
  const uint32 kBlockSize = 1024 * 1024;

  WaveInfo header;
  header.Read(is);

  data_.Resize(0, 0);  // clear the data.
  samp_freq_ = header.SampFreq();

  std::vector<char> buffer;
  uint32 bytes_to_go = header.IsStreamed() ? kBlockSize : header.DataBytes();

  // Once in a while header.DataBytes() will report an insane value;
  // read the file to the end
  while (is && bytes_to_go > 0) {
    uint32 block_bytes = std::min(bytes_to_go, kBlockSize);
    uint32 offset = buffer.size();
    buffer.resize(offset + block_bytes);
    is.read(&buffer[offset], block_bytes);
    uint32 bytes_read = is.gcount();
    buffer.resize(offset + bytes_read);
    if (!header.IsStreamed()) bytes_to_go -= bytes_read;
  }

  if (is.bad()) KALDIIO_ERR << "WaveData: file read error";

  if (buffer.size() == 0) KALDIIO_ERR << "WaveData: empty file (no data)";

  if (!header.IsStreamed() && buffer.size() < header.DataBytes()) {
    KALDIIO_WARN << "Expected " << header.DataBytes() << " bytes of wave data, "
                 << "but read only " << buffer.size() << " bytes. "
                 << "Truncated file?";
  }

  const int16 *data_ptr = reinterpret_cast<const int16 *>(&buffer[0]);

  // The matrix is arranged row per channel, column per sample.
  data_.Resize(header.NumChannels(), buffer.size() / header.BlockAlign());
  for (int32 i = 0; i < data_.NumCols(); ++i) {
    for (int32 j = 0; j < data_.NumRows(); ++j) {
      int16 k = *data_ptr++;
      if (header.ReverseBytes()) KALDIIO_SWAP2(k);
      data_(j, i) = k;
    }
  }
}


// Write 16-bit PCM.

// note: the WAVE chunk contains 2 subchunks.
//
// subchunk2size = data.NumRows() * data.NumCols() * 2.

void WaveData::Write(std::ostream &os) const {
  os << "RIFF";
  if (data_.NumRows() == 0)
    KALDIIO_ERR << "Error: attempting to write empty WAVE file";

  int32_t num_chan = data_.NumRows(), num_samp = data_.NumCols(),
          bytes_per_samp = 2;

  int32_t subchunk2size = (num_chan * num_samp * bytes_per_samp);
  int32_t chunk_size = 36 + subchunk2size;
  WriteUint32(os, chunk_size);
  os << "WAVE";
  os << "fmt ";
  WriteUint32(os, 16);
  WriteUint16(os, 1);
  WriteUint16(os, num_chan);
  KALDIIO_ASSERT(samp_freq_ > 0);
  WriteUint32(os, static_cast<int32_t>(samp_freq_));
  WriteUint32(os, static_cast<int32_t>(samp_freq_) * num_chan * bytes_per_samp);
  WriteUint16(os, num_chan * bytes_per_samp);
  WriteUint16(os, 8 * bytes_per_samp);
  os << "data";
  WriteUint32(os, subchunk2size);

  const float *data_ptr = data_.Data();
  int32_t stride = data_.Stride();

  int num_clipped = 0;
  for (int32_t i = 0; i < num_samp; i++) {
    for (int32_t j = 0; j < num_chan; j++) {
      int32_t elem = static_cast<int32_t>(trunc(data_ptr[j * stride + i]));
      int16 elem_16 = static_cast<int16>(elem);
      if (elem < std::numeric_limits<int16>::min()) {
        elem_16 = std::numeric_limits<int16>::min();
        ++num_clipped;
      } else if (elem > std::numeric_limits<int16>::max()) {
        elem_16 = std::numeric_limits<int16>::max();
        ++num_clipped;
      }
#ifdef __BIG_ENDIAN__
      KALDIIO_SWAP2(elem_16);
#endif
      os.write(reinterpret_cast<char *>(&elem_16), 2);
    }
  }
  if (os.fail()) KALDIIO_ERR << "Error writing wave data to stream.";
  if (num_clipped > 0)
    KALDIIO_WARN << "WARNING: clipped " << num_clipped
                 << " samples out of total " << num_chan * num_samp
                 << ". Reduce volume?";
}

}  // namespace kaldiio
