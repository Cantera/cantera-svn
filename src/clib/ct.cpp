/**
 *  @file ct.cpp
 *   Cantera interface library. This library of functions is designed
 *   to encapsulate Cantera functionality and make it available for
 *   use in languages and applications other than C++. A set of
 *   library functions is provided that are declared "extern C". All
 *   Cantera objects are stored and referenced by integers - no
 *   pointers are passed to or from the calling application.
 */

#define CANTERA_USE_INTERNAL
#include "ct.h"

// Cantera includes
#include "cantera/equil/equil.h"
#include "cantera/kinetics/KineticsFactory.h"
#include "cantera/transport/TransportFactory.h"
#include "cantera/base/ctml.h"
#include "cantera/kinetics/importKinetics.h"
#include "cantera/thermo/ThermoFactory.h"
#include "converters/ck2ct.h"
#include "Cabinet.h"
#include "cantera/kinetics/InterfaceKinetics.h"
#include "cantera/thermo/PureFluidPhase.h"

using namespace std;
using namespace Cantera;

#ifdef _WIN32
#include "windows.h"
#endif

typedef Cabinet<ThermoPhase> ThermoCabinet;
typedef Cabinet<Kinetics> KineticsCabinet;
typedef Cabinet<Transport> TransportCabinet;
typedef Cabinet<XML_Node, false> XmlCabinet;

template<> ThermoCabinet* ThermoCabinet::__storage = 0;
template<> KineticsCabinet* KineticsCabinet::__storage = 0;
template<> TransportCabinet* TransportCabinet::__storage = 0;

#ifdef WITH_PURE_FLUIDS
static PureFluidPhase* purefluid(int n)
{
    try {
        ThermoPhase& tp = ThermoCabinet::item(n);
        if (tp.eosType() == cPureFluid) {
            return dynamic_cast<PureFluidPhase*>(&tp);
        } else {
            throw CanteraError("purefluid","object is not a PureFluidPhase object");
        }
    } catch (CanteraError) {
        return 0;
    }
    return 0;
}

static double pfprop(int n, int i, double v=0.0, double x=0.0)
{
    PureFluidPhase* p = purefluid(n);
    if (p) {
        switch (i) {
        case 0:
            return p->critTemperature();
        case 1:
            return p->critPressure();
        case 2:
            return p->critDensity();
        case 3:
            return p->vaporFraction();
        case 4:
            return p->satTemperature(v);
        case 5:
            return p->satPressure(v);
        case 6:
            p->setState_Psat(v, x);
            return 0.0;
        case 7:
            p->setState_Tsat(v, x);
            return 0.0;
        }
    }
    return DERR;
}


#else

static ThermoPhase* purefluid(int n)
{
    return th(n);
}
static double pfprop(int n, int i, double v=0.0, double x=0.0)
{
    return DERR;
}
#endif


namespace Cantera
{
void writephase(const ThermoPhase& th, bool show_thermo);
}

/**
 * Exported functions.
 */
