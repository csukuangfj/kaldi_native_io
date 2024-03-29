// kaldi_native_io/csrc/kaldi-io.cc
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/util/kaldi-io.cc

// Copyright 2009-2011  Microsoft Corporation;  Jan Silovsky
//                2016  Xiaohui Zhang
#include "kaldi_native_io/csrc/kaldi-io.h"

#include <string.h>

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#endif

#include "kaldi_native_io/csrc/io-funcs.h"
#include "kaldi_native_io/csrc/kaldi-pipebuf.h"
#include "kaldi_native_io/csrc/kaldi-table.h"
#include "kaldi_native_io/csrc/log.h"
#include "kaldi_native_io/csrc/parse-options.h"
#include "kaldi_native_io/csrc/text-utils.h"

#define MapOsPath(x) x

#if defined(_MSC_VER)
static FILE *popen(const char *command, const char *mode) {
  return _popen(command, mode);
}
#endif  // _MSC_VER

namespace kaldiio {

std::string InputTypeToString(InputType t) {
  switch (t) {
    case kNoInput:
      return "kNoInput";
    case kFileInput:
      return "kFileInput";
    case kStandardInput:
      return "kStandardInput";
    case kOffsetFileInput:
      return "kOffsetFileInput";
    case kPipeInput:
      return "kPipeInput";
    default:
      KALDIIO_ERR << "Unknown type";
      return "Unknown";
  }
}

std::string OutputTypeToString(OutputType t) {
  switch (t) {
    case kNoOutput:
      return "kNoOutput";
    case kFileOutput:
      return "kFileOutput";
    case kStandardOutput:
      return "kStandardOutput";
    case kPipeOutput:
      return "kPipeOutput";
    default:
      KALDIIO_ERR << "Unknown type";
      return "Unknown";
  }
}

std::string PrintableRxfilename(const std::string &rxfilename) {
  if (rxfilename == "" || rxfilename == "-") {
    return "standard input";
  } else {
    // If this call to Escape later causes compilation issues,
    // just replace it with "return rxfilename"; it's only a
    // pretty-printing issue.
    return ParseOptions::Escape(rxfilename);
  }
}

std::string PrintableWxfilename(const std::string &wxfilename) {
  if (wxfilename == "" || wxfilename == "-") {
    return "standard output";
  } else {
    // If this call to Escape later causes compilation issues,
    // just replace it with "return wxfilename"; it's only a
    // pretty-printing issue.
    return ParseOptions::Escape(wxfilename);
  }
}

OutputType ClassifyWxfilename(const std::string &filename) {
  const char *c = filename.c_str();
  size_t length = filename.length();
  char first_char = c[0],
       last_char = (length == 0 ? '\0' : c[filename.length() - 1]);

  // if 'filename' is "" or "-", return kStandardOutput.
  if (length == 0 || (length == 1 && first_char == '-'))
    return kStandardOutput;
  else if (first_char == '|')
    return kPipeOutput;  // An output pipe like "|blah".
  else if (isspace(first_char) || isspace(last_char) || last_char == '|') {
    return kNoOutput;  // Leading or trailing space: can't interpret this.
                       // Final '|' would represent an input pipe, not an
                       // output pipe.
  } else if ((first_char == 'a' || first_char == 's') &&
             strchr(c, ':') != NULL &&
             (ClassifyWspecifier(filename, NULL, NULL, NULL) != kNoWspecifier ||
              ClassifyRspecifier(filename, NULL, NULL) != kNoRspecifier)) {
    // e.g. ark:something or scp:something... this is almost certainly a
    // scripting error, so call it an error rather than treating it as a file.
    // In practice in modern kaldi scripts all (r,w)filenames begin with "ark"
    // or "scp", even though technically speaking options like "b", "t", "s" or
    // "cs" can appear before the ark or scp, like "b,ark".  For efficiency,
    // and because this code is really just a nicety to catch errors earlier
    // than they would otherwise be caught, we only call those extra functions
    // for filenames beginning with 'a' or 's'.
    return kNoOutput;
  } else if (isdigit(last_char)) {
    // This could be a file, but we have to see if it's an offset into a file
    // (like foo.ark:4314328), which is not allowed for writing (but is
    // allowed for reaching).  This eliminates some things which would be
    // valid UNIX filenames but are not allowed by Kaldi.  (Even if we allowed
    // such filenames for writing, we woudln't be able to correctly read them).
    const char *d = c + length - 1;
    while (isdigit(*d) && d > c) d--;
    if (*d == ':') return kNoOutput;
    // else it could still be a filename; continue to the next check.
  }

  // At this point it matched no other pattern so we assume a filename, but we
  // check for internal '|' as it's a common source of errors to have pipe
  // commands without the pipe in the right place.  Say that it can't be
  // classified.
  if (strchr(c, '|') != NULL) {
    KALDIIO_WARN << "Trying to classify wxfilename with pipe symbol in the"
                    " wrong place (pipe without | at the beginning?): "
                 << filename;
    return kNoOutput;
  }
  return kFileOutput;  // It matched no other pattern: assume it's a filename.
}

InputType ClassifyRxfilename(const std::string &filename) {
  const char *c = filename.c_str();
  size_t length = filename.length();
  char first_char = c[0],
       last_char = (length == 0 ? '\0' : c[filename.length() - 1]);

  // if 'filename' is "" or "-", return kStandardInput.
  if (length == 0 || (length == 1 && first_char == '-')) {
    return kStandardInput;
  } else if (first_char == '|') {
    return kNoInput;  // An output pipe like "|blah": not
                      // valid for input.
  } else if (last_char == '|') {
    return kPipeInput;
  } else if (isspace(first_char) || isspace(last_char)) {
    return kNoInput;  // We don't allow leading or trailing space in a filename.
  } else if ((first_char == 'a' || first_char == 's') &&
             strchr(c, ':') != NULL &&
             (ClassifyWspecifier(filename, NULL, NULL, NULL) != kNoWspecifier ||
              ClassifyRspecifier(filename, NULL, NULL) != kNoRspecifier)) {
    // e.g. ark:something or scp:something... this is almost certainly a
    // scripting error, so call it an error rather than treating it as a file.
    // In practice in modern kaldi scripts all (r,w)filenames begin with "ark"
    // or "scp", even though technically speaking options like "b", "t", "s" or
    // "cs" can appear before the ark or scp, like "b,ark".  For efficiency,
    // and because this code is really just a nicety to catch errors earlier
    // than they would otherwise be caught, we only call those extra functions
    // for filenames beginning with 'a' or 's'.
    return kNoInput;
  } else if (isdigit(last_char)) {
    const char *d = c + length - 1;
    while (isdigit(*d) && d > c) d--;
    if (*d == ':')
      return kOffsetFileInput;  // Filename is like
                                // some_file:12345
    // otherwise it could still be a filename; continue to the next check.
  }

  // At this point it matched no other pattern so we assume a filename, but
  // we check for '|' as it's a common source of errors to have pipe
  // commands without the pipe in the right place.  Say that it can't be
  // classified in this case.
  if (strchr(c, '|') != NULL) {
    KALDIIO_WARN << "Trying to classify rxfilename with pipe symbol in the"
                    " wrong place (pipe without | at the end?): "
                 << filename;
    return kNoInput;
  }
  return kFileInput;  // It matched no other pattern: assume it's a filename.
}

#ifndef _MSC_VER  // on VS, we don't need this type.
// could replace basic_pipebuf<char> with stdio_filebuf<char> on some platforms.
// Would mean we could use less of our own code.
typedef basic_pipebuf<char> PipebufType;
#endif

class OutputImplBase {
 public:
  // Open will open it as a file (no header), and return true
  // on success.  It cannot be called on an already open stream.
  virtual bool Open(const std::string &filename, bool binary) = 0;
  virtual std::ostream &Stream() = 0;
  virtual bool Close() = 0;
  virtual ~OutputImplBase() {}
};

class FileOutputImpl : public OutputImplBase {
 public:
  virtual bool Open(const std::string &filename, bool binary) {
    if (os_.is_open())
      KALDIIO_ERR << "FileOutputImpl::Open(), "
                  << "open called on already open file.";
    filename_ = filename;
    os_.open(MapOsPath(filename_).c_str(),
             binary ? std::ios_base::out | std::ios_base::binary
                    : std::ios_base::out);
    return os_.is_open();
  }

