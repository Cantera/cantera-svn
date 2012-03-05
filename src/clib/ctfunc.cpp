/**
 * @file ctfunc.cpp
 */
#define CANTERA_USE_INTERNAL
#include "ctfunc.h"

#include "cantera/numerics/Func1.h"
#include "cantera/base/ctexceptions.h"

#include "Cabinet.h"

using namespace Cantera;
using namespace std;

typedef Func1 func_t;

typedef Cabinet<Func1> FuncCabinet;
// Assign storage to the Cabinet<Func1> static member
template<> FuncCabinet* FuncCabinet::__storage = 0;

extern "C" {

    // functions

    int func_new(int type, size_t n, size_t lenp, double* params)
    {
        func_t* r=0;
        size_t m = lenp;
        try {
            if (type == SinFuncType) {
                r = new Sin1(params[0]);
            } else if (type == CosFuncType) {
                r = new Cos1(params[0]);
            } else if (type == ExpFuncType) {
                r = new Exp1(params[0]);
            } else if (type == PowFuncType) {
                if (lenp < 1)
                    throw CanteraError("func_new",
                                       "exponent for pow must be supplied");
                r = new Pow1(params[0]);
            } else if (type == ConstFuncType) {
                r = new Const1(params[0]);
            } else if (type == FourierFuncType) {
                if (lenp < 2*n + 2)
                    throw CanteraError("func_new",
                                       "not enough Fourier coefficients");
                r = new Fourier1(n, params[n+1], params[0], params + 1,
                                 params + n + 2);
            } else if (type == GaussianFuncType) {
                if (lenp < 3)
                    throw CanteraError("func_new",
                                       "not enough Gaussian coefficients");
                r = new Gaussian(params[0], params[1], params[2]);
            } else if (type == PolyFuncType) {
                if (lenp < n + 1)
                    throw CanteraError("func_new",
                                       "not enough polynomial coefficients");
                r = new Poly1(n, params);
            } else if (type == ArrheniusFuncType) {
                if (lenp < 3*n)
                    throw CanteraError("func_new",
                                       "not enough Arrhenius coefficients");
                r = new Arrhenius1(n, params);
            } else if (type == PeriodicFuncType) {
                r = new Periodic1(FuncCabinet::item(n), params[0]);
            } else if (type == SumFuncType) {
                r = &newSumFunction(FuncCabinet::item(n).duplicate(),
                                    FuncCabinet::item(m).duplicate());
            } else if (type == DiffFuncType) {
                r = &newDiffFunction(FuncCabinet::item(n).duplicate(),
                                     FuncCabinet::item(m).duplicate());
            } else if (type == ProdFuncType) {
                r = &newProdFunction(FuncCabinet::item(n).duplicate(),
                                     FuncCabinet::item(m).duplicate());
            } else if (type == RatioFuncType) {
                r = &newRatioFunction(FuncCabinet::item(n).duplicate(),
                                      FuncCabinet::item(m).duplicate());
            } else if (type == CompositeFuncType) {
                r = &newCompositeFunction(FuncCabinet::item(n).duplicate(),
                                          FuncCabinet::item(m).duplicate());
            } else if (type == TimesConstantFuncType) {
                r = &newTimesConstFunction(FuncCabinet::item(n).duplicate(), params[0]);
            } else if (type == PlusConstantFuncType) {
                r = &newPlusConstFunction(FuncCabinet::item(n).duplicate(), params[0]);
            } else {
                throw CanteraError("func_new","unknown function type");
                r = new Func1();
            }
            return FuncCabinet::add(r);
        } catch (...) {
            return Cantera::handleAllExceptions(-1, ERR);
        }
    }


    int func_del(int i)
    {
        FuncCabinet::del(i);
        return 0;
    }

    int func_copy(int i)
    {
        return FuncCabinet::newCopy(i);
    }

    int func_assign(int i, int j)
    {
        return FuncCabinet::assign(i,j);
    }

    double func_value(int i, double t)
    {
        return FuncCabinet::item(i).eval(t);
    }

    int func_derivative(int i)
    {
        func_t* r = 0;
        r = &FuncCabinet::item(i).derivative();
        return FuncCabinet::add(r);
    }

    int func_duplicate(int i)
    {
        func_t* r = 0;
        r = &FuncCabinet::item(i).duplicate();
        return FuncCabinet::add(r);
    }

    int func_write(int i, size_t lennm, const char* arg, char* nm)
    {
        try {
            std::string a = std::string(arg);
            std::string w = FuncCabinet::item(i).write(a);
            size_t ws = w.size();
            size_t lout = (lennm > ws ? ws : lennm);
            std::copy(w.c_str(), w.c_str() + lout, nm);
            nm[lout] = '\0';
            return 0;
        } catch (...) {
            return Cantera::handleAllExceptions(-1, ERR);
        }
    }

}
