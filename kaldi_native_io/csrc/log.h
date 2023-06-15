// kaldi_native_io/csrc/log.h
//
// Copyright (c)  2020  Xiaomi Corporation (authors: Fangjun Kuang)

#ifndef KALDI_NATIVE_IO_CSRC_LOG_H_
#define KALDI_NATIVE_IO_CSRC_LOG_H_
#include <cstdint>
#include <iostream>
#include <sstream>

namespace kaldiio {

enum class LogLevel {
  kInfo = 0,
  kWarn = 1,
  kError = 2,  // abort the program
};

class Logger {
 public:
  Logger(const char *filename, const char *func_name, uint32_t line_num,
         LogLevel level)
      : level_(level) {
    os_ << filename << ":" << func_name << ":" << line_num << "\n";
    switch (level_) {
      case LogLevel::kInfo:
        os_ << "[I] ";
        break;
      case LogLevel::kWarn:
        os_ << "[W] ";
        break;
      case LogLevel::kError:
        os_ << "[E] ";
        break;
    }
  }

  template <typename T>
  Logger &operator<<(const T &val) {
    os_ << val;
    return *this;
  }

  ~Logger() noexcept(false) {
    if (level_ == LogLevel::kError) {
      throw std::runtime_error(os_.str());
    } else {
      fprintf(stderr, "%s\n", os_.str().c_str());
    }
  }

 private:
  std::ostringstream os_;
  LogLevel level_;
};

class Voidifier {
 public:
  void operator&(const Logger &) const {}
};

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || \
    defined(__PRETTY_FUNCTION__)
// for clang and GCC
#define KALDIIO_FUNC __PRETTY_FUNCTION__
#else
// for other compilers
#define KALDIIO_FUNC __func__
#endif

#define KALDIIO_LOG \
  kaldiio::Logger(__FILE__, KALDIIO_FUNC, __LINE__, kaldiio::LogLevel::kInfo)

#define KALDIIO_WARN \
  kaldiio::Logger(__FILE__, KALDIIO_FUNC, __LINE__, kaldiio::LogLevel::kWarn)

#define KALDIIO_ERR \
  kaldiio::Logger(__FILE__, KALDIIO_FUNC, __LINE__, kaldiio::LogLevel::kError)

#define KALDIIO_ASSERT(x)                                       \
  (x) ? (void)0                                                 \
      : kaldiio::Voidifier() & KALDIIO_ERR << "Check failed!\n" \
                                           << "x: " << #x

#define KALDIIO_PARANOID_ASSERT KALDIIO_ASSERT

#define KALDIIO_DISALLOW_COPY_AND_ASSIGN(Class) \
 public:                                        \
  Class(const Class &) = delete;                \
  Class &operator=(const Class &) = delete;

using int32 = int32_t;
using int16 = int16_t;
using int8 = int8_t;

using uint32 = uint32_t;
using uint8 = uint8_t;
using uint16 = uint16_t;

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_LOG_H_