  virtual std::ostream &Stream() {
    if (!os_.is_open())
      KALDIIO_ERR << "FileOutputImpl::Stream(), file is not open.";
    // I believe this error can only arise from coding error.
    return os_;
  }

  virtual bool Close() {
    if (!os_.is_open())
      KALDIIO_ERR << "FileOutputImpl::Close(), file is not open.";
    // I believe this error can only arise from coding error.
    os_.close();
    return !(os_.fail());
  }
  virtual ~FileOutputImpl() {
    if (os_.is_open()) {
      os_.close();
      if (os_.fail()) KALDIIO_ERR << "Error closing output file " << filename_;
    }
  }

 private:
  std::string filename_;
  std::ofstream os_;
};

class StandardOutputImpl : public OutputImplBase {
 public:
  StandardOutputImpl() : is_open_(false) {}

  virtual bool Open(const std::string & /*filename*/, bool binary) {
    if (is_open_)
      KALDIIO_ERR << "StandardOutputImpl::Open(), "
                     "open called on already open file.";
#ifdef _MSC_VER
    _setmode(_fileno(stdout), binary ? _O_BINARY : _O_TEXT);
#endif
    (void)binary;
    is_open_ = std::cout.good();
    return is_open_;
  }

  virtual std::ostream &Stream() {
    if (!is_open_)
      KALDIIO_ERR << "StandardOutputImpl::Stream(), object not initialized.";
    // I believe this error can only arise from coding error.
    return std::cout;
  }

