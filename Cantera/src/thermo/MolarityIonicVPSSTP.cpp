/**
 *  @file MolarityIonicVPSSTP.cpp
 *   Definitions for intermediate ThermoPhase object for phases which
 *   employ excess gibbs free energy formulations
 *  (see \ref thermoprops 
 * and class \link Cantera::MolarityIonicVPSSTP MolarityIonicVPSSTP\endlink).
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
 *  $Date: 2009-11-09 16:36:49 -0700 (Mon, 09 Nov 2009) $
 *  $Revision: 255 $
 */


#include "MolarityIonicVPSSTP.h"
#include "ThermoFactory.h"
#include <cmath>

using namespace std;

namespace Cantera {
  static  const double xxSmall = 1.0E-150; 
  //====================================================================================================================
  /*
   * Default constructor.
   *
   */
  MolarityIonicVPSSTP::MolarityIonicVPSSTP() :
    GibbsExcessVPSSTP(),
    PBType_(PBTYPE_PASSTHROUGH),
    numPBSpecies_(m_kk),
    indexSpecialSpecies_(-1),
    numCationSpecies_(0),
    numAnionSpecies_(0),
    numPassThroughSpecies_(0),
    neutralPBindexStart(0)
  {
  } 
  //====================================================================================================================
  /*
   * Working constructors
   *
   *  The two constructors below are the normal way
   *  the phase initializes itself. They are shells that call
   *  the routine initThermo(), with a reference to the
   *  XML database to get the info for the phase.
   */
  MolarityIonicVPSSTP::MolarityIonicVPSSTP(std::string inputFile, std::string id) :
    GibbsExcessVPSSTP(),
    PBType_(PBTYPE_PASSTHROUGH),
    numPBSpecies_(m_kk),
    indexSpecialSpecies_(-1),
    numCationSpecies_(0),
    numAnionSpecies_(0),
    numPassThroughSpecies_(0),
    neutralPBindexStart(0)
  {
    constructPhaseFile(inputFile, id);
  }
 //====================================================================================================================
  MolarityIonicVPSSTP::MolarityIonicVPSSTP(XML_Node& phaseRoot, std::string id) :
    GibbsExcessVPSSTP(),
    PBType_(PBTYPE_PASSTHROUGH),
    numPBSpecies_(m_kk),
    indexSpecialSpecies_(-1),
    numCationSpecies_(0),
    numAnionSpecies_(0),
    numPassThroughSpecies_(0),
    neutralPBindexStart(0)
  {
    constructPhaseXML(phaseRoot, id);
  }
  //====================================================================================================================
  /*
   * Copy Constructor:
   *
   *  Note this stuff will not work until the underlying phase
   *  has a working copy constructor
   */
  MolarityIonicVPSSTP::MolarityIonicVPSSTP(const MolarityIonicVPSSTP &b) :
    GibbsExcessVPSSTP(),
    PBType_(PBTYPE_PASSTHROUGH),
    numPBSpecies_(m_kk),
    indexSpecialSpecies_(-1),
    numCationSpecies_(0),
    numAnionSpecies_(0),
    numPassThroughSpecies_(0),
    neutralPBindexStart(0)
  {
    *this = operator=(b);
  }
  //====================================================================================================================
  /*
   * operator=()
   *
   *  Note this stuff will not work until the underlying phase
   *  has a working assignment operator
   */
  MolarityIonicVPSSTP& MolarityIonicVPSSTP::
  operator=(const MolarityIonicVPSSTP &b) {
    if (&b != this) {
      GibbsExcessVPSSTP::operator=(b);
    }

    PBType_                     = b.PBType_;
    numPBSpecies_               = b.numPBSpecies_;
    indexSpecialSpecies_        = b.indexSpecialSpecies_;
    PBMoleFractions_            = b.PBMoleFractions_;
    cationList_                 = b.cationList_;
    numCationSpecies_           = b.numCationSpecies_;
    anionList_                  = b.anionList_;
    numAnionSpecies_            = b.numAnionSpecies_;
    passThroughList_            = b.passThroughList_;
    numPassThroughSpecies_      = b.numPassThroughSpecies_;
    neutralPBindexStart         = b.neutralPBindexStart;
    moleFractionsTmp_           = b.moleFractionsTmp_;

    return *this;
  }
  //====================================================================================================================
  /**
   *
   * ~MolarityIonicVPSSTP():   (virtual)
   *
   * Destructor: does nothing:
   *
   */
  MolarityIonicVPSSTP::~MolarityIonicVPSSTP() {
  }

