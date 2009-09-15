/**
 *  @file WaterSSTP.cpp
 * Definitions for a %ThermoPhase class consisting of  pure water (see \ref thermoprops
 * and class \link Cantera::WaterSSTP WaterSSTP\endlink).
 */
/*
 * Copywrite (2006) Sandia Corporation. Under the terms of
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */
/*
 * $Id: WaterSSTP.cpp,v 1.17 2009/03/27 00:38:58 hkmoffa Exp $
 */

#include "xml.h"
#include "WaterSSTP.h"
#include "WaterPropsIAPWS.h"
//#include "importCTML.h"
#include "WaterProps.h"
#include "ThermoFactory.h"
#include <cmath>

using namespace std;

namespace Cantera {
  /**
   * Basic list of constructors and duplicators
   */

  WaterSSTP::WaterSSTP() :
    SingleSpeciesTP(),
    m_sub(0),
    m_waterProps(0),
    m_mw(0.0),
    EW_Offset(0.0),
    SW_Offset(0.0),
    m_verbose(0),
    m_ready(false),
    m_allowGasPhase(false)
  {
    //constructPhase();
  }


  WaterSSTP::WaterSSTP(std::string inputFile, std::string id) :
    SingleSpeciesTP(),
    m_sub(0),
    m_waterProps(0),
    m_mw(0.0),
    EW_Offset(0.0),
    SW_Offset(0.0),
    m_verbose(0),
    m_ready(false),
    m_allowGasPhase(false)
  {
    constructPhaseFile(inputFile, id);
  }


  WaterSSTP::WaterSSTP(XML_Node& phaseRoot, std::string id) :
    SingleSpeciesTP(),
    m_sub(0),
    m_waterProps(0),
    m_mw(0.0),
    EW_Offset(0.0),
    SW_Offset(0.0),
    m_verbose(0),
    m_ready(false),
    m_allowGasPhase(false)
  {
    constructPhaseXML(phaseRoot, id) ;
  }



  WaterSSTP::WaterSSTP(const WaterSSTP &b) :
    SingleSpeciesTP(b),
    m_sub(0),
    m_waterProps(0),
    m_mw(b.m_mw),
    EW_Offset(b.EW_Offset),
    SW_Offset(b.SW_Offset),
    m_verbose(b.m_verbose),
    m_ready(false),
    m_allowGasPhase(b.m_allowGasPhase)
  {
    m_sub = new WaterPropsIAPWS(*(b.m_sub));
    m_waterProps =  new WaterProps(m_sub);

    /*
     * Use the assignment operator to do the brunt
     * of the work for the copy construtor.
     */
    *this = b;
  }

  /*
   * Assignment operator
   */
  WaterSSTP& WaterSSTP::operator=(const WaterSSTP&b) {
    if (&b == this) return *this;
    m_sub->operator=(*(b.m_sub));

    if (!m_waterProps) {
      m_waterProps = new WaterProps(m_sub);
    }
    m_waterProps->operator=(*(b.m_waterProps));


    m_mw = b.m_mw;
    m_verbose = b.m_verbose;
    m_ready = b.m_ready;
    m_allowGasPhase = b.m_allowGasPhase;
    return *this;
  }


  ThermoPhase *WaterSSTP::duplMyselfAsThermoPhase() const {
    WaterSSTP* wtp = new WaterSSTP(*this);
    return (ThermoPhase *) wtp;
  }

  WaterSSTP::~WaterSSTP() { 
    delete m_sub; 
    delete m_waterProps;
  }


  
  void WaterSSTP::constructPhase() {
    throw CanteraError("WaterSSTP::constructPhase()", "unimplemented");

  }

   
  /*
   * @param infile XML file containing the description of the
   *        phase
   *
   * @param id  Optional parameter identifying the name of the
   *            phase. If none is given, the first XML
   *            phase element will be used.
   */
  void WaterSSTP::constructPhaseXML(XML_Node& phaseNode, std::string id) {

    /*
     * Call the Cantera importPhase() function. This will import
     * all of the species into the phase. This will also handle
     * all of the solvent and solute standard states.
     */
    bool m_ok = importPhase(phaseNode, this);
    if (!m_ok) {
      throw CanteraError("initThermo","importPhase failed ");
    }
	
  }