  virtual bool Close() {
    if (!is_open_)
      KALDIIO_ERR << "StandardOutputImpl::Close(), file is not open.";
    is_open_ = false;
    std::cout << std::flush;
    return !(std::cout.fail());
  }
  virtual ~StandardOutputImpl() {
    if (is_open_) {
      std::cout << std::flush;
      if (std::cout.fail()) KALDIIO_ERR << "Error writing to standard output";
    }
  }

 private:
  bool is_open_;
};

class PipeOutputImpl : public OutputImplBase {
 public:
  PipeOutputImpl() : f_(NULL), os_(NULL) {}

  virtual bool Open(const std::string &wxfilename, bool binary) {
    filename_ = wxfilename;
    KALDIIO_ASSERT(f_ == NULL);  // Make sure closed.
    KALDIIO_ASSERT(wxfilename.length() != 0 && wxfilename[0] == '|');  // should
    // start with '|'
    std::string cmd_name(wxfilename, 1);
#if defined(_MSC_VER) || defined(__CYGWIN__)
    f_ = popen(cmd_name.c_str(), (binary ? "wb" : "w"));
#else
    f_ = popen(cmd_name.c_str(), "w");
#endif
    if (!f_) {  // Failure.
      KALDIIO_WARN << "Failed opening pipe for writing, command is: "
                   << cmd_name << ", errno is " << strerror(errno);
      return false;
    } else {
#ifndef _MSC_VER
      fb_ = new PipebufType(f_,  // Using this constructor won't make the
                                 // destructor try to close the stream when
                                 // we're done.
                            (binary ? std::ios_base::out | std::ios_base::binary
                                    : std::ios_base::out));
      KALDIIO_ASSERT(fb_ != NULL);  // or would be alloc error.
      os_ = new std::ostream(fb_);
#else
      os_ = new std::ofstream(f_);
#endif
      return os_->good();
    }
  }

  virtual std::ostream &Stream() {
    if (os_ == NULL)
      KALDIIO_ERR << "PipeOutputImpl::Stream(),"
                     " object not initialized.";
    // I believe this error can only arise from coding error.
    return *os_;
  }

  virtual bool Close() {
    if (os_ == NULL)
      KALDIIO_ERR << "PipeOutputImpl::Close(), file is not open.";
    bool ok = true;
    os_->flush();
    if (os_->fail()) ok = false;
    delete os_;
    os_ = NULL;
    int status;
#ifdef _MSC_VER
    status = _pclose(f_);
#else
    status = pclose(f_);
#endif
    if (status)
      KALDIIO_WARN << "Pipe " << filename_ << " had nonzero return status "
                   << status;
    f_ = NULL;
#ifndef _MSC_VER
    delete fb_;
    fb_ = NULL;
#endif
    return ok;
  }
  virtual ~PipeOutputImpl() {
    if (os_) {
      if (!Close())
        KALDIIO_ERR << "Error writing to pipe "
                    << PrintableWxfilename(filename_);
    }
  }