extern "C" {

    int ct_appdelete()
    {
        appdelete();
        return 0;
    }

    //--------------- Phase ---------------------//

    size_t phase_nElements(int n)
    {
        return ThermoCabinet::item(n).nElements();
    }

    size_t phase_nSpecies(int n)
    {
        return ThermoCabinet::item(n).nSpecies();
    }

    doublereal phase_temperature(int n)
    {
        return ThermoCabinet::item(n).temperature();
    }

    int phase_setTemperature(int n, double t)
    {
        try {
            ThermoCabinet::item(n).setTemperature(t);
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    doublereal phase_density(int n)
    {
        return ThermoCabinet::item(n).density();
    }

    int phase_setDensity(int n, double rho)
    {
        if (rho < 0.0) {
            return -1;
        }
        ThermoCabinet::item(n).setDensity(rho);
        return 0;
    }

    doublereal phase_molarDensity(int n)
    {
        return ThermoCabinet::item(n).molarDensity();
    }

    int phase_setMolarDensity(int n, double ndens)
    {
        if (ndens < 0.0) {
            return -1;
        }
        ThermoCabinet::item(n).setMolarDensity(ndens);
        return 0;
    }

    doublereal phase_meanMolecularWeight(int n)
    {
        return ThermoCabinet::item(n).meanMolecularWeight();
    }

    size_t phase_elementIndex(int n, char* nm)
    {
        string elnm = string(nm);
        return ThermoCabinet::item(n).elementIndex(elnm);
    }

    size_t phase_speciesIndex(int n, char* nm)
    {
        string spnm = string(nm);
        return ThermoCabinet::item(n).speciesIndex(spnm);
    }

    int phase_getMoleFractions(int n, size_t lenx, double* x)
    {
        ThermoPhase& p = ThermoCabinet::item(n);
        if (lenx >= p.nSpecies()) {
            p.getMoleFractions(x);
            return 0;
        } else {
            return -1;
        }
    }

    doublereal phase_moleFraction(int n, size_t k)
    {
        return ThermoCabinet::item(n).moleFraction(k);
    }

    int phase_getMassFractions(int n, size_t leny, double* y)
    {
        ThermoPhase& p = ThermoCabinet::item(n);
        if (leny >= p.nSpecies()) {
            p.getMassFractions(y);
            return 0;
        } else {
            return -1;
        }
    }

    doublereal phase_massFraction(int n, size_t k)
    {
        return ThermoCabinet::item(n).massFraction(k);
    }

    int phase_setMoleFractions(int n, size_t lenx, double* x, int norm)
    {
        ThermoPhase& p = ThermoCabinet::item(n);
        if (lenx >= p.nSpecies()) {
            if (norm) {
                p.setMoleFractions(x);
            } else {
                p.setMoleFractions_NoNorm(x);
            }
            return 0;
        } else {
            return -1;
        }
    }

    int phase_setMoleFractionsByName(int n, char* x)
    {
        try {
            ThermoPhase& p = ThermoCabinet::item(n);
            compositionMap xx;
            size_t nsp = p.nSpecies();
            for (size_t n = 0; n < nsp; n++) {
                xx[p.speciesName(n)] = -1;
            }
            parseCompString(string(x), xx);
            p.setMoleFractionsByName(xx);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
        //catch (...) {return ERR;}
    }

    int phase_setMassFractions(int n, size_t leny,
                                          double* y, int norm)
    {
        ThermoPhase& p = ThermoCabinet::item(n);
        if (leny >= p.nSpecies()) {
            if (norm) {
                p.setMassFractions(y);
            } else {
                p.setMassFractions_NoNorm(y);
            }
            return 0;
        } else {
            return -10;
        }
    }

    int phase_setMassFractionsByName(int n, char* y)
    {
        try {
            ThermoPhase& p = ThermoCabinet::item(n);
            compositionMap yy;
            size_t nsp = p.nSpecies();
            for (size_t n = 0; n < nsp; n++) {
                yy[p.speciesName(n)] = -1;
            }
            parseCompString(string(y), yy);
            p.setMassFractionsByName(yy);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int phase_getAtomicWeights(int n,
                                          size_t lenm, double* atw)
    {
        ThermoPhase& p = ThermoCabinet::item(n);
        if (lenm >= p.nElements()) {
            const vector_fp& wt = p.atomicWeights();
            copy(wt.begin(), wt.end(), atw);
            return 0;
        } else {
            return -10;
        }
    }

    int phase_getMolecularWeights(int n,
            size_t lenm, double* mw)
    {
        ThermoPhase& p = ThermoCabinet::item(n);
        if (lenm >= p.nSpecies()) {
            const vector_fp& wt = p.molecularWeights();
            copy(wt.begin(), wt.end(), mw);
            return 0;
        } else {
            return -10;
        }
    }

    int phase_getName(int n, size_t lennm, char* nm)
    {
        string name = ThermoCabinet::item(n).name();
        size_t lout = min(lennm, name.size());
        copy(name.c_str(), name.c_str() + lout, nm);
        nm[lout] = '\0';
        return 0;
    }

    int phase_setName(int n, const char* nm)
    {
        string name = string(nm);
        ThermoCabinet::item(n).setName(name);
        return 0;
    }

    int phase_getSpeciesName(int n, size_t k, size_t lennm, char* nm)
    {
        try {
            string spnm = ThermoCabinet::item(n).speciesName(k);
            size_t lout = min(lennm, spnm.size());
            copy(spnm.c_str(), spnm.c_str() + lout, nm);
            nm[lout] = '\0';
            return 0;
        } catch (CanteraError) {
            return -1;
        }
        //catch (...) {return ERR;}
    }

    int phase_getElementName(int n, size_t m, size_t lennm, char* nm)
    {
        try {
            string elnm = ThermoCabinet::item(n).elementName(m);
            size_t lout = min(lennm, elnm.size());
            copy(elnm.c_str(), elnm.c_str() + lout, nm);
            nm[lout] = '\0';
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }


    doublereal phase_nAtoms(int n, size_t k, size_t m)
    {
        try {
            return ThermoCabinet::item(n).nAtoms(k,m);
        } catch (CanteraError) {
            return -1;
        }
    }

    int phase_addElement(int n, char* name, doublereal weight)
    {
        try {
            ThermoCabinet::item(n).addElement(string(name),weight);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    //     int phase_addSpecies(int n, char* name, int phase,
    //         int ncomp, doublereal* comp, int thermoType, int ncoeffs,
    //         double* coeffs, double minTemp, double maxTemp, double refPressure,
    //         doublereal charge, doublereal weight) {
    //         try {
    //             vector_fp cmp(ncomp);
    //             copy(comp, comp + ncomp, cmp.begin());
    //             vector_fp c(ncoeffs);
    //             copy(coeffs, coeffs + ncoeffs, c.begin());
    //             ph(n)->addSpecies(string(name), phase, cmp,
    //                 thermoType, c, minTemp, maxTemp, refPressure,
    //                 charge, weight);
    //             return 0;
    //         }
    //         catch (CanteraError) { return -1; }
    //         catch (...) {return ERR;}
    //     }



    //-------------- Thermo --------------------//

    size_t newThermoFromXML(int mxml)
    {
        try {
            XML_Node& x = XmlCabinet::item(mxml);
            thermo_t* th = newPhase(x);
            return ThermoCabinet::add(th);
        } catch (CanteraError) {
            return -1;
        }
    }

    size_t th_nSpecies(size_t n)
    {
        return ThermoCabinet::item(n).nSpecies();
    }

    int th_eosType(int n)
    {
        return ThermoCabinet::item(n).eosType();
    }

    double th_enthalpy_mole(int n)
    {
        try {
            return ThermoCabinet::item(n).enthalpy_mole();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_intEnergy_mole(int n)
    {
        try {
            return ThermoCabinet::item(n).intEnergy_mole();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_entropy_mole(int n)
    {
        try {
            return ThermoCabinet::item(n).entropy_mole();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_gibbs_mole(int n)
    {
        try {
            return ThermoCabinet::item(n).gibbs_mole();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_cp_mole(int n)
    {
        try {
            return ThermoCabinet::item(n).cp_mole();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_cv_mole(int n)
    {
        try {
            return ThermoCabinet::item(n).cv_mole();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_pressure(int n)
    {
        try {
            return ThermoCabinet::item(n).pressure();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_enthalpy_mass(int n)
    {
        try {
            return ThermoCabinet::item(n).enthalpy_mass();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_intEnergy_mass(int n)
    {
        try {
            return ThermoCabinet::item(n).intEnergy_mass();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_entropy_mass(int n)
    {
        try {
            return ThermoCabinet::item(n).entropy_mass();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_gibbs_mass(int n)
    {
        try {
            return ThermoCabinet::item(n).gibbs_mass();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_cp_mass(int n)
    {
        try {
            return ThermoCabinet::item(n).cp_mass();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_cv_mass(int n)
    {
        try {
            return ThermoCabinet::item(n).cv_mass();
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_electricPotential(int n)
    {
        try {
            return ThermoCabinet::item(n).electricPotential();
        } catch (CanteraError) {
            return DERR;
        }
    }

    int th_chemPotentials(int n, size_t lenm, double* murt)
    {
        ThermoPhase& thrm = ThermoCabinet::item(n);
        size_t nsp = thrm.nSpecies();
        if (lenm >= nsp) {
            thrm.getChemPotentials(murt);
            return 0;
        } else {
            return -10;
        }
    }

    int th_elementPotentials(int n, size_t lenm, double* lambda)
    {
        ThermoPhase& thrm = ThermoCabinet::item(n);
        size_t nel = thrm.nElements();
        if (lenm >= nel) {
            equilibrate(thrm, "TP", 0);
            thrm.getElementPotentials(lambda);
            return 0;
        } else {
            return -10;
        }
    }

    int th_setPressure(int n, double p)
    {
        try {
            if (p < 0.0) throw CanteraError("th_setPressure",
                                                "pressure cannot be negative");
            ThermoCabinet::item(n).setPressure(p);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int th_set_HP(int n, double* vals)
    {
        try {
            if (vals[1] < 0.0)
                throw CanteraError("th_set_HP",
                                   "pressure cannot be negative");
            ThermoCabinet::item(n).setState_HP(vals[0],vals[1]);
            if (ThermoCabinet::item(n).temperature() < 0.0)
                throw CanteraError("th_set_HP",
                                   "temperature cannot be negative");
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int th_set_UV(int n, double* vals)
    {
        try {
            if (vals[1] < 0.0)
                throw CanteraError("th_set_UV",
                                   "specific volume cannot be negative");
            ThermoCabinet::item(n).setState_UV(vals[0],vals[1]);
            if (ThermoCabinet::item(n).temperature() < 0.0)
                throw CanteraError("th_set_UV",
                                   "temperature cannot be negative");
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int th_set_SV(int n, double* vals)
    {
        try {
            ThermoCabinet::item(n).setState_SV(vals[0],vals[1]);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int th_set_SP(int n, double* vals)
    {
        try {
            ThermoCabinet::item(n).setState_SP(vals[0],vals[1]);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int th_equil(int n, char* XY, int solver,
                            double rtol, int maxsteps, int maxiter, int loglevel)
    {
        try {
            equilibrate(ThermoCabinet::item(n), XY, solver, rtol, maxsteps,
                        maxiter, loglevel);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    doublereal th_refPressure(int n)
    {
        return ThermoCabinet::item(n).refPressure();
    }

    doublereal th_minTemp(int n, int k)
    {
        return ThermoCabinet::item(n).minTemp(k);
    }

    doublereal th_maxTemp(int n, int k)
    {
        return ThermoCabinet::item(n).maxTemp(k);
    }


    int th_getEnthalpies_RT(int n, size_t lenm, double* h_rt)
    {
        try {
            ThermoPhase& thrm = ThermoCabinet::item(n);
            size_t nsp = thrm.nSpecies();
            if (lenm >= nsp) {
                thrm.getEnthalpy_RT_ref(h_rt);
                return 0;
            } else {
                return -10;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int th_getEntropies_R(int n, size_t lenm, double* s_r)
    {
        try {
            ThermoPhase& thrm = ThermoCabinet::item(n);
            size_t nsp = thrm.nSpecies();
            if (lenm >= nsp) {
                thrm.getEntropy_R_ref(s_r);
                return 0;
            } else {
                return -10;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int th_getCp_R(int n, size_t lenm, double* cp_r)
    {
        try {
            ThermoPhase& thrm = ThermoCabinet::item(n);
            size_t nsp = thrm.nSpecies();
            if (lenm >= nsp) {
                thrm.getCp_R_ref(cp_r);
                return 0;
            } else {
                return -10;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int th_setElectricPotential(int n, double v)
    {
        ThermoCabinet::item(n).setElectricPotential(v);
        return 0;
    }

    //-------------- pure fluids ---------------//
#ifdef WITH_PURE_FLUIDS

    double th_critTemperature(int n)
    {
        return pfprop(n,0);
    }

    double th_critPressure(int n)
    {
        return purefluid(n)->critPressure();
    }

    double th_critDensity(int n)
    {
        return purefluid(n)->critDensity();
    }

    double th_vaporFraction(int n)
    {
        return purefluid(n)->vaporFraction();
    }

    double th_satTemperature(int n, double p)
    {
        try {
            return purefluid(n)->satTemperature(p);
        } catch (CanteraError) {
            return DERR;
        }
    }

    double th_satPressure(int n, double t)
    {
        try {
            return purefluid(n)->satPressure(t);
        } catch (CanteraError) {
            return DERR;
        }
    }

    int th_setState_Psat(int n, double p, double x)
    {
        try {
            purefluid(n)->setState_Psat(p, x);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int th_setState_Tsat(int n, double t, double x)
    {
        try {
            purefluid(n)->setState_Tsat(t, x);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }
#else

    double th_critTemperature(int n)
    {
        return DERR;
    }

    double th_critPressure(int n)
    {
        return DERR;
    }

    double th_critDensity(int n)
    {
        return DERR;
    }

    double th_vaporFraction(int n)
    {
        return DERR;
    }

    double th_satTemperature(int n, double p)
    {
        return DERR;
    }

    double th_satPressure(int n, double t)
    {
        return DERR;
    }

    int th_setState_Psat(int n, double p, double x)
    {
        return DERR;
    }

    int th_setState_Tsat(int n, double t, double x)
    {
        return DERR;
    }
#endif



    //-------------- Kinetics ------------------//

    size_t newKineticsFromXML(int mxml, int iphase,
                                         int neighbor1, int neighbor2, int neighbor3,
                                         int neighbor4)
    {
        try {
            XML_Node& x = XmlCabinet::item(mxml);
            vector<thermo_t*> phases;
            phases.push_back(&ThermoCabinet::item(iphase));
            if (neighbor1 >= 0) {
                phases.push_back(&ThermoCabinet::item(neighbor1));
                if (neighbor2 >= 0) {
                    phases.push_back(&ThermoCabinet::item(neighbor2));
                    if (neighbor3 >= 0) {
                        phases.push_back(&ThermoCabinet::item(neighbor3));
                        if (neighbor4 >= 0) {
                            phases.push_back(&ThermoCabinet::item(neighbor4));
                        }
                    }
                }
            }
            Kinetics* kin = newKineticsMgr(x, phases);
            if (kin) {
                return KineticsCabinet::add(kin);
            } else {
                return 0;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int installRxnArrays(int pxml, int ikin,
                                    char* default_phase)
    {
        try {
            XML_Node& p = XmlCabinet::item(pxml);
            Kinetics& k = KineticsCabinet::item(ikin);
            string defphase = string(default_phase);
            installReactionArrays(p, k, defphase);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    //-------------------------------------
    int kin_type(int n)
    {
        return KineticsCabinet::item(n).type();
    }

    size_t kin_start(int n, int p)
    {
        return KineticsCabinet::item(n).kineticsSpeciesIndex(0,p);
    }

    size_t kin_speciesIndex(int n, const char* nm, const char* ph)
    {
        return KineticsCabinet::item(n).kineticsSpeciesIndex(string(nm), string(ph));
    }

    //---------------------------------------

    size_t kin_nSpecies(int n)
    {
        return KineticsCabinet::item(n).nTotalSpecies();
    }

    size_t kin_nReactions(int n)
    {
        return KineticsCabinet::item(n).nReactions();
    }

    size_t kin_nPhases(int n)
    {
        return KineticsCabinet::item(n).nPhases();
    }

    size_t kin_phaseIndex(int n, char* ph)
    {
        return KineticsCabinet::item(n).phaseIndex(string(ph));
    }

    size_t kin_reactionPhaseIndex(int n)
    {
        return KineticsCabinet::item(n).reactionPhaseIndex();
    }

    double kin_reactantStoichCoeff(int n, int k, int i)
    {
        return KineticsCabinet::item(n).reactantStoichCoeff(k,i);
    }

    double kin_productStoichCoeff(int n, int k, int i)
    {
        return KineticsCabinet::item(n).productStoichCoeff(k,i);
    }

    int kin_reactionType(int n, int i)
    {
        return KineticsCabinet::item(n).reactionType(i);
    }

    int kin_getFwdRatesOfProgress(int n, size_t len, double* fwdROP)
    {
        Kinetics& k = KineticsCabinet::item(n);
        try {
            if (len >= k.nReactions()) {
                k.getFwdRatesOfProgress(fwdROP);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int kin_getRevRatesOfProgress(int n, size_t len, double* revROP)
    {
        Kinetics& k = KineticsCabinet::item(n);
        try {
            if (len >= k.nReactions()) {
                k.getRevRatesOfProgress(revROP);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int kin_isReversible(int n, int i)
    {
        return (int) KineticsCabinet::item(n).isReversible(i);
    }

    int kin_getNetRatesOfProgress(int n, size_t len, double* netROP)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            if (len >= k.nReactions()) {
                k.getNetRatesOfProgress(netROP);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int kin_getFwdRateConstants(int n, size_t len, double* kfwd)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            if (len >= k.nReactions()) {
                k.getFwdRateConstants(kfwd);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int kin_getRevRateConstants(int n, int doIrreversible, size_t len, double* krev)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            bool doirrev = false;
            if (doIrreversible != 0) {
                doirrev = true;
            }
            if (len >= k.nReactions()) {
                k.getRevRateConstants(krev, doirrev);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }


    int kin_getActivationEnergies(int n, size_t len, double* E)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            if (len >= k.nReactions()) {
                k.getActivationEnergies(E);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }


    int kin_getDelta(int n, int job, size_t len, double* delta)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            if (len < k.nReactions()) {
                return ERR;
            }
            switch (job) {
            case 0:
                k.getDeltaEnthalpy(delta);
                break;
            case 1:
                k.getDeltaGibbs(delta);
                break;
            case 2:
                k.getDeltaEntropy(delta);
                break;
            case 3:
                k.getDeltaSSEnthalpy(delta);
                break;
            case 4:
                k.getDeltaSSGibbs(delta);
                break;
            case 5:
                k.getDeltaSSEntropy(delta);
                break;
            default:
                return ERR;
            }
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }


    int kin_getDeltaEntropy(int n, size_t len, double* deltaS)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            if (len >= k.nReactions()) {
                k.getDeltaEntropy(deltaS);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }


    int kin_getCreationRates(int n, size_t len, double* cdot)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            if (len >= k.nTotalSpecies()) {
                k.getCreationRates(cdot);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int kin_getDestructionRates(int n, size_t len, double* ddot)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            if (len >= k.nTotalSpecies()) {
                k.getDestructionRates(ddot);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
        //catch (...) {return ERR;}
    }

    int kin_getNetProductionRates(int n, size_t len, double* wdot)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            if (len >= k.nTotalSpecies()) {
                k.getNetProductionRates(wdot);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int kin_getSourceTerms(int n, size_t len, double* ydot)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            ThermoPhase& p = k.thermo();
            const vector_fp& mw = p.molecularWeights();
            size_t nsp = mw.size();
            double rrho = 1.0/p.density();
            if (len >= nsp) {
                k.getNetProductionRates(ydot);
                multiply_each(ydot, ydot + nsp, mw.begin());
                scale(ydot, ydot + nsp, ydot, rrho);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    double kin_multiplier(int n, int i)
    {
        return KineticsCabinet::item(n).multiplier(i);
    }

    size_t kin_phase(int n, size_t i)
    {
        return ThermoCabinet::index(KineticsCabinet::item(n).thermo(i));
    }

    int kin_getEquilibriumConstants(int n, size_t len, double* kc)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            if (len >= k.nReactions()) {
                k.getEquilibriumConstants(kc);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int kin_getReactionString(int n, int i, int len, char* buf)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            string r = k.reactionString(i);
            int lout = min(len, (int)r.size());
            copy(r.c_str(), r.c_str() + lout, buf);
            buf[lout] = '\0';
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int kin_setMultiplier(int n, int i, double v)
    {
        try {
            if (v >= 0.0) {
                KineticsCabinet::item(n).setMultiplier(i,v);
                return 0;
            } else {
                return ERR;
            }
        } catch (CanteraError) {
            return -1;
        }
    }

    int kin_advanceCoverages(int n, double tstep)
    {
        try {
            Kinetics& k = KineticsCabinet::item(n);
            if (k.type() == cInterfaceKinetics) {
                dynamic_cast<InterfaceKinetics*>(&k)->advanceCoverages(tstep);
            } else {
                throw CanteraError("kin_advanceCoverages",
                                   "wrong kinetics manager type");
            }
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    //------------------- Transport ---------------------------

    size_t newTransport(char* model,
                                   int ith, int loglevel)
    {
        string mstr = string(model);
        ThermoPhase& t = ThermoCabinet::item(ith);
        try {
            Transport* tr = newTransportMgr(mstr, &t, loglevel);
            return TransportCabinet::add(tr);
        } catch (CanteraError) {
            return -1;
        }
    }

    double trans_viscosity(int n)
    {
        try {
            return TransportCabinet::item(n).viscosity();
        } catch (CanteraError) {
            return -1.0;
        }
    }

    double trans_thermalConductivity(int n)
    {
        try {
            return TransportCabinet::item(n).thermalConductivity();
        } catch (CanteraError) {
            return -1.0;
        }
    }

    int trans_getThermalDiffCoeffs(int n, int ldt, double* dt)
    {
        try {
            TransportCabinet::item(n).getThermalDiffCoeffs(dt);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int trans_getMixDiffCoeffs(int n, int ld, double* d)
    {
        try {
            TransportCabinet::item(n).getMixDiffCoeffs(d);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int trans_getBinDiffCoeffs(int n, int ld, double* d)
    {
        try {
            TransportCabinet::item(n).getBinaryDiffCoeffs(ld,d);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int trans_getMultiDiffCoeffs(int n, int ld, double* d)
    {
        try {
            TransportCabinet::item(n).getMultiDiffCoeffs(ld,d);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int trans_setParameters(int n, int type, int k, double* d)
    {
        try {
            TransportCabinet::item(n).setParameters(type, k, d);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int trans_getMolarFluxes(int n, const double* state1,
                                        const double* state2, double delta, double* fluxes)
    {
        try {
            TransportCabinet::item(n).getMolarFluxes(state1, state2, delta, fluxes);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int trans_getMassFluxes(int n, const double* state1,
                                       const double* state2, double delta, double* fluxes)
    {
        try {
            TransportCabinet::item(n).getMassFluxes(state1, state2, delta, fluxes);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    //-------------------- Functions ---------------------------

    int import_phase(int nth, int nxml, char* id)
    {
        ThermoPhase& thrm = ThermoCabinet::item(nth);
        XML_Node& node = XmlCabinet::item(nxml);
        string idstr = string(id);
        try {
            importPhase(node, &thrm);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int import_kinetics(int nxml, char* id,
                                   int nphases, integer* ith, int nkin)
    {
        vector<thermo_t*> phases;
        for (int i = 0; i < nphases; i++) {
            phases.push_back(&ThermoCabinet::item(ith[i]));
        }
        XML_Node& node = XmlCabinet::item(nxml);
        Kinetics& k = KineticsCabinet::item(nkin);
        string idstr = string(id);
        try {
            importKinetics(node, phases, &k);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }


    int phase_report(int nth,
                                int ibuf, char* buf, int show_thermo)
    {
        try {
            bool stherm = (show_thermo != 0);
            string s = report(ThermoCabinet::item(nth), stherm);
            if (int(s.size()) > ibuf - 1) {
                return -(static_cast<int>(s.size()) + 1);
            }
            copy(s.begin(), s.end(), buf);
            buf[s.size() - 1] = '\0';
            return 0;

        } catch (CanteraError) {
            return -1;
        }
    }

    int write_phase(int nth, int show_thermo)
    {
        try {
            bool stherm = (show_thermo != 0);
            writephase(ThermoCabinet::item(nth), stherm);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int write_HTML_log(char* file)
    {
        write_logfile(string(file));
        return 0;
    }

    int getCanteraError(int buflen, char* buf)
    {
        string e;
        e = lastErrorMessage();
        if (buflen > 0) {
            int n = min(static_cast<int>(e.size()), buflen-1);
            copy(e.begin(), e.begin() + n, buf);
            buf[min(n, buflen-1)] = '\0';
        }
        return int(e.size());
    }

    int showCanteraErrors()
    {
        showErrors();
        return 0;
    }

    int addCanteraDirectory(size_t buflen, char* buf)
    {
        addDirectory(string(buf));
        return 0;
    }

    int setLogWriter(void* logger)
    {
        Logger* logwriter = (Logger*)logger;
        setLogger(logwriter);
        return 0;
    }

    int readlog(int n, char* buf)
    {
        string s;
        writelog("function readlog is deprecated!");
        //getlog(s);
        int nlog = static_cast<int>(s.size());
        if (n < 0) {
            return nlog;
        }
        int nn = min(n-1, nlog);
        copy(s.begin(), s.begin() + nn,
             buf);
        buf[min(nlog, n-1)] = '\0';
        //clearlog();
        return 0;

    }
    int clearStorage()
    {
        try {
            ThermoCabinet::clear();
            KineticsCabinet::clear();
            TransportCabinet::clear();
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int delThermo(int n)
    {
        try {
            ThermoCabinet::del(n);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int delKinetics(int n)
    {
        KineticsCabinet::del(n);
        return 0;
    }

    int delTransport(int n)
    {
        TransportCabinet::del(n);
        return 0;
    }

    int buildSolutionFromXML(char* src, int ixml, char* id,
                                        int ith, int ikin)
    {

        XML_Node* root = 0;
        if (ixml > 0) {
            root = &XmlCabinet::item(ixml);
        }

        ThermoPhase& t = ThermoCabinet::item(ith);
        Kinetics& kin = KineticsCabinet::item(ikin);
        XML_Node* x, *r=0;
        if (root) {
            r = &root->root();
        }
        x = get_XML_Node(string(src), r);
        //x = find_XML(string(src), r, string(id), "", "phase");
        if (!x) {
            return false;
        }
        importPhase(*x, &t);
        kin.addPhase(t);
        kin.init();
        installReactionArrays(*x, kin, x->id());
        t.setState_TP(300.0, OneAtm);
        if (r) {
            if (&x->root() != &r->root()) {
                delete &x->root();
            }
        } else {
            delete &x->root();
        }
        return 0;
    }


    int ck_to_cti(char* in_file, char* db_file,
                             char* tr_file, char* id_tag, int debug, int validate)
    {
        bool dbg = (debug != 0);
        bool val = (validate != 0);
        return pip::convert_ck(in_file, db_file, tr_file, id_tag, dbg, val);
    }


    int writelogfile(char* logfile)
    {
        write_logfile(string(logfile));
        return 0;
    }


}
