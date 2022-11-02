// kaldi_native_io/csrc/posterior.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/hmm/posterior.h

// Copyright 2009-2011     Microsoft Corporation
//           2013-2014     Johns Hopkins University (author: Daniel Povey)
//                2014     Guoguo Chen

#ifndef KALDI_NATIVE_IO_CSRC_POSTERIOR_H_
#define KALDI_NATIVE_IO_CSRC_POSTERIOR_H_

#include <string>
#include <utility>
#include <vector>

#include "kaldi_native_io/csrc/kaldi-vector.h"
#include "kaldi_native_io/csrc/log.h"

namespace kaldiio {

/// Posterior is a typedef for storing acoustic-state (actually, transition-id)
/// posteriors over an utterance.  The "int32" is a transition-id, and the
/// BaseFloat is a probability (typically between zero and one).
typedef std::vector<std::vector<std::pair<int32_t, float>>> Posterior;

/// GaussPost is a typedef for storing Gaussian-level posteriors for an
/// utterance. the "int32" is a transition-id, and the Vector<BaseFloat> is a
/// vector of Gaussian posteriors. WARNING: We changed "int32" from
/// transition-id to pdf-id, and the change is applied for all programs using
/// GaussPost. This is for efficiency purpose. We also changed the name slightly
/// from GauPost to GaussPost to reduce the chance that the change will go
/// un-noticed in downstream code.
typedef std::vector<std::vector<std::pair<int32_t, Vector<float>>>> GaussPost;

// PosteriorHolder is a holder for Posterior, which is
// std::vector<std::vector<std::pair<int32_t, float>>>
// This is used for storing posteriors of transition id's for an
// utterance.
class PosteriorHolder {
 public:
  typedef Posterior T;

  PosteriorHolder() {}

  static bool Write(std::ostream &os, bool binary, const T &t);

  void Clear() {
    Posterior tmp;
    std::swap(tmp, t_);
  }

  // Reads into the holder.
  bool Read(std::istream &is);

  // Kaldi objects always have the stream open in binary mode for
  // reading.
  static bool IsReadInBinary() { return true; }

  T &Value() { return t_; }
  const T &Value() const { return t_; }

  void Swap(PosteriorHolder *other) { t_.swap(other->t_); }

  bool ExtractRange(const PosteriorHolder & /*other*/,
                    const std::string & /*range*/) {
    KALDIIO_ERR << "ExtractRange is not defined for this type of holder.";
    return false;
  }

 private:
  KALDIIO_DISALLOW_COPY_AND_ASSIGN(PosteriorHolder)
  T t_;
};

// GaussPostHolder is a holder for GaussPost, which is
// std::vector<std::vector<std::pair<int32, Vector<BaseFloat> > > >
// This is used for storing posteriors of transition id's for an
// utterance.
class GaussPostHolder {
 public:
  typedef GaussPost T;

  GaussPostHolder() {}

  static bool Write(std::ostream &os, bool binary, const T &t);

  void Clear() {
    GaussPost tmp;
    std::swap(tmp, t_);
  }

  // Reads into the holder.
  bool Read(std::istream &is);

  // Kaldi objects always have the stream open in binary mode for
  // reading.
  static bool IsReadInBinary() { return true; }

  const T &Value() const { return t_; }
  T &Value() { return t_; }

  void Swap(GaussPostHolder *other) { t_.swap(other->t_); }

  bool ExtractRange(const GaussPostHolder & /*other*/,
                    const std::string & /*range*/) {
    KALDIIO_ERR << "ExtractRange is not defined for this type of holder.";
    return false;
  }

 private:
  KALDIIO_DISALLOW_COPY_AND_ASSIGN(GaussPostHolder)
  T t_;
};

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_POSTERIOR_H_
