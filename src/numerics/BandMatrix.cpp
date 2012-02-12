/**
 *  @file BandMatrix.cpp
 *
 *  Banded matrices.
 */

// Copyright 2001  California Institute of Technology

#include "BandMatrix.h"
#include "ctlapack.h"
#include "utilities.h"
#include "ctexceptions.h"
#include "stringUtils.h"
#include "global.h"
#include <cstring>

using namespace std;

namespace Cantera
{

//====================================================================================================================
BandMatrix::BandMatrix() :
    GeneralMatrix(1),
    m_factored(false),
    m_n(0),
    m_kl(0),
    m_ku(0),
    m_zero(0.0)
{
    data.clear();
    ludata.clear();
}
//====================================================================================================================
BandMatrix::BandMatrix(size_t n, size_t kl, size_t ku, doublereal v)   :
    GeneralMatrix(1),
    m_factored(false),
    m_n(n),
    m_kl(kl),
    m_ku(ku),
    m_zero(0.0)
{
    data.resize(n*(2*kl + ku + 1));
    ludata.resize(n*(2*kl + ku + 1));
    fill(data.begin(), data.end(), v);
    fill(ludata.begin(), ludata.end(), 0.0);
    m_ipiv.resize(m_n);
    m_colPtrs.resize(n);
    size_t ldab = (2*kl + ku + 1);
    for (size_t j = 0; j < n; j++) {
        m_colPtrs[j] = &(data[ldab * j]);
    }
}
//====================================================================================================================
BandMatrix::BandMatrix(const BandMatrix& y) :
    GeneralMatrix(1),
    m_factored(false),
    m_n(0),
    m_kl(0),
    m_ku(0),
    m_zero(0.0)
{
    m_n = y.m_n;
    m_kl = y.m_kl;
    m_ku = y.m_ku;
    data = y.data;
    ludata = y.ludata;
    m_factored = y.m_factored;
    m_ipiv = y.m_ipiv;
    m_colPtrs.resize(m_n);
    size_t ldab = (2 *m_kl + m_ku + 1);
    for (size_t j = 0; j < m_n; j++) {
        m_colPtrs[j] = &(data[ldab * j]);
    }
}
//====================================================================================================================
BandMatrix::~BandMatrix()
{

}
//====================================================================================================================
BandMatrix& BandMatrix::operator=(const BandMatrix& y)
{
    if (&y == this) {
        return *this;
    }
    GeneralMatrix::operator=(y);
    m_n = y.m_n;
    m_kl = y.m_kl;
    m_ku = y.m_ku;
    m_ipiv = y.m_ipiv;
    data = y.data;
    ludata = y.ludata;
    m_factored = y.m_factored;
    m_colPtrs.resize(m_n);
    size_t ldab = (2 * m_kl + m_ku + 1);
    for (size_t j = 0; j < m_n; j++) {
        m_colPtrs[j] = &(data[ldab * j]);
    }
    return *this;
}
//====================================================================================================================
void BandMatrix::resize(size_t n, size_t kl, size_t ku, doublereal v)
{
    m_n = n;
    m_kl = kl;
    m_ku = ku;
    data.resize(n*(2*kl + ku + 1));
    ludata.resize(n*(2*kl + ku + 1));
    m_ipiv.resize(m_n);
    fill(data.begin(), data.end(), v);
    m_colPtrs.resize(m_n);
    size_t ldab = (2 * m_kl + m_ku + 1);
    for (size_t j = 0; j < n; j++) {
        m_colPtrs[j] = &(data[ldab * j]);
    }
    m_factored = false;
}
//====================================================================================================================
void BandMatrix::bfill(doublereal v)
{
    std::fill(data.begin(), data.end(), v);
    m_factored = false;
}
//====================================================================================================================
void BandMatrix::zero()
{
    std::fill(data.begin(), data.end(), 0.0);
    m_factored = false;
}
//====================================================================================================================
doublereal& BandMatrix::operator()(int i, int j)
{
    return value(i,j);
}
//====================================================================================================================
doublereal BandMatrix::operator()(int i, int j) const
{
    return value(i,j);
}
//====================================================================================================================
doublereal& BandMatrix::value(int i, int j)
{
    m_factored = false;
    if (i < j - m_ku || i > j + m_kl) {
        return m_zero;
    }
    return data[index(i,j)];
}
//====================================================================================================================
doublereal  BandMatrix::value(int i, int j) const
{
    if (i < j - m_ku || i > j + m_kl) {
        return 0.0;
    }
    return data[index(i,j)];
}
//====================================================================================================================
int  BandMatrix::index(int i, int j) const
{
    int rw = m_kl + m_ku + i - j;
    return (2*m_kl + m_ku + 1)*j + rw;
}
//====================================================================================================================
doublereal BandMatrix::_value(int i, int j) const
{
    return data[index(i,j)];
}
//====================================================================================================================
// Number of rows
size_t BandMatrix::nRows() const
{
    return m_n;
}
//====================================================================================================================
// Number of rows
size_t BandMatrix::nRowsAndStruct(int* const iStruct) const
{
    if (iStruct) {
        iStruct[0] = m_kl;
        iStruct[1] = m_ku;
    }
    return m_n;
}
//====================================================================================================================
// Number of columns
int BandMatrix::nColumns() const
{
    return m_n;
}
//====================================================================================================================
// Number of subdiagonals
int BandMatrix::nSubDiagonals() const
{
    return m_kl;
}
//====================================================================================================================
// Number of superdiagonals
int BandMatrix::nSuperDiagonals() const
{
    return m_ku;
}
//====================================================================================================================
int  BandMatrix::ldim() const
{
    return 2*m_kl + m_ku + 1;
}
//====================================================================================================================
vector_int&   BandMatrix::ipiv()
{
    return m_ipiv;
}
//====================================================================================================================
/*
 * Multiply A*b and write result to \c prod.
 */
void BandMatrix::mult(const doublereal* const b, doublereal* const prod) const
{
    size_t nr = nRows();
    doublereal sum = 0.0;
    for (size_t m = 0; m < nr; m++) {
        sum = 0.0;
        for (size_t j = m - m_kl; j <= m + m_ku; j++) {
            if (j >= 0 && j < m_n) {
                sum += _value(m,j) * b[j];
            }
        }
        prod[m] = sum;
    }
}
//====================================================================================================================
/*
 * Multiply b*A and write result to \c prod.
 */
void BandMatrix::leftMult(const doublereal* const b, doublereal* const prod) const
{
    size_t nc = nColumns();
    doublereal sum = 0.0;
    for (size_t n = 0; n < nc; n++) {
        sum = 0.0;
        for (size_t i = n - m_ku; i <= n + m_kl; i++) {
            if (i >= 0 && i < m_n) {
                sum += _value(i,n) * b[i];
            }
        }
        prod[n] = sum;
    }
}
//====================================================================================================================
/*
 * Perform an LU decomposition. LAPACK routine DGBTRF is used.
 * The factorization is saved in ludata.
 */
int BandMatrix::factor()
{
    int info=0;
    copy(data.begin(), data.end(), ludata.begin());
    ct_dgbtrf(nRows(), nColumns(), nSubDiagonals(), nSuperDiagonals(),
              DATA_PTR(ludata), ldim(), DATA_PTR(ipiv()), info);

    // if info = 0, LU decomp succeeded.
    if (info == 0) {
        m_factored = true;
    } else {
        m_factored = false;
        ofstream fout("bandmatrix.csv");
        fout << *this << endl;
        fout.close();
    }
    return info;
}
//====================================================================================================================
int BandMatrix::solve(const doublereal* const b, doublereal* const x)
{
    copy(b, b + m_n, x);
    return solve(x);
}
//====================================================================================================================
int BandMatrix::solve(doublereal* b)
{
    int info = 0;
    if (!m_factored) {
        info = factor();
    }
    if (info == 0)
        ct_dgbtrs(ctlapack::NoTranspose, nColumns(), nSubDiagonals(),
                  nSuperDiagonals(), 1, DATA_PTR(ludata), ldim(),
                  DATA_PTR(ipiv()), b, nColumns(), info);

    // error handling
    if (info != 0) {
        ofstream fout("bandmatrix.csv");
        fout << *this << endl;
        fout.close();
    }
    return info;
}
//====================================================================================================================
vector_fp::iterator  BandMatrix::begin()
{
    m_factored = false;
    return data.begin();
}
//====================================================================================================================
vector_fp::iterator BandMatrix::end()
{
    m_factored = false;
    return data.end();
}
//====================================================================================================================
vector_fp::const_iterator BandMatrix::begin() const
{
    return data.begin();
}
//====================================================================================================================
vector_fp::const_iterator BandMatrix::end() const
{
    return data.end();
}
//====================================================================================================================
ostream& operator<<(ostream& s, const BandMatrix& m)
{
    size_t nr = m.nRows();
    size_t nc = m.nColumns();
    for (size_t i = 0; i < nr; i++) {
        for (size_t j = 0; j < nc; j++) {
            s << m(i,j) << ", ";
        }
        s << endl;
    }
    return s;
}
//====================================================================================================================
void BandMatrix::err(std::string msg) const
{
    throw CanteraError("BandMatrix() unimplemented function", msg);
}
//====================================================================================================================
// Factors the A matrix using the QR algorithm, overwriting A
/*
 * we set m_factored to 2 to indicate the matrix is now QR factored
 *
 * @return  Returns the info variable from lapack
 */
int  BandMatrix::factorQR()
{
    factor();
    return 0;
}
//====================================================================================================================
// Factors the A matrix using the QR algorithm, overwriting A
// Returns an estimate of the inverse of the condition number for the matrix
/*
 *   The matrix must have been previously factored using the QR algorithm
 *
 * @return  returns the inverse of the condition number
 */
doublereal  BandMatrix::rcondQR()
{
    double a1norm = oneNorm();
    return rcond(a1norm);
}
//====================================================================================================================
// Returns an estimate of the inverse of the condition number for the matrix
/*
 *   The matrix must have been previously factored using the LU algorithm
 *
 * @param a1norm Norm of the matrix
 *
 * @return  returns the inverse of the condition number
 */
doublereal  BandMatrix::rcond(doublereal a1norm)
{
    int printLevel = 0;
    int useReturnErrorCode = 0;
    if ((int) iwork_.size() < m_n) {
        iwork_.resize(m_n);
    }
    if ((int) work_.size() < 3 * m_n) {
        work_.resize(3 * m_n);
    }
    doublereal rcond = 0.0;
    if (m_factored != 1) {
        throw CanteraError("BandMatrix::rcond()", "matrix isn't factored correctly");
    }

    // doublereal anorm = oneNorm();
    int ldab = (2 *m_kl + m_ku + 1);
    int rinfo;
    rcond = ct_dgbcon('1', m_n, m_kl, m_ku, DATA_PTR(ludata), ldab, DATA_PTR(m_ipiv), a1norm, DATA_PTR(work_),
                      DATA_PTR(iwork_), rinfo);
    if (rinfo != 0) {
        if (printLevel) {
            writelogf("BandMatrix::rcond(): DGBCON returned INFO = %d\n", rinfo);
        }
        if (! useReturnErrorCode) {
            throw CanteraError("BandMatrix::rcond()", "DGBCON returned INFO = " + int2str(rinfo));
        }
    }
    return rcond;
}
//====================================================================================================================
// Change the way the matrix is factored
/*
 *  @param fAlgorithm   integer
 *                   0 LU factorization
 *                   1 QR factorization
 */
void BandMatrix::useFactorAlgorithm(int fAlgorithm)
{
    // QR algorithm isn't implemented for banded matrix.
}
//====================================================================================================================
int BandMatrix::factorAlgorithm() const
{
    return 0;
}
//====================================================================================================================
// Returns the one norm of the matrix
doublereal BandMatrix::oneNorm() const
{
    doublereal value = 0.0;
    for (int j = 0; j < m_n; j++) {
        doublereal sum = 0.0;
        doublereal* colP =  m_colPtrs[j];
        for (int i = j - m_ku; i <= j + m_kl; i++) {
            sum += fabs(colP[m_kl + m_ku + i - j]);
        }
        if (sum > value) {
            value = sum;
        }
    }
    return value;
}
//====================================================================================================================
int BandMatrix::checkRows(doublereal& valueSmall) const
{
    valueSmall = 1.0E300;
    int iSmall = -1;
    double vv;
    for (int i = 0; i < m_n; i++) {
        double valueS = 0.0;
        for (int j = i - m_kl; j <= i + m_ku; j++) {
            if (j >= 0 && (j < m_n)) {
                vv = fabs(value(i,j));
                if (vv > valueS) {
                    valueS = vv;
                }
            }
        }
        if (valueS < valueSmall) {
            iSmall = i;
            valueSmall = valueS;
            if (valueSmall == 0.0) {
                return iSmall;
            }
        }
    }
    return iSmall;
}
//====================================================================================================================
int BandMatrix::checkColumns(doublereal& valueSmall) const
{
    valueSmall = 1.0E300;
    int jSmall = -1;
    double vv;
    for (int j = 0; j < m_n; j++) {
        double valueS = 0.0;
        for (int i = j - m_ku; i <= j + m_kl; i++) {
            if (i >= 0 && (i < m_n)) {
                vv = fabs(value(i,j));
                if (vv > valueS) {
                    valueS = vv;
                }
            }
        }
        if (valueS < valueSmall) {
            jSmall = j;
            valueSmall = valueS;
            if (valueSmall == 0.0) {
                return jSmall;
            }
        }
    }
    return jSmall;
}
//====================================================================================================================
GeneralMatrix* BandMatrix::duplMyselfAsGeneralMatrix() const
{
    BandMatrix* dd = new BandMatrix(*this);
    return static_cast<GeneralMatrix*>(dd);
}
//====================================================================================================================
bool BandMatrix::factored() const
{
    return m_factored;
}
//====================================================================================================================
// Return a pointer to the top of column j, columns are assumed to be contiguous in memory
/*
 *  @param j   Value of the column
 *
 *  @return  Returns a pointer to the top of the column
 */
doublereal* BandMatrix::ptrColumn(int j)
{
    return m_colPtrs[j];
}
//====================================================================================================================
// Return a vector of const pointers to the columns
/*
 *  Note the value of the pointers are protected by their being const.
 *  However, the value of the matrix is open to being changed.
 *
 *   @return returns a vector of pointers to the top of the columns
 *           of the matrices.
 */
doublereal*   const* BandMatrix::colPts()
{
    return &(m_colPtrs[0]);
}
//====================================================================================================================
// Copy the data from one array into another without doing any checking
/*
 *  This differs from the assignment operator as no resizing is done and memcpy() is used.
 *  @param y Array to be copied
 */
void BandMatrix::copyData(const GeneralMatrix& y)
{
    m_factored = false;
    size_t n = sizeof(doublereal) * m_n * (2 *m_kl + m_ku + 1);
    GeneralMatrix* yyPtr = const_cast<GeneralMatrix*>(&y);
    (void) memcpy(DATA_PTR(data), yyPtr->ptrColumn(0), n);
}
//====================================================================================================================
/*
 * clear the factored flag
 */
void BandMatrix::clearFactorFlag()
{
    m_factored = 0;
}
//====================================================================================================================
//====================================================================================================================
}

