/**
 *  @file CVodesIntegrator.cpp
 *
 */

// Copyright 2001  California Institute of Technology
#include "config.h"

#include "CVodesIntegrator.h"
#include "stringUtils.h"

#include <iostream>
using namespace std;

#ifdef SUNDIALS_VERSION_22

#include <sundials_types.h>
#include <sundials_math.h>
#include <cvodes.h>
#include <cvodes_dense.h>
#include <cvodes_diag.h>
#include <cvodes_spgmr.h>
#include <cvodes_band.h>
#include <nvector_serial.h>

#else

#if defined(SUNDIALS_VERSION_23) || defined (SUNDIALS_VERSION_24)
#include <sundials/sundials_types.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_nvector.h>
#include <nvector/nvector_serial.h>
#include <cvodes/cvodes.h>
#include <cvodes/cvodes_dense.h>
#include <cvodes/cvodes_diag.h>
#include <cvodes/cvodes_spgmr.h>
#include <cvodes/cvodes_band.h>

#else

unsupported sundials version!

#endif

#if defined (SUNDIALS_VERSION_24)
#define CV_SS 1
#define CV_SV 2

#endif

#endif

inline static N_Vector nv(void* x)
{
    return reinterpret_cast<N_Vector>(x);
}

namespace Cantera
{

class FuncData
{
public:
    FuncData(FuncEval* f, int npar = 0) {
        m_pars.resize(npar, 1.0);
        m_func = f;
    }
    virtual ~FuncData() {}
    vector_fp m_pars;
    FuncEval* m_func;
};
}


extern "C" {

    /**
     *  Function called by cvodes to evaluate ydot given y.  The cvode
     *  integrator allows passing in a void* pointer to access
     *  external data. This pointer is cast to a pointer to a instance
     *  of class FuncEval. The equations to be integrated should be
     *  specified by deriving a class from FuncEval that evaluates the
     *  desired equations.
     *  @ingroup odeGroup
     */
    static int cvodes_rhs(realtype t, N_Vector y, N_Vector ydot,
                          void* f_data)
    {
        double* ydata = NV_DATA_S(y); //N_VDATA(y);
        double* ydotdata = NV_DATA_S(ydot); //N_VDATA(ydot);
        Cantera::FuncData* d = (Cantera::FuncData*)f_data;
        Cantera::FuncEval* f = d->m_func;
        //try {
        if (d->m_pars.size() == 0) {
            f->eval(t, ydata, ydotdata, NULL);
        } else {
            f->eval(t, ydata, ydotdata, DATA_PTR(d->m_pars));
        }
        //}
        //catch (...) {
        //Cantera::showErrors();
        //Cantera::error("Teminating execution");
        //}
        return 0;
    }

}

