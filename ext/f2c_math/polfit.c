/* polfit.f -- translated by f2c (version 20030320).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* DECK POLFIT */
/* Subroutine */ int polfit_(integer *n, real *x, real *y, real *w, integer *
	maxdeg, integer *ndeg, real *eps, real *r__, integer *ierr, real *a)
{
    /* System generated locals */
    integer i__1;
    real r__1;

    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    integer i__, j, m, k1, k2, k3, k4, k5;
    real w1, w11, xm, yp;
    integer jp1;
    real sig;
    integer k1pj, k2pj, k4pi, k3pi, k5pi, mop1, nder;
    real sigj;
    integer jpas;
    real temp, etst;
    doublereal temd1, temd2;
    integer nfail;
    real sigjm1, sigpas;
    extern /* Subroutine */ int pvalue_(integer *, integer *, real *, real *, 
	    real *, real *);

/* ***BEGIN PROLOGUE  POLFIT */
/* ***PURPOSE  Fit discrete data in a least squares sense by polynomials */
/*            in one variable. */
/* ***LIBRARY   SLATEC */
/* ***CATEGORY  K1A1A2 */
/* ***TYPE      SINGLE PRECISION (POLFIT-S, DPOLFT-D) */
/* ***KEYWORDS  CURVE FITTING, DATA FITTING, LEAST SQUARES, POLYNOMIAL FIT */
/* ***AUTHOR  Shampine, L. F., (SNLA) */
/*           Davenport, S. M., (SNLA) */
/*           Huddleston, R. E., (SNLL) */
/* ***DESCRIPTION */

/*     Abstract */

/*     Given a collection of points X(I) and a set of values Y(I) which */
/*     correspond to some function or measurement at each of the X(I), */
/*     subroutine  POLFIT  computes the weighted least-squares polynomial */
/*     fits of all degrees up to some degree either specified by the user */
/*     or determined by the routine.  The fits thus obtained are in */
/*     orthogonal polynomial form.  Subroutine  PVALUE  may then be */
/*     called to evaluate the fitted polynomials and any of their */
/*     derivatives at any point.  The subroutine  PCOEF  may be used to */
/*     express the polynomial fits as powers of (X-C) for any specified */
/*     point C. */

/*     The parameters for  POLFIT  are */

/*     Input -- */
/*         N -      the number of data points.  The arrays X, Y and W */
/*                  must be dimensioned at least  N  (N .GE. 1). */
/*         X -      array of values of the independent variable.  These */
/*                  values may appear in any order and need not all be */
/*                  distinct. */
/*         Y -      array of corresponding function values. */
/*         W -      array of positive values to be used as weights.  If */
/*                  W(1) is negative,  POLFIT  will set all the weights */
/*                  to 1.0, which means unweighted least squares error */
/*                  will be minimized.  To minimize relative error, the */
/*                  user should set the weights to:  W(I) = 1.0/Y(I)**2, */
/*                  I = 1,...,N . */
/*         MAXDEG - maximum degree to be allowed for polynomial fit. */
/*                  MAXDEG  may be any non-negative integer less than  N. */
/*                  Note -- MAXDEG  cannot be equal to  N-1  when a */
/*                  statistical test is to be used for degree selection, */
/*                  i.e., when input value of  EPS  is negative. */
/*         EPS -    specifies the criterion to be used in determining */
/*                  the degree of fit to be computed. */
/*                  (1)  If  EPS  is input negative,  POLFIT  chooses the */
/*                       degree based on a statistical F test of */
/*                       significance.  One of three possible */
/*                       significance levels will be used:  .01, .05 or */
/*                       .10.  If  EPS=-1.0 , the routine will */
/*                       automatically select one of these levels based */
/*                       on the number of data points and the maximum */
/*                       degree to be considered.  If  EPS  is input as */
/*                       -.01, -.05, or -.10, a significance level of */
/*                       .01, .05, or .10, respectively, will be used. */
/*                  (2)  If  EPS  is set to 0.,  POLFIT  computes the */
/*                       polynomials of degrees 0 through  MAXDEG . */
/*                  (3)  If  EPS  is input positive,  EPS  is the RMS */
/*                       error tolerance which must be satisfied by the */
/*                       fitted polynomial.  POLFIT  will increase the */
/*                       degree of fit until this criterion is met or */
/*                       until the maximum degree is reached. */

/*     Output -- */
/*         NDEG -   degree of the highest degree fit computed. */
/*         EPS -    RMS error of the polynomial of degree  NDEG . */
/*         R -      vector of dimension at least NDEG containing values */
/*                  of the fit of degree  NDEG  at each of the  X(I) . */
/*                  Except when the statistical test is used, these */
/*                  values are more accurate than results from subroutine */
/*                  PVALUE  normally are. */
/*         IERR -   error flag with the following possible values. */
/*             1 -- indicates normal execution, i.e., either */
/*                  (1)  the input value of  EPS  was negative, and the */
/*                       computed polynomial fit of degree  NDEG */
/*                       satisfies the specified F test, or */
/*                  (2)  the input value of  EPS  was 0., and the fits of */
/*                       all degrees up to  MAXDEG  are complete, or */
/*                  (3)  the input value of  EPS  was positive, and the */
/*                       polynomial of degree  NDEG  satisfies the RMS */
/*                       error requirement. */
/*             2 -- invalid input parameter.  At least one of the input */
/*                  parameters has an illegal value and must be corrected */
/*                  before  POLFIT  can proceed.  Valid input results */
/*                  when the following restrictions are observed */
/*                       N .GE. 1 */
/*                       0 .LE. MAXDEG .LE. N-1  for  EPS .GE. 0. */
/*                       0 .LE. MAXDEG .LE. N-2  for  EPS .LT. 0. */
/*                       W(1)=-1.0  or  W(I) .GT. 0., I=1,...,N . */
/*             3 -- cannot satisfy the RMS error requirement with a */
/*                  polynomial of degree no greater than  MAXDEG .  Best */
/*                  fit found is of degree  MAXDEG . */
/*             4 -- cannot satisfy the test for significance using */
/*                  current value of  MAXDEG .  Statistically, the */
/*                  best fit found is of order  NORD .  (In this case, */
/*                  NDEG will have one of the values:  MAXDEG-2, */
/*                  MAXDEG-1, or MAXDEG).  Using a higher value of */
/*                  MAXDEG  may result in passing the test. */
/*         A -      work and output array having at least 3N+3MAXDEG+3 */
/*                  locations */

/*     Note - POLFIT  calculates all fits of degrees up to and including */
/*            NDEG .  Any or all of these fits can be evaluated or */
/*            expressed as powers of (X-C) using  PVALUE  and  PCOEF */
/*            after just one call to  POLFIT . */

/* ***REFERENCES  L. F. Shampine, S. M. Davenport and R. E. Huddleston, */
/*                 Curve fitting by polynomials in one variable, Report */
/*                 SLA-74-0270, Sandia Laboratories, June 1974. */
/* ***ROUTINES CALLED  PVALUE, XERMSG */
/* ***REVISION HISTORY  (YYMMDD) */
/*   740601  DATE WRITTEN */
/*   890531  Changed all specific intrinsics to generic.  (WRB) */
/*   890531  REVISION DATE from Version 3.2 */
/*   891214  Prologue converted to Version 4.0 format.  (BAB) */
/*   900315  CALLs to XERROR changed to CALLs to XERMSG.  (THJ) */
/*   920501  Reformatted the REFERENCES section.  (WRB) */
/*   920527  Corrected erroneous statements in DESCRIPTION.  (WRB) */
/* ***END PROLOGUE  POLFIT */
/*      DIMENSION CO(4,3) */
/*      SAVE CO */
/*      DATA  CO(1,1), CO(2,1), CO(3,1), CO(4,1), CO(1,2), CO(2,2), */
/*     1      CO(3,2), CO(4,2), CO(1,3), CO(2,3), CO(3,3), */
/*     2  CO(4,3)/-13.086850,-2.4648165,-3.3846535,-1.2973162, */
/*     3          -3.3381146,-1.7812271,-3.2578406,-1.6589279, */
/*     4          -1.6282703,-1.3152745,-3.2640179,-1.9829776/ */
/* ***FIRST EXECUTABLE STATEMENT  POLFIT */
    /* Parameter adjustments */
    --a;
    --r__;
    --w;
    --y;
    --x;

	/* Uninitialized local variables -> note, I don't see how this
	 * function can be working */
	k1=0;
	k2 = 0;
	k3 = 0;
	k4 = 0;
	k5 = 0;
	etst = 1.0E-13f;
	xm = 1.0;

    /* Function Body */
    m = abs(*n);
    if (m == 0) {
	goto L30;
    }
    if (*maxdeg < 0) {
	goto L30;
    }
    a[1] = (real) (*maxdeg);
    mop1 = *maxdeg + 1;
    if (m < mop1) {
	goto L30;
    }
    if (*eps < 0.f && m == mop1) {
	goto L30;
    }
    j = 0;

/* SEE IF POLYNOMIAL OF DEGREE 0 SATISFIES THE DEGREE SELECTION CRITERION */

    if (*eps < 0.f) {
	goto L24;
    } else if (*eps == 0) {
	goto L26;
    } else {
	goto L27;
    }

/* INCREMENT DEGREE */

L16:
    ++j;
    jp1 = j + 1;
    k1pj = k1 + j;
    k2pj = k2 + j;
    sigjm1 = sigj;

/* COMPUTE NEW B COEFFICIENT EXCEPT WHEN J = 1 */

    if (j > 1) {
	a[k1pj] = w11 / w1;
    }

/* COMPUTE NEW A COEFFICIENT */

    temd1 = 0.;
    i__1 = m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	k4pi = k4 + i__;
	temd2 = a[k4pi];
	temd1 += (doublereal) x[i__] * (doublereal) w[i__] * temd2 * temd2;
/* L18: */
    }
    a[jp1] = (real) (temd1 / w11);

