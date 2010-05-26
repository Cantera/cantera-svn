/**
 *  @file GibbsExcessVPSSTP.cpp
 *   Definitions for intermediate ThermoPhase object for phases which
 *   employ excess gibbs free energy formulations
 *  (see \ref thermoprops 
 * and class \link Cantera::GibbsExcessVPSSTP GibbsExcessVPSSTP\endlink).
 *
 * Header file for a derived class of ThermoPhase that handles
 * variable pressure standard state methods for calculating
 * thermodynamic properties that are further based upon expressions
 * for the excess gibbs free energy expressed as a function of
 * the mole fractions.
 */
/*
 * Copywrite (2009) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */
/*
 *  $Date$
 *  $Revision$
 */


#include "GibbsExcessVPSSTP.h"
#include <iomanip>
using namespace std;

namespace Cantera {

  /*
   * Default constructor.
   *
   */
  GibbsExcessVPSSTP::GibbsExcessVPSSTP() :
    VPStandardStateTP()
  {
  }

  /*
   * Copy Constructor:
   *
   *  Note this stuff will not work until the underlying phase
   *  has a working copy constructor
   */
  GibbsExcessVPSSTP::GibbsExcessVPSSTP(const GibbsExcessVPSSTP &b) :
    VPStandardStateTP()
  {
    GibbsExcessVPSSTP::operator=(b);
  }

  /*
   * operator=()
   *
   *  Note this stuff will not work until the underlying phase
   *  has a working assignment operator
   */
  GibbsExcessVPSSTP& GibbsExcessVPSSTP::
  operator=(const GibbsExcessVPSSTP &b) {
    if (&b == this) {
      return *this;
    }

    VPStandardStateTP::operator=(b);

    moleFractions_       = b.moleFractions_;
    lnActCoeff_Scaled_   = b.lnActCoeff_Scaled_;
    dlnActCoeffdT_Scaled_   = b.dlnActCoeffdT_Scaled_;
    d2lnActCoeffdT2_Scaled_   = b.d2lnActCoeffdT2_Scaled_;
    dlnActCoeffdlnX_Scaled_ = b.dlnActCoeffdlnX_Scaled_;
    dlnActCoeffdlnN_Scaled_ = b.dlnActCoeffdlnN_Scaled_;
    m_pp                 = b.m_pp;

    return *this;
  }

  /*
   *
   * ~GibbsExcessVPSSTP():   (virtual)
   *
   * Destructor: does nothing:
   *
   */
  GibbsExcessVPSSTP::~GibbsExcessVPSSTP() {
  }

  /*
   * This routine duplicates the current object and returns
   * a pointer to ThermoPhase.
   */
  ThermoPhase* 
  GibbsExcessVPSSTP::duplMyselfAsThermoPhase() const {
    GibbsExcessVPSSTP* mtp = new GibbsExcessVPSSTP(*this);
    return (ThermoPhase *) mtp;
  }

  /*
   *  -------------- Utilities -------------------------------
   */

  void GibbsExcessVPSSTP::setMassFractions(const doublereal* const y) {
    State::setMassFractions(y);
    getMoleFractions(DATA_PTR(moleFractions_));
  }

  void GibbsExcessVPSSTP::setMassFractions_NoNorm(const doublereal* const y) {
    State::setMassFractions_NoNorm(y);
    getMoleFractions(DATA_PTR(moleFractions_));
  }

 void GibbsExcessVPSSTP::setMoleFractions(const doublereal* const x) {
    State::setMoleFractions(x);
    getMoleFractions(DATA_PTR(moleFractions_));
  }

  void GibbsExcessVPSSTP::setMoleFractions_NoNorm(const doublereal* const x) {
    State::setMoleFractions_NoNorm(x);
    getMoleFractions(DATA_PTR(moleFractions_));
  }


  void GibbsExcessVPSSTP::setConcentrations(const doublereal* const c) {
    State::setConcentrations(c);
    getMoleFractions(DATA_PTR(moleFractions_));
  }


  // Equation of state type flag.
  /*
   * The ThermoPhase base class returns
   * zero. Subclasses should define this to return a unique
   * non-zero value. Known constants defined for this purpose are
   * listed in mix_defs.h. The GibbsExcessVPSSTP class also returns
   * zero, as it is a non-complete class.
   */
  int GibbsExcessVPSSTP::eosType() const { 
    return 0;
  }

 

  /*
   * ------------ Molar Thermodynamic Properties ----------------------
   */

  /*
   *
   * ------------ Mechanical Properties ------------------------------
   *
   */

  /*
   * Set the pressure at constant temperature. Units: Pa.
   * This method sets a constant within the object.
   * The mass density is not a function of pressure.
   */
  void GibbsExcessVPSSTP::setPressure(doublereal p) {
    setState_TP(temperature(), p);
  }

  void GibbsExcessVPSSTP::calcDensity() {
    doublereal* vbar = NULL;
    vbar = new doublereal[m_kk];
    //    double *vbar = &m_pp[0];
    getPartialMolarVolumes(vbar);
   
    doublereal vtotal = 0.0;
    for (int i = 0; i < m_kk; i++) {
      vtotal += vbar[i] * moleFractions_[i];
    }
    doublereal dd = meanMolecularWeight() / vtotal;
    State::setDensity(dd);
    delete [] vbar;
  }

