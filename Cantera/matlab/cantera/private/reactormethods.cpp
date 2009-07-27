/**
 * @file reactormethods.cpp
 */
/*
 *  $Id: reactormethods.cpp,v 1.5 2009/07/11 16:43:12 hkmoffa Exp $
 */

#include "mex.h"
#include "../../../clib/src/ctreactor.h"
#include "../../../clib/src/ct.h"
#include "ctmatutils.h"

    void reactormethods( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
    {
        int iok, n;

        int job = getInt(prhs[1]);
        int i = getInt(prhs[2]);

        double r = Undef;
        double v = Undef;
        if (nrhs > 3) v = getDouble(prhs[3]);

        // constructor
        if (job == 0) {
            n = reactor_new(i);
            plhs[0] = mxCreateNumericMatrix(1,1,mxDOUBLE_CLASS,mxREAL);
            double *h = mxGetPr(plhs[0]);
            *h = double(n);
            if (n < 0) reportError();
            return;
        }

        // options that do not return a value

        if (job < 20) {
            switch (job) {

            case 1:
                iok = reactor_del(i);
                break;
            case 2:
                iok = reactor_copy(i);
                break;
            case 3:
                iok = reactor_assign(i,int(v));
                break;
            case 4:
                iok = reactor_setInitialVolume(i, v);
                break;
            case 5:
                iok = reactor_setInitialTime(i, v);
                break;
            case 6:
                iok = reactor_setThermoMgr(i, int(v));
                break;
            case 7:
                iok = reactor_setKineticsMgr(i, int(v));
                break;
                case 8:
                iok = reactor_advance(i, v);
                break;
            case 9:
                iok = reactor_setEnergy(i, int(v));
                break;
            default:
                mexErrMsgTxt("unknown job parameter");
            }
            plhs[0] = mxCreateNumericMatrix(1,1,mxDOUBLE_CLASS,mxREAL);
            double *h = mxGetPr(plhs[0]);
            *h = double(iok);
            if (iok < 0) reportError();
            return;
        }


        // options that return a value of type 'double'

        else if (job < 40) {
            switch (job) {
                case 21:
                r = reactor_step(i, v);
                break;
            case 22:
                r = reactor_time(i);
                break;
            case 23:
                r = reactor_mass(i);
                break;
            case 24:
                r = reactor_volume(i);
                break;
            case 25:
                r = reactor_density(i);
                break;
            case 26:
                r = reactor_temperature(i);
                break;
            case 27:
                r = reactor_enthalpy_mass(i);
                break;
            case 28:
                r = reactor_intEnergy_mass(i);
                break;
            case 29:
                r = reactor_pressure(i);
                break;
            case 30:
                r = reactor_massFraction(i, int(v));
                break;
            default:
                mexErrMsgTxt("unknown job parameter");
            }
            plhs[0] = mxCreateNumericMatrix(1,1,mxDOUBLE_CLASS,mxREAL);
            double *h = mxGetPr(plhs[0]);
            *h = r;
            if (r == Undef) reportError();
            return;
        }
    }
 