  /*
   * This routine duplicates the current object and returns
   * a pointer to ThermoPhase.
   */
  ThermoPhase* 
  MolarityIonicVPSSTP::duplMyselfAsThermoPhase() const {
    MolarityIonicVPSSTP* mtp = new MolarityIonicVPSSTP(*this);
    return (ThermoPhase *) mtp;
  }

  /*
   *  -------------- Utilities -------------------------------
   */
  //====================================================================================================================
 
  // Equation of state type flag.
  /*
   * The ThermoPhase base class returns
   * zero. Subclasses should define this to return a unique
   * non-zero value. Known constants defined for this purpose are
   * listed in mix_defs.h. The MolarityIonicVPSSTP class also returns
   * zero, as it is a non-complete class.
   */
  int MolarityIonicVPSSTP::eosType() const { 
    return 0;
  }

  //====================================================================================================================
  /*
   *   Import, construct, and initialize a phase
   *   specification from an XML tree into the current object.
   *
   * This routine is a precursor to constructPhaseXML(XML_Node*)
   * routine, which does most of the work.
   *
   * @param infile XML file containing the description of the
   *        phase
   *
   * @param id  Optional parameter identifying the name of the
   *            phase. If none is given, the first XML
   *            phase element will be used.
   */
  void MolarityIonicVPSSTP::constructPhaseFile(std::string inputFile, std::string id) {

    if ((int) inputFile.size() == 0) {
      throw CanteraError("MolarityIonicVPSSTP:constructPhaseFile",
                         "input file is null");
    }
    string path = findInputFile(inputFile);
    std::ifstream fin(path.c_str());
    if (!fin) {
      throw CanteraError("MolarityIonicVPSSTP:constructPhaseFile","could not open "
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
      throw CanteraError("MolarityIonicVPSSTP:constructPhaseFile",
                         "ERROR: Can not find phase named " +
                         id + " in file named " + inputFile);
    }
    fxml_phase->copy(&phaseNode_XML);
    constructPhaseXML(*fxml_phase, id);
    delete fxml;
  }
  //====================================================================================================================
  /*
   *   Import, construct, and initialize a HMWSoln phase 
   *   specification from an XML tree into the current object.
   *
   *   Most of the work is carried out by the cantera base
   *   routine, importPhase(). That routine imports all of the
   *   species and element data, including the standard states
   *   of the species.
   *
   *   Then, In this routine, we read the information 
   *   particular to the specification of the activity 
   *   coefficient model for the Pitzer parameterization.
   *
   *   We also read information about the molar volumes of the
   *   standard states if present in the XML file.
   *
   * @param phaseNode This object must be the phase node of a
   *             complete XML tree
   *             description of the phase, including all of the
   *             species data. In other words while "phase" must
   *             point to an XML phase object, it must have
   *             sibling nodes "speciesData" that describe
   *             the species in the phase.
   * @param id   ID of the phase. If nonnull, a check is done
   *             to see if phaseNode is pointing to the phase
   *             with the correct id. 
   */
  void MolarityIonicVPSSTP::constructPhaseXML(XML_Node& phaseNode, std::string id) {
    string stemp;
    if ((int) id.size() > 0) {
      string idp = phaseNode.id();
      if (idp != id) {
	throw CanteraError("MolarityIonicVPSSTP::constructPhaseXML", 
			   "phasenode and Id are incompatible");
      }
    }

    /*
     * Find the Thermo XML node 
     */
    if (!phaseNode.hasChild("thermo")) {
      throw CanteraError("MolarityIonicVPSSTP::constructPhaseXML",
			 "no thermo XML node");
    }
    XML_Node& thermoNode = phaseNode.child("thermo");

    /*
     * Make sure that the thermo model is MolarityIonic
     */ 
    stemp = thermoNode.attrib("model");
    string formString = lowercase(stemp);
    if (formString != "molarityionicvpss" && formString != "molarityionicvpsstp") {
      throw CanteraError("MolarityIonicVPSSTP::constructPhaseXML",
			 "model name isn't MolarityIonicVPSSTP: " + formString);
    }

    /*
     * Call the Cantera importPhase() function. This will import
     * all of the species into the phase. This will also handle
     * all of the solvent and solute standard states
     */
    bool m_ok = importPhase(phaseNode, this);
    if (!m_ok) {
      throw CanteraError("MolarityIonicVPSSTP::constructPhaseXML","importPhase failed "); 
    }
    
  }
  //====================================================================================================================
  /*
   * ------------ Molar Thermodynamic Properties ----------------------
   */


  /*
   * - Activities, Standard States, Activity Concentrations -----------
   */
 // This method returns an array of generalized concentrations
  /*
   * \f$ C^a_k\f$ are defined such that \f$ a_k = C^a_k /
   * C^0_k, \f$ where \f$ C^0_k \f$ is a standard concentration
   * defined below and \f$ a_k \f$ are activities used in the
   * thermodynamic functions.  These activity (or generalized)
   * concentrations are used
   * by kinetics manager classes to compute the forward and
   * reverse rates of elementary reactions. Note that they may
   * or may not have units of concentration --- they might be
   * partial pressures, mole fractions, or surface coverages,
   * for example.
   *
   *  Here we define the activity concentrations as equal
   *  to the activities, because the standard concentration is 1.
   *
   * @param c Output array of generalized concentrations. The
   *           units depend upon the implementation of the
   *           reaction rate expressions within the phase.
   */
  void MolarityIonicVPSSTP::getActivityConcentrations(doublereal* c) const {
    getActivities(c);
  }
  //====================================================================================================================
  doublereal MolarityIonicVPSSTP::standardConcentration(int k) const {
    return -1.0;
  }
  //====================================================================================================================
  doublereal MolarityIonicVPSSTP::logStandardConc(int k) const {
    return 0.0;
  }
  //====================================================================================================================
  // Get the array of non-dimensional molar-based activity coefficients at
  // the current solution temperature, pressure, and solution concentration.
  /*
   * @param ac Output vector of activity coefficients. Length: m_kk.
   */
  void MolarityIonicVPSSTP::getLnActivityCoefficients(doublereal* lnac) const {
    /*
     * Update the activity coefficients
     */
    s_update_lnActCoeff();

    /*
     * take the exp of the internally storred coefficients.
     */
    for (int k = 0; k < m_kk; k++) {
      lnac[k] = lnActCoeff_Scaled_[k]; 
    }
  }
  //====================================================================================================================
  void MolarityIonicVPSSTP::getChemPotentials(doublereal* mu) const {
    doublereal xx;
    /*
     * First get the standard chemical potentials in
     * molar form.
     *  -> this requires updates of standard state as a function
     *     of T and P
     */
    getStandardChemPotentials(mu);
    /*
     * Update the activity coefficients
     */
    s_update_lnActCoeff();
    /*
     *
     */
    doublereal RT = GasConstant * temperature();
    for (int k = 0; k < m_kk; k++) {
      xx = fmaxx(moleFractions_[k], xxSmall);
      mu[k] += RT * (log(xx) + lnActCoeff_Scaled_[k]); 
    }
  }
  //====================================================================================================================

  void MolarityIonicVPSSTP::getElectrochemPotentials(doublereal* mu) const {
    getChemPotentials(mu);
    double ve = Faraday * electricPotential();
    for (int k = 0; k < m_kk; k++) {
      mu[k] += ve*charge(k);
    }
  }

  //====================================================================================================================
  // Returns an array of partial molar enthalpies for the species
  // in the mixture.
  /*
   * Units (J/kmol)
   *
   * For this phase, the partial molar enthalpies are equal to the
   * standard state enthalpies modified by the derivative of the
   * molality-based activity coefficent wrt temperature
   *
   *  \f[
   * \bar h_k(T,P) = h^o_k(T,P) - R T^2 \frac{d \ln(\gamma_k)}{dT}
   * \f]
   *
   */
  void MolarityIonicVPSSTP::getPartialMolarEnthalpies(doublereal* hbar) const {
    /*
     * Get the nondimensional standard state enthalpies
     */
    getEnthalpy_RT(hbar);
    /*
     * dimensionalize it.
     */
    double T = temperature();
    double RT = GasConstant * T;
    for (int k = 0; k < m_kk; k++) {
      hbar[k] *= RT;
    }
    /*
     * Update the activity coefficients, This also update the
     * internally storred molalities.
     */
    s_update_lnActCoeff();
    s_update_dlnActCoeff_dT();
    double RTT = RT * T;
    for (int k = 0; k < m_kk; k++) {
      hbar[k] -= RTT * dlnActCoeffdT_Scaled_[k];
    }
  }
  //====================================================================================================================
  // Returns an array of partial molar heat capacities for the species
  // in the mixture.
  /*
   * Units (J/kmol)
   *
   * For this phase, the partial molar enthalpies are equal to the
   * standard state enthalpies modified by the derivative of the
   * activity coefficent wrt temperature
   *
   *  \f[
   * ??????????? \bar s_k(T,P) = s^o_k(T,P) - R T^2 \frac{d \ln(\gamma_k)}{dT}
   * \f]
   *
   */
  void MolarityIonicVPSSTP::getPartialMolarCp(doublereal* cpbar) const {
    /*
     * Get the nondimensional standard state entropies
     */
    getCp_R(cpbar);
    double T = temperature();
    /*
     * Update the activity coefficients, This also update the
     * internally storred molalities.
     */
    s_update_lnActCoeff();
    s_update_dlnActCoeff_dT();

    for (int k = 0; k < m_kk; k++) {
      cpbar[k] -= 2 * T * dlnActCoeffdT_Scaled_[k] + T * T * d2lnActCoeffdT2_Scaled_[k];
    }  
    /*
     * dimensionalize it.
     */
   for (int k = 0; k < m_kk; k++) {
      cpbar[k] *= GasConstant;
    }
  }
  //====================================================================================================================
  // Returns an array of partial molar entropies for the species
  // in the mixture.
  /*
   * Units (J/kmol)
   *
   * For this phase, the partial molar enthalpies are equal to the
   * standard state enthalpies modified by the derivative of the
   * activity coefficent wrt temperature
   *
   *  \f[
   * \bar s_k(T,P) = s^o_k(T,P) - R T^2 \frac{d \ln(\gamma_k)}{dT}
   * \f]
   *
   */
  void MolarityIonicVPSSTP::getPartialMolarEntropies(doublereal* sbar) const {
    double xx;
    /*
     * Get the nondimensional standard state entropies
     */
    getEntropy_R(sbar);
    double T = temperature();
    /*
     * Update the activity coefficients, This also update the
     * internally storred molalities.
     */
    s_update_lnActCoeff();
    s_update_dlnActCoeff_dT();

    for (int k = 0; k < m_kk; k++) {
      xx = fmaxx(moleFractions_[k], xxSmall);
      sbar[k] += - lnActCoeff_Scaled_[k] -log(xx) - T * dlnActCoeffdT_Scaled_[k];
    }  
    /*
     * dimensionalize it.
     */
   for (int k = 0; k < m_kk; k++) {
      sbar[k] *= GasConstant;
    }
  }
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
  void MolarityIonicVPSSTP::getPartialMolarVolumes(doublereal* vbar) const {
    int  iK;
    /*
     * Get the standard state values in m^3 kmol-1
     */
    getStandardVolumes(vbar);   
    for ( iK = 0; iK < m_kk; iK++ ){
      
      vbar[iK] += 0.0;
    }
  }
  //====================================================================================================================
  void MolarityIonicVPSSTP::calcPseudoBinaryMoleFractions() const {
    int k;
    int kCat;
    int kMax;
    doublereal sumCat; 
    doublereal sumAnion;
    doublereal chP, chM;
    doublereal sum = 0.0;
    doublereal sumMax;
    switch (PBType_) {
    case PBTYPE_PASSTHROUGH:
      for (k = 0; k < m_kk; k++) {
	PBMoleFractions_[k] = moleFractions_[k];
      }
      break;
    case PBTYPE_SINGLEANION:
      sumCat = 0.0;
      sumAnion = 0.0;
      for (k = 0; k < m_kk; k++) {
	moleFractionsTmp_[k] = moleFractions_[k];
      }
      kMax = -1;
      sumMax = 0.0;
      for (k = 0; k < (int) cationList_.size(); k++) {
	kCat = cationList_[k];
	chP = m_speciesCharge[kCat];
	if (moleFractions_[kCat] > sumMax) {
	  kMax = k;
	  sumMax = moleFractions_[kCat];
	}
	sumCat += chP * moleFractions_[kCat];
      }
      k = anionList_[0];
      chM = m_speciesCharge[k];
      sumAnion = moleFractions_[k] * chM;
      sum = sumCat - sumAnion;
      if (fabs(sum) > 1.0E-16) {
	moleFractionsTmp_[cationList_[kMax]] -= sum / m_speciesCharge[kMax];
	sum = 0.0;
	for (k = 0; k < numCationSpecies_; k++) {
	  sum +=  moleFractionsTmp_[k];
	}
	for (k = 0; k < numCationSpecies_; k++) {
	  moleFractionsTmp_[k]/= sum;
	}
      }

      for (k = 0; k < numCationSpecies_; k++) {
        PBMoleFractions_[k] = moleFractionsTmp_[cationList_[k]];
      }
      for (k = 0; k <  numPassThroughSpecies_; k++) {
	PBMoleFractions_[neutralPBindexStart + k] = moleFractions_[passThroughList_[k]];
      }
      
      sum = fmaxx(0.0, PBMoleFractions_[0]);
      for (k = 1; k < numPBSpecies_; k++) {
        sum += PBMoleFractions_[k];
	
      }
      for (k = 0; k < numPBSpecies_; k++) {
        PBMoleFractions_[k] /= sum;
      }

      break;
    case PBTYPE_SINGLECATION:
      throw CanteraError("eosType", "Unknown type");
     
      break;
     
    case PBTYPE_MULTICATIONANION:
      throw CanteraError("eosType", "Unknown type");
     
      break;
    default:
      throw CanteraError("eosType", "Unknown type");
      break;

    } 
  }
  //====================================================================================================================

 // Update the activity coefficients
  /*
   * This function will be called to update the internally storred
   * natural logarithm of the activity coefficients
   *
   */
  void MolarityIonicVPSSTP::s_update_lnActCoeff() const {
    int  k;
    for (k = 0; k < m_kk; k++) {
      lnActCoeff_Scaled_[k] = 0.0;
    }
  } 
  //====================================================================================================================
  void MolarityIonicVPSSTP::s_update_dlnActCoeff_dT() const {


  }
  //====================================================================================================================
  // Internal routine that calculates the derivative of the activity coefficients wrt
  // the mole fractions.
  /*
   *  This routine calculates the the derivative of the activity coefficients wrt to mole fraction
   *  with all other mole fractions held constant. This is strictly not permitted. However, if the
   *  resulting matrix is multiplied by a permissible deltaX vector then everything is ok.
   *
   *  This is the natural way to handle concentration derivatives in this routine.
   */
  void  MolarityIonicVPSSTP::s_update_dlnActCoeff_dX_() const {

  }
  //====================================================================================================================
  /*
   * ------------ Partial Molar Properties of the Solution ------------
   */
  //====================================================================================================================
  doublereal MolarityIonicVPSSTP::err(std::string msg) const {
    throw CanteraError("MolarityIonicVPSSTP","Base class method "
		       +msg+" called. Equation of state type: "+int2str(eosType()));
    return 0;
  }
  //====================================================================================================================
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
  void MolarityIonicVPSSTP::initThermo() {
    GibbsExcessVPSSTP::initThermo();
    initLengths();
    /*
     *  Go find the list of cations and anions
     */
    double ch;
    numCationSpecies_ = 0.0;
    cationList_.clear();
    anionList_.clear();
    passThroughList_.clear();
    for (int k = 0; k < m_kk; k++) {
      ch = m_speciesCharge[k];
      if (ch > 0.0) {
	cationList_.push_back(k);
	numCationSpecies_++;
      } else if (ch < 0.0) {
	anionList_.push_back(k);
	numAnionSpecies_++;
      } else {
	passThroughList_.push_back(k);
	numPassThroughSpecies_++;
      }
    }
    numPBSpecies_ = numCationSpecies_ + numAnionSpecies_ - 1;
    neutralPBindexStart = numPBSpecies_;
    PBType_ = PBTYPE_MULTICATIONANION;
    if (numAnionSpecies_ == 1) {
      PBType_ = PBTYPE_SINGLEANION;
    } else if (numCationSpecies_ == 1) {
      PBType_ = PBTYPE_SINGLECATION;
    }
    if (numAnionSpecies_ == 0 && numCationSpecies_ == 0) {
      PBType_ = PBTYPE_PASSTHROUGH;
    }
  }
  //====================================================================================================================
  //   Initialize lengths of local variables after all species have been identified.
  void  MolarityIonicVPSSTP::initLengths() {
    m_kk = nSpecies();
    moleFractionsTmp_.resize(m_kk);    
  }
  //====================================================================================================================
  /*
   * initThermoXML()                (virtual from ThermoPhase)
   *   Import and initialize a ThermoPhase object
   *
   * @param phaseNode This object must be the phase node of a
   *             complete XML tree
   *             description of the phase, including all of the
   *             species data. In other words while "phase" must
   *             point to an XML phase object, it must have
   *             sibling nodes "speciesData" that describe
   *             the species in the phase.
   * @param id   ID of the phase. If nonnull, a check is done
   *             to see if phaseNode is pointing to the phase
   *             with the correct id. 
   */
  void MolarityIonicVPSSTP::initThermoXML(XML_Node& phaseNode, std::string id) {
    std::string subname = "MolarityIonicVPSSTP::initThermoXML";
    std::string stemp;
    /*
     * Check on the thermo field. Must have:
     * <thermo model="MolarityIonic" />
     */
  
    XML_Node& thermoNode = phaseNode.child("thermo");
    std::string mStringa = thermoNode.attrib("model");
    std::string mString = lowercase(mStringa);
    if (mString != "molarityionicvpss" && mString != "molarityionicvpsstp") {
      throw CanteraError(subname.c_str(),
			 "Unknown thermo model: " + mStringa + " - This object only knows \"MolarityIonicVPSSTP\" ");
    }
    /*
     * Go get all of the coefficients and factors in the
     * activityCoefficients XML block
     */
    /*
     * Go get all of the coefficients and factors in the
     * activityCoefficients XML block
     */
    XML_Node *acNodePtr = 0;
    if (thermoNode.hasChild("activityCoefficients")) {
      XML_Node& acNode = thermoNode.child("activityCoefficients");
      acNodePtr = &acNode;
      std::string mStringa = acNode.attrib("model");
      std::string mString = lowercase(mStringa);
      // if (mString != "redlich-kister") {
      //	throw CanteraError(subname.c_str(),
      //			   "Unknown activity coefficient model: " + mStringa);
      //}
      int n = acNodePtr->nChildren();
      for (int i = 0; i < n; i++) {
	XML_Node &xmlACChild = acNodePtr->child(i);
	stemp = xmlACChild.name();
	std::string nodeName = lowercase(stemp);
	/*
	 * Process a binary interaction
	 */
	if (nodeName == "binaryneutralspeciesparameters") {
	  readXMLBinarySpecies(xmlACChild);
	}
      }
    }


    /*
     * Go down the chain
     */
    GibbsExcessVPSSTP::initThermoXML(phaseNode, id);
  }
  //====================================================================================================================
  // Process an XML node called "binaryNeutralSpeciesParameters"
  /*
   *  This node contains all of the parameters necessary to describe 
   *  a single binary interaction. This function reads the XML file and writes the coefficients
   *  it finds to an internal data structures.
   */
  void MolarityIonicVPSSTP::readXMLBinarySpecies(XML_Node &xmLBinarySpecies) {
    std::string xname = xmLBinarySpecies.name();

  }
  //====================================================================================================================
  /*
   * Format a summary of the mixture state for output.
   */           
  std::string MolarityIonicVPSSTP::report(bool show_thermo) const {
    char p[800];
    string s = "";
    try {
      if (name() != "") {
	sprintf(p, " \n  %s:\n", name().c_str());
	s += p;
      }
      sprintf(p, " \n       temperature    %12.6g  K\n", temperature());
      s += p;
      sprintf(p, "          pressure    %12.6g  Pa\n", pressure());
      s += p;
      sprintf(p, "           density    %12.6g  kg/m^3\n", density());
      s += p;
      sprintf(p, "  mean mol. weight    %12.6g  amu\n", meanMolecularWeight());
      s += p;

      doublereal phi = electricPotential();
      sprintf(p, "         potential    %12.6g  V\n", phi);
      s += p;

      int kk = nSpecies();
      array_fp x(kk);
      array_fp molal(kk);
      array_fp mu(kk);
      array_fp muss(kk);
      array_fp acMolal(kk);
      array_fp actMolal(kk);
      getMoleFractions(&x[0]);
   
      getChemPotentials(&mu[0]);
      getStandardChemPotentials(&muss[0]);
      getActivities(&actMolal[0]);
 

      if (show_thermo) {
        sprintf(p, " \n");
        s += p;
        sprintf(p, "                          1 kg            1 kmol\n");
        s += p;
        sprintf(p, "                       -----------      ------------\n");
        s += p;
        sprintf(p, "          enthalpy    %12.6g     %12.4g     J\n", 
		enthalpy_mass(), enthalpy_mole());
        s += p;
        sprintf(p, "   internal energy    %12.6g     %12.4g     J\n", 
		intEnergy_mass(), intEnergy_mole());
        s += p;
        sprintf(p, "           entropy    %12.6g     %12.4g     J/K\n", 
		entropy_mass(), entropy_mole());
        s += p;
        sprintf(p, "    Gibbs function    %12.6g     %12.4g     J\n", 
		gibbs_mass(), gibbs_mole());
        s += p;
        sprintf(p, " heat capacity c_p    %12.6g     %12.4g     J/K\n", 
		cp_mass(), cp_mole());
        s += p;
        try {
	  sprintf(p, " heat capacity c_v    %12.6g     %12.4g     J/K\n", 
		  cv_mass(), cv_mole());
	  s += p;
        }
        catch(CanteraError) {
	  sprintf(p, " heat capacity c_v    <not implemented>       \n");
	  s += p;
        }
      }
  
    } catch (CanteraError) {
      ;
    }
    return s;
  }
  //====================================================================================================================
}