 private:
  std::string filename_;
  FILE *f_;
#ifndef _MSC_VER
  PipebufType *fb_;
#endif
  std::ostream *os_;
};

Output::Output(const std::string &wxfilename, bool binary, bool write_header)
    : impl_(NULL) {
  if (!Open(wxfilename, binary, write_header)) {
    if (impl_) {
      delete impl_;
      impl_ = NULL;
    }
    KALDIIO_ERR << "Error opening output stream "
                << PrintableWxfilename(wxfilename);
  }
}

bool Output::Close() {
  if (!impl_) {
    return false;  // error to call Close if not open.
  } else {
    bool ans = impl_->Close();
    delete impl_;
    impl_ = NULL;
    return ans;
  }
}

Output::~Output() {
  if (impl_) {
    bool ok = impl_->Close();
    delete impl_;
    impl_ = NULL;
    if (!ok)
      KALDIIO_ERR << "Error closing output file "
                  << PrintableWxfilename(filename_)
                  << (ClassifyWxfilename(filename_) == kFileOutput
                          ? " (disk full?)"
                          : "");
  }
}

std::ostream &Output::Stream() {  // will throw if not open; else returns
  // stream.
  if (!impl_) KALDIIO_ERR << "Output::Stream() called but not open.";
  return impl_->Stream();
}

bool Output::Open(const std::string &wxfn, bool binary, bool header) {
  if (IsOpen()) {
    if (!Close()) {  // Throw here rather than return status, as it's an error
      // about something else: if the user wanted to avoid the exception he/she
      // could have called Close().
      KALDIIO_ERR << "Output::Open(), failed to close output stream: "
                  << PrintableWxfilename(filename_);
    }
  }

  filename_ = wxfn;

  OutputType type = ClassifyWxfilename(wxfn);
  KALDIIO_ASSERT(impl_ == NULL);

  if (type == kFileOutput) {
    impl_ = new FileOutputImpl();
  } else if (type == kStandardOutput) {
    impl_ = new StandardOutputImpl();
  } else if (type == kPipeOutput) {
    impl_ = new PipeOutputImpl();
  } else {  // type == kNoOutput
    KALDIIO_WARN << "Invalid output filename format "
                 << PrintableWxfilename(wxfn);
    return false;
  }
  if (!impl_->Open(wxfn, binary)) {
    delete impl_;
    impl_ = NULL;
    return false;  // failed to open.
  } else {         // successfully opened it.
    if (header) {
      InitKaldiOutputStream(impl_->Stream(), binary);
      bool ok = impl_->Stream().good();  // still OK?
      if (!ok) {
        delete impl_;
        impl_ = NULL;
        return false;
      }
      return true;
    } else {
      return true;
    }
  }
}

class InputImplBase {
 public:
  // Open will open it as a file, and return true on success.
  // May be called twice only for kOffsetFileInput (otherwise,
  // if called twice, we just create a new Input object, to avoid
  // having to deal with the extra hassle of reopening with the
  // same object.
  // Note that we will to call Open with true (binary) for
  // for text-mode Kaldi files; the only actual text-mode input
  // is for non-Kaldi files.
  virtual bool Open(const std::string &filename, bool binary) = 0;
  virtual std::istream &Stream() = 0;
  virtual int32_t Close() = 0;  // We only need to check failure in the case of
                                // kPipeInput.
  // on close for input streams.
  virtual InputType MyType() = 0;  // Because if it's kOffsetFileInput, we may
                                   // call Open twice
  // (has efficiency benefits).

  virtual ~InputImplBase() {}
};

class FileInputImpl : public InputImplBase {
 public:
  virtual bool Open(const std::string &filename, bool binary) {
    if (is_.is_open())
      KALDIIO_ERR << "FileInputImpl::Open(), "
                  << "open called on already open file.";
    is_.open(
        MapOsPath(filename).c_str(),
        binary ? std::ios_base::in | std::ios_base::binary : std::ios_base::in);
    return is_.is_open();
  }

  virtual std::istream &Stream() {
    if (!is_.is_open())
      KALDIIO_ERR << "FileInputImpl::Stream(), file is not open.";
    // I believe this error can only arise from coding error.
    return is_;
  }

