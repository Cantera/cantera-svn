/**
 *  @file sort.cpp
 */

#include "sort.h"

namespace Cantera {

  // sort (x,y) pairs by x

  void heapsort(vector_fp& x, std::vector<size_t>& y) {
    size_t n = x.size();
    if (n < 2) return;
    doublereal rra;
    size_t rrb;
    size_t ll = n/2;
    size_t iret = n-1;
    
    while (1 > 0) {
      if (ll > 0) {
	ll--;
	rra = x[ll];
	rrb = y[ll];
      }
      else {
	rra = x[iret];
	rrb = y[iret];
	x[iret] = x[0];
	y[iret] = y[0];
	iret--;
	if (iret == 0) {
	  x[0] = rra;
	  y[0] = rrb;
	  return;
	}
      }
      
      size_t i = ll;
      size_t j = ll + ll + 1;
      
      while (j <= iret) {
	if (j < iret) {
	  if (x[j] < x[j+1])
	    j++;
	}
	if (rra < x[j]) {
	  x[i] = x[j];
	  y[i] = y[j];
	  i = j;
	  j = j + j + 1;
	}
	else {
	  j = iret + 1;
	}
      }
      x[i] = rra;
      y[i] = rrb;
    }
  }

  void heapsort(vector_fp& x, vector_fp& y) {
    size_t n = x.size();
    if (n < 2) return;
    doublereal rra;
    doublereal rrb;
    size_t ll = n/2;
    size_t iret = n-1;
    
    while (1 > 0) {
      if (ll > 0) {
	ll--;
	rra = x[ll];
	rrb = y[ll];
      }
      else {
	rra = x[iret];
	rrb = y[iret];
	x[iret] = x[0];
	y[iret] = y[0];
	iret--;
	if (iret == 0) {
	  x[0] = rra;
	  y[0] = rrb;
	  return;
	}
      }
      
      size_t i = ll;
      size_t j = ll + ll + 1;
      
      while (j <= iret) {
	if (j < iret) {
	  if (x[j] < x[j+1])
	    j++;
	}
	if (rra < x[j]) {
	  x[i] = x[j];
	  y[i] = y[j];
	  i = j;
	  j = j + j + 1;
	}
	else {
	  j = iret + 1;
	}
      }
      x[i] = rra;
      y[i] = rrb;
    }
  }

}

