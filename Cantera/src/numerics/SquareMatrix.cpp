/**
 *  @file DenseMatrix.cpp
 *
 */
/*
 * $Revision$
 * $Date$
 */
/*
 * Copywrite 2004 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * See file License.txt for licensing information.
 */

#include "ct_defs.h"
#include "stringUtils.h"
#include "ctlapack.h"
#include "SquareMatrix.h"
#include "global.h"

#include <iostream>
#include <vector>

#include <cstring>

using namespace std;

namespace Cantera {


  //====================================================================================================================
  SquareMatrix::SquareMatrix() :
    DenseMatrix(),
    GeneralMatrix(0),
    m_factored(0), 
    a1norm_(0.0),
    useQR_(0)
  {
  }

  //====================================================================================================================
  // Constructor.
  /*
   * Create an \c n by \c n matrix, and initialize
   * all elements to \c v.
   *
   * @param n   size of the square matrix
   * @param v   intial value of all matrix components.
   */
  SquareMatrix::SquareMatrix(int n, doublereal v)  : 
    DenseMatrix(n, n, v), 
   GeneralMatrix(0),
    m_factored(0), 
    a1norm_(0.0),
    useQR_(0)
  
  {
  }
  //====================================================================================================================
  /*
   *
   * copy constructor
   */
  SquareMatrix::SquareMatrix(const SquareMatrix& y) :
    DenseMatrix(y), 
    GeneralMatrix(0),
    m_factored(y.m_factored),
    a1norm_(y.a1norm_),
    useQR_(y.useQR_)
  {
  }
    
  //====================================================================================================================
  /*
   * Assignment operator
   */
  SquareMatrix& SquareMatrix::operator=(const SquareMatrix& y) {
    if (&y == this) return *this;
    DenseMatrix::operator=(y);
    GeneralMatrix::operator=(y);
    m_factored = y.m_factored; 
    a1norm_ = y.a1norm_;
    useQR_ = y.useQR_;
    return *this;
  }
  //====================================================================================================================
  SquareMatrix::~SquareMatrix() {
  }
  //====================================================================================================================
  /*
   * Solve Ax = b. Vector b is overwritten on exit with x.
   */
  int SquareMatrix::solve(doublereal * b) 
  {
    if (useQR_) {
      return solveQR(b);
    }
    int info=0;
    /*
     * Check to see whether the matrix has been factored.
     */
    if (!m_factored) {
      int retn = factor();
      if (retn) {
	return retn;
      }
    }
    /*
     * Solve the factored system
     */
    ct_dgetrs(ctlapack::NoTranspose, static_cast<int>(nRows()),
	      1, &(*(begin())), static_cast<int>(nRows()), 
	      DATA_PTR(ipiv()), b, static_cast<int>(nColumns()), info);
    if (info != 0) {
      if (m_printLevel) {
	writelogf("SquareMatrix::solve(): DGETRS returned INFO = %d\n", info);
      }
      if (! m_useReturnErrorCode) {
	throw CELapackError("SquareMatrix::solve()", "DGETRS returned INFO = " + int2str(info));
      }
    }
    return info;
  }
  //====================================================================================================================
  /*
   * Set all entries to zero
   */
  void SquareMatrix::zero() {
    int n = static_cast<int>(nRows());
    if (n > 0) {
      int nn = n * n;
      double *sm = &m_data[0];
      /*
       * Using memset is the fastest way to zero a contiguous
       * section of memory.
       */
      (void) memset((void *) sm, 0, nn * sizeof(double));
    }
  }
  //====================================================================================================================
  void SquareMatrix::resize(int n, int m, doublereal v) {
    DenseMatrix::resize(n, m, v);
  } 

