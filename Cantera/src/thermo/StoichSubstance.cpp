/**
 *  @file StoichSubstance.cpp
 *  This file contains the class definitions for the StoichSubstance
 *  ThermoPhase class.
 */

//  Copyright 2001 California Institute of Technology

#include "ct_defs.h"
#include "mix_defs.h"
#include "StoichSubstance.h"
#include "SpeciesThermo.h"

namespace Cantera {

  // Default empty constructor
  StoichSubstance::StoichSubstance() :
    m_kk(0),
    m_tmin(0.0),
    m_tmax(0.0),
    m_press(OneAtm),
    m_p0(OneAtm),
    m_tlast(-1.0)  {
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
  StoichSubstance::StoichSubstance(const StoichSubstance &right) :
	m_kk(0),
	m_tmin(0.0),
	m_tmax(0.0),
	m_press(OneAtm),
	m_p0(OneAtm),
	m_tlast(-1.0)  { 
    *this = operator=(right);
  }

  // Asignment operator
  /*
   * Assignment operator for the object. Constructed
   * object will be a clone of this object, but will
   * also own all of its data.
   *
   * @param right Object to be copied.
   */
  StoichSubstance& StoichSubstance::
  operator=(const StoichSubstance &right) {
    if (&right != this) {
      ThermoPhase::operator=(right);
      m_kk      = right.m_kk;
      m_tmin    = right.m_tmin;
      m_tmax    = right.m_tmax;
      m_press   = right.m_press;
      m_p0      = right.m_p0;
      m_tlast   = right.m_tlast;
      m_h0_RT   = right.m_h0_RT;
      m_cp0_R   = right.m_cp0_R;
      m_s0_R    = right.m_s0_R;
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
  ThermoPhase *StoichSubstance::duplMyselfAsThermoPhase() const {
    ThermoPhase *igp = new StoichSubstance(*this);
    return (ThermoPhase *) igp;
  }

  // Destructor
  StoichSubstance::~StoichSubstance() {
  }

  doublereal StoichSubstance::enthalpy_mole() const {
    double hh = intEnergy_mole() + m_press / molarDensity();
    return hh;
  }

  doublereal StoichSubstance::intEnergy_mole() const {
    _updateThermo();
    return GasConstant * temperature() * m_h0_RT[0]
      - m_p0 / molarDensity();
  }

  doublereal StoichSubstance::entropy_mole() const {
    _updateThermo();
    return GasConstant * m_s0_R[0];
  }

  doublereal StoichSubstance::gibbs_mole() const {
    return enthalpy_mole() - temperature() * entropy_mole();
  }

  doublereal StoichSubstance::cp_mole() const {
    _updateThermo();
    return GasConstant * m_cp0_R[0];
  }

  doublereal StoichSubstance::cv_mole() const {
    return cp_mole();
  }

  void StoichSubstance::initThermo() {
    m_kk = nSpecies();
    if (m_kk > 1) {
      throw CanteraError("initThermo",
			 "stoichiometric substances may only contain one species.");
    } 
    doublereal tmin = m_spthermo->minTemp();
    doublereal tmax = m_spthermo->maxTemp();
    if (tmin > 0.0) m_tmin = tmin;
    if (tmax > 0.0) m_tmax = tmax;
    m_p0 = refPressure();

    m_h0_RT.resize(m_kk);
    m_cp0_R.resize(m_kk);
    m_s0_R.resize(m_kk);

    // Put the object on a valid temperature point.
    double tnow = 300.;
    if (tnow > tmin && tnow < tmax) {

    } else {
      tnow = 0.1 * (9 * tmin + tmax);
    }
    setState_TP(tnow, m_p0);
  }

  void StoichSubstance::_updateThermo() const {
    doublereal tnow = temperature();
    if (m_tlast != tnow) {
      m_spthermo->update(tnow, &m_cp0_R[0], &m_h0_RT[0], 
			 &m_s0_R[0]);
      m_tlast = tnow;
    }
  }

  doublereal StoichSubstance::pressure() const {
    return m_press;
  }

  void StoichSubstance::setPressure(doublereal p) {
    m_press = p;
  }

  void StoichSubstance::getActivityConcentrations(doublereal* c) const {
    c[0] = 1.0;
  }

  doublereal StoichSubstance::standardConcentration(size_t k) const {
    return 1.0;
  }

  doublereal StoichSubstance::logStandardConc(size_t k) const {
    return 0.0;
  }

  void StoichSubstance::getStandardChemPotentials(doublereal*  mu0) const {
    mu0[0] = gibbs_mole();
  }

  void StoichSubstance::
  getUnitsStandardConc(double *uA, int k, int sizeUA) const {
    for (int i = 0; i < sizeUA; i++) {
      uA[i] = 0.0;
    }
  }

  void StoichSubstance::getChemPotentials_RT(doublereal* mu) const {
    mu[0] = gibbs_mole() / (GasConstant * temperature());
  }

  void StoichSubstance::getChemPotentials(doublereal* mu) const {
    mu[0] = gibbs_mole();
  }

  void StoichSubstance::getElectrochemPotentials(doublereal* mu) const {
    getChemPotentials(mu);
  }

  void StoichSubstance::getPartialMolarEnthalpies(doublereal* hbar) const {
    hbar[0] = enthalpy_mole();
  }

  void StoichSubstance::getPartialMolarEntropies(doublereal* sbar) const {
    sbar[0] = entropy_mole();
  }

  void StoichSubstance::getPartialMolarVolumes(doublereal* vbar) const {
    vbar[0] = 1.0 / molarDensity();
  }

  void StoichSubstance::getEnthalpy_RT(doublereal* hrt) const {
    hrt[0] = enthalpy_mole() / (GasConstant * temperature());
  }

  void StoichSubstance::getEntropy_R(doublereal* sr) const {
    sr[0] = entropy_mole() / GasConstant;
  }

  void StoichSubstance::getGibbs_RT(doublereal* grt) const {
    grt[0] =  gibbs_mole() / (GasConstant * temperature());
  }

  void StoichSubstance::getPureGibbs(doublereal* gpure) const {
    gpure[0] = gibbs_mole();
  }

  void StoichSubstance::getCp_R(doublereal* cpr) const {
    cpr[0] = cp_mole() / GasConstant;
  }

  void StoichSubstance::getStandardVolumes(doublereal*vol) const {
    vol[0] = 1.0 / molarDensity();
  }

  void StoichSubstance::getEnthalpy_RT_ref(doublereal *hrt) const {
    _updateThermo();
    hrt[0] = m_h0_RT[0];
  }

  void StoichSubstance::getGibbs_RT_ref(doublereal *grt) const {
    _updateThermo();
    grt[0] = m_h0_RT[0] - m_s0_R[0];
  }

  void StoichSubstance::getGibbs_ref(doublereal *g) const {
    getGibbs_RT_ref(g);
    g[0] *= GasConstant * temperature();
  }

  void StoichSubstance::getEntropy_R_ref(doublereal *er) const {
    _updateThermo();
    er[0] = m_s0_R[0];
  }

  void StoichSubstance::getCp_R_ref(doublereal* cprt) const {
    _updateThermo();
    cprt[0] = m_cp0_R[0];
  }

  void StoichSubstance::setParameters(int n, double * const c) {
    double rho = c[0];
    setDensity(rho);
  }

  void StoichSubstance::getParameters(int &n, double * const c) const {
    double rho = density();
    c[0] = rho;
  }

  void StoichSubstance::setParametersFromXML(const XML_Node& eosdata) {
    eosdata._require("model","StoichSubstance");
    doublereal rho = ctml::getFloat(eosdata, "density", "toSI");
    setDensity(rho);
  }

}




