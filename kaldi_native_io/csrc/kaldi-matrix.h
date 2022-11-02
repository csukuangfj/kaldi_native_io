// kaldi_native_io/csrc/kaldi-matrix.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/matrix/kaldi-matrix.h

// Copyright 2009-2011  Ondrej Glembek;  Microsoft Corporation;  Lukas Burget;
//                      Saarland University;  Petr Schwarz;  Yanmin Qian;
//                      Karel Vesely;  Go Vivace Inc.;  Haihua Xu
//           2017       Shiyin Kang
//           2019       Yiwen Shao

#ifndef KALDI_NATIVE_IO_CSRC_KALDI_MATRIX_H_
#define KALDI_NATIVE_IO_CSRC_KALDI_MATRIX_H_

#include "kaldi_native_io/csrc/kaldi-vector.h"
#include "kaldi_native_io/csrc/log.h"
#include "kaldi_native_io/csrc/matrix-common.h"

namespace kaldiio {

template <typename Real>
class SubMatrix;

template <typename Real>
class MatrixBase;

template <typename Real>
class Matrix;

/// Base class which provides matrix operations not involving resizing
/// or allocation.   Classes Matrix and SubMatrix inherit from it and take care
/// of allocation and resizing.
template <typename Real>
class MatrixBase {
 public:
  // so this child can access protected members of other instances.
  friend class Matrix<Real>;
  friend class SubMatrix<Real>;
  /// Sets matrix to zero.
  void SetZero();

  /// Returns number of rows (or zero for empty matrix).
  inline MatrixIndexT NumRows() const { return num_rows_; }

  /// Returns number of columns (or zero for empty matrix).
  inline MatrixIndexT NumCols() const { return num_cols_; }

  /// Stride (distance in memory between each row).  Will be >= NumCols.
  inline MatrixIndexT Stride() const { return stride_; }

  /// Indexing operator, non-const
  /// (only checks sizes if compiled with -DKALDI_PARANOID)
  inline Real &operator()(MatrixIndexT r, MatrixIndexT c) {
    KALDIIO_PARANOID_ASSERT(static_cast<UnsignedMatrixIndexT>(r) <
                                static_cast<UnsignedMatrixIndexT>(num_rows_) &&
                            static_cast<UnsignedMatrixIndexT>(c) <
                                static_cast<UnsignedMatrixIndexT>(num_cols_));
    return *(data_ + r * stride_ + c);
  }

  /// Indexing operator, const
  inline const Real operator()(MatrixIndexT r, MatrixIndexT c) const {
    KALDIIO_PARANOID_ASSERT(static_cast<UnsignedMatrixIndexT>(r) <
                                static_cast<UnsignedMatrixIndexT>(num_rows_) &&
                            static_cast<UnsignedMatrixIndexT>(c) <
                                static_cast<UnsignedMatrixIndexT>(num_cols_));
    return *(data_ + r * stride_ + c);
  }

  /// Gives pointer to raw data (const).
  inline const Real *Data() const { return data_; }

  /// Gives pointer to raw data (non-const).
  inline Real *Data() { return data_; }

  /// Returns pointer to data for one row (non-const)
  inline Real *RowData(MatrixIndexT i) {
    KALDIIO_ASSERT(static_cast<UnsignedMatrixIndexT>(i) <
                   static_cast<UnsignedMatrixIndexT>(num_rows_));
    return data_ + i * stride_;
  }

  /// Returns pointer to data for one row (const)
  inline const Real *RowData(MatrixIndexT i) const {
    KALDIIO_ASSERT(static_cast<UnsignedMatrixIndexT>(i) <
                   static_cast<UnsignedMatrixIndexT>(num_rows_));
    return data_ + i * stride_;
  }

  /// Return a sub-part of matrix.
  inline SubMatrix<Real> Range(const MatrixIndexT row_offset,
                               const MatrixIndexT num_rows,
                               const MatrixIndexT col_offset,
                               const MatrixIndexT num_cols) const {
    return SubMatrix<Real>(*this, row_offset, num_rows, col_offset, num_cols);
  }

  /// Copy given matrix. (no resize is done).
  template <typename OtherReal>
  void CopyFromMat(const MatrixBase<OtherReal> &M,
                   MatrixTransposeType trans = kNoTrans);

  /// Return specific row of matrix [const].
  inline const SubVector<Real> Row(MatrixIndexT i) const {
    KALDIIO_ASSERT(static_cast<UnsignedMatrixIndexT>(i) <
                   static_cast<UnsignedMatrixIndexT>(num_rows_));
    return SubVector<Real>(data_ + (i * stride_), NumCols());
  }

  /// Return specific row of matrix.
  inline SubVector<Real> Row(MatrixIndexT i) {
    KALDIIO_ASSERT(static_cast<UnsignedMatrixIndexT>(i) <
                   static_cast<UnsignedMatrixIndexT>(num_rows_));
    return SubVector<Real>(data_ + (i * stride_), NumCols());
  }

  /// Returns maximum element of matrix.
  Real Max() const;

  /// Returns minimum element of matrix.
  Real Min() const;

  /// write to stream.
  void Write(std::ostream &out, bool binary) const;

 protected:
  ///  Initializer, callable only from child.
  explicit MatrixBase(Real *data, MatrixIndexT cols, MatrixIndexT rows,
                      MatrixIndexT stride)
      : data_(data), num_cols_(cols), num_rows_(rows), stride_(stride) {
    static_assert(std::is_floating_point<Real>::value, "");
  }
  ///  Initializer, callable only from child.
  /// Empty initializer, for un-initialized matrix.
  MatrixBase() : data_(NULL) {
    static_assert(std::is_floating_point<Real>::value, "");
  }