/* EVALUATE ORTHOGONAL POLYNOMIAL AT DATA POINTS */

    w1 = w11;
    w11 = 0.f;
    i__1 = m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	k3pi = k3 + i__;
	k4pi = k4 + i__;
	temp = a[k3pi];
	a[k3pi] = a[k4pi];
	a[k4pi] = (x[i__] - a[jp1]) * a[k3pi] - a[k1pj] * temp;
/* L19: */
/* Computing 2nd power */
	r__1 = a[k4pi];
	w11 += w[i__] * (r__1 * r__1);
    }

/* GET NEW ORTHOGONAL POLYNOMIAL COEFFICIENT USING PARTIAL DOUBLE */
/* PRECISION */

    temd1 = 0.;
    i__1 = m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	k4pi = k4 + i__;
	k5pi = k5 + i__;
	temd2 = (doublereal) w[i__] * (doublereal) (y[i__] - r__[i__] - a[
		k5pi]) * (doublereal) a[k4pi];
/* L20: */
	temd1 += temd2;
    }
    temd1 /= (doublereal) w11;
    a[k2pj + 1] = (real) temd1;

/* UPDATE POLYNOMIAL EVALUATIONS AT EACH OF THE DATA POINTS, AND */
/* ACCUMULATE SUM OF SQUARES OF ERRORS.  THE POLYNOMIAL EVALUATIONS ARE */
/* COMPUTED AND STORED IN EXTENDED PRECISION.  FOR THE I-TH DATA POINT, */
/* THE MOST SIGNIFICANT BITS ARE STORED IN  R(I) , AND THE LEAST */
/* SIGNIFICANT BITS ARE IN  A(K5PI) . */

    sigj = 0.f;
    i__1 = m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	k4pi = k4 + i__;
	k5pi = k5 + i__;
	temd2 = (doublereal) r__[i__] + (doublereal) a[k5pi] + temd1 * (
		doublereal) a[k4pi];
	r__[i__] = (real) temd2;
	a[k5pi] = (real) (temd2 - r__[i__]);