  virtual int32_t Close() {
    if (!is_.is_open())
      KALDIIO_ERR << "FileInputImpl::Close(), file is not open.";
    // I believe this error can only arise from coding error.
    is_.close();
    // Don't check status.
    return 0;
  }

  virtual InputType MyType() { return kFileInput; }

  virtual ~FileInputImpl() {
    // Stream will automatically be closed, and we don't care about
    // whether it fails.
  }

 private:
  std::ifstream is_;
};

class StandardInputImpl : public InputImplBase {
 public:
  StandardInputImpl() : is_open_(false) {}

  virtual bool Open(const std::string & /*filename*/, bool binary) {
    if (is_open_)
      KALDIIO_ERR << "StandardInputImpl::Open(), "
                     "open called on already open file.";
    is_open_ = true;
#ifdef _MSC_VER
    _setmode(_fileno(stdin), binary ? _O_BINARY : _O_TEXT);
#endif
    (void)binary;
    return true;  // Don't check good() because would be false if
    // eof, which may be valid input.
  }

  virtual std::istream &Stream() {
    if (!is_open_)
      KALDIIO_ERR << "StandardInputImpl::Stream(), object not initialized.";
    // I believe this error can only arise from coding error.
    return std::cin;
  }

  virtual InputType MyType() { return kStandardInput; }

  virtual int32_t Close() {
    if (!is_open_)
      KALDIIO_ERR << "StandardInputImpl::Close(), file is not open.";
    is_open_ = false;
    return 0;
  }
  virtual ~StandardInputImpl() {}

 private:
  bool is_open_;
};

class PipeInputImpl : public InputImplBase {
 public:
  PipeInputImpl() : f_(NULL), is_(NULL) {}

  virtual bool Open(const std::string &rxfilename, bool binary) {
    filename_ = rxfilename;
    KALDIIO_ASSERT(f_ == NULL);  // Make sure closed.
    KALDIIO_ASSERT(rxfilename.length() != 0 &&
                   rxfilename[rxfilename.length() - 1] ==
                       '|');  // should end with '|'
    std::string cmd_name(rxfilename, 0, rxfilename.length() - 1);
#if defined(_MSC_VER) || defined(__CYGWIN__)
    f_ = popen(cmd_name.c_str(), (binary ? "rb" : "r"));
#else
    f_ = popen(cmd_name.c_str(), "r");
#endif

    if (!f_) {  // Failure.
      KALDIIO_WARN << "Failed opening pipe for reading, command is: "
                   << cmd_name << ", errno is " << strerror(errno);
      return false;
    } else {
#ifndef _MSC_VER
      fb_ = new PipebufType(f_,  // Using this constructor won't lead the
                                 // destructor to close the stream.
                            (binary ? std::ios_base::in | std::ios_base::binary
                                    : std::ios_base::in));
      KALDIIO_ASSERT(fb_ != NULL);  // or would be alloc error.
      is_ = new std::istream(fb_);
#else
      is_ = new std::ifstream(f_);
#endif
      if (is_->fail() || is_->bad()) return false;
      if (is_->eof()) {
        KALDIIO_WARN << "Pipe opened with command "
                     << PrintableRxfilename(rxfilename) << " is empty.";
        // don't return false: empty may be valid.
      }
      return true;
    }
  }

  virtual std::istream &Stream() {
    if (is_ == NULL)
      KALDIIO_ERR << "PipeInputImpl::Stream(), object not initialized.";
    // I believe this error can only arise from coding error.
    return *is_;
  }

  virtual int32_t Close() {
    if (is_ == NULL) KALDIIO_ERR << "PipeInputImpl::Close(), file is not open.";
    delete is_;
    is_ = NULL;
    int32_t status;
#ifdef _MSC_VER
    status = _pclose(f_);
#else
    status = pclose(f_);
#endif
    if (status)
      KALDIIO_WARN << "Pipe " << filename_ << " had nonzero return status "
                   << status;
    f_ = NULL;
#ifndef _MSC_VER
    delete fb_;
    fb_ = NULL;
#endif
    return status;
  }
  virtual ~PipeInputImpl() {
    if (is_) Close();
  }
  virtual InputType MyType() { return kPipeInput; }

