// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2018 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace Eigen {

namespace internal {

template<typename XprType,typename Derived>
class indexed_based_stl_iterator_base
{
public:
  typedef Index difference_type;
  typedef std::random_access_iterator_tag iterator_category;

  indexed_based_stl_iterator_base() : mp_xpr(0), m_index(0) {}
  indexed_based_stl_iterator_base(XprType& xpr, Index index) : mp_xpr(&xpr), m_index(index) {}

  Derived& operator++() { ++m_index; return derived(); }
  Derived& operator--() { --m_index; return derived(); }

  Derived operator++(int) { Derived prev(derived()); operator++(); return prev;}
  Derived operator--(int) { Derived prev(derived()); operator--(); return prev;}

  friend Derived operator+(const indexed_based_stl_iterator_base& a, Index b) { Derived ret(a.derived()); ret += b; return ret; }
  friend Derived operator-(const indexed_based_stl_iterator_base& a, Index b) { Derived ret(a.derived()); ret -= b; return ret; }
  friend Derived operator+(Index a, const indexed_based_stl_iterator_base& b) { Derived ret(b.derived()); ret += a; return ret; }
  friend Derived operator-(Index a, const indexed_based_stl_iterator_base& b) { Derived ret(b.derived()); ret -= a; return ret; }
  
  Derived& operator+=(Index b) { m_index += b; return derived(); }
  Derived& operator-=(Index b) { m_index -= b; return derived(); }

  difference_type operator-(const indexed_based_stl_iterator_base& other) const { eigen_assert(mp_xpr == other.mp_xpr);return m_index - other.m_index; }

  bool operator==(const indexed_based_stl_iterator_base& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index == other.m_index; }
  bool operator!=(const indexed_based_stl_iterator_base& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index != other.m_index; }
  bool operator< (const indexed_based_stl_iterator_base& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index <  other.m_index; }
  bool operator<=(const indexed_based_stl_iterator_base& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index <= other.m_index; }
  bool operator> (const indexed_based_stl_iterator_base& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index >  other.m_index; }
  bool operator>=(const indexed_based_stl_iterator_base& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index >= other.m_index; }

protected:

  Derived& derived() { return static_cast<Derived&>(*this); }
  const Derived& derived() const { return static_cast<const Derived&>(*this); }

  XprType *mp_xpr;
  Index m_index;
};

template<typename XprType>
class pointer_based_stl_iterator
{
  enum { is_lvalue  = internal::is_lvalue<XprType>::value };
public:
  typedef Index difference_type;
  typedef typename XprType::Scalar value_type;
  typedef std::random_access_iterator_tag iterator_category;
  typedef typename internal::conditional<bool(is_lvalue), value_type*, const value_type*>::type pointer;
  typedef typename internal::conditional<bool(is_lvalue), value_type&, const value_type&>::type reference;

  pointer_based_stl_iterator() : m_ptr(0) {}
  pointer_based_stl_iterator(XprType& xpr, Index index) : m_incr(xpr.innerStride())
  {
    m_ptr = xpr.data() + index * m_incr.value();
  }

  reference operator*()         const { return *m_ptr;   }
  reference operator[](Index i) const { return *(m_ptr+i*m_incr.value()); }
  pointer   operator->()        const { return m_ptr;    }

  pointer_based_stl_iterator& operator++() { m_ptr += m_incr.value(); return *this; }
  pointer_based_stl_iterator& operator--() { m_ptr -= m_incr.value(); return *this; }

  pointer_based_stl_iterator operator++(int) { pointer_based_stl_iterator prev(*this); operator++(); return prev;}
  pointer_based_stl_iterator operator--(int) { pointer_based_stl_iterator prev(*this); operator--(); return prev;}

  friend pointer_based_stl_iterator operator+(const pointer_based_stl_iterator& a, Index b) { pointer_based_stl_iterator ret(a); ret += b; return ret; }
  friend pointer_based_stl_iterator operator-(const pointer_based_stl_iterator& a, Index b) { pointer_based_stl_iterator ret(a); ret -= b; return ret; }
  friend pointer_based_stl_iterator operator+(Index a, const pointer_based_stl_iterator& b) { pointer_based_stl_iterator ret(b); ret += a; return ret; }
  friend pointer_based_stl_iterator operator-(Index a, const pointer_based_stl_iterator& b) { pointer_based_stl_iterator ret(b); ret -= a; return ret; }
  
  pointer_based_stl_iterator& operator+=(Index b) { m_ptr += b*m_incr.value(); return *this; }
  pointer_based_stl_iterator& operator-=(Index b) { m_ptr -= b*m_incr.value(); return *this; }

  difference_type operator-(const pointer_based_stl_iterator& other) const {
    return (m_ptr - other.m_ptr)/m_incr.value();
  }

  bool operator==(const pointer_based_stl_iterator& other) { return m_ptr == other.m_ptr; }
  bool operator!=(const pointer_based_stl_iterator& other) { return m_ptr != other.m_ptr; }
  bool operator< (const pointer_based_stl_iterator& other) { return m_ptr <  other.m_ptr; }
  bool operator<=(const pointer_based_stl_iterator& other) { return m_ptr <= other.m_ptr; }
  bool operator> (const pointer_based_stl_iterator& other) { return m_ptr >  other.m_ptr; }
  bool operator>=(const pointer_based_stl_iterator& other) { return m_ptr >= other.m_ptr; }

protected:

  pointer m_ptr;
  internal::variable_if_dynamic<Index, XprType::InnerStrideAtCompileTime> m_incr;
};

template<typename XprType>
class generic_randaccess_stl_iterator : public indexed_based_stl_iterator_base<XprType, generic_randaccess_stl_iterator<XprType> >
{
public:
  typedef typename XprType::Scalar value_type;

protected:

