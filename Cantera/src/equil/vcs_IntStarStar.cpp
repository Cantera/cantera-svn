/**
 *  @file IntStarStar.cpp
 *
 *  Header file for class IntStarStar
 */

/*
 *  $Author: hkmoffa $
 *  $Revision: 1.1 $
 *  $Date: 2007/12/19 21:19:28 $
 */

#include "vcs_IntStarStar.h"

namespace VCSnonideal {

//!Default constructor. Create an empty array.
IntStarStar::IntStarStar() :
  m_nrows(0),
  m_ncols(0)
{
  m_data.clear();
  m_colAddr.clear();
}

/*
 *  Constructor. Create an \c m by \c n array, and initialize
 *  all elements to \c v.
 */
IntStarStar::IntStarStar(int m, int n, int v) :
  m_nrows(n),
  m_ncols(m) 
{
  m_data.resize(n*m);
  std::fill(m_data.begin(), m_data.end(), v);
  m_colAddr.resize(m);
  for (int jcol = 0; jcol < m_ncols; jcol++) {
    m_colAddr[jcol] = &(m_data[jcol*m_nrows]);
  }
}

// copy constructor
IntStarStar::IntStarStar(const IntStarStar& y) {
    m_nrows = y.m_nrows;
    m_ncols = y.m_ncols;
    m_data.resize(m_nrows*m_ncols);
    m_data = y.m_data;
    m_colAddr.resize(m_ncols);
    for (int jcol = 0; jcol < m_ncols; jcol++) {
      m_colAddr[jcol] = &(m_data[jcol*m_nrows]);
    }
  }

// assignment operator
IntStarStar& IntStarStar::operator=(const IntStarStar& y) {
  if (&y == this) return *this;
  m_nrows = y.m_nrows;
  m_ncols = y.m_ncols;
  m_data.resize(m_nrows*m_ncols);
  m_data = y.m_data;
  m_colAddr.resize(m_ncols);
  for (int jcol = 0; jcol < m_ncols; jcol++) {
    m_colAddr[jcol] = &(m_data[jcol*m_nrows]);
  }
  return *this;
}


//! resize the array, and fill the new entries with 'v'
/*!
 * @param n  This is the number of rows
 * @param m  This is the number of columns in the new matrix
 * @param v  Default fill value -> defaults to zero.
 */
void IntStarStar::resize(int m, int n, int v) {
  std::vector<int> old_data;
  bool doCopy = false;
  if (m_nrows > 0 && m_ncols > 0) {
    if (m_ncols != m) {
      doCopy = true;
      old_data = m_data;
    }
  }
  m_data.resize(n*m, v);
  if (doCopy) {
    if (n >= m_nrows && m >= m_ncols) {
      for (int jcol = 0; jcol < m_ncols; jcol++) {
        for (int irow = 0; irow < m_nrows; irow++) {
          m_data[jcol*m + irow] = old_data[jcol*m_ncols + irow];
        } 
        for (int irow = m_nrows; irow < n; irow++) {
          m_data[jcol*m + irow] = v;
        } 
      }
      for (int jcol = m_ncols; jcol < m; jcol++) {
        for (int irow = 0; irow < n; irow++) {
          m_data[jcol*m + irow] = v;
        }
      }
    } else {
      std::fill(m_data.begin(), m_data.end(), v);
      for (int jcol = 0; jcol < m_ncols; jcol++) {
        for (int irow = 0; irow < m_nrows; irow++) {
          m_data[jcol*m + irow] = old_data[jcol*m_ncols + irow];
        } 
      }
    }
  }
  m_nrows = n;
  m_ncols = m;
  m_colAddr.resize(m_ncols);
  for (int jcol = 0; jcol < m_ncols; jcol++) {
    m_colAddr[jcol] = &(m_data[jcol*m_nrows]);
  }
}

int * const IntStarStar::operator[](int jcol) {
  return m_colAddr[jcol];
}

const int * const IntStarStar::operator[](int jcol) const {
  return (const int * const) m_colAddr[jcol];
}

int * const * const IntStarStar::baseDataAddr() {
  return (int * const * const) &(m_colAddr[0]);
}

/// Number of rows
int IntStarStar::nRows() const { 
  return m_nrows; 
}

/// Number of columns
int IntStarStar::nColumns() const { 
  return m_ncols;
}

}
  
  
