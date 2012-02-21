/**
 *  @file SurfPhase.cpp
 *  Definitions for a simple thermoydnamics model of a surface phase
 *  derived from ThermoPhase,  assuming an ideal solution model
 *  (see \ref thermoprops and class
 *  \link Cantera::SurfPhase SurfPhase\endlink).
 */
// Copyright 2002  California Institute of Technology

#include "cantera/thermo/SurfPhase.h"
#include "cantera/thermo/EdgePhase.h"
#include "cantera/thermo/ThermoFactory.h"

using namespace ctml;

using namespace std;

///////////////////////////////////////////////////////////
//
//    class SurfPhase methods
//
///////////////////////////////////////////////////////////

namespace Cantera
{

SurfPhase::SurfPhase(doublereal n0):
    ThermoPhase(),
    m_n0(n0),
    m_logn0(0.0),
    m_tmin(0.0),
    m_tmax(0.0),
    m_press(OneAtm),
    m_tlast(0.0)
{
    if (n0 > 0.0) {
        m_logn0 = log(n0);
    }
    setNDim(2);
}

SurfPhase::SurfPhase(std::string infile, std::string id) :
    ThermoPhase(),
    m_n0(0.0),
    m_logn0(0.0),
    m_tmin(0.0),
    m_tmax(0.0),
    m_press(OneAtm),
    m_tlast(0.0)
{
    XML_Node* root = get_XML_File(infile);
    if (id == "-") {
        id = "";
    }
    XML_Node* xphase = get_XML_NameID("phase", std::string("#")+id, root);
    if (!xphase) {
        throw CanteraError("SurfPhase::SurfPhase",
                           "Couldn't find phase name in file:" + id);
    }
    // Check the model name to ensure we have compatibility
    const XML_Node& th = xphase->child("thermo");
    string model = th["model"];
    if (model != "Surface" && model != "Edge") {
        throw CanteraError("SurfPhase::SurfPhase",
                           "thermo model attribute must be Surface or Edge");
    }
    importPhase(*xphase, this);
}


SurfPhase::SurfPhase(XML_Node& xmlphase) :
    ThermoPhase(),
    m_n0(0.0),
    m_logn0(0.0),
    m_tmin(0.0),
    m_tmax(0.0),
    m_press(OneAtm),
    m_tlast(0.0)
{
    const XML_Node& th = xmlphase.child("thermo");
    string model = th["model"];
    if (model != "Surface" && model != "Edge") {
        throw CanteraError("SurfPhase::SurfPhase",
                           "thermo model attribute must be Surface or Edge");
    }
    importPhase(xmlphase, this);
}

// Copy Constructor
/*
 * Copy constructor for the object. Constructed
 * object will be a clone of this object, but will
 * also own all of its data.
 * This is a wrapper around the assignment operator
 *
 * @param right Object to be copied.
 */
SurfPhase::SurfPhase(const SurfPhase& right) :
    m_n0(right.m_n0),
    m_logn0(right.m_logn0),
    m_tmin(right.m_tmin),
    m_tmax(right.m_tmax),
    m_press(right.m_press),
    m_tlast(right.m_tlast)
{
    *this = operator=(right);
}

// Assignment operator
/*
 * Assignment operator for the object. Constructed
 * object will be a clone of this object, but will
 * also own all of its data.
 *
 * @param right Object to be copied.
 */
SurfPhase& SurfPhase::
operator=(const SurfPhase& right)
{
    if (&right != this) {
        ThermoPhase::operator=(right);
        m_n0         = right.m_n0;
        m_logn0      = right.m_logn0;
        m_tmin       = right.m_tmin;
        m_tmax       = right.m_tmax;
        m_press      = right.m_press;
        m_tlast      = right.m_tlast;
        m_h0         = right.m_h0;
        m_s0         = right.m_s0;
        m_cp0        = right.m_cp0;
        m_mu0        = right.m_mu0;
        m_work       = right.m_work;
        m_pe         = right.m_pe;
        m_logsize    = right.m_logsize;
    }
    return *this;
}

// Duplicator from the %ThermoPhase parent class
/*
 * Given a pointer to a %ThermoPhase object, this function will
 * duplicate the %ThermoPhase object and all underlying structures.
 * This is basically a wrapper around the copy constructor.
 *
 * @return returns a pointer to a %ThermoPhase
 */
ThermoPhase* SurfPhase::duplMyselfAsThermoPhase() const
{
    SurfPhase* igp = new SurfPhase(*this);
    return (ThermoPhase*) igp;
}

doublereal SurfPhase::
enthalpy_mole() const
{
    if (m_n0 <= 0.0) {
        return 0.0;
    }
    _updateThermo();
    return mean_X(DATA_PTR(m_h0));
}

SurfPhase::~SurfPhase()
{
}

/*
 * For a surface phase, the pressure is not a relevant
 * thermodynamic variable, and so the Enthalpy is equal to the
 * internal energy.
 */
doublereal SurfPhase::
intEnergy_mole() const
{
    return enthalpy_mole();
}

/*
 * Get the array of partial molar enthalpies of the species
 * units = J / kmol
 */
void SurfPhase::getPartialMolarEnthalpies(doublereal* hbar) const
{
    getEnthalpy_RT(hbar);
    doublereal rt = GasConstant * temperature();
    for (size_t k = 0; k < m_kk; k++) {
        hbar[k] *= rt;
    }
}

// Returns an array of partial molar entropies of the species in the
// solution. Units: J/kmol/K.
/*
 * @param sbar    Output vector of species partial molar entropies.
 *                Length = m_kk. units are J/kmol/K.
 */
void SurfPhase::getPartialMolarEntropies(doublereal* sbar) const
{
    getEntropy_R(sbar);
    for (size_t k = 0; k < m_kk; k++) {
        sbar[k] *= GasConstant;
    }
}

// Returns an array of partial molar heat capacities of the species in the
// solution. Units: J/kmol/K.
/*
 * @param sbar    Output vector of species partial molar entropies.
 *                Length = m_kk. units are J/kmol/K.
 */
void SurfPhase::getPartialMolarCp(doublereal* cpbar) const
{
    getCp_R(cpbar);
    for (size_t k = 0; k < m_kk; k++) {
        cpbar[k] *= GasConstant;
    }
}

// HKM 9/1/11  The partial molar volumes returned here are really partial molar areas.
//             Partial molar volumes for this phase should actually be equal to zero.
void SurfPhase::getPartialMolarVolumes(doublereal* vbar) const
{
    getStandardVolumes(vbar);
}

void SurfPhase::getStandardChemPotentials(doublereal* mu0) const
{
    _updateThermo();
    copy(m_mu0.begin(), m_mu0.end(), mu0);
}

void SurfPhase::getChemPotentials(doublereal* mu) const
{
    _updateThermo();
    copy(m_mu0.begin(), m_mu0.end(), mu);
    getActivityConcentrations(DATA_PTR(m_work));
    for (size_t k = 0; k < m_kk; k++) {
        mu[k] += GasConstant * temperature() *
                 (log(m_work[k]) - logStandardConc(k));
    }
}

void SurfPhase::getActivityConcentrations(doublereal* c) const
{
    getConcentrations(c);
}

doublereal SurfPhase::standardConcentration(size_t k) const
{
    return m_n0/size(k);
}

doublereal SurfPhase::logStandardConc(size_t k) const
{
    return m_logn0 - m_logsize[k];
}

/// The only parameter that can be set is the site density.
void SurfPhase::setParameters(int n, doublereal* const c)
{
    if (n != 1) {
        throw CanteraError("SurfPhase::setParameters",
                           "Bad value for number of parameter");
    }
    m_n0 = c[0];
    if (m_n0 <= 0.0) {
        throw CanteraError("SurfPhase::setParameters",
                           "Bad value for parameter");
    }
    m_logn0 = log(m_n0);
}

void SurfPhase::getGibbs_RT(doublereal* grt) const
{
    _updateThermo();
    double rrt = 1.0/(GasConstant*temperature());
    scale(m_mu0.begin(), m_mu0.end(), grt, rrt);
}

void SurfPhase::
getEnthalpy_RT(doublereal* hrt) const
{
    _updateThermo();
    double rrt = 1.0/(GasConstant*temperature());
    scale(m_h0.begin(), m_h0.end(), hrt, rrt);
}

void SurfPhase::getEntropy_R(doublereal* sr) const
{
    _updateThermo();
    double rr = 1.0/GasConstant;
    scale(m_s0.begin(), m_s0.end(), sr, rr);
}

void SurfPhase::getCp_R(doublereal* cpr) const
{
    _updateThermo();
    double rr = 1.0/GasConstant;
    scale(m_cp0.begin(), m_cp0.end(), cpr, rr);
}

void SurfPhase::getStandardVolumes(doublereal* vol) const
{
    _updateThermo();
    for (size_t k = 0; k < m_kk; k++) {
        vol[k] = 1.0/standardConcentration(k);
    }
}

void SurfPhase::getGibbs_RT_ref(doublereal* grt) const
{
    getGibbs_RT(grt);
}

void SurfPhase::getEnthalpy_RT_ref(doublereal* hrt) const
{
    getEnthalpy_RT(hrt);
}

void SurfPhase::getEntropy_R_ref(doublereal* sr) const
{
    getEntropy_R(sr);
}

void SurfPhase::getCp_R_ref(doublereal* cprt) const
{
    getCp_R(cprt);
}

void SurfPhase::initThermo()
{
    if (m_kk == 0) {
        throw CanteraError("SurfPhase::initThermo",
                           "Number of species is equal to zero");
    }
    m_h0.resize(m_kk);
    m_s0.resize(m_kk);
    m_cp0.resize(m_kk);
    m_mu0.resize(m_kk);
    m_work.resize(m_kk);
    m_pe.resize(m_kk, 0.0);
    vector_fp cov(m_kk, 0.0);
    cov[0] = 1.0;
    setCoverages(DATA_PTR(cov));
    m_logsize.resize(m_kk);
    for (size_t k = 0; k < m_kk; k++) {
        m_logsize[k] = log(size(k));
    }
}

void SurfPhase::setPotentialEnergy(int k, doublereal pe)
{
    m_pe[k] = pe;
    _updateThermo(true);
}

void SurfPhase::setSiteDensity(doublereal n0)
{
    doublereal x = n0;
    setParameters(1, &x);
}

//void SurfPhase::
//setElectricPotential(doublereal V) {
//    for (int k = 0; k < m_kk; k++) {
//        m_pe[k] = charge(k)*Faraday*V;
//    }
//    _updateThermo(true);
//}


/**
 * Set the coverage fractions to a specified
 * state. This routine converts to concentrations
 * in kmol/m2, using m_n0, the surface site density,
 * and size(k), which is defined to be the number of
 * surface sites occupied by the kth molecule.
 * It then calls State::setConcentrations to set the
 * internal concentration in the object.
 */
void SurfPhase::
setCoverages(const doublereal* theta)
{
    double sum = 0.0;
    for (size_t k = 0; k < m_kk; k++) {
        sum += theta[k];
    }
    if (sum <= 0.0) {
        for (size_t k = 0; k < m_kk; k++) {
            cout << "theta(" << k << ") = " << theta[k] << endl;
        }
        throw CanteraError("SurfPhase::setCoverages",
                           "Sum of Coverage fractions is zero or negative");
    }
    for (size_t k = 0; k < m_kk; k++) {
        m_work[k] = m_n0*theta[k]/(sum*size(k));
    }
    /*
     * Call the State:: class function
     * setConcentrations.
     */
    setConcentrations(DATA_PTR(m_work));
}

void SurfPhase::
setCoveragesNoNorm(const doublereal* theta)
{
    for (size_t k = 0; k < m_kk; k++) {
        m_work[k] = m_n0*theta[k]/(size(k));
    }
    /*
     * Call the State:: class function
     * setConcentrations.
     */
    setConcentrations(DATA_PTR(m_work));
}

void SurfPhase::
getCoverages(doublereal* theta) const
{
    getConcentrations(theta);
    for (size_t k = 0; k < m_kk; k++) {
        theta[k] *= size(k)/m_n0;
    }
}

void SurfPhase::
setCoveragesByName(std::string cov)
{
    size_t kk = nSpecies();
    compositionMap cc;
    for (size_t k = 0; k < kk; k++) {
        cc[speciesName(k)] = -1.0;
    }
    parseCompString(cov, cc);
    doublereal c;
    vector_fp cv(kk, 0.0);
    bool ifound = false;
    for (size_t k = 0; k < kk; k++) {
        c = cc[speciesName(k)];
        if (c > 0.0) {
            ifound = true;
            cv[k] = c;
        }
    }
    if (!ifound) {
        throw CanteraError("SurfPhase::setCoveragesByName",
                           "Input coverages are all zero or negative");
    }
    setCoverages(DATA_PTR(cv));
}


void SurfPhase::
_updateThermo(bool force) const
{
    doublereal tnow = temperature();
    if (m_tlast != tnow || force) {
        m_spthermo->update(tnow, DATA_PTR(m_cp0), DATA_PTR(m_h0),
                           DATA_PTR(m_s0));
        m_tlast = tnow;
        doublereal rt = GasConstant * tnow;
        for (size_t k = 0; k < m_kk; k++) {
            m_h0[k] *= rt;
            m_s0[k] *= GasConstant;
            m_cp0[k] *= GasConstant;
            m_mu0[k] = m_h0[k] - tnow*m_s0[k];
        }
        m_tlast = tnow;
    }
}

void SurfPhase::
setParametersFromXML(const XML_Node& eosdata)
{
    eosdata._require("model","Surface");
    doublereal n = getFloat(eosdata, "site_density", "toSI");
    if (n <= 0.0)
        throw CanteraError("SurfPhase::setParametersFromXML",
                           "missing or negative site density");
    m_n0 = n;
    m_logn0 = log(m_n0);
}


void SurfPhase::setStateFromXML(const XML_Node& state)
{

    double t;
    if (getOptionalFloat(state, "temperature", t, "temperature")) {
        setTemperature(t);
    }

    if (state.hasChild("coverages")) {
        string comp = getChildValue(state,"coverages");
        setCoveragesByName(comp);
    }
}

// Default constructor
EdgePhase::EdgePhase(doublereal n0) : SurfPhase(n0)
{
    setNDim(1);
}

// Copy Constructor
/*
 * @param right Object to be copied
 */
EdgePhase::EdgePhase(const EdgePhase& right) :
    SurfPhase(right.m_n0)
{
    setNDim(1);
    *this = operator=(right);
}

// Assignment Operator
/*
 * @param right Object to be copied
 */
EdgePhase& EdgePhase::operator=(const EdgePhase& right)
{
    if (&right != this) {
        SurfPhase::operator=(right);
        setNDim(1);
    }
    return *this;
}

// Duplicator from the %ThermoPhase parent class
/*
 * Given a pointer to a %ThermoPhase object, this function will
 * duplicate the %ThermoPhase object and all underlying structures.
 * This is basically a wrapper around the copy constructor.
 *
 * @return returns a pointer to a %ThermoPhase
 */
ThermoPhase* EdgePhase::duplMyselfAsThermoPhase() const
{
    EdgePhase* igp = new EdgePhase(*this);
    return (ThermoPhase*) igp;
}

void EdgePhase::
setParametersFromXML(const XML_Node& eosdata)
{
    eosdata._require("model","Edge");
    doublereal n = getFloat(eosdata, "site_density", "toSI");
    if (n <= 0.0)
        throw CanteraError("EdgePhase::setParametersFromXML",
                           "missing or negative site density");
    m_n0 = n;
    m_logn0 = log(m_n0);
}


}
