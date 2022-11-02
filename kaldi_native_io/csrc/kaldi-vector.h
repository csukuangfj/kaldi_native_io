// kaldi_native_io/csrc/kaldi-vector.h
//
// This file is copied/modified from
// https://github.com/kaldi-asr/kaldi/blob/master/src/matrix/kaldi-vector.h

// Copyright 2009-2012   Ondrej Glembek;  Microsoft Corporation;  Lukas Burget;
//                       Saarland University (Author: Arnab Ghoshal);
//                       Ariya Rastrow;  Petr Schwarz;  Yanmin Qian;
//                       Karel Vesely;  Go Vivace Inc.;  Arnab Ghoshal
//                       Wei Shi;
//                2015   Guoguo Chen
//                2017   Daniel Galvez
//                2019   Yiwen Shao

#ifndef KALDI_NATIVE_IO_CSRC_KALDI_VECTOR_H_
#define KALDI_NATIVE_IO_CSRC_KALDI_VECTOR_H_

#include "kaldi_native_io/csrc/io-funcs.h"
#include "kaldi_native_io/csrc/log.h"
#include "kaldi_native_io/csrc/matrix-common.h"

namespace kaldiio {

template <typename Real>
class VectorBase {
 public:
  /// Returns the  dimension of the vector.
  inline MatrixIndexT Dim() const { return dim_; }

  /// Returns a pointer to the start of the vector's data.
  inline Real *Data() { return data_; }

  /// Returns a pointer to the start of the vector's data (const).
  inline const Real *Data() const { return data_; }

  /// Indexing  operator (const).
  inline Real operator()(MatrixIndexT i) const {
    KALDIIO_PARANOID_ASSERT(static_cast<UnsignedMatrixIndexT>(i) <
                            static_cast<UnsignedMatrixIndexT>(dim_));
    return *(data_ + i);
  }

  /// Indexing operator (non-const).
  inline Real &operator()(MatrixIndexT i) {
    KALDIIO_PARANOID_ASSERT(static_cast<UnsignedMatrixIndexT>(i) <
                            static_cast<UnsignedMatrixIndexT>(dim_));
    return *(data_ + i);
  }

  /// Set vector to all zeros.
  void SetZero();

  /// Add vector : *this = *this + alpha * rv (with casting between floats and
  /// doubles)
  template <typename OtherReal>
  void AddVec(const Real alpha, const VectorBase<OtherReal> &v);

  /// Copy data from another vector (must match own size).
  void CopyFromVec(const VectorBase<Real> &v);

  /// Copy data from another vector of different type (double vs. float)
  template <typename OtherReal>
  void CopyFromVec(const VectorBase<OtherReal> &v);

  /** @brief Returns a sub-vector of a vector (a range of elements).
   *  @param o [in] Origin, 0 < o < Dim()
   *  @param l [in] Length 0 < l < Dim()-o
   *  @return A SubVector object that aliases the data of the Vector object.
   *  See @c SubVector class for details   */
  SubVector<Real> Range(const MatrixIndexT o, const MatrixIndexT l) {
    return SubVector<Real>(*this, o, l);
  }

  /** @brief Returns a const sub-vector of a vector (a range of elements).
   *  @param o [in] Origin, 0 < o < Dim()
   *  @param l [in] Length 0 < l < Dim()-o
   *  @return A SubVector object that aliases the data of the Vector object.
   *  See @c SubVector class for details   */
  const SubVector<Real> Range(const MatrixIndexT o,
                              const MatrixIndexT l) const {
    return SubVector<Real>(*this, o, l);
  }

  /// Reads from C++ stream (option to add to existing contents).
  /// Throws exception on failure
  void Read(std::istream &in, bool binary, bool add = false);

  /// Writes to C++ stream (option to write in binary).
  void Write(std::ostream &Out, bool binary) const;

 protected:
  /// Destructor;  does not deallocate memory, this is handled by child classes.
  /// This destructor is protected so this object can only be
  /// deleted via a child.
  ~VectorBase() {}

  /// Empty initializer, corresponds to vector of zero size.
  VectorBase() : data_(NULL), dim_(0) {
    static_assert(std::is_floating_point<Real>::value, "");
  }