/* L21: */
/* Computing 2nd power */
	r__1 = y[i__] - r__[i__] - a[k5pi];
	sigj += w[i__] * (r__1 * r__1);
    }

/* SEE IF DEGREE SELECTION CRITERION HAS BEEN SATISFIED OR IF DEGREE */
/* MAXDEG  HAS BEEN REACHED */

    if (*eps < 0.f) {
	goto L23;
    } else if (*eps == 0) {
	goto L26;
    } else {
	goto L27;
    }

/* COMPUTE F STATISTICS  (INPUT EPS .LT. 0.) */

L23:
    if (sigj == 0.f) {
	goto L29;
    }
/*      DEGF = M - J - 1 */
/*      DEN = (CO(4,KSIG)*DEGF + 1.0)*DEGF */
/*      FCRIT = (((CO(3,KSIG)*DEGF) + CO(2,KSIG))*DEGF + CO(1,KSIG))/DEN */
/*      FCRIT = FCRIT*FCRIT */
/*      F = (SIGJM1 - SIGJ)*DEGF/SIGJ */
/*      IF (F .LT. FCRIT) GO TO 25 */

/* POLYNOMIAL OF DEGREE J SATISFIES F TEST */

L24:
    sigpas = sigj;
    jpas = j;
    nfail = 0;
    if (*maxdeg == j) {
	goto L32;
    }
    goto L16;