 private:
  std::string filename_;
  FILE *f_;
#ifndef _MSC_VER
  PipebufType *fb_;
#endif
  std::istream *is_;
};

/*
#else

// Just have an empty implementation of the pipe input that crashes if
// called.
class PipeInputImpl: public InputImplBase {
 public:
  PipeInputImpl() { KALDIIO_ASSERT(0 && "Pipe input not yet supported on this
  platform."); }
  virtual bool Open(const std::string, bool) { return 0; }
  virtual std::istream &Stream() const { return NULL; }
  virtual void Close() {}
  virtual InputType MyType() { return kPipeInput; }
};

#endif
*/

class OffsetFileInputImpl : public InputImplBase {
  // This class is a bit more complicated than the

 public:
  // splits a filename like /my/file:123 into /my/file and the
  // number 123.  Crashes if not this format.
  static void SplitFilename(const std::string &rxfilename,
                            std::string *filename, size_t *offset) {
    size_t pos = rxfilename.find_last_of(':');
    KALDIIO_ASSERT(pos !=
                   std::string::npos);  // would indicate error in calling
    // code, as the filename is supposed to be of the correct form at this
    // point.
    *filename = std::string(rxfilename, 0, pos);
    std::string number(rxfilename, pos + 1);
    bool ans = ConvertStringToInteger(number, offset);
    if (!ans)
      KALDIIO_ERR << "Cannot get offset from filename " << rxfilename
                  << " (possibly you compiled in 32-bit and have a >32-bit"
                  << " byte offset into a file; you'll have to compile 64-bit.";
  }

  bool Seek(size_t offset) {
    size_t cur_pos = is_.tellg();
    if (cur_pos == offset)
      return true;
    else if (cur_pos < offset && cur_pos + 100 > offset) {
      // We're close enough that it may be faster to just
      // read that data, rather than seek.
      for (size_t i = cur_pos; i < offset; i++) is_.get();
      return (is_.tellg() == std::streampos(offset));
    }
    // Try to actually seek.
    is_.seekg(offset, std::ios_base::beg);
    if (is_.fail()) {  // failbit or badbit is set [error happened]
      is_.close();
      return false;  // failure.
    } else {
      is_.clear();  // Clear any failure bits (e.g. eof).
      return true;  // success.
    }
  }

  // This Open routine is unusual in that it is designed to work even
  // if it was already open.  This for efficiency when seeking multiple
  // times.
  virtual bool Open(const std::string &rxfilename, bool binary) {
    if (is_.is_open()) {
      // We are opening when we have an already-open file.
      // We may have to seek within this file, or else close it and
      // open a different one.
      std::string tmp_filename;
      size_t offset;
      SplitFilename(rxfilename, &tmp_filename, &offset);
      if (tmp_filename == filename_ && binary == binary_) {  // Just seek
        is_.clear();  // clear fail bit, etc.
        return Seek(offset);
      } else {
        is_.close();  // don't bother checking error status of is_.
        filename_ = tmp_filename;
        is_.open(MapOsPath(filename_).c_str(),
                 binary ? std::ios_base::in | std::ios_base::binary
                        : std::ios_base::in);
        if (!is_.is_open())
          return false;
        else
          return Seek(offset);
      }
    } else {
      size_t offset;
      SplitFilename(rxfilename, &filename_, &offset);
      binary_ = binary;
      is_.open(MapOsPath(filename_).c_str(),
               binary ? std::ios_base::in | std::ios_base::binary
                      : std::ios_base::in);
      if (!is_.is_open())
        return false;
      else
        return Seek(offset);
    }
  }

  virtual std::istream &Stream() {
    if (!is_.is_open())
      KALDIIO_ERR << "FileInputImpl::Stream(), file is not open.";
    // I believe this error can only arise from coding error.
    return is_;
  }

  virtual int32_t Close() {
    if (!is_.is_open())
      KALDIIO_ERR << "FileInputImpl::Close(), file is not open.";
    // I believe this error can only arise from coding error.
    is_.close();
    // Don't check status.
    return 0;
  }

  virtual InputType MyType() { return kOffsetFileInput; }

  virtual ~OffsetFileInputImpl() {
    // Stream will automatically be closed, and we don't care about
    // whether it fails.
  }