  // Make sure pointers to MatrixBase cannot be deleted.
  ~MatrixBase() {}

  /// A workaround that allows SubMatrix to get a pointer to non-const data
  /// for const Matrix. Unfortunately C++ does not allow us to declare a
  /// "public const" inheritance or anything like that, so it would require
  /// a lot of work to make the SubMatrix class totally const-correct--
  /// we would have to override many of the Matrix functions.
  inline Real *Data_workaround() const { return data_; }

  /// data memory area
  Real *data_;

  /// these attributes store the real matrix size as it is stored in memory
  /// including memalignment
  MatrixIndexT num_cols_;  /// < Number of columns
  MatrixIndexT num_rows_;  /// < Number of rows
  /** True number of columns for the internal matrix. This number may differ
   * from num_cols_ as memory alignment might be used. */
  MatrixIndexT stride_;

 private:
  KALDIIO_DISALLOW_COPY_AND_ASSIGN(MatrixBase)
};

/// A class for storing matrices.
template <typename Real>
class Matrix : public MatrixBase<Real> {
 public:
  /// Empty constructor.
  Matrix();

  Matrix(const Matrix<Real> &M);  //  (cannot make explicit)

  Matrix(const MatrixIndexT r, const MatrixIndexT c,
         MatrixResizeType resize_type = kSetZero,
         MatrixStrideType stride_type = kDefaultStride)
      : MatrixBase<Real>() {
    Resize(r, c, resize_type, stride_type);
  }

  /// Constructor from any MatrixBase. Can also copy with transpose.
  /// Allocates new memory.
  explicit Matrix(const MatrixBase<Real> &M,
                  MatrixTransposeType trans = kNoTrans);

  /// Swaps the contents of *this and *other.  Shallow swap.
  void Swap(Matrix<Real> *other);

  /// read from stream.
  // Unlike one in base, allows resizing.
  void Read(std::istream &in, bool binary, bool add = false);

  /// Distructor to free matrices.
  ~Matrix() { Destroy(); }

  /// Sets matrix to a specified size (zero is OK as long as both r and c are
  /// zero).  The value of the new data depends on resize_type:
  ///   -if kSetZero, the new data will be zero
  ///   -if kUndefined, the new data will be undefined
  ///   -if kCopyData, the new data will be the same as the old data in any
  ///      shared positions, and zero elsewhere.
  ///
  /// You can set stride_type to kStrideEqualNumCols to force the stride
  /// to equal the number of columns; by default it is set so that the stride
  /// in bytes is a multiple of 16.
  ///
  /// This function takes time proportional to the number of data elements.
  void Resize(const MatrixIndexT r, const MatrixIndexT c,
              MatrixResizeType resize_type = kSetZero,
              MatrixStrideType stride_type = kDefaultStride);

 private:
  /// Deallocates memory and sets to empty matrix (dimension 0, 0).
  void Destroy();

  /// Init assumes the current class contents are invalid (i.e. junk or have
  /// already been freed), and it sets the matrix to newly allocated memory with
  /// the specified number of rows and columns.  r == c == 0 is acceptable.  The
  /// data memory contents will be undefined.
  void Init(const MatrixIndexT r, const MatrixIndexT c,
            const MatrixStrideType stride_type);
};

template <typename Real>
class SubMatrix : public MatrixBase<Real> {
 public:
  // Initialize a SubMatrix from part of a matrix; this is
  // a bit like A(b:c, d:e) in Matlab.
  // This initializer is against the proper semantics of "const", since
  // SubMatrix can change its contents.  It would be hard to implement
  // a "const-safe" version of this class.
  SubMatrix(const MatrixBase<Real> &T,
            const MatrixIndexT ro,  // row offset, 0 < ro < NumRows()
            const MatrixIndexT r,   // number of rows, r > 0
            const MatrixIndexT co,  // column offset, 0 < co < NumCols()
            const MatrixIndexT c);  // number of columns, c > 0

  /// This type of constructor is needed for Range() to work [in Matrix base
  /// class]. Cannot make it explicit.
  SubMatrix<Real>(const SubMatrix &other)
      : MatrixBase<Real>(other.data_, other.num_cols_, other.num_rows_,
                         other.stride_) {}

  ~SubMatrix<Real>() {}

  // private:
  //  /// Disallow assignment.
  //  SubMatrix<Real> &operator=(const SubMatrix<Real> &);
};

/// A structure containing the HTK header.
/// [TODO: change the style of the variables to Kaldi-compliant]
struct HtkHeader {
  /// Number of samples.
  int32_t mNSamples;
  /// Sample period.
  int32_t mSamplePeriod;
  /// Sample size
  int16_t mSampleSize;
  /// Sample kind.
  uint16_t mSampleKind;
};

// Read HTK formatted features from file into matrix.
template <typename Real>
bool ReadHtk(std::istream &is, Matrix<Real> *M, HtkHeader *header_ptr);

// Write (HTK format) features to file from matrix.
template <typename Real>
bool WriteHtk(std::ostream &os, const MatrixBase<Real> &M, HtkHeader htk_hdr);

// Write (CMUSphinx format) features to file from matrix.
template <typename Real>
bool WriteSphinx(std::ostream &os, const MatrixBase<Real> &M);

}  // namespace kaldiio

// we need to include the implementation and some
// template specializations.
#include "kaldi_native_io/csrc/kaldi-matrix-inl.h"

#endif  // KALDI_NATIVE_IO_CSRC_KALDI_MATRIX_H_