  /*
   * constructPhaseFile
   *
   *
   * This routine is a precursor to constructPhaseXML(XML_Node*)
   * routine, which does most of the work.
   *
   * @param inputFile XML file containing the description of the
   *        phase
   *
   * @param id  Optional parameter identifying the name of the
   *            phase. If none is given, the first XML
   *            phase element will be used.
   */
  void WaterSSTP::constructPhaseFile(std::string inputFile, std::string id) {

    if (inputFile.size() == 0) {
      throw CanteraError("WaterTp::initThermo",
			 "input file is null");
    }
    std::string path = findInputFile(inputFile);
    std::ifstream fin(path.c_str());
    if (!fin) {
      throw CanteraError("WaterSSTP::initThermo","could not open "
			 +path+" for reading.");
    }
    /*
     * The phase object automatically constructs an XML object.
     * Use this object to store information.
     */
    XML_Node &phaseNode_XML = xml();
    XML_Node *fxml = new XML_Node();
    fxml->build(fin);
    XML_Node *fxml_phase = findXMLPhase(fxml, id);
    if (!fxml_phase) {
      throw CanteraError("WaterSSTP::initThermo",
			 "ERROR: Can not find phase named " +
			 id + " in file named " + inputFile);
    }
    fxml_phase->copy(&phaseNode_XML);	
    constructPhaseXML(*fxml_phase, id);
    delete fxml;
  }



  void WaterSSTP::initThermo() {
    SingleSpeciesTP::initThermo();
  }

  void WaterSSTP::
  initThermoXML(XML_Node& phaseNode, std::string id) {

    /*
     * Do initializations that don't depend on knowing the XML file
     */
    initThermo();
    if (m_sub) delete m_sub;
    m_sub = new WaterPropsIAPWS();
    if (m_sub == 0) {
      throw CanteraError("WaterSSTP::initThermo",
			 "could not create new substance object.");
    }
    /*
     * Calculate the molecular weight. Note while there may
     * be a very good calculated weight in the steam table
     * class, using this weight may lead to codes exhibiting
     * mass loss issues. We need to grab the elemental
     * atomic weights used in the Element class and calculate
     * a consistent H2O molecular weight based on that.
     */
    int nH = elementIndex("H");
    if (nH < 0) {
      throw CanteraError("WaterSSTP::initThermo",
			 "H not an element");
    }
    double mw_H = atomicWeight(nH);
    int nO = elementIndex("O");
    if (nO < 0) {
      throw CanteraError("WaterSSTP::initThermo",
			 "O not an element");
    }
    double mw_O = atomicWeight(nO);
    m_mw = 2.0 * mw_H + mw_O;
    m_weight[0] = m_mw;
    setMolecularWeight(0,m_mw);
    double one = 1.0;
    setMoleFractions(&one);

    /*
     * Set the baseline 
     */
    doublereal T = 298.15;
    State::setDensity(7.0E-8);
    State::setTemperature(T);

    doublereal presLow = 1.0E-2;
    doublereal oneBar = 1.0E5;
    doublereal dd = m_sub->density(T, presLow, WATER_GAS, 7.0E-8);
    setDensity(dd);
    setTemperature(T);
    SW_Offset = 0.0;
    doublereal s = entropy_mole();
    s -=  GasConstant * log(oneBar/presLow);
    if (s != 188.835E3) {
      SW_Offset = 188.835E3 - s;
    }
    s = entropy_mole();
    s -=  GasConstant * log(oneBar/presLow);
    //printf("s = %g\n", s);

    doublereal h = enthalpy_mole();
    if (h != -241.826E6) {
      EW_Offset = -241.826E6 - h;
    }
    h = enthalpy_mole();

    //printf("h = %g\n", h);


    /*
     * Set the initial state of the system to 298.15 K and 
     * 1 bar.
     */
    setTemperature(298.15);
    double rho0 = m_sub->density(298.15, OneAtm, WATER_LIQUID);
    setDensity(rho0);

    m_waterProps =  new WaterProps(m_sub);


    /*
     * We have to do something with the thermo function here.
     */
    if (m_spthermo) {
      delete m_spthermo;
      m_spthermo = 0;
    }

    /*
     * Set the flag to say we are ready to calculate stuff
     */
    m_ready = true;
  }

  void WaterSSTP::
  setParametersFromXML(const XML_Node& eosdata) {
    eosdata._require("model","PureLiquidWater");
  }

  /*
   * Return the molar dimensionless enthalpy
   */
  void WaterSSTP::getEnthalpy_RT(doublereal* hrt) const {
    double T = temperature();
    doublereal h = m_sub->enthalpy();
    *hrt = (h + EW_Offset)/(GasConstant*T);
  }

  /*
   * Calculate the internal energy in mks units of
   * J kmol-1 
   */
  void WaterSSTP::getIntEnergy_RT(doublereal *ubar) const {
    doublereal u = m_sub->intEnergy();
    *ubar = (u + EW_Offset)/GasConstant;            
  }

