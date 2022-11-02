// kaldi_native_io/csrc/posterior.cc
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/hmm/posterior.cc

// Copyright 2009-2011  Microsoft Corporation
//           2013-2014  Johns Hopkins University (author: Daniel Povey)
//                2014  Guoguo Chen
//                2014  Guoguo Chen

#include "kaldi_native_io/csrc/posterior.h"

#include <string>
#include <vector>

#include "kaldi_native_io/csrc/io-funcs.h"
#include "kaldi_native_io/csrc/text-utils.h"

namespace kaldiio {

void WritePosterior(std::ostream &os, bool binary, const Posterior &post) {
  if (binary) {
    int32_t sz = post.size();
    WriteBasicType(os, binary, sz);
    for (Posterior::const_iterator iter = post.begin(); iter != post.end();
         ++iter) {
      int32_t sz2 = iter->size();
      WriteBasicType(os, binary, sz2);
      for (std::vector<std::pair<int32_t, float>>::const_iterator iter2 =
               iter->begin();
           iter2 != iter->end(); ++iter2) {
        WriteBasicType(os, binary, iter2->first);
        WriteBasicType(os, binary, iter2->second);
      }
    }
  } else {  // In text-mode, choose a human-friendly, script-friendly format.
    // format is [ 1235 0.6 12 0.4 ] [ 34 1.0 ] ...
    // We could have used the same code as in the binary case above,
    // but this would have resulted in less readable output.
    for (Posterior::const_iterator iter = post.begin(); iter != post.end();
         ++iter) {
      os << "[ ";
      for (std::vector<std::pair<int32_t, float>>::const_iterator iter2 =
               iter->begin();
           iter2 != iter->end(); iter2++) {
        os << iter2->first << ' ' << iter2->second << ' ';
      }
      os << "] ";
    }
    os << '\n';  // newline terminates the Posterior.
  }
  if (!os.good()) KALDIIO_ERR << "Output stream error writing Posterior.";
}

void ReadPosterior(std::istream &is, bool binary, Posterior *post) {
  post->clear();
  if (binary) {
    int32_t sz;
    ReadBasicType(is, true, &sz);
    if (sz < 0 || sz > 10000000)
      KALDIIO_ERR << "Reading posterior: got negative or improbably large size"
                  << sz;
    post->resize(sz);
    for (Posterior::iterator iter = post->begin(); iter != post->end();
         ++iter) {
      int32_t sz2;
      ReadBasicType(is, true, &sz2);
      if (sz2 < 0) KALDIIO_ERR << "Reading posteriors: got negative size";
      iter->resize(sz2);
      for (std::vector<std::pair<int32_t, float>>::iterator iter2 =
               iter->begin();
           iter2 != iter->end(); iter2++) {
        ReadBasicType(is, true, &(iter2->first));
        ReadBasicType(is, true, &(iter2->second));
      }
    }
  } else {
    std::string line;
    getline(is, line);  // This will discard the \n, if present.
                        // The Posterior is terminated by a newlinhe.
    if (is.fail())
      KALDIIO_ERR << "holder of Posterior: error reading line "
                  << (is.eof() ? "[eof]" : "");
    std::istringstream line_is(line);
    while (1) {
      std::string str;
      line_is >> std::ws;  // eat up whitespace.
      if (line_is.eof()) break;
      line_is >> str;
      if (str != "[") {
        int32_t str_int;
        // if str is an integer, we can give a slightly more concrete suggestion
        // of what might have gone wrong.
        KALDIIO_ERR
            << "Reading Posterior object: expecting [, got '" << str
            << (ConvertStringToInteger(str, &str_int)
                    ? "': did you provide alignments instead of posteriors?"
                    : "'.");
      }
      std::vector<std::pair<int32_t, float>> this_vec;
      while (1) {
        line_is >> std::ws;
        if (line_is.peek() == ']') {
          line_is.get();
          break;
        }
        int32_t i;
        float p;
        line_is >> i >> p;
        if (line_is.fail())
          KALDIIO_ERR << "Error reading Posterior object (could not get data "
                         "after \"[\");";
        this_vec.push_back(std::make_pair(i, p));
      }
      post->push_back(this_vec);
    }
  }
}

// static
bool PosteriorHolder::Write(std::ostream &os, bool binary, const T &t) {
  InitKaldiOutputStream(os, binary);  // Puts binary header if binary mode.
  try {
    WritePosterior(os, binary, t);
    return true;
  } catch (const std::exception &e) {
    KALDIIO_WARN << "Exception caught writing table of posteriors. "
                 << e.what();
    return false;  // Write failure.
  }
}

bool PosteriorHolder::Read(std::istream &is) {
  t_.clear();

  bool is_binary;
  if (!InitKaldiInputStream(is, &is_binary)) {
    KALDIIO_WARN << "Reading Table object, failed reading binary header";
    return false;
  }
  try {
    ReadPosterior(is, is_binary, &t_);
    return true;
  } catch (std::exception &e) {
    KALDIIO_WARN << "Exception caught reading table of posteriors. "
                 << e.what();
    t_.clear();
    return false;
  }
}

// static
bool GaussPostHolder::Write(std::ostream &os, bool binary, const T &t) {
  InitKaldiOutputStream(os, binary);  // Puts binary header if binary mode.
  try {
    // We don't bother making this a one-line format.
    int32_t sz = t.size();
    WriteBasicType(os, binary, sz);
    for (GaussPost::const_iterator iter = t.begin(); iter != t.end(); ++iter) {
      int32_t sz2 = iter->size();
      WriteBasicType(os, binary, sz2);
      for (std::vector<std::pair<int32_t, Vector<float>>>::const_iterator
               iter2 = iter->begin();
           iter2 != iter->end(); iter2++) {
        WriteBasicType(os, binary, iter2->first);
        iter2->second.Write(os, binary);
      }
    }
    if (!binary) os << '\n';
    return os.good();
  } catch (const std::exception &e) {
    KALDIIO_WARN << "Exception caught writing table of posteriors. "
                 << e.what();
    return false;  // Write failure.
  }
}

bool GaussPostHolder::Read(std::istream &is) {
  t_.clear();

  bool is_binary;
  if (!InitKaldiInputStream(is, &is_binary)) {
    KALDIIO_WARN << "Reading Table object, failed reading binary header";
    return false;
  }
  try {
    int32_t sz;
    ReadBasicType(is, is_binary, &sz);
    if (sz < 0) KALDIIO_ERR << "Reading posteriors: got negative size";
    t_.resize(sz);
    for (GaussPost::iterator iter = t_.begin(); iter != t_.end(); ++iter) {
      int32_t sz2;
      ReadBasicType(is, is_binary, &sz2);
      if (sz2 < 0) KALDIIO_ERR << "Reading posteriors: got negative size";
      iter->resize(sz2);
      for (std::vector<std::pair<int32_t, Vector<float>>>::iterator iter2 =
               iter->begin();
           iter2 != iter->end(); iter2++) {
        ReadBasicType(is, is_binary, &(iter2->first));
        iter2->second.Read(is, is_binary);
      }
    }
    return true;
  } catch (std::exception &e) {
    KALDIIO_WARN << "Exception caught reading table of posteriors. "
                 << e.what();
    t_.clear();
    return false;
  }
}

}  // namespace kaldiio
