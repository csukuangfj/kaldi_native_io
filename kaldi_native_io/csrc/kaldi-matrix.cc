// kaldi_native_io/csrc/kaldi-matrix.cc
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/matrix/kaldi-matrix.cc

// Copyright 2009-2011   Lukas Burget;  Ondrej Glembek;  Go Vivace Inc.;
//                       Microsoft Corporation;  Saarland University;
//                       Yanmin Qian;  Petr Schwarz;  Jan Silovsky;
//                       Haihua Xu
//           2017        Shiyin Kang
//           2019        Yiwen Shao

#include "kaldi_native_io/csrc/kaldi-matrix.h"

#include <string.h>

#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "kaldi_native_io/csrc/compressed-matrix.h"
#include "kaldi_native_io/csrc/io-funcs.h"
#include "kaldi_native_io/csrc/kaldi-utils.h"

namespace kaldiio {

// Copy constructor.  Copies data to newly allocated memory.
template <typename Real>
Matrix<Real>::Matrix(const MatrixBase<Real> &M,
                     MatrixTransposeType trans /*=kNoTrans*/)
    : MatrixBase<Real>() {
  if (trans == kNoTrans) {
    Resize(M.num_rows_, M.num_cols_);
    this->CopyFromMat(M);
  } else {
    Resize(M.num_cols_, M.num_rows_);
    this->CopyFromMat(M, kTrans);
  }
}

// Copy constructor.  Copies data to newly allocated memory.
template <typename Real>
Matrix<Real>::Matrix(const Matrix<Real> &M) : MatrixBase<Real>() {
  Resize(M.num_rows_, M.num_cols_);
  this->CopyFromMat(M);
}

template <typename Real>
void Matrix<Real>::Swap(Matrix<Real> *other) {
  std::swap(this->data_, other->data_);
  std::swap(this->num_cols_, other->num_cols_);
  std::swap(this->num_rows_, other->num_rows_);
  std::swap(this->stride_, other->stride_);
}

template <typename Real>
void MatrixBase<Real>::SetZero() {
  // Avoid calling memset on NULL, that's undefined behaviour.
  if (data_ == NULL) return;

  if (num_cols_ == stride_)
    memset(data_, 0, sizeof(Real) * num_rows_ * num_cols_);
  else
    for (MatrixIndexT row = 0; row < num_rows_; row++)
      memset(data_ + row * stride_, 0, sizeof(Real) * num_cols_);
}

template <typename Real>
Real MatrixBase<Real>::Max() const {
  KALDIIO_ASSERT(num_rows_ > 0 && num_cols_ > 0);
  Real ans = *data_;
  for (MatrixIndexT r = 0; r < num_rows_; r++)
    for (MatrixIndexT c = 0; c < num_cols_; c++)
      if (data_[c + stride_ * r] > ans) ans = data_[c + stride_ * r];
  return ans;
}

template <typename Real>
Real MatrixBase<Real>::Min() const {
  KALDIIO_ASSERT(num_rows_ > 0 && num_cols_ > 0);
  Real ans = *data_;
  for (MatrixIndexT r = 0; r < num_rows_; r++)
    for (MatrixIndexT c = 0; c < num_cols_; c++)
      if (data_[c + stride_ * r] < ans) ans = data_[c + stride_ * r];
  return ans;
}

// Constructor... note that this is not const-safe as it would
// be quite complicated to implement a "const SubMatrix" class that
// would not allow its contents to be changed.
template <typename Real>
SubMatrix<Real>::SubMatrix(const MatrixBase<Real> &M, const MatrixIndexT ro,
                           const MatrixIndexT r, const MatrixIndexT co,
                           const MatrixIndexT c) {
  if (r == 0 || c == 0) {
    // we support the empty sub-matrix as a special case.
    KALDIIO_ASSERT(c == 0 && r == 0);
    this->data_ = NULL;
    this->num_cols_ = 0;
    this->num_rows_ = 0;
    this->stride_ = 0;
    return;
  }
  KALDIIO_ASSERT(static_cast<UnsignedMatrixIndexT>(ro) <
                     static_cast<UnsignedMatrixIndexT>(M.num_rows_) &&
                 static_cast<UnsignedMatrixIndexT>(co) <
                     static_cast<UnsignedMatrixIndexT>(M.num_cols_) &&
                 static_cast<UnsignedMatrixIndexT>(r) <=
                     static_cast<UnsignedMatrixIndexT>(M.num_rows_ - ro) &&
                 static_cast<UnsignedMatrixIndexT>(c) <=
                     static_cast<UnsignedMatrixIndexT>(M.num_cols_ - co));
  // point to the begining of window
  MatrixBase<Real>::num_rows_ = r;
  MatrixBase<Real>::num_cols_ = c;
  MatrixBase<Real>::stride_ = M.Stride();
  MatrixBase<Real>::data_ =
      M.Data_workaround() + static_cast<size_t>(co) +
      static_cast<size_t>(ro) * static_cast<size_t>(M.Stride());
}

template <typename Real>
template <typename OtherReal>
void MatrixBase<Real>::CopyFromMat(const MatrixBase<OtherReal> &M,
                                   MatrixTransposeType Trans) {
  if (sizeof(Real) == sizeof(OtherReal) &&
      static_cast<const void *>(M.Data()) ==
          static_cast<const void *>(this->Data())) {
    // CopyFromMat called on same data.  Nothing to do (except sanity checks).
    KALDIIO_ASSERT(Trans == kNoTrans && M.NumRows() == NumRows() &&
                   M.NumCols() == NumCols() && M.Stride() == Stride());
    return;
  }
  if (Trans == kNoTrans) {
    KALDIIO_ASSERT(num_rows_ == M.NumRows() && num_cols_ == M.NumCols());
    for (MatrixIndexT i = 0; i < num_rows_; i++)
      (*this).Row(i).CopyFromVec(M.Row(i));
  } else {
    KALDIIO_ASSERT(num_cols_ == M.NumRows() && num_rows_ == M.NumCols());
    int32_t this_stride = stride_, other_stride = M.Stride();
    Real *this_data = data_;
    const OtherReal *other_data = M.Data();
    for (MatrixIndexT i = 0; i < num_rows_; i++)
      for (MatrixIndexT j = 0; j < num_cols_; j++)
        this_data[i * this_stride + j] = other_data[j * other_stride + i];
  }
}

// template instantiations.
template void MatrixBase<float>::CopyFromMat(const MatrixBase<double> &M,
                                             MatrixTransposeType Trans);
template void MatrixBase<double>::CopyFromMat(const MatrixBase<float> &M,
                                              MatrixTransposeType Trans);
template void MatrixBase<float>::CopyFromMat(const MatrixBase<float> &M,
                                             MatrixTransposeType Trans);
template void MatrixBase<double>::CopyFromMat(const MatrixBase<double> &M,
                                              MatrixTransposeType Trans);

template <typename Real>
inline void Matrix<Real>::Init(const MatrixIndexT rows, const MatrixIndexT cols,
                               const MatrixStrideType stride_type) {
  if (rows * cols == 0) {
    KALDIIO_ASSERT(rows == 0 && cols == 0);
    this->num_rows_ = 0;
    this->num_cols_ = 0;
    this->stride_ = 0;
    this->data_ = NULL;
    return;
  }
  KALDIIO_ASSERT(rows > 0 && cols > 0);
  MatrixIndexT skip, stride;
  size_t size;
  void *data;  // aligned memory block
  void *temp;  // memory block to be really freed

  // compute the size of skip and real cols
  skip =
      ((16 / sizeof(Real)) - cols % (16 / sizeof(Real))) % (16 / sizeof(Real));
  stride = cols + skip;
  size = static_cast<size_t>(rows) * static_cast<size_t>(stride) * sizeof(Real);

  // allocate the memory and set the right dimensions and parameters
  if (NULL != (data = KALDIIO_MEMALIGN(16, size, &temp))) {
    MatrixBase<Real>::data_ = static_cast<Real *>(data);
    MatrixBase<Real>::num_rows_ = rows;
    MatrixBase<Real>::num_cols_ = cols;
    MatrixBase<Real>::stride_ = (stride_type == kDefaultStride ? stride : cols);
  } else {
    throw std::bad_alloc();
  }
}

template <typename Real>
void Matrix<Real>::Destroy() {
  // we need to free the data block if it was defined
  if (NULL != MatrixBase<Real>::data_)
    KALDIIO_MEMALIGN_FREE(MatrixBase<Real>::data_);
  MatrixBase<Real>::data_ = NULL;
  MatrixBase<Real>::num_rows_ = MatrixBase<Real>::num_cols_ =
      MatrixBase<Real>::stride_ = 0;
}

template <typename Real>
void Matrix<Real>::Resize(const MatrixIndexT rows, const MatrixIndexT cols,
                          MatrixResizeType resize_type,
                          MatrixStrideType stride_type) {
  // the next block uses recursion to handle what we have to do if
  // resize_type == kCopyData.
  if (resize_type == kCopyData) {
    if (this->data_ == NULL || rows == 0)
      resize_type = kSetZero;  // nothing to copy.
    else if (rows == this->num_rows_ && cols == this->num_cols_ &&
             (stride_type == kDefaultStride ||
              this->stride_ == this->num_cols_)) {
      // nothing to do.
      return;
    } else {
      // set tmp to a matrix of the desired size; if new matrix
      // is bigger in some dimension, zero it.
      MatrixResizeType new_resize_type =
          (rows > this->num_rows_ || cols > this->num_cols_) ? kSetZero
                                                             : kUndefined;
      Matrix<Real> tmp(rows, cols, new_resize_type, stride_type);
      MatrixIndexT rows_min = std::min(rows, this->num_rows_),
                   cols_min = std::min(cols, this->num_cols_);
      tmp.Range(0, rows_min, 0, cols_min)
          .CopyFromMat(this->Range(0, rows_min, 0, cols_min));
      tmp.Swap(this);
      // and now let tmp go out of scope, deleting what was in *this.
      return;
    }
  }
  // At this point, resize_type == kSetZero or kUndefined.

  if (MatrixBase<Real>::data_ != NULL) {
    if (rows == MatrixBase<Real>::num_rows_ &&
        cols == MatrixBase<Real>::num_cols_) {
      if (resize_type == kSetZero) this->SetZero();
      return;
    } else
      Destroy();
  }
  Init(rows, cols, stride_type);
  if (resize_type == kSetZero) MatrixBase<Real>::SetZero();
}

template <typename Real>
void MatrixBase<Real>::Write(std::ostream &os, bool binary) const {
  if (!os.good()) {
    KALDIIO_ERR << "Failed to write matrix to stream: stream not good";
  }
  if (binary) {  // Use separate binary and text formats,
    // since in binary mode we need to know if it's float or double.
    std::string my_token = (sizeof(Real) == 4 ? "FM" : "DM");

    WriteToken(os, binary, my_token);
    {
      int32_t rows = this->num_rows_;  // make the size 32-bit on disk.
      int32_t cols = this->num_cols_;
      KALDIIO_ASSERT(this->num_rows_ == (MatrixIndexT)rows);
      KALDIIO_ASSERT(this->num_cols_ == (MatrixIndexT)cols);
      WriteBasicType(os, binary, rows);
      WriteBasicType(os, binary, cols);
    }
    if (Stride() == NumCols())
      os.write(reinterpret_cast<const char *>(Data()),
               sizeof(Real) * static_cast<size_t>(num_rows_) *
                   static_cast<size_t>(num_cols_));
    else
      for (MatrixIndexT i = 0; i < num_rows_; i++)
        os.write(reinterpret_cast<const char *>(RowData(i)),
                 sizeof(Real) * num_cols_);
    if (!os.good()) {
      KALDIIO_ERR << "Failed to write matrix to stream";
    }
  } else {  // text mode.
    if (num_cols_ == 0) {
      os << " [ ]\n";
    } else {
      os << " [";
      for (MatrixIndexT i = 0; i < num_rows_; i++) {
        os << "\n  ";
        for (MatrixIndexT j = 0; j < num_cols_; j++) os << (*this)(i, j) << " ";
      }
      os << "]\n";
    }
  }
}

template <typename Real>
void Matrix<Real>::Read(std::istream &is, bool binary, bool add) {
  KALDIIO_ASSERT(add == false);
#if 0
  if (add) {
    Matrix<Real> tmp;
    tmp.Read(is, binary, false);  // read without adding.
    if (this->num_rows_ == 0)
      this->Resize(tmp.num_rows_, tmp.num_cols_);
    else {
      if (this->num_rows_ != tmp.num_rows_ ||
          this->num_cols_ != tmp.num_cols_) {
        if (tmp.num_rows_ == 0)
          return;  // do nothing in this case.
        else
          KALDIIO_ERR << "Matrix::Read, size mismatch " << this->num_rows_
                      << ", " << this->num_cols_ << " vs. " << tmp.num_rows_
                      << ", " << tmp.num_cols_;
      }
    }
    this->AddMat(1.0, tmp);
    return;
  }
#endif

  // now assume add == false.
  MatrixIndexT pos_at_start = is.tellg();
  std::ostringstream specific_error;

  if (binary) {  // Read in binary mode.
    int peekval = Peek(is, binary);
#if 1
    if (peekval == 'C') {
      // This code enables us to read CompressedMatrix as a regular matrix.
      CompressedMatrix compressed_mat;
      compressed_mat.Read(is, binary);  // at this point, add == false.
      this->Resize(compressed_mat.NumRows(), compressed_mat.NumCols());
      compressed_mat.CopyToMat(this);
      return;
    }
#endif
    const char *my_token = (sizeof(Real) == 4 ? "FM" : "DM");
    char other_token_start = (sizeof(Real) == 4 ? 'D' : 'F');
    if (peekval ==
        other_token_start) {  // need to instantiate the other type to read it.
      // if Real == float, OtherType == double, and vice versa.
      using OtherType =
          typename std::conditional<std::is_same<Real, float>::value, double,
                                    float>::type;

      Matrix<OtherType> other(this->num_rows_, this->num_cols_);
      other.Read(is, binary, false);  // add is false at this point anyway.
      this->Resize(other.NumRows(), other.NumCols());
      this->CopyFromMat(other);
      return;
    }
    std::string token;
    ReadToken(is, binary, &token);
    if (token != my_token) {
      if (token.length() > 20) token = token.substr(0, 17) + "...";
      specific_error << ": Expected token " << my_token << ", got " << token;
      goto bad;
    }
    int32_t rows, cols;
    ReadBasicType(is, binary, &rows);  // throws on error.
    ReadBasicType(is, binary, &cols);  // throws on error.
    if ((MatrixIndexT)rows != this->num_rows_ ||
        (MatrixIndexT)cols != this->num_cols_) {
      this->Resize(rows, cols);
    }
    if (this->Stride() == this->NumCols() && rows * cols != 0) {
      is.read(reinterpret_cast<char *>(this->Data()),
              sizeof(Real) * rows * cols);
      if (is.fail()) goto bad;
    } else {
      for (MatrixIndexT i = 0; i < (MatrixIndexT)rows; i++) {
        is.read(reinterpret_cast<char *>(this->RowData(i)),
                sizeof(Real) * cols);
        if (is.fail()) goto bad;
      }
    }
    if (is.eof()) return;
    if (is.fail()) goto bad;
    return;
  } else {  // Text mode.
    std::string str;
    is >> str;  // get a token
    if (is.fail()) {
      specific_error << ": Expected \"[\", got EOF";
      goto bad;
    }
    // if ((str.compare("DM") == 0) || (str.compare("FM") == 0)) {  // Back
    // compatibility. is >> str;  // get #rows
    //  is >> str;  // get #cols
    //  is >> str;  // get "["
    // }
    if (str == "[]") {
      Resize(0, 0);
      // Be tolerant of variants.
      return;
    } else if (str != "[") {
      if (str.length() > 20) str = str.substr(0, 17) + "...";
      specific_error << ": Expected \"[\", got \"" << str << '"';
      goto bad;
    }
    // At this point, we have read "[".
    std::vector<std::vector<Real> *> data;
    std::vector<Real> *cur_row = new std::vector<Real>;
    while (1) {
      int i = is.peek();
      if (i == -1) {
        specific_error << "Got EOF while reading matrix data";
        goto cleanup;
      } else if (static_cast<char>(i) == ']') {  // Finished reading matrix.
        is.get();                                // eat the "]".
        i = is.peek();
        if (static_cast<char>(i) == '\r') {
          is.get();
          is.get();  // get \r\n (must eat what we wrote)
        } else if (static_cast<char>(i) == '\n') {
          is.get();
        }  // get \n (must eat what we wrote)
        if (is.fail()) {
          KALDIIO_WARN << "After end of matrix data, read error.";
          // we got the data we needed, so just warn for this error.
        }
        // Now process the data.
        if (!cur_row->empty())
          data.push_back(cur_row);
        else
          delete (cur_row);
        cur_row = NULL;
        if (data.empty()) {
          this->Resize(0, 0);
          return;
        } else {
          int32_t num_rows = data.size(), num_cols = data[0]->size();
          this->Resize(num_rows, num_cols);
          for (int32_t i = 0; i < num_rows; i++) {
            if (static_cast<int32_t>(data[i]->size()) != num_cols) {
              specific_error << "Matrix has inconsistent #cols: " << num_cols
                             << " vs." << data[i]->size() << " (processing row"
                             << i << ")";
              goto cleanup;
            }
            for (int32_t j = 0; j < num_cols; j++)
              (*this)(i, j) = (*(data[i]))[j];
            delete data[i];
            data[i] = NULL;
          }
        }
        return;
      } else if (static_cast<char>(i) == '\n' || static_cast<char>(i) == ';') {
        // End of matrix row.
        is.get();
        if (cur_row->size() != 0) {
          data.push_back(cur_row);
          cur_row = new std::vector<Real>;
          cur_row->reserve(data.back()->size());
        }
      } else if ((i >= '0' && i <= '9') || i == '-') {  // A number...
        Real r;
        is >> r;
        if (is.fail()) {
          specific_error << "Stream failure/EOF while reading matrix data.";
          goto cleanup;
        }
        cur_row->push_back(r);
      } else if (isspace(i)) {
        is.get();  // eat the space and do nothing.
      } else {     // NaN or inf or error.
        std::string str;
        is >> str;
        if (!KALDIIO_STRCASECMP(str.c_str(), "inf") ||
            !KALDIIO_STRCASECMP(str.c_str(), "infinity")) {
          cur_row->push_back(std::numeric_limits<Real>::infinity());
          KALDIIO_WARN << "Reading infinite value into matrix.";
        } else if (!KALDIIO_STRCASECMP(str.c_str(), "nan")) {
          cur_row->push_back(std::numeric_limits<Real>::quiet_NaN());
          KALDIIO_WARN << "Reading NaN value into matrix.";
        } else {
          if (str.length() > 20) str = str.substr(0, 17) + "...";
          specific_error << "Expecting numeric matrix data, got " << str;
          goto cleanup;
        }
      }
    }
    // Note, we never leave the while () loop before this
    // line (we return from it.)
  cleanup:  // We only reach here in case of error in the while loop above.
    if (cur_row != NULL) delete cur_row;
    for (size_t i = 0; i < data.size(); i++)
      if (data[i] != NULL) delete data[i];
    // and then go on to "bad" below, where we print error.
  }
bad:
  KALDIIO_ERR << "Failed to read matrix from stream.  " << specific_error.str()
              << " File position at start is " << pos_at_start << ", currently "
              << is.tellg();
}

template <typename Real>
bool ReadHtk(std::istream &is, Matrix<Real> *M_ptr, HtkHeader *header_ptr) {
  // check instantiated with double or float.
  static_assert(std::is_floating_point<Real>::value, "");
  Matrix<Real> &M = *M_ptr;
  HtkHeader htk_hdr;

  // TODO(arnab): this fails if the HTK file has CRC cheksum or is compressed.
  is.read(reinterpret_cast<char *>(&htk_hdr),
          sizeof(htk_hdr));  // we're being really POSIX here!
  if (is.fail()) {
    KALDIIO_WARN << "Could not read header from HTK feature file ";
    return false;
  }

  KALDIIO_SWAP4(htk_hdr.mNSamples);
  KALDIIO_SWAP4(htk_hdr.mSamplePeriod);
  KALDIIO_SWAP2(htk_hdr.mSampleSize);
  KALDIIO_SWAP2(htk_hdr.mSampleKind);

  bool has_checksum = false;
  {
    // See HParm.h in HTK code for sources of these things.
    enum BaseParmKind {
      Waveform,
      Lpc,
      Lprefc,
      Lpcepstra,
      Lpdelcep,
      Irefc,
      Mfcc,
      Fbank,
      Melspec,
      User,
      Discrete,
      Plp,
      Anon
    };

    const int32_t IsCompressed = 02000, HasChecksum = 010000, HasVq = 040000,
                  Problem = IsCompressed | HasVq;
    int32_t base_parm = htk_hdr.mSampleKind & (077);
    has_checksum = (base_parm & HasChecksum) != 0;
    htk_hdr.mSampleKind &= ~HasChecksum;  // We don't support writing with
                                          // checksum so turn it off.
    if (htk_hdr.mSampleKind & Problem)
      KALDIIO_ERR << "Code to read HTK features does not support compressed "
                     "features, or features with VQ.";
    if (base_parm == Waveform || base_parm == Irefc || base_parm == Discrete)
      KALDIIO_ERR << "Attempting to read HTK features from unsupported type "
                     "(e.g. waveform or discrete features.";
  }

  // KALDIIO_VLOG(3) << "HTK header: Num Samples: " << htk_hdr.mNSamples
  //                 << "; Sample period: " << htk_hdr.mSamplePeriod
  //                 << "; Sample size: " << htk_hdr.mSampleSize
  //                 << "; Sample kind: " << htk_hdr.mSampleKind;

  M.Resize(htk_hdr.mNSamples, htk_hdr.mSampleSize / sizeof(float));

  MatrixIndexT i;
  MatrixIndexT j;
  if (sizeof(Real) == sizeof(float)) {
    for (i = 0; i < M.NumRows(); i++) {
      is.read(reinterpret_cast<char *>(M.RowData(i)),
              sizeof(float) * M.NumCols());
      if (is.fail()) {
        KALDIIO_WARN << "Could not read data from HTK feature file ";
        return false;
      }
      if (MachineIsLittleEndian()) {
        MatrixIndexT C = M.NumCols();
        for (j = 0; j < C; j++) {
          KALDIIO_SWAP4((M(i, j)));  // The HTK standard is big-endian!
        }
      }
    }
  } else {
    float *pmem = new float[M.NumCols()];
    for (i = 0; i < M.NumRows(); i++) {
      is.read(reinterpret_cast<char *>(pmem), sizeof(float) * M.NumCols());
      if (is.fail()) {
        KALDIIO_WARN << "Could not read data from HTK feature file ";
        delete[] pmem;
        return false;
      }
      MatrixIndexT C = M.NumCols();
      for (j = 0; j < C; j++) {
        if (MachineIsLittleEndian())  // HTK standard is big-endian!
          KALDIIO_SWAP4(pmem[j]);
        M(i, j) = static_cast<Real>(pmem[j]);
      }
    }
    delete[] pmem;
  }
  if (header_ptr) *header_ptr = htk_hdr;
  if (has_checksum) {
    int16_t checksum;
    is.read(reinterpret_cast<char *>(&checksum), sizeof(checksum));
    if (is.fail())
      KALDIIO_WARN << "Could not read checksum from HTK feature file ";
    // We ignore the checksum.
  }
  return true;
}

template bool ReadHtk(std::istream &is, Matrix<float> *M,
                      HtkHeader *header_ptr);

template bool ReadHtk(std::istream &is, Matrix<double> *M,
                      HtkHeader *header_ptr);

// header may be derived from a previous call
// to ReadHtk.  Must be in binary mode.
template <typename Real>
bool WriteHtk(std::ostream &os, const MatrixBase<Real> &M, HtkHeader htk_hdr) {
  KALDIIO_ASSERT(M.NumRows() == static_cast<MatrixIndexT>(htk_hdr.mNSamples));
  KALDIIO_ASSERT(M.NumCols() == static_cast<MatrixIndexT>(htk_hdr.mSampleSize) /
                                    static_cast<MatrixIndexT>(sizeof(float)));

  KALDIIO_SWAP4(htk_hdr.mNSamples);
  KALDIIO_SWAP4(htk_hdr.mSamplePeriod);
  KALDIIO_SWAP2(htk_hdr.mSampleSize);
  KALDIIO_SWAP2(htk_hdr.mSampleKind);

  os.write(reinterpret_cast<char *>(&htk_hdr), sizeof(htk_hdr));
  if (os.fail()) goto bad;

  MatrixIndexT i;
  MatrixIndexT j;
  if (sizeof(Real) == sizeof(float) && !MachineIsLittleEndian()) {
    for (i = 0; i < M.NumRows(); i++) {  // Unlikely to reach here ever!
      os.write(reinterpret_cast<char *>(const_cast<Real *>(M.RowData(i))),
               sizeof(float) * M.NumCols());
      if (os.fail()) goto bad;
    }
  } else {
    float *pmem = new float[M.NumCols()];

    for (i = 0; i < M.NumRows(); i++) {
      const Real *rowData = M.RowData(i);
      for (j = 0; j < M.NumCols(); j++)
        pmem[j] = static_cast<float>(rowData[j]);
      if (MachineIsLittleEndian())
        for (j = 0; j < M.NumCols(); j++) KALDIIO_SWAP4(pmem[j]);
      os.write(reinterpret_cast<char *>(pmem), sizeof(float) * M.NumCols());
      if (os.fail()) {
        delete[] pmem;
        goto bad;
      }
    }
    delete[] pmem;
  }
  return true;
bad:
  KALDIIO_WARN << "Could not write to HTK feature file ";
  return false;
}

template bool WriteHtk(std::ostream &os, const MatrixBase<float> &M,
                       HtkHeader htk_hdr);

template bool WriteHtk(std::ostream &os, const MatrixBase<double> &M,
                       HtkHeader htk_hdr);

template class Matrix<float>;
template class Matrix<double>;
template class MatrixBase<float>;
template class MatrixBase<double>;
template class SubMatrix<float>;
template class SubMatrix<double>;

}  // namespace kaldiio