  /*
   * Calculate the dimensionless entropy
   */
  void WaterSSTP::getEntropy_R(doublereal* sr) const {
    doublereal s = m_sub->entropy();
    sr[0] = (s + SW_Offset) / GasConstant;
  }

  /*
   * Calculate the Gibbs free energy in mks units of
   * J kmol-1 K-1.
   */
  void WaterSSTP::getGibbs_RT(doublereal *grt) const {
    double T = temperature();
    doublereal g = m_sub->Gibbs();
    *grt = (g + EW_Offset - SW_Offset*T) / (GasConstant * T);
    if (!m_ready) {
      throw CanteraError("waterSSTP::", "Phase not ready");
    }
  }

  /*
   * Calculate the Gibbs free energy in mks units of
   * J kmol-1 K-1.
   */
  void WaterSSTP::getStandardChemPotentials(doublereal *gss) const {
    double T = temperature();
    doublereal g = m_sub->Gibbs();
    *gss = (g + EW_Offset - SW_Offset*T);
    if (!m_ready) {
      throw CanteraError("waterSSTP::", "Phase not ready");
    }
  }
  
  void WaterSSTP::getCp_R(doublereal* cpr) const {
    doublereal cp = m_sub->cp();
    cpr[0] = cp / GasConstant;
  }

  /*
   * Calculate the constant volume heat capacity
   * in mks units of J kmol-1 K-1
   */
  doublereal WaterSSTP::cv_mole() const {
    doublereal cv = m_sub->cv();
    return cv;
  }

  // @name Thermodynamic Values for the Species Reference State


  void WaterSSTP::getEnthalpy_RT_ref(doublereal *hrt) const {
    doublereal p = pressure();
    double T = temperature();
    double dens = density();
    int waterState = WATER_GAS;
    double rc = m_sub->Rhocrit();
    if (dens > rc) {
      waterState = WATER_LIQUID;
    }
    doublereal dd = m_sub->density(T, OneAtm, waterState, dens);
    if (dd <= 0.0) {
      throw CanteraError("setPressure", "error");
    }
    doublereal h = m_sub->enthalpy();
    *hrt = (h + EW_Offset) / (GasConstant * T);
    dd = m_sub->density(T, p, waterState, dens);
  }

  void WaterSSTP::getGibbs_RT_ref(doublereal *grt) const {
    doublereal p = pressure();
    double T = temperature();
    double dens = density();
    int waterState = WATER_GAS;
    double rc = m_sub->Rhocrit();
    if (dens > rc) {
      waterState = WATER_LIQUID;
    }
    doublereal dd = m_sub->density(T, OneAtm, waterState, dens);
    if (dd <= 0.0) {
      throw CanteraError("setPressure", "error");
    }
    m_sub->setState_TR(T, dd);
    doublereal g = m_sub->Gibbs();
    *grt = (g + EW_Offset - SW_Offset*T)/ (GasConstant * T);
    dd = m_sub->density(T, p, waterState, dens);
 
  }

  void WaterSSTP::getGibbs_ref(doublereal *g) const {
    getGibbs_RT_ref(g);
    doublereal rt = _RT();
    for (int k = 0; k < m_kk; k++) {
      g[k] *= rt;
    }
  }

  void WaterSSTP::getEntropy_R_ref(doublereal *sr) const {
    doublereal p = pressure();
    double T = temperature();
    double dens = density();
    int waterState = WATER_GAS;
    double rc = m_sub->Rhocrit();
    if (dens > rc) {
      waterState = WATER_LIQUID;
    }
    doublereal dd = m_sub->density(T, OneAtm, waterState, dens);
   
    if (dd <= 0.0) {
      throw CanteraError("setPressure", "error");
    }
    m_sub->setState_TR(T, dd);

    doublereal s = m_sub->entropy();
    *sr = (s + SW_Offset)/ (GasConstant);
    dd = m_sub->density(T, p, waterState, dens); 
 
  }

  void WaterSSTP::getCp_R_ref(doublereal *cpr) const {
    doublereal p = pressure();
    double T = temperature();
    double dens = density();
    int waterState = WATER_GAS;
    double rc = m_sub->Rhocrit();
    if (dens > rc) {
      waterState = WATER_LIQUID;
    }
    doublereal dd = m_sub->density(T, OneAtm, waterState, dens);
    m_sub->setState_TR(T, dd);
    if (dd <= 0.0) {
      throw CanteraError("setPressure", "error");
    }
    doublereal cp = m_sub->cp();
    *cpr = cp / (GasConstant);
    dd = m_sub->density(T, p, waterState, dens); 
  }