  enum {
    has_direct_access = (internal::traits<XprType>::Flags & DirectAccessBit) ? 1 : 0,
    is_lvalue  = internal::is_lvalue<XprType>::value
  };

  typedef indexed_based_stl_iterator_base<XprType,generic_randaccess_stl_iterator> Base;
  using Base::m_index;
  using Base::mp_xpr;

  // TODO currently const Transpose/Reshape expressions never returns const references,
  // so lets return by value too.
  //typedef typename internal::conditional<bool(has_direct_access), const value_type&, const value_type>::type read_only_ref_t;
  typedef const value_type read_only_ref_t;

public:
  
  typedef typename internal::conditional<bool(is_lvalue), value_type *, const value_type *>::type pointer;
  typedef typename internal::conditional<bool(is_lvalue), value_type&, read_only_ref_t>::type reference;
  
  generic_randaccess_stl_iterator() : Base() {}
  generic_randaccess_stl_iterator(XprType& xpr, Index index) : Base(xpr,index) {}

  reference operator*()         const { return   (*mp_xpr)(m_index);   }
  reference operator[](Index i) const { return   (*mp_xpr)(m_index+i); }
  pointer   operator->()        const { return &((*mp_xpr)(m_index)); }
};

template<typename XprType, DirectionType Direction>
class subvector_stl_iterator : public indexed_based_stl_iterator_base<XprType, subvector_stl_iterator<XprType,Direction> >
{
protected:

  enum { is_lvalue  = internal::is_lvalue<XprType>::value };

  typedef indexed_based_stl_iterator_base<XprType,subvector_stl_iterator> Base;
  using Base::m_index;
  using Base::mp_xpr;

  typedef typename internal::conditional<Direction==Vertical,typename XprType::ColXpr,typename XprType::RowXpr>::type SubVectorType;
  typedef typename internal::conditional<Direction==Vertical,typename XprType::ConstColXpr,typename XprType::ConstRowXpr>::type ConstSubVectorType;

public:
  typedef typename internal::conditional<bool(is_lvalue), SubVectorType, ConstSubVectorType>::type value_type;
  typedef value_type* pointer;
  typedef value_type  reference;
  
  subvector_stl_iterator() : Base() {}
  subvector_stl_iterator(XprType& xpr, Index index) : Base(xpr,index) {}

  reference operator*()         const { return   (*mp_xpr).template subVector<Direction>(m_index);   }
  reference operator[](Index i) const { return   (*mp_xpr).template subVector<Direction>(m_index+i); }
  pointer   operator->()        const { return &((*mp_xpr).template subVector<Direction>(m_index)); }
};

} // namespace internal


/** returns an iterator to the first element of the 1D vector or array
  * \only_for_vectors
  * \sa end(), cbegin()
  */
template<typename Derived>
inline typename DenseBase<Derived>::iterator DenseBase<Derived>::begin()
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived);
  return iterator(derived(), 0);
}

/** const version of begin() */
template<typename Derived>
inline typename DenseBase<Derived>::const_iterator DenseBase<Derived>::begin() const
{
  return cbegin();
}

/** returns a read-only const_iterator to the first element of the 1D vector or array
  * \only_for_vectors
  * \sa cend(), begin()
  */
template<typename Derived>
inline typename DenseBase<Derived>::const_iterator DenseBase<Derived>::cbegin() const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived);
  return const_iterator(derived(), 0);
}

/** returns an iterator to the element following the last element of the 1D vector or array
  * \only_for_vectors
  * \sa begin(), cend()
  */
template<typename Derived>
inline typename DenseBase<Derived>::iterator DenseBase<Derived>::end()
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived);
  return iterator(derived(), size());
}

/** const version of end() */
template<typename Derived>
inline typename DenseBase<Derived>::const_iterator DenseBase<Derived>::end() const
{
  return cend();
}

/** returns a read-only const_iterator to the element following the last element of the 1D vector or array
  * \only_for_vectors
  * \sa begin(), cend()
  */
template<typename Derived>
inline typename DenseBase<Derived>::const_iterator DenseBase<Derived>::cend() const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived);
  return const_iterator(derived(), size());
}

template<typename XprType, DirectionType Direction>
class SubVectorsProxy
{
public:
  typedef internal::subvector_stl_iterator<XprType,       Direction> iterator;
  typedef internal::subvector_stl_iterator<const XprType, Direction> const_iterator;

  SubVectorsProxy(XprType& xpr) : m_xpr(xpr) {}

  iterator        begin() const { return iterator      (m_xpr, 0); }
  const_iterator cbegin() const { return const_iterator(m_xpr, 0); }

  iterator        end()   const { return iterator      (m_xpr, m_xpr.template subVectors<Direction>()); }
  const_iterator cend()   const { return const_iterator(m_xpr, m_xpr.template subVectors<Direction>()); }

protected:
  XprType& m_xpr;
};

template<typename Derived>
SubVectorsProxy<Derived,Vertical> DenseBase<Derived>::allCols()
{ return SubVectorsProxy<Derived,Vertical>(derived()); }

template<typename Derived>
SubVectorsProxy<const Derived,Vertical> DenseBase<Derived>::allCols() const
{ return SubVectorsProxy<const Derived,Vertical>(derived()); }

template<typename Derived>
SubVectorsProxy<Derived,Horizontal> DenseBase<Derived>::allRows()
{ return SubVectorsProxy<Derived,Horizontal>(derived()); }

template<typename Derived>
SubVectorsProxy<const Derived,Horizontal> DenseBase<Derived>::allRows() const
{ return SubVectorsProxy<const Derived,Horizontal>(derived()); }

} // namespace Eigen