namespace Cantera
{


/**
 *  Constructor. Default settings: dense jacobian, no user-supplied
 *  Jacobian function, Newton iteration.
 */
CVodesIntegrator::CVodesIntegrator() :
    m_neq(0),
    m_cvode_mem(0),
    m_t0(0.0),
    m_y(0),
    m_abstol(0),
    m_type(DENSE+NOJAC),
    m_itol(CV_SS),
    m_method(CV_BDF),
    m_iter(CV_NEWTON),
    m_maxord(0),
    m_reltol(1.e-9),
    m_abstols(1.e-15),
    m_reltolsens(1.0e-5),
    m_abstolsens(1.0e-4),
    m_nabs(0),
    m_hmax(0.0),
    m_maxsteps(20000), m_np(0),
    m_mupper(0), m_mlower(0)
{
    //m_ropt.resize(OPT_SIZE,0.0);
    //m_iopt = new long[OPT_SIZE];
    //fill(m_iopt, m_iopt+OPT_SIZE,0);
}


/// Destructor.
CVodesIntegrator::~CVodesIntegrator()
{
    if (m_cvode_mem) {
        if (m_np > 0) {
            CVodeSensFree(m_cvode_mem);
        }
        CVodeFree(&m_cvode_mem);
    }
    if (m_y) {
        N_VDestroy_Serial(nv(m_y));
    }
    if (m_abstol) {
        N_VDestroy_Serial(nv(m_abstol));
    }
    delete m_fdata;

    //delete[] m_iopt;
}

double& CVodesIntegrator::solution(int k)
{
    return NV_Ith_S(nv(m_y),k);
}

double* CVodesIntegrator::solution()
{
    return NV_DATA_S(nv(m_y));
}

void CVodesIntegrator::setTolerances(double reltol, int n, double* abstol)
{
    m_itol = CV_SV;
    m_nabs = n;
    if (n != m_neq) {
        if (m_abstol) {
            N_VDestroy_Serial(nv(m_abstol));
        }
        m_abstol = reinterpret_cast<void*>(N_VNew_Serial(n));
    }
    for (int i=0; i<n; i++) {
        NV_Ith_S(nv(m_abstol), i) = abstol[i];
    }
    m_reltol = reltol;
}

void CVodesIntegrator::setTolerances(double reltol, double abstol)
{
    m_itol = CV_SS;
    m_reltol = reltol;
    m_abstols = abstol;
}

void CVodesIntegrator::setSensitivityTolerances(double reltol, double abstol)
{
    m_reltolsens = reltol;
    m_abstolsens = abstol;
}

void CVodesIntegrator::setProblemType(int probtype)
{
    m_type = probtype;
}

void CVodesIntegrator::setMethod(MethodType t)
{
    if (t == BDF_Method) {
        m_method = CV_BDF;
    } else if (t == Adams_Method) {
        m_method = CV_ADAMS;
    } else {
        throw CVodesErr("unknown method");
    }
}

void CVodesIntegrator::setMaxStepSize(doublereal hmax)
{
    m_hmax = hmax;
    if (m_cvode_mem) {
        CVodeSetMaxStep(m_cvode_mem, hmax);
    }
}

void CVodesIntegrator::setMinStepSize(doublereal hmin)
{
    m_hmin = hmin;
    if (m_cvode_mem) {
        CVodeSetMinStep(m_cvode_mem, hmin);
    }
}

void CVodesIntegrator::setMaxSteps(int nmax)
{
    m_maxsteps = nmax;
    if (m_cvode_mem) {
        CVodeSetMaxNumSteps(m_cvode_mem, m_maxsteps);
    }
}

void CVodesIntegrator::setIterator(IterType t)
{
    if (t == Newton_Iter) {
        m_iter = CV_NEWTON;
    } else if (t == Functional_Iter) {
        m_iter = CV_FUNCTIONAL;
    } else {
        throw CVodesErr("unknown iterator");
    }
}

void CVodesIntegrator::sensInit(double t0, FuncEval& func)
{
    m_np = func.nparams();
    long int nv = func.neq();

    doublereal* data;
    int n, j;
    N_Vector y;
    y = N_VNew_Serial(nv);
    m_yS = N_VCloneVectorArray_Serial(m_np, y);
    for (n = 0; n < m_np; n++) {
        data = NV_DATA_S(m_yS[n]);
        for (j = 0; j < nv; j++) {
            data[j] =0.0;
        }
    }

    int flag;

#if defined(SUNDIALS_VERSION_22) || defined(SUNDIALS_VERSION_23)
    flag = CVodeSensMalloc(m_cvode_mem, m_np, CV_STAGGERED, m_yS);
    if (flag != CV_SUCCESS) {
        throw CVodesErr("Error in CVodeSensMalloc");
    }
    vector_fp atol(m_np, m_abstolsens);
    double rtol = m_reltolsens;
    flag = CVodeSetSensTolerances(m_cvode_mem, CV_SS, rtol, DATA_PTR(atol));
#elif defined(SUNDIALS_VERSION_24)
    flag = CVodeSensInit(m_cvode_mem, m_np, CV_STAGGERED,
                         CVSensRhsFn(0), m_yS);

    if (flag != CV_SUCCESS) {
        throw CVodesErr("Error in CVodeSensMalloc");
    }
    vector_fp atol(m_np, m_abstolsens);
    double rtol = m_reltolsens;
    flag = CVodeSensSStolerances(m_cvode_mem, rtol, DATA_PTR(atol));

#endif

}

void CVodesIntegrator::initialize(double t0, FuncEval& func)
{
    m_neq = func.neq();
    m_t0  = t0;

    if (m_y) {
        N_VDestroy_Serial(nv(m_y));    // free solution vector if already allocated
    }
    m_y = reinterpret_cast<void*>(N_VNew_Serial(m_neq));   // allocate solution vector
    for (size_t i = 0; i < m_neq; i++) {
        NV_Ith_S(nv(m_y), i) = 0.0;
    }
    // check abs tolerance array size
    if (m_itol == CV_SV && m_nabs < m_neq) {
        throw CVodesErr("not enough absolute tolerance values specified.");
    }

    func.getInitialConditions(m_t0, m_neq, NV_DATA_S(nv(m_y)));

    if (m_cvode_mem) {
        CVodeFree(&m_cvode_mem);
    }

    /*
     *  Specify the method and the iteration type:
     *      Cantera Defaults:
     *         CV_BDF  - Use BDF methods
     *         CV_NEWTON - use newton's method
     */
    m_cvode_mem = CVodeCreate(m_method, m_iter);
    if (!m_cvode_mem) {
        throw CVodesErr("CVodeCreate failed.");
    }

    int flag = 0;
#if defined(SUNDIALS_VERSION_22) || defined(SUNDIALS_VERSION_23)
    if (m_itol == CV_SV) {
        // vector atol
        flag = CVodeMalloc(m_cvode_mem, cvodes_rhs, m_t0, nv(m_y), m_itol,
                           m_reltol, nv(m_abstol));
    } else {
        // scalar atol
        flag = CVodeMalloc(m_cvode_mem, cvodes_rhs, m_t0, nv(m_y), m_itol,
                           m_reltol, &m_abstols);
    }
    if (flag != CV_SUCCESS) {
        if (flag == CV_MEM_FAIL) {
            throw CVodesErr("Memory allocation failed.");
        } else if (flag == CV_ILL_INPUT) {
            throw CVodesErr("Illegal value for CVodeMalloc input argument.");
        } else {
            throw CVodesErr("CVodeMalloc failed.");
        }
    }
#elif defined(SUNDIALS_VERSION_24)

    flag = CVodeInit(m_cvode_mem, cvodes_rhs, m_t0, nv(m_y));
    if (flag != CV_SUCCESS) {
        if (flag == CV_MEM_FAIL) {
            throw CVodesErr("Memory allocation failed.");
        } else if (flag == CV_ILL_INPUT) {
            throw CVodesErr("Illegal value for CVodeInit input argument.");
        } else {
            throw CVodesErr("CVodeInit failed.");
        }
    }

    if (m_itol == CV_SV) {
        flag = CVodeSVtolerances(m_cvode_mem, m_reltol, nv(m_abstol));
    } else {
        flag = CVodeSStolerances(m_cvode_mem, m_reltol, m_abstols);
    }
    if (flag != CV_SUCCESS) {
        if (flag == CV_MEM_FAIL) {
            throw CVodesErr("Memory allocation failed.");
        } else if (flag == CV_ILL_INPUT) {
            throw CVodesErr("Illegal value for CVodeInit input argument.");
        } else {
            throw CVodesErr("CVodeInit failed.");
        }
    }
#else
    printf("unknown sundials verson\n");
    exit(-1);
#endif



    if (m_type == DENSE + NOJAC) {
        long int N = m_neq;
        CVDense(m_cvode_mem, N);
    } else if (m_type == DIAG) {
        CVDiag(m_cvode_mem);
    } else if (m_type == GMRES) {
        CVSpgmr(m_cvode_mem, PREC_NONE, 0);
    } else if (m_type == BAND + NOJAC) {
        long int N = m_neq;
        long int nu = m_mupper;
        long int nl = m_mlower;
        CVBand(m_cvode_mem, N, nu, nl);
    } else {
        throw CVodesErr("unsupported option");
    }

    // pass a pointer to func in m_data
    m_fdata = new FuncData(&func, func.nparams());

    //m_data = (void*)&func;
#if defined(SUNDIALS_VERSION_22) || defined(SUNDIALS_VERSION_23)
    flag = CVodeSetFdata(m_cvode_mem, (void*)m_fdata);
    if (flag != CV_SUCCESS) {
        throw CVodesErr("CVodeSetFdata failed.");
    }
#elif defined(SUNDIALS_VERSION_24)
    flag = CVodeSetUserData(m_cvode_mem, (void*)m_fdata);
    if (flag != CV_SUCCESS) {
        throw CVodesErr("CVodeSetUserData failed.");
    }
#endif
    if (func.nparams() > 0) {
        sensInit(t0, func);
        flag = CVodeSetSensParams(m_cvode_mem, DATA_PTR(m_fdata->m_pars),
                                  NULL, NULL);
    }

    // set options
    if (m_maxord > 0) {
        flag = CVodeSetMaxOrd(m_cvode_mem, m_maxord);
    }
    if (m_maxsteps > 0) {
        flag = CVodeSetMaxNumSteps(m_cvode_mem, m_maxsteps);
    }
    if (m_hmax > 0) {
        flag = CVodeSetMaxStep(m_cvode_mem, m_hmax);
    }
}


void CVodesIntegrator::reinitialize(double t0, FuncEval& func)
{
    m_t0  = t0;
    //try {
    func.getInitialConditions(m_t0, m_neq, NV_DATA_S(nv(m_y)));
    //}
    //catch (CanteraError) {
    //showErrors();
    //error("Teminating execution");
    //}

    int result, flag;

#if defined(SUNDIALS_VERSION_22) || defined(SUNDIALS_VERSION_23)
    if (m_itol == CV_SV) {
        result = CVodeReInit(m_cvode_mem, cvodes_rhs, m_t0, nv(m_y),
                             m_itol, m_reltol,
                             nv(m_abstol));
    } else {
        result = CVodeReInit(m_cvode_mem, cvodes_rhs, m_t0, nv(m_y),
                             m_itol, m_reltol,
                             &m_abstols);
    }
    if (result != CV_SUCCESS) {
        throw CVodesErr("CVodeReInit failed. result = "+int2str(result));
    }
#elif defined(SUNDIALS_VERSION_24)
    result = CVodeReInit(m_cvode_mem, m_t0, nv(m_y));
    if (result != CV_SUCCESS) {
        throw CVodesErr("CVodeReInit failed. result = "+int2str(result));
    }
#endif

    if (m_type == DENSE + NOJAC) {
        long int N = m_neq;
        CVDense(m_cvode_mem, N);
    } else if (m_type == DIAG) {
        CVDiag(m_cvode_mem);
    } else if (m_type == BAND + NOJAC) {
        long int N = m_neq;
        long int nu = m_mupper;
        long int nl = m_mlower;
        CVBand(m_cvode_mem, N, nu, nl);
    } else if (m_type == GMRES) {
        CVSpgmr(m_cvode_mem, PREC_NONE, 0);
    } else {
        throw CVodesErr("unsupported option");
    }


    // set options
    if (m_maxord > 0) {
        flag = CVodeSetMaxOrd(m_cvode_mem, m_maxord);
    }
    if (m_maxsteps > 0) {
        flag = CVodeSetMaxNumSteps(m_cvode_mem, m_maxsteps);
    }
    if (m_hmax > 0) {
        flag = CVodeSetMaxStep(m_cvode_mem, m_hmax);
    }
}

void CVodesIntegrator::integrate(double tout)
{
    double t;
    int flag;
    flag = CVode(m_cvode_mem, tout, nv(m_y), &t, CV_NORMAL);
    if (flag != CV_SUCCESS) {
        throw CVodesErr(" CVodes error encountered.");
    }
#if defined(SUNDIALS_VERSION_22) || defined(SUNDIALS_VERSION_23)
    if (m_np > 0) {
        CVodeGetSens(m_cvode_mem, tout, m_yS);
    }
#elif defined(SUNDIALS_VERSION_24)
    double tretn;
    if (m_np > 0) {
        CVodeGetSens(m_cvode_mem, &tretn, m_yS);
        if (fabs(tretn - tout) > 1.0E-5) {
            throw CVodesErr("Time of Sensitivities different than time of tout");
        }
    }
#endif
}

double CVodesIntegrator::step(double tout)
{
    double t;
    int flag;
    flag = CVode(m_cvode_mem, tout, nv(m_y), &t, CV_ONE_STEP);
    if (flag != CV_SUCCESS) {
        throw CVodesErr(" CVodes error encountered.");
    }
    return t;
}

int CVodesIntegrator::nEvals() const
{
    long int ne;
    CVodeGetNumRhsEvals(m_cvode_mem, &ne);
    return ne;
    //return m_iopt[NFE];
}

double CVodesIntegrator::sensitivity(int k, int p)
{
    if (k < 0 || k >= m_neq) {
        throw CVodesErr("sensitivity: k out of range ("+int2str(p)+")");
    }
    if (p < 0 || p >= m_np) {
        throw CVodesErr("sensitivity: p out of range ("+int2str(p)+")");
    }
    return NV_Ith_S(m_yS[p],k);
}
}