 private:
  std::string filename_;  // the actual filename
  bool binary_;           // true if was opened in binary mode.
  std::ifstream is_;
};

Input::Input(const std::string &rxfilename, bool *binary) : impl_(NULL) {
  if (!Open(rxfilename, binary)) {
    KALDIIO_ERR << "Error opening input stream "
                << PrintableRxfilename(rxfilename);
  }
}

int32_t Input::Close() {
  if (impl_) {
    int32_t ans = impl_->Close();
    delete impl_;
    impl_ = NULL;
    return ans;
  } else {
    return 0;
  }
}

bool Input::OpenInternal(const std::string &rxfilename, bool file_binary,
                         bool *contents_binary) {
  InputType type = ClassifyRxfilename(rxfilename);
  if (IsOpen()) {
    // May have to close the stream first.
    if (type == kOffsetFileInput && impl_->MyType() == kOffsetFileInput) {
      // We want to use the same object to Open... this is in case
      // the files are the same, so we can just seek.
      if (!impl_->Open(rxfilename, file_binary)) {  // true is binary mode--
        // always open in binary.
        delete impl_;
        impl_ = NULL;
        return false;
      }
      // read the binary header, if requested.
      if (contents_binary != NULL)
        return InitKaldiInputStream(impl_->Stream(), contents_binary);
      else
        return true;
    } else {
      Close();
      // and fall through to code below which actually opens the file.
    }
  }
  if (type == kFileInput) {
    impl_ = new FileInputImpl();
  } else if (type == kStandardInput) {
    impl_ = new StandardInputImpl();
  } else if (type == kPipeInput) {
    impl_ = new PipeInputImpl();
  } else if (type == kOffsetFileInput) {
    impl_ = new OffsetFileInputImpl();
  } else {  // type == kNoInput
    KALDIIO_WARN << "Invalid input filename format "
                 << PrintableRxfilename(rxfilename);
    return false;
  }
  if (!impl_->Open(rxfilename, file_binary)) {  // true is binary mode--
    // always read in binary.
    delete impl_;
    impl_ = NULL;
    return false;
  }
  if (contents_binary != NULL)
    return InitKaldiInputStream(impl_->Stream(), contents_binary);
  else
    return true;
}

Input::~Input() {
  if (impl_) Close();
}

std::istream &Input::Stream() {
  if (!IsOpen()) KALDIIO_ERR << "Input::Stream(), not open.";
  return impl_->Stream();
}

template <>
void ReadKaldiObject(const std::string &filename, Matrix<float> *m) {
  if (!filename.empty() && filename[filename.size() - 1] == ']') {
    // This filename seems to have a 'range'... like foo.ark:4312423[20:30].
    // (the bit in square brackets is the range).
    std::string rxfilename, range;
    if (!ExtractRangeSpecifier(filename, &rxfilename, &range)) {
      KALDIIO_ERR
          << "Could not make sense of possible range specifier in filename "
          << "while reading matrix: " << filename;
    }
    Matrix<float> temp;
    bool binary_in;
    Input ki(rxfilename, &binary_in);
    temp.Read(ki.Stream(), binary_in);
    if (!ExtractObjectRange(temp, range, m)) {
      KALDIIO_ERR << "Error extracting range of object: " << filename;
    }
  } else {
    // The normal case, there is no range.
    bool binary_in;
    Input ki(filename, &binary_in);
    m->Read(ki.Stream(), binary_in);
  }
}

template <>
void ReadKaldiObject(const std::string &filename, Matrix<double> *m) {
  if (!filename.empty() && filename[filename.size() - 1] == ']') {
    // This filename seems to have a 'range'... like foo.ark:4312423[20:30].
    // (the bit in square brackets is the range).
    std::string rxfilename, range;
    if (!ExtractRangeSpecifier(filename, &rxfilename, &range)) {
      KALDIIO_ERR
          << "Could not make sense of possible range specifier in filename "
          << "while reading matrix: " << filename;
    }
    Matrix<double> temp;
    bool binary_in;
    Input ki(rxfilename, &binary_in);
    temp.Read(ki.Stream(), binary_in);
    if (!ExtractObjectRange(temp, range, m)) {
      KALDIIO_ERR << "Error extracting range of object: " << filename;
    }
  } else {
    // The normal case, there is no range.
    bool binary_in;
    Input ki(filename, &binary_in);
    m->Read(ki.Stream(), binary_in);
  }
}

}  // namespace kaldiio