  //====================================================================================================================
  // Multiply A*b and write result to prod.
  /*
   *  @param b    Vector to do the rh multiplcation
   *  @param prod OUTPUT vector to receive the result 
   */
  void  SquareMatrix::mult(const doublereal * const b, doublereal * const prod) const {
    DenseMatrix::mult(b, prod);
  }
  //====================================================================================================================
  // Multiply b*A and write result to prod.
  /*
   *  @param b    Vector to do the lh multiplcation
   *  @param prod OUTPUT vector to receive the result 
   */
  void  SquareMatrix::leftMult(const doublereal * const b, doublereal * const prod) const {
    DenseMatrix::leftMult(b, prod);
  }
  //====================================================================================================================
  /*
   * Factor A. A is overwritten with the LU decomposition of A.
   */
  int SquareMatrix::factor() {
    if (useQR_) {
      return factorQR();
    }
    a1norm_ = ct_dlange('1', m_nrows, m_nrows, &(*(begin())), m_nrows, DATA_PTR(work));
    integer n = static_cast<int>(nRows());
    int info=0;
    m_factored = 1;
    ct_dgetrf(n, n, &(*(begin())), static_cast<int>(nRows()), DATA_PTR(ipiv()), info);
    if (info != 0) {
      if (m_printLevel) {
	writelogf("SquareMatrix::factor(): DGETRS returned INFO = %d\n", info);
      }
      if (! m_useReturnErrorCode) {
        throw CELapackError("SquareMatrix::factor()", "DGETRS returned INFO = "+int2str(info));
      }
    }
    return info;
  }
  //=====================================================================================================================
  /*
   * clear the factored flag
   */
  void SquareMatrix::clearFactorFlag() {
    m_factored = 0;
  }
  //=====================================================================================================================
  /*
   * set the factored flag
   */
  void SquareMatrix::setFactorFlag() {
    m_factored = 1;
  }
  //=====================================================================================================================
  int SquareMatrix::factorQR() {
     if ((int) tau.size() < m_nrows)  {
       tau.resize(m_nrows, 0.0);
       work.resize(8 * m_nrows, 0.0);
     } 
     a1norm_ = ct_dlange('1', m_nrows, m_nrows, &(*(begin())), m_nrows, DATA_PTR(work));
     int info;
     m_factored = 2;
     int lwork = work.size(); 
     ct_dgeqrf(m_nrows, m_nrows, &(*(begin())), m_nrows, DATA_PTR(tau), DATA_PTR(work), lwork, info); 
     if (info != 0) {
      if (m_printLevel) {
	writelogf("SquareMatrix::factorQR(): DGEQRF returned INFO = %d\n", info);
      }
      if (! m_useReturnErrorCode) {
        throw CELapackError("SquareMatrix::factorQR()", "DGEQRF returned INFO = " + int2str(info));
      }
     }
     int lworkOpt = work[0];
     if (lworkOpt > lwork) {
       work.resize(lworkOpt);
     }

 
     return info;
  }
  //=====================================================================================================================
  /*
   * Solve Ax = b. Vector b is overwritten on exit with x.
   */
  int SquareMatrix::solveQR(doublereal * b)
  {
    int info=0;
    /*
     * Check to see whether the matrix has been factored.
     */
    if (!m_factored) {
      int retn = factorQR();
      if (retn) {
        return retn;
      }
    }
    
    int lwork = work.size(); 
    if (lwork < m_nrows) {
      work.resize(8 * m_nrows, 0.0);
      lwork = 8 * m_nrows;
    }

    /*
     * Solve the factored system
     */
    ct_dormqr(ctlapack::Left, ctlapack::Transpose, m_nrows, 1, m_nrows, &(*(begin())), m_nrows, DATA_PTR(tau), b, m_nrows, 
                        DATA_PTR(work), lwork, info);
    if (info != 0) {
      if (m_printLevel) {
        writelogf("SquareMatrix::solveQR(): DORMQR returned INFO = %d\n", info);
      }
      if (! m_useReturnErrorCode) {
        throw CELapackError("SquareMatrix::solveQR()", "DORMQR returned INFO = " + int2str(info));
      }
    }
    int lworkOpt = work[0];
    if (lworkOpt > lwork) {
      work.resize(lworkOpt);
    }

    char dd = 'N';

    ct_dtrtrs(ctlapack::UpperTriangular, ctlapack::NoTranspose, &dd, m_nrows, 1,  &(*(begin())), m_nrows, b,
              m_nrows, info);
    if (info != 0) {
      if (m_printLevel) {
        writelogf("SquareMatrix::solveQR(): DTRTRS returned INFO = %d\n", info);
      }
      if (! m_useReturnErrorCode) {
        throw CELapackError("SquareMatrix::solveQR()", "DTRTRS returned INFO = " + int2str(info));
      }
    }

    return info;
  }
  //=====================================================================================================================
  doublereal SquareMatrix::rcond(doublereal anorm) {
    
    if ((int) iwork_.size() < m_nrows) {
      iwork_.resize(m_nrows);
    }
    if ((int) work.size() <4 * m_nrows) {
      work.resize(4 * m_nrows);
    }
    doublereal rcond = 0.0;
    if (m_factored != 1) {
      throw CELapackError("SquareMatrix::rcond()", "matrix isn't factored correctly");
    }
    
    //  doublereal anorm = ct_dlange('1', m_nrows, m_nrows, &(*(begin())), m_nrows, DATA_PTR(work));


    int rinfo;
    rcond = ct_dgecon('1', m_nrows, &(*(begin())), m_nrows, anorm, DATA_PTR(work), 
		      DATA_PTR(iwork_), rinfo);
    if (rinfo != 0) {
      if (m_printLevel) {
        writelogf("SquareMatrix::rcond(): DGECON returned INFO = %d\n", rinfo);
      }
      if (! m_useReturnErrorCode) {
        throw CELapackError("SquareMatrix::rcond()", "DGECON returned INFO = " + int2str(rinfo));
      }
    }
    return rcond;
  }
  //=====================================================================================================================
  doublereal SquareMatrix::oneNorm() const {
    return a1norm_;
  }
  //=====================================================================================================================
  doublereal SquareMatrix::rcondQR() {
    
    if ((int) iwork_.size() < m_nrows) {
      iwork_.resize(m_nrows);
    }
    if ((int) work.size() <3 * m_nrows) {
      work.resize(3 * m_nrows);
    }
    doublereal rcond = 0.0;
    if (m_factored != 2) {
      throw CELapackError("SquareMatrix::rcondQR()", "matrix isn't factored correctly");
    }
    
    int rinfo;
    rcond =  ct_dtrcon(0, ctlapack::UpperTriangular, 0, m_nrows, &(*(begin())), m_nrows, DATA_PTR(work), 
		       DATA_PTR(iwork_), rinfo);
    if (rinfo != 0) {
      if (m_printLevel) {
        writelogf("SquareMatrix::rcondQR(): DTRCON returned INFO = %d\n", rinfo);
      }
      if (! m_useReturnErrorCode) {
        throw CELapackError("SquareMatrix::rcondQR()", "DTRCON returned INFO = " + int2str(rinfo));
      }
    }
    return rcond;
  }
  //=====================================================================================================================
  void SquareMatrix::useFactorAlgorithm(int fAlgorithm) {
    useQR_ = fAlgorithm;
  }
  //=====================================================================================================================
  int SquareMatrix::factorAlgorithm() const {
    return (int) useQR_;
  }
  //=====================================================================================================================
  bool SquareMatrix::factored() const {
    return m_factored;
  }
  //=====================================================================================================================
  // Return a pointer to the top of column j, columns are contiguous in memory
  /*
   *  @param j   Value of the column
   *
   *  @return  Returns a pointer to the top of the column
   */
  doublereal * SquareMatrix::ptrColumn(int j) {
    return Array2D::ptrColumn(j);
  }
  //=====================================================================================================================
  // Copy the data from one array into another without doing any checking
  /*
   *  This differs from the assignment operator as no resizing is done and memcpy() is used.
   *  @param y Array to be copied
   */
  void  SquareMatrix::copyData(const GeneralMatrix& y) {
    const SquareMatrix *yy_ptr = dynamic_cast<const SquareMatrix *>(& y);
    Array2D::copyData(*yy_ptr);
  }
 //=====================================================================================================================
  size_t  SquareMatrix::nRows() const {
    return m_nrows;
  } 
  //=====================================================================================================================
  size_t SquareMatrix::nRowsAndStruct(int * const iStruct) const {
    return m_nrows;
  }
 //=====================================================================================================================
  GeneralMatrix * SquareMatrix::duplMyselfAsGeneralMatrix() const {
    SquareMatrix *dd = new SquareMatrix(*this);
    return static_cast<GeneralMatrix *>(dd);
  }
  //=====================================================================================================================
  // Return an iterator pointing to the first element
  vector_fp::iterator SquareMatrix::begin() {
    return m_data.begin();
  }
  //=====================================================================================================================
  // Return a const iterator pointing to the first element
  vector_fp::const_iterator SquareMatrix::begin() const {
    return m_data.begin(); 
  }
  //=====================================================================================================================
  // Return a vector of const pointers to the columns
  /*
   *  Note the value of the pointers are protected by their being const.
   *  However, the value of the matrix is open to being changed.
   *
   *   @return returns a vector of pointers to the top of the columns
   *           of the matrices.  
   */
  doublereal  * const * SquareMatrix::colPts() {
    return DenseMatrix::colPts();
  }
 //=====================================================================================================================

 int SquareMatrix::checkRows(doublereal &valueSmall) const {
    valueSmall = 1.0E300;
    int iSmall = -1;
    for (int i = 0; i < m_nrows; i++) {
      double valueS = 0.0;
      for (int j = 0; j < m_nrows; j++) {
	if (fabs(value(i,j)) > valueS) {
	  valueS = fabs(value(i,j));
	}
      }
      if (valueS < valueSmall) {
	iSmall = i;
	valueSmall = valueS;
      }
    }
    return iSmall;
  }
  //=====================================================================================================================
  int SquareMatrix::checkColumns(doublereal &valueSmall) const {
    valueSmall = 1.0E300;
    int jSmall = -1;
    for (int j = 0; j < m_nrows; j++) {
      double valueS = 0.0;
      for (int i = 0; i < m_nrows; i++) {
	if (fabs(value(i,j)) > valueS) {
	  valueS = fabs(value(i,j));
	}
      }
      if (valueS < valueSmall) {
	jSmall = j;
	valueSmall = valueS;
      }
    }
    return jSmall;
  }
 //=====================================================================================================================


}