  /// data memory area
  Real *data_;
  /// dimension of vector
  MatrixIndexT dim_;
  KALDIIO_DISALLOW_COPY_AND_ASSIGN(VectorBase)
};

/** @brief A class representing a vector.
 *
 *  This class provides a way to work with vectors in kaldi.
 *  It encapsulates basic operations and memory optimizations.  */
template <typename Real>
class Vector : public VectorBase<Real> {
 public:
  /// Constructor that takes no arguments.  Initializes to empty.
  Vector() : VectorBase<Real>() {}

  /// Constructor with specific size.  Sets to all-zero by default
  /// if set_zero == false, memory contents are undefined.
  explicit Vector(const MatrixIndexT s, MatrixResizeType resize_type = kSetZero)
      : VectorBase<Real>() {
    Resize(s, resize_type);
  }

  /// Copy constructor.  The need for this is controversial.
  Vector(const Vector<Real> &v) : VectorBase<Real>() {  //  (cannot be explicit)
    Resize(v.Dim(), kUndefined);
    this->CopyFromVec(v);
  }

  /// Copy-constructor from base-class, needed to copy from SubVector.
  explicit Vector(const VectorBase<Real> &v) : VectorBase<Real>() {
    Resize(v.Dim(), kUndefined);
    this->CopyFromVec(v);
  }

  /// Swaps the contents of *this and *other.  Shallow swap.
  void Swap(Vector<Real> *other);

  /// Destructor.  Deallocates memory.
  ~Vector() { Destroy(); }

  /// Read function using C++ streams.  Can also add to existing contents
  /// of matrix.
  void Read(std::istream &in, bool binary, bool add = false);

  /// Set vector to a specified size (can be zero).
  /// The value of the new data depends on resize_type:
  ///   -if kSetZero, the new data will be zero
  ///   -if kUndefined, the new data will be undefined
  ///   -if kCopyData, the new data will be the same as the old data in any
  ///      shared positions, and zero elsewhere.
  /// This function takes time proportional to the number of data elements.
  void Resize(MatrixIndexT length, MatrixResizeType resize_type = kSetZero);

 private:
  /// Init assumes the current contents of the class are invalid (i.e. junk or
  /// has already been freed), and it sets the vector to newly allocated memory
  /// with the specified dimension.  dim == 0 is acceptable.  The memory
  /// contents pointed to by data_ will be undefined.
  void Init(const MatrixIndexT dim);

  /// Destroy function, called internally.
  void Destroy();
};

template <typename Real>
class SubVector : public VectorBase<Real> {
 public:
  /// Constructor from a Vector or SubVector.
  /// SubVectors are not const-safe and it's very hard to make them
  /// so for now we just give up.  This function contains const_cast.
  SubVector(const VectorBase<Real> &t, const MatrixIndexT origin,
            const MatrixIndexT length)
      : VectorBase<Real>() {
    // following assert equiv to origin>=0 && length>=0 &&
    // origin+length <= rt.dim_
    KALDIIO_ASSERT(static_cast<UnsignedMatrixIndexT>(origin) +
                       static_cast<UnsignedMatrixIndexT>(length) <=
                   static_cast<UnsignedMatrixIndexT>(t.Dim()));
    this->data_ = const_cast<Real *>(t.Data() + origin);
    this->dim_ = length;
  }

  /// Copy constructor
  SubVector(const SubVector &other) : VectorBase<Real>() {
    // this copy constructor needed for Range() to work in base class.
    this->data_ = other.data_;
    this->dim_ = other.dim_;
  }

  /// Constructor from a pointer to memory and a length.  Keeps a pointer
  /// to the data but does not take ownership (will never delete).
  /// Caution: this constructor enables you to evade const constraints.
  SubVector(const Real *data, MatrixIndexT length) : VectorBase<Real>() {
    this->data_ = const_cast<Real *>(data);
    this->dim_ = length;
  }

  ~SubVector() {}  ///< Destructor (does nothing; no pointers are owned here).

 private:
  /// Disallow assignment operator.
  SubVector &operator=(const SubVector &other) {}
};

}  // namespace kaldiio

#endif  // KALDI_NATIVE_IO_CSRC_KALDI_VECTOR_H_