  void WaterSSTP::getStandardVolumes_ref(doublereal *vol) const {
    doublereal p = pressure();
    double T = temperature();
    double dens = density();
    int waterState = WATER_GAS;
    double rc = m_sub->Rhocrit();
    if (dens > rc) {
      waterState = WATER_LIQUID;
    }
    doublereal dd = m_sub->density(T, OneAtm, waterState, dens);
    if (dd <= 0.0) {
      throw CanteraError("setPressure", "error");
    }
    *vol = meanMolecularWeight() /dd;
    dd = m_sub->density(T, p, waterState, dens); 
  }

  /*
   * Calculate the pressure (Pascals), given the temperature and density
   *  Temperature: kelvin
   *  rho: density in kg m-3
   */
  doublereal WaterSSTP::pressure() const {
    doublereal p = m_sub->pressure();
    return p;
  }
        
  void WaterSSTP::
  setPressure(doublereal p) {
    double T = temperature();
    double dens = density();
    int waterState = WATER_GAS;
    double rc = m_sub->Rhocrit();
    if (dens > rc) {
      waterState = WATER_LIQUID;
    }
    doublereal dd = m_sub->density(T, p, waterState, dens);
    if (dd <= 0.0) {
      throw CanteraError("setPressure", "error");
    }
    setDensity(dd);
  }
  
  // Returns  the isothermal compressibility. Units: 1/Pa.
  /*
   * The isothermal compressibility is defined as
   * \f[
   * \kappa_T = -\frac{1}{v}\left(\frac{\partial v}{\partial P}\right)_T
   * \f]
   *  or
   * \f[
   * \kappa_T = \frac{1}{\rho}\left(\frac{\partial \rho}{\partial P}\right)_T
   * \f]
   */
  doublereal WaterSSTP::isothermalCompressibility() const {
    doublereal val = m_sub->isothermalCompressibility();
    return val;
  }

  // Return the volumetric thermal expansion coefficient. Units: 1/K.
  /*
   * The thermal expansion coefficient is defined as
   * \f[
   * \beta = \frac{1}{v}\left(\frac{\partial v}{\partial T}\right)_P
   * \f]
   */
  doublereal WaterSSTP::thermalExpansionCoeff() const {
    doublereal val = m_sub->coeffThermExp();
    return val;
  }

  doublereal WaterSSTP::dthermalExpansionCoeffdT() const {
    doublereal pres = pressure();
    doublereal dens_save = density();
    double T = temperature();
    double tt = T - 0.04;
    doublereal dd = m_sub->density(tt, pres, WATER_LIQUID, dens_save);
    if (dd < 0.0) {
      throw CanteraError("WaterSSTP::dthermalExpansionCoeffdT", 
			 "Unable to solve for the density at T = " + fp2str(tt) + ", P = " + fp2str(pres));
    }
    doublereal vald = m_sub->coeffThermExp();
    m_sub->setState_TR(T, dens_save);
    doublereal val2 = m_sub->coeffThermExp();
    doublereal val = (val2 - vald) / 0.04;
    return val;
  }


  // critical temperature 
  doublereal WaterSSTP::critTemperature() const { return m_sub->Tcrit(); }
        
  // critical pressure
  doublereal WaterSSTP::critPressure() const { return m_sub->Pcrit(); }
        
  // critical density
  doublereal WaterSSTP::critDensity() const { return m_sub->Rhocrit(); }
        

  void WaterSSTP::setTemperature(const doublereal temp) {
    State::setTemperature(temp);
    doublereal dd = density();
    m_sub->setState_TR(temp, dd);
  }

  void WaterSSTP::setDensity(const doublereal dens) {
    State::setDensity(dens);
    doublereal temp = temperature();
    m_sub->setState_TR(temp, dens);
  }

  // saturation pressure
  doublereal WaterSSTP::satPressure(doublereal t) const {
    doublereal tsave = temperature();
    doublereal dsave = density();
    doublereal pp = m_sub->psat(t);
    m_sub->setState_TR(tsave, dsave);
    return pp;
  }

  // Return the fraction of vapor at the current conditions
  doublereal WaterSSTP::vaporFraction() const {
    if (temperature() >= m_sub->Tcrit()) {
      double dens = density();
      if (dens >= m_sub->Rhocrit()) {
	return 0.0;
      }
      return 1.0;
    }
    /*
     * If below tcrit we always return 0 from this class
     */
    return 0.0;
  }


}
