/*
 * @file: RootFind.cpp  root finder for 1D problems
 */
/*
 * $Id$
 */
/*
 * Copywrite 2004 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * See file License.txt for licensing information.
 */

#include "ct_defs.h"
#include "RootFind.h"

#include "global.h"
#ifdef DEBUG_MODE
#include "mdp_allo.h"
#endif

/* Standard include files */

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <vector>

using namespace std;
namespace Cantera {



#ifndef MAX
#  define MAX(x,y) (( (x) > (y) ) ? (x) : (y))     /* max function */
#endif

#ifndef MIN
#  define MIN(x,y) (( (x) < (y) ) ? (x) : (y))     /* min function */
#endif

#ifndef SQUARE
#  define SQUARE(x) ( (x) * (x) )
#endif

#ifndef DSIGN
#define DSIGN(x) (( (x) == (0.0) ) ? (0.0) : ( ((x) > 0.0) ? 1.0 : -1.0 ))
#endif

  /*****************************************************************************/
  /*****************************************************************************/
  /*****************************************************************************/
#ifdef DEBUG_MODE
  //!  Print out a form for the current function evaluation
  /*!
   *  @param fp     Pointer to the FILE object
   *  @param xval   Current value of x
   *  @param fval   Current value of f
   *  @param its    Current iteration value
   */
  static void print_funcEval(FILE *fp, doublereal xval, doublereal fval, int its)  
  {
    fprintf(fp,"\n");
    fprintf(fp,"...............................................................\n");
    fprintf(fp,".................. RootFind Function Evaluation ...............\n");
    fprintf(fp,"..................  iteration = %5d ........................\n", its);
    fprintf(fp,"..................  value = %12.5g ......................\n", xval);
    fprintf(fp,"..................  funct = %12.5g ......................\n", fval);
    fprintf(fp,"...............................................................\n");  
    fprintf(fp,"\n");
  }
#endif
  //================================================================================================
  //! Solve Ax = b using gauss's method
  /*!
   *     @param c      Matrix
   *     @param idem   Assumed number of rows in the matrix
   *     @param n      Number of rows and columns
   *     @param b      right hand side
   *     @param m      Number of right hand sides
   */
  static int smlequ(doublereal *c, int idem, int n, doublereal *b, int m) {
    int i, j, k, l;
    doublereal R;
    if (n > idem || n <= 0) {
      writelogf("smlequ ERROR: badly dimensioned matrix: %d %d\n", n, idem);
      return 1;
    }

    /*
     * Loop over the rows
     *    -> At the end of each loop, the only nonzero entry in the column
     *       will be on the diagonal. We can therfore just invert the
     *       diagonal at the end of the program to solve the equation system.
     */
    for (i = 0; i < n; ++i) {
      if (c[i + i * idem] == 0.0) {
        /*
         *   Do a simple form of row pivoting to find a non-zero pivot
         */
        for (k = i + 1; k < n; ++k) {
          if (c[k + i * idem] != 0.0) goto FOUND_PIVOT;
        }
        writelogf("smlequ ERROR: Encountered a zero column: %d\n", i);
        return 1;
      FOUND_PIVOT: ;
        for (j = 0; j < n; ++j) c[i + j * idem] += c[k + j * idem];
        for (j = 0; j < m; ++j) b[i + j * idem] += b[k + j * idem];
      }

      for (l = 0; l < n; ++l) {
        if (l != i && c[l + i * idem] != 0.0) {
          R = c[l + i * idem] / c[i + i * idem];
          c[l + i * idem] = 0.0;
          for (j = i+1; j < n; ++j) c[l + j * idem] -= c[i + j * idem] * R;
          for (j = 0; j < m; ++j)   b[l + j * idem] -= b[i + j * idem] * R;
        }
      }
    }
    /*
     *  The negative in the last expression is due to the form of B upon
     *  input
     */
    for (i = 0; i < n; ++i) {
      for (j = 0; j < m; ++j) {
        b[i + j * idem] = -b[i + j * idem] / c[i + i*idem];
      }
    }
    return 0;
  }
  //================================================================================================
  // Main constructor
  RootFind::RootFind (ResidEval* resid) :
    m_residFunc(resid),
    m_funcTargetValue(0.0),
    m_atol(1.0E-11),
    m_rtol(1.0E-5),
    m_maxstep(1000),
    printLvl(0),
    DeltaXnorm_(0.01),
    FuncIsGenerallyIncreasing_(false),
    FuncIsGenerallyDecreasing_(false),
    deltaXConverged_(0.0)
  {
  
  }
  //================================================================================================ 
  // Empty destructor
  RootFind::~RootFind() {
  }
  //================================================================================================
  // Calculate a deltaX from an input value of x
  /*
   *  This routine ensure that the deltaX will be greater or equal to DeltaXNorm_
   *  or 1.0E-14 x
   *
   * @param x1  input value of x
   */
  doublereal RootFind::delXNonzero(doublereal x1) const {
    doublereal deltaX = 1.0E-14 * fabs(x1);
    doublereal delmin = DeltaXnorm_ * 1.0E-14;
    if (delmin > deltaX) {
      return delmin;
    }
    return deltaX;
  }
  //================================================================================================
  // Calculate a deltaX from an input value of x
  /*
   *  This routine ensure that the deltaX will be greater or equal to DeltaXNorm_
   *  or 1.0E-14 x or deltaXConverged_.
   *
   * @param x1  input value of x
   */
  doublereal RootFind::delXMeaningful(doublereal x1) const {
    doublereal del = delXNonzero(x1);
    if (deltaXConverged_ > del) {
      return deltaXConverged_;
    }
    return del;
  }
  //================================================================================================ 
  // Calcuated a controlled, nonzero delta between two numbers
  /*
   *  The delta is designed to be greater than or equal to delXMeaningful(x) defined above
   *  with the same sign as the original delta. Therefore if you subtract it from either
   *  of the two original numbers, you get a different number.
   *
   *  @param x2   first number
   *  @param x2   second number
   */
  double RootFind::deltaXControlled(doublereal x2, doublereal x1) const {
    doublereal sgnn = 1.0;
    if (x1 > x2) {
      sgnn = -1.0;
    }
    doublereal deltaX = x2 - x1;
    doublereal x = fabs(x2) + fabs(x1);
    doublereal deltaXm = delXMeaningful(x);
    if (fabs(deltaX) < deltaXm) {
      deltaX = sgnn * deltaXm;
    }
    return deltaX;
  }
  //====================================================================================================================
  // Function to decide whether two real numbers are the same or not
  /*
   *  A comparison is made between the two numbers to decide whether they
   *  are close to one another. This is defined as being within delXMeaningful() of each other
   *
   * @param x2  First number
   * @param x2  second number
   *
   * @return Returns a boolean indicating whether the two numbers are the same or not.
   */
  bool RootFind::theSame(doublereal x2, doublereal x1) const {
    doublereal x = fabs(x2) + fabs(x1);
    doublereal deltaX = delXMeaningful(x);
    if (fabs(x2 - x1) < deltaX) {
      return true;
    }
    return false;
  }
  //====================================================================================================================
  /*
   * The following calculation is a line search method to find the root of a function
   * 
   *
   *   xbest   Returns the x that satisfies the function
   *           On input, xbest should contain the best estimate
   *
   *   return:
   *    0  Found function
   */
  int RootFind::solve(doublereal xmin, doublereal xmax, int itmax, doublereal &funcTargetValue, doublereal *xbest) {

    /*
     *   We store the function target and then actually calculate a modified functional
     *
     *       func = eval(x1) -  m_funcTargetValue = 0
     */
    m_funcTargetValue = funcTargetValue;

    static int callNum = 0;
    const char *stre = "RootFind ERROR: ";
    const char *strw = "RootFind WARNING: ";
    int converged = 0;
    int bottomBump = 0;
    int topBump = 0;
#ifdef DEBUG_MODE
    char fileName[80];
    FILE *fp = 0;
#endif
    doublereal x1, x2, xnew, f1, f2, fnew, slope;
    doublereal deltaX1 = 0.0, deltaX2 = 0.0, deltaXnew = 0.0;
    int its = 0;
    int posStraddle = 0;
    int retn = 0;
    int foundPosF = 0;
    int foundNegF = 0;
    int foundStraddle = 0;
    doublereal xPosF = 0.0;
    doublereal xNegF = 0.0;
    doublereal fnorm;   /* A valid norm for the making the function value  dimensionless */
    doublereal c[9], f[3], xn1, xn2, x0 = 0.0, f0 = 0.0, root, theta, xquad, xDelMin;
    doublereal CR0, CR1, CR2, CRnew, CRdenom;
    doublereal sgn;

    callNum++;
#ifdef DEBUG_MODE
    if (printLvl >= 3) {
      sprintf(fileName, "RootFind_%d.log", callNum);
      fp = fopen(fileName, "w");
      fprintf(fp, " Iter   TP_its  xval   Func_val  |  Reasoning\n");
      fprintf(fp, "-----------------------------------------------------"
	      "-------------------------------\n");
    }
#else
    if (printLvl >= 3) {
      writelog("WARNING: RootFind: printlvl >= 3, but debug mode not turned on\n");
    }
#endif
    if (xmax <= xmin) {
      writelogf("%sxmin and xmax are bad: %g %g\n", stre, xmin, xmax);
      funcTargetValue = func(*xbest);
      return ROOTFIND_BADINPUT;
    }
    /*
     *  Find the first function value f1 = func(x1), by using the value entered into xbest.
     *  Process it 
     */
    x1 = *xbest;
    if (x1 < xmin || x1 > xmax) {
      x1 = (xmin + xmax) / 2.0;     
    }

    f1 = func(x1);

#ifdef DEBUG_MODE
    if (printLvl >= 3) {
      print_funcEval(fp, x1, f1, its); 
      fprintf(fp, "%-5d  %-5d  %-15.5E %-15.5E\n", -2, 0, x1, f1);
    }
#endif

    if (f1 == 0.0) {
      *xbest = x1;
      return 0; 
    } else if (f1 > 0.0) {
      foundPosF = 1;
      xPosF = x1;
    } else {
      foundNegF = 1;
      xNegF = x1;
    }

    if (x1 == 0.0) {
      x2 = 0.00001 * (xmax - xmin);
    } else {
      x2 = x1 * 1.0001;
    }
    if (x2 > xmax) {
      x2 = x1 - (xmax - xmin) / 100.;
    }

    deltaX2 = x2 - x1;
    f2 = func(x2);
#ifdef DEBUG_MODE
    if (printLvl >= 3) {
      print_funcEval(fp, x2, f2, its);
      fprintf(fp, "%-5d  %-5d  %-15.5E %-15.5E", -1, 0, x2, f2);
    }
#endif
 
    if (m_funcTargetValue != 0.0) {
      fnorm =  1.0E-6 + m_atol / m_rtol;
    } else {
      fnorm = 0.5*(fabs(f1) + fabs(f2)) + fabs(m_funcTargetValue);
    }

    if (f2 == 0.0) {
      *xbest = x2;
      return retn;
    } else if (f2 > 0.0) {
      if (!foundPosF) {
	foundPosF = 1;
	xPosF = x2;
      }
    } else {
      if (!foundNegF) {
	foundNegF = 1;
	xNegF = x2;
      }
    }
    /*
     *  See if we have already achieved a straddle
     */
    foundStraddle = foundPosF && foundNegF;
    if (foundStraddle) {
      if (xPosF > xNegF) posStraddle = 1; 
     else               posStraddle = 0    ;
    }
    bool doQuad = false;
    bool useNextStrat = false;
    bool slopePointingToHigher = true;
    // ---------------------------------------------------------------------------------------------
    //                MAIN LOOP
    // ---------------------------------------------------------------------------------------------
    do {
      /*
       *    Find an estimate of the next point, xnew, to try based on
       *    a linear approximation from the last two points.  
       */
#ifdef DEBUG_HKM
      if (fabs(x2 - x1) < 1.0E-14) {
	printf(" RootFind: we are here x2 = %g x1 = %g\n", x2, x1);
      }
#endif
      doublereal delXtmp = deltaXControlled(x2, x1);
      slope = (f2 - f1) / delXtmp;
      if (fabs(slope) <= 1.0E-100) {
	if (printLvl >= 2) {
	  writelogf("%s functions evals produced the same result, %g, at %g and %g\n",
		    strw, f2, x1, x2);
	}
	xnew = x2 +  DeltaXnorm_;
	slopePointingToHigher = true;
      } else {
        useNextStrat = false;
	xnew = x2 - f2 / slope; 
	if (xnew > x2) {
	  slopePointingToHigher = true;
	} else {
	  slopePointingToHigher = false;
	}
      }
#ifdef DEBUG_MODE
      if (printLvl >= 3) {
	fprintf(fp, " | xlin = %-11.5E", xnew);
      }
#endif
      deltaXnew = xnew - x2;
      /*
       * If the suggested step size is too big, throw out step
       */
      if (!foundStraddle) {
	if (fabs(xnew - x2) > 3.0 * DeltaXnorm_) {
	  useNextStrat = true;
	}
	if (fabs(deltaXnew) < fabs(deltaX2)) {
	  deltaXnew = DSIGN(deltaXnew) * 1.1 * fabs(deltaX2);
	  xnew = deltaXnew + x2;
	}
      }
      if (useNextStrat) {
	if (f2 < 0.0) {
	  if (FuncIsGenerallyIncreasing_) {
	    if (slopePointingToHigher) {
	      xnew = MIN(x2 + 3.0*DeltaXnorm_, xnew);
	    } else {
	      xnew = x2 + DeltaXnorm_;
	    }
	  } else if (FuncIsGenerallyDecreasing_) {
	    if ( !slopePointingToHigher) {
	      xnew = MAX(x2 - 3.0*DeltaXnorm_, xnew);
	    } else {
	      xnew = x2 - DeltaXnorm_;
	    }
	  } else {
	    if (slopePointingToHigher) {
	      xnew = x2 + DeltaXnorm_;
	    } else {
	      xnew = x2 - DeltaXnorm_;
	    }
	  }
	} else {
	  if (FuncIsGenerallyDecreasing_) {
	    if (!slopePointingToHigher) {
	      xnew = MAX(x2 + 3.0*DeltaXnorm_, xnew);
	    } else {
	      xnew = x2 + DeltaXnorm_;
	    }
	  } else if (FuncIsGenerallyIncreasing_) {
	    if (! slopePointingToHigher) {
	      xnew = MIN(x2 - 3.0*DeltaXnorm_, xnew);
	    } else {
	      xnew = x2 - DeltaXnorm_;
	    }
	  } else {
	    if (slopePointingToHigher) {
	      xnew = x2 + DeltaXnorm_;
	    } else {
	      xnew = x2 - DeltaXnorm_;
	    }
	  }
	}
      }


      /*
       *  Do a quadratic fit -> Note this algorithm seems
       *  to work OK. The quadratic approximation doesn't kick in until
       *  the end of the run, when it becomes reliable.
       */
      if (its > 0 && doQuad) {
	c[0] = 1.; c[1] = 1.; c[2] = 1.;
	c[3] = x0; c[4] = x1; c[5] = x2;
	c[6] = SQUARE(x0); c[7] = SQUARE(x1); c[8] = SQUARE(x2);
	f[0] = - f0; f[1] = - f1; f[2] = - f2;
	retn = smlequ(c, 3, 3, f, 1);
	if (retn == 1) goto QUAD_BAIL;
	root = f[1]* f[1] - 4.0 * f[0] * f[2];
	if (root >= 0.0) {
	  xn1 = (- f[1] + sqrt(root)) / (2.0 * f[2]);
	  xn2 = (- f[1] - sqrt(root)) / (2.0 * f[2]);	
	  if (fabs(xn2 - x2) < fabs(xn1 - x2) && xn2 > 0.0 ) xquad = xn2;
	  else                                               xquad = xn1;
	  theta = fabs(xquad - xnew) / fabs(xnew - x2);
	  theta = MIN(1.0, theta);
	  xnew = theta * xnew + (1.0 - theta) * xquad;
#ifdef DEBUG_MODE
	  if (printLvl >= 3) {
	    if (theta != 1.0) {
	      fprintf(fp, " | xquad = %-11.5E", xnew);
	    }
	  }
#endif
	} else {
	  /*
	   *   Pick out situations where the convergence may be
	   *   accelerated.
	   */
	  if ((DSIGN(xnew - x2) == DSIGN(x2 - x1)) &&
	      (DSIGN(x2   - x1) == DSIGN(x1 - x0))    ) {
	    xnew += xnew - x2;
#ifdef DEBUG_MODE
	    if (printLvl >= 3) {
	      fprintf(fp, " | xquada = %-11.5E", xnew);
	    }
#endif
	  }
	}
      }
    QUAD_BAIL: ;
      
      /*   
       *  OK, we have an estimate xnew.
       *
       *
       *  Put heuristic bounds on the step jump
       */
      if ((xnew > x1 && xnew < x2) || (xnew < x1 && xnew > x2)) {
	/*
	 *   If we are doing a jump in between the two previous points, make sure
	 *   the new trial is no closer that 10% of the distances between x2-x1 to
	 *   any of the original points.
	 */
	xDelMin = fabs(x2 - x1) / 10.;
	if (fabs(xnew - x1) < xDelMin) {
	  xnew = x1 + DSIGN(xnew-x1) * xDelMin;
#ifdef DEBUG_MODE
	  if (printLvl >= 3) {
	    fprintf(fp, " | x10%% = %-11.5E", xnew);
	  }
#endif
	}
	if (fabs(xnew - x2) < 0.1 * xDelMin) {
	  xnew = x2 + DSIGN(xnew-x2) * 0.1 *  xDelMin; 
#ifdef DEBUG_MODE
	  if (printLvl >= 3) {
	    fprintf(fp, " | x10%% = %-11.5E", xnew);
	  }
#endif
	}
      } else {
	/*
	 *   If we are venturing into new ground, only allow the step jump
	 *   to increase by 50% at each interation
	 */
	doublereal xDelMax = 1.5 * fabs(x2 - x1);
	if (fabs(xDelMax) < fabs(xnew - x2)) {
	  xnew = x2 + DSIGN(xnew-x2) * xDelMax;
#ifdef DEBUG_MODE
	  if (printLvl >= 3) {
	    fprintf(fp, " | xlimitsize = %-11.5E", xnew);
	  }
#endif
	}
      }
 
      if (foundStraddle) {
#ifdef DEBUG_MODE
	slope = xnew;	 
#endif
	if (posStraddle) {
	  if (f2 > 0.0) {
	    if (xnew > x2) {
	      xnew = (xNegF + x2)/2;
	    }
	    if (xnew < xNegF) {
	      xnew = (xNegF + x2)/2;
	    }
	  } else {
	    if (xnew < x2) {
	      xnew = (xPosF + x2)/2;
	    }
	    if (xnew > xPosF) {
	      xnew = (xPosF + x2)/2;
	    }
	  }
	} else {
	  if (f2 > 0.0) {
	    if (xnew < x2)  {
	      xnew = (xNegF + x2)/2;
	    }
	    if (xnew > xNegF) {
	      xnew = (xNegF + x2)/2;
	    }
	  } else {
	    if (xnew > x2)  {
	      xnew = (xPosF + x2)/2;
	    }
	    if (xnew < xPosF) {
	      xnew = (xPosF + x2)/2;
	    }
	  }
	}
#ifdef DEBUG_MODE
	if (printLvl >= 3) {
	  if (slope != xnew) {
	    fprintf(fp, " | xstraddle = %-11.5E", xnew);	    
	  }
	}
#endif	
      }

      /*
       *  Enforce a minimum stepsize if we haven't found a straddle.
       */
      deltaXnew = xnew - x2;
      if (fabs(deltaXnew) < 1.2 * delXMeaningful(xnew)) {
	if (!foundStraddle) {
	  sgn = 1.0;
	  if (x2 < xnew) {
	    sgn = -1.0;
	  }
	  deltaXnew = 1.2 * delXMeaningful(xnew) * sgn;
	  xnew = x2 + deltaXnew;
	}
      }

      /*
       *  Guard against going above xmax or below xmin
       */
      if (xnew > xmax) {
	topBump++;
	if (topBump < 5) {
	  xnew = x2 + (xmax - x2) / 2.0;
	} else {
	  if (x2 == xmax || x1 == xmax) {
	    // we are here when we are bumping against the top limit.
	    // No further action is possible
	    goto done;
	  } else {
	    xnew = xmax;
	  }
	}
#ifdef DEBUG_MODE
	if (printLvl >= 3) {
	  fprintf(fp, " | xlimitmax = %-11.5E", xnew);
	}
#endif
      }
      if (xnew < xmin) {
	bottomBump++;
	if (bottomBump < 5) {
	  xnew = x2 + (x2 - xmin) / 2.0;
	} else {
	  if (x2 == xmin || x1 == xmin) {
	    // we are here when we are bumping against the bottom limit.
	    // No further action is possible
	    goto done;
	  } else {
	    xnew = xmin;
	  }
	}
#ifdef DEBUG_MODE
	if (printLvl >= 3) {
	  fprintf(fp, " | xlimitmin = %-11.5E", xnew);
	}
#endif
      }
    
      fnew = func(xnew);
      CRdenom = MAX(fabs(fnew), MAX(fabs(f2), MAX(fabs(f1), fnorm)));
      CRnew = sqrt(fabs(fnew) / CRdenom);
#ifdef DEBUG_MODE
      if (printLvl >= 3) {
	fprintf(fp,"\n");
	print_funcEval(fp, xnew, fnew, its);
	fprintf(fp, "%-5d  %-5d  %-15.5E %-15.5E", its, 0, xnew, fnew);
      }
#endif
      
      if (foundStraddle) {
	if (posStraddle) {
	  if (fnew > 0.0) {
	    if (xnew < xPosF) xPosF = xnew;
	  } else {
	    if (xnew > xNegF) xNegF = xnew;	       
	  }
	} else {
	  if (fnew > 0.0) {
	    if (xnew > xPosF) xPosF = xnew;	       
	  } else {
	    if (xnew < xNegF) xNegF = xnew;	  	       
	  }	   
	}
      }

      if (! foundStraddle) {
	if (fnew > 0.0) {
	  if (!foundPosF) {
	    foundPosF = 1;
	    xPosF = xnew;
	    foundStraddle = 1;
	    if (xPosF > xNegF) posStraddle = 1;
	    else    	       posStraddle = 0;
	  }	    
	} else {
	  if (!foundNegF) {
	    foundNegF = 1;
	    xNegF = xnew;
	    foundStraddle = 1;
	    if (xPosF > xNegF) posStraddle = 1;
	    else    	       posStraddle = 0;
	  }	   
	}
      }
      
      x0 = x1;
      f0 = f1;
      CR0 = CR1;
      x1 = x2;
      f1 = f2;
      CR1 = CR2;
      x2 = xnew; 
      f2 = fnew;
      deltaX1 = deltaX2;
      deltaX2 = deltaXnew;
      CR2 = CRnew;
      if (fabs(fnew / fnorm) < m_rtol) {
        converged = 1;	 
      }
      /*
       *  Check for excess convergence in the x coordinate
       */
      if (foundStraddle) {
	doublereal denom = fabs(x1) + fabs(x2);
	if (denom < 1.0E-200) {
	  retn = ROOTFIND_FAILEDCONVERGENCE;
	  converged = true;
	}
	if (theSame(x2, x1)) {
	  converged = true;
	}
      }
      its++;
    } while (! converged && its < itmax);

  done:
    if (converged) {
      if (printLvl >= 1) {
	writelogf("RootFind success: convergence achieved\n");
      }
#ifdef DEBUG_MODE
      if (printLvl >= 3) {
	fprintf(fp, " | RootFind success in %d its, fnorm = %g\n", its, fnorm);
      }
#endif  
    } else {
      retn = ROOTFIND_FAILEDCONVERGENCE;
      if (printLvl >= 1) {
	writelogf("RootFind ERROR: maximum iterations exceeded without convergence\n");
      }
#ifdef DEBUG_MODE
      if (printLvl >= 3) {
	fprintf(fp, "\nRootFind failure in %d its\n", its);
      }
#endif
    }
    *xbest = x2;
    funcTargetValue = f2 + m_funcTargetValue;
#ifdef DEBUG_MODE
    if (printLvl >= 3) {
      fclose(fp);
    }
#endif
    
    return retn;
  }
  //====================================================================================================================
  doublereal RootFind::func(doublereal x) {
    doublereal r;
#ifdef DEBUG_MODE
    mdp::checkFinite(x);
#endif
    m_residFunc->evalSS(0.0, &x, &r);
#ifdef DEBUG_MODE
    mdp::checkFinite(r);
#endif
    return (r - m_funcTargetValue);
  } 
  //====================================================================================================================
  // Set the tolerance parameters for the rootfinder
  /*
   *  These tolerance parameters are used on the function value to determine convergence
   *  
   *
   * @param rtol  Relative tolerance. The default is 10^-5
   * @param atol  absolute tolerance. The default is 10^-11
   */
  void RootFind::setTol(doublereal rtol, doublereal atol)
  {
    m_atol = atol;
    m_rtol = rtol;
  }
  //====================================================================================================================
  // Set the print level from the rootfinder
  /*
   * 
   *   0 -> absolutely nothing is printed for a single time step.
   *   1 -> One line summary per solve_nonlinear call
   *   2 -> short description, points of interest: Table of nonlinear solve - one line per iteration
   *   3 -> Table is included -> More printing per nonlinear iteration (default) that occurs during the table
   *   4 -> Summaries of the nonlinear solve iteration as they are occurring -> table no longer printed
   *   5 -> Algorithm information on the nonlinear iterates are printed out
   *   6 -> Additional info on the nonlinear iterates are printed out
   *   7 -> Additional info on the linear solve is printed out.
   *   8 -> Info on a per iterate of the linear solve is printed out.
   *
   *  @param printLvl  integer value
   */
  void RootFind::setPrintLvl(int printlvl) 
  {
    printLvl = printlvl;
  }
  //====================================================================================================================
  // Set the function behavior flag
  /*
   *  If this is true, the function is generally an increasing function of x.
   *  In particular, if the algorithm is seeking a higher value of f, it will look
   *  in the positive x direction.
   *
   *  This type of function is needed because this algorithm must deal with regions of f(x) where 
   *  f is not changing with x.
   *  
   *  @param value   boolean value
   */
  void RootFind::setFuncIsGenerallyIncreasing(bool value) 
  {
    if (value) {
      FuncIsGenerallyDecreasing_ = false;
    }
    FuncIsGenerallyIncreasing_ = value;
  }
  //====================================================================================================================
  // Set the function behavior flag
  /*
   *  If this is true, the function is generally a decreasing function of x.
   *  In particular, if the algorithm is seeking a higher value of f, it will look
   *  in the negative x direction.
   *
   *  This type of function is needed because this algorithm must deal with regions of f(x) where 
   *  f is not changing with x.
   *  
   *  @param value   boolean value
   */
  void RootFind::setFuncIsGenerallyDecreasing(bool value) 
  {
    if (value) {
      FuncIsGenerallyIncreasing_ = false;
    }
    FuncIsGenerallyDecreasing_ = value;
  }
  //====================================================================================================================
  // Set the minimum value of deltaX
  /*
   *  This sets the value of deltaXNorm_
   *
   *  @param deltaXNorm
   */
  void RootFind::setDeltaX(doublereal deltaXNorm) 
  {
    DeltaXnorm_ = deltaXNorm;
  }
  //====================================================================================================================
}