  void GibbsExcessVPSSTP::setState_TP(doublereal t, doublereal p) {
    State::setTemperature(t);
    /*
     * Store the current pressure
     */
    m_Pcurrent = p;
    /*
     * update the standard state thermo
     * -> This involves calling the water function and setting the pressure
     */
    updateStandardStateThermo();
  
    /*
     * Calculate the partial molar volumes, and then the density of the fluid
     */
    calcDensity();
  }
 


  /*
   * - Activities, Standard States, Activity Concentrations -----------
   */


  doublereal GibbsExcessVPSSTP::standardConcentration(int k) const {
    err("standardConcentration");
    return -1.0;
  }

  doublereal GibbsExcessVPSSTP::logStandardConc(int k) const {
    err("logStandardConc");
    return -1.0;
  }

  void GibbsExcessVPSSTP::getActivities(doublereal* ac) const {
    getActivityCoefficients(ac);
    getMoleFractions(DATA_PTR(moleFractions_));
    for (int k = 0; k < m_kk; k++) {
      ac[k] *= moleFractions_[k];
    }
  }


  void GibbsExcessVPSSTP::getElectrochemPotentials(doublereal* mu) const {
    getChemPotentials(mu);
    double ve = Faraday * electricPotential();
    for (int k = 0; k < m_kk; k++) {
      mu[k] += ve*charge(k);
    }
  }
  
  /*
   * ------------ Partial Molar Properties of the Solution ------------
   */

  // Return an array of partial molar volumes for the
  // species in the mixture. Units: m^3/kmol.
  /*
   *  Frequently, for this class of thermodynamics representations,
   *  the excess Volume due to mixing is zero. Here, we set it as
   *  a default. It may be overriden in derived classes.
   *
   *  @param vbar   Output vector of speciar partial molar volumes.
   *                Length = m_kk. units are m^3/kmol.
   */
  void GibbsExcessVPSSTP::getPartialMolarVolumes(doublereal* vbar) const {
    /*
     * Get the standard state values in m^3 kmol-1
     */
    getStandardVolumes(vbar);
  }
  


  doublereal GibbsExcessVPSSTP::err(std::string msg) const {
    throw CanteraError("GibbsExcessVPSSTP","Base class method "
		       +msg+" called. Equation of state type: "+int2str(eosType()));
    return 0;
  }

 

  double GibbsExcessVPSSTP::checkMFSum(const doublereal * const x) const {
    doublereal norm = accumulate(x, x + m_kk, 0.0);
    if (fabs(norm - 1.0) > 1.0E-9) {
      throw CanteraError("GibbsExcessVPSSTP::checkMFSum",
			 "(MF sum - 1) exceeded tolerance of 1.0E-9:" + fp2str(norm));
    }
    return norm;
  }

  /*
   * Returns the units of the standard and general concentrations
   * Note they have the same units, as their divisor is 
   * defined to be equal to the activity of the kth species
   * in the solution, which is unitless.
   *
   * This routine is used in print out applications where the
   * units are needed. Usually, MKS units are assumed throughout
   * the program and in the XML input files. 
   *
   * On return uA contains the powers of the units (MKS assumed)
   * of the standard concentrations and generalized concentrations
   * for the kth species.
   *
   *  uA[0] = kmol units - default  = 1
   *  uA[1] = m    units - default  = -nDim(), the number of spatial
   *                                dimensions in the Phase class.
   *  uA[2] = kg   units - default  = 0;
   *  uA[3] = Pa(pressure) units - default = 0;
   *  uA[4] = Temperature units - default = 0;
   *  uA[5] = time units - default = 0
   */
  void GibbsExcessVPSSTP::getUnitsStandardConc(double *uA, int k, int sizeUA) const {
    for (int i = 0; i < sizeUA; i++) {
      if (i == 0) uA[0] = 1.0;
      if (i == 1) uA[1] = -nDim();
      if (i == 2) uA[2] = 0.0;
      if (i == 3) uA[3] = 0.0;
      if (i == 4) uA[4] = 0.0;
      if (i == 5) uA[5] = 0.0;
    }
  }


  /*
   * @internal Initialize. This method is provided to allow
   * subclasses to perform any initialization required after all
   * species have been added. For example, it might be used to
   * resize internal work arrays that must have an entry for
   * each species.  The base class implementation does nothing,
   * and subclasses that do not require initialization do not
   * need to overload this method.  When importing a CTML phase
   * description, this method is called just prior to returning
   * from function importPhase.
   *
   * @see importCTML.cpp
   */
  void GibbsExcessVPSSTP::initThermo() {
    initLengths();
    VPStandardStateTP::initThermo();
  }


  //   Initialize lengths of local variables after all species have
  //   been identified.
  void  GibbsExcessVPSSTP::initLengths() {
    m_kk = nSpecies();
    moleFractions_.resize(m_kk);
    lnActCoeff_Scaled_.resize(m_kk);
    dlnActCoeffdT_Scaled_.resize(m_kk);
    d2lnActCoeffdT2_Scaled_.resize(m_kk);
    dlnActCoeffdlnX_Scaled_.resize(m_kk);
    dlnActCoeffdlnN_Scaled_.resize(m_kk);
    m_pp.resize(m_kk);
  }
  
 
}