/* POLYNOMIAL OF DEGREE J FAILS F TEST.  IF THERE HAVE BEEN THREE */
/* SUCCESSIVE FAILURES, A STATISTICALLY BEST DEGREE HAS BEEN FOUND. */

/* L25: */
    ++nfail;
    if (nfail >= 3) {
	goto L29;
    }
    if (*maxdeg == j) {
	goto L32;
    }
    goto L16;

/* RAISE THE DEGREE IF DEGREE  MAXDEG  HAS NOT YET BEEN REACHED  (INPUT */
/* EPS = 0.) */

L26:
    if (*maxdeg == j) {
	goto L28;
    }
    goto L16;

/* SEE IF RMS ERROR CRITERION IS SATISFIED  (INPUT EPS .GT. 0.) */

L27:
    if (sigj <= etst) {
	goto L28;
    }
    if (*maxdeg == j) {
	goto L31;
    }
    goto L16;

/* RETURNS */

L28:
    *ierr = 1;
    *ndeg = j;
    sig = sigj;
    goto L33;
L29:
    *ierr = 1;
    *ndeg = jpas;
    sig = sigpas;
    goto L33;
L30:
    *ierr = 2;
/*      CALL XERMSG ('SLATEC', 'POLFIT', 'INVALID INPUT PARAMETER.', 2, */
/*     +   1) */
    goto L37;
L31:
    *ierr = 3;
    *ndeg = *maxdeg;
    sig = sigj;
    goto L33;
L32:
    *ierr = 4;
    *ndeg = jpas;
    sig = sigpas;

L33:
    a[k3] = (real) (*ndeg);

/* WHEN STATISTICAL TEST HAS BEEN USED, EVALUATE THE BEST POLYNOMIAL AT */
/* ALL THE DATA POINTS IF  R  DOES NOT ALREADY CONTAIN THESE VALUES */

    if (*eps >= 0.f || *ndeg == *maxdeg) {
	goto L36;
    }
    nder = 0;
    i__1 = m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	pvalue_(ndeg, &nder, &x[i__], &r__[i__], &yp, &a[1]);
/* L35: */
    }
L36:
    *eps = (real) sqrt(sig / xm);
L37:
    return 0;
} /* polfit_ */

