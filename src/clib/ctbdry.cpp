/**
 * @file ctbdry.cpp
 */
#define CANTERA_USE_INTERNAL
#include "ctbdry.h"

// Cantera includes
#include "cantera/oneD/OneDim.h"
#include "cantera/oneD/Inlet1D.h"
#include "cantera/kinetics/InterfaceKinetics.h"
#include "Cabinet.h"

using namespace std;
using namespace Cantera;

typedef Cabinet<Bdry1D> BoundaryCabinet;
template<> BoundaryCabinet* BoundaryCabinet::__storage = 0;

extern "C" {

    int bndry_new(int itype)
    {
        Bdry1D* s;
        switch (itype) {
        case 1:
            s = new Inlet1D();
            break;
        case 2:
            s = new Symm1D();
            break;
        case 3:
            s = new Surf1D();
            break;
        case 4:
            s = new ReactingSurf1D();
            break;
        default:
            return -2;
        }
        int i = BoundaryCabinet::add(s);
        return i;
    }

    int bndry_del(int i)
    {
        BoundaryCabinet::del(i);
        return 0;
    }

    double bndry_temperature(int i)
    {
        return BoundaryCabinet::item(i).temperature();
    }

    int bndry_settemperature(int i, double t)
    {
        try {
            BoundaryCabinet::item(i).setTemperature(t);
        } catch (CanteraError& err) {
            err.save();
            return -1;
        }
        return 0;
    }

    double bndry_spreadrate(int i)
    {
        try {
            return dynamic_cast<Inlet1D*>(&BoundaryCabinet::item(i))->spreadRate();
        } catch (CanteraError& err) {
            err.save();
            return -1;
        }
        return 0;
    }

    int bndry_setSpreadRate(int i, double v)
    {
        try {
            dynamic_cast<Inlet1D*>(&BoundaryCabinet::item(i))->setSpreadRate(v);
        } catch (CanteraError& err) {
            err.save();
            return -1;
        }
        return 0;
    }

    int bndry_setmdot(int i, double mdot)
    {
        try {
            BoundaryCabinet::item(i).setMdot(mdot);
        } catch (CanteraError& err) {
            err.save();
            return -1;
        }
        return 0;
    }


    double bndry_mdot(int i)
    {
        return BoundaryCabinet::item(i).mdot();
    }

    int bndry_setxin(int i, double* xin)
    {
        try {
            BoundaryCabinet::item(i).setMoleFractions(xin);
        } catch (CanteraError& err) {
            err.save();
            return -1;
        }
        return 0;
    }

    int bndry_setxinbyname(int i, char* xin)
    {
        try {
            BoundaryCabinet::item(i).setMoleFractions(string(xin));
        } catch (CanteraError& err) {
            err.save();
            return -1;
        }
        return 0;
    }

    int surf_setkinetics(int i, int j)
    {
        try {
            ReactingSurf1D* srf =
                dynamic_cast<ReactingSurf1D*>(&BoundaryCabinet::item(i));
            InterfaceKinetics* k =
                dynamic_cast<InterfaceKinetics*>(&Cabinet<Kinetics>::item(j));
            srf->setKineticsMgr(k);
        } catch (CanteraError& err) {
            err.save();
            return -1;
        }
        return 0;
    }
}
