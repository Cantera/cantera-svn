#include "ctmatutils.h"
#include "clib/ctfunc.h"
#include "clib/ct.h"

void funcmethods(int nlhs, mxArray* plhs[],
                 int nrhs, const mxArray* prhs[])
{
    int job = getInt(prhs[1]);
    int nn;
    double* ptr = 0;

    // constructor
    if (job == 0) {
        int type = getInt(prhs[2]);
        int n = getInt(prhs[3]);
        if (type < 20) {
            ptr = mxGetPr(prhs[4]);
            size_t msize = mxGetM(prhs[4]);
            size_t nsize = mxGetN(prhs[4]);
            size_t lenp = msize*nsize;
            nn = func_new(type, n, lenp, ptr);
        } else if (type < 45) {
            int m = getInt(prhs[4]);
            nn = func_new(type, n, m, ptr);
        } else {
            ptr = mxGetPr(prhs[4]);
            nn = func_new(type, n, 0, ptr);
        }
        plhs[0] = mxCreateNumericMatrix(1,1,mxDOUBLE_CLASS,mxREAL);
        double* h = mxGetPr(plhs[0]);
        *h = double(nn);
        if (nn < 0) {
            reportError();
        }
        return;
    }

    else {
        int nn = 0;
        double t;
        double v;
        int i = getInt(prhs[2]);
        if (job == 1) {
            nn = func_del(i);
            if (nn < 0) {
                reportError();
            }
            v = double(nn);
        } else if (job == 2) {
            t = getDouble(prhs[3]);
            v = func_value(i, t);
            if (v == Undef) {
                reportError();
            }
        } else {
            mexErrMsgTxt("unknown job parameter");
        }
        plhs[0] = mxCreateNumericMatrix(1,1,mxDOUBLE_CLASS,mxREAL);
        double* h = mxGetPr(plhs[0]);
        *h = v;
        return;
    }
}

