/**
 *  @file 
 *
 */
/*
 * Copywrite (2009) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */
/*
 *  $Date: 2010-11-12 14:37:41 -0700 (Fri, 12 Nov 2010) $
 *  $Revision: 641 $
 */


#include "PhaseCombo_Interaction.h"
#include "ThermoFactory.h"
#include <iomanip>

using namespace std;

namespace Cantera {

 static  const double xxSmall = 1.0E-150; 
  //====================================================================================================================
  /*
   * Default constructor.
   *  
   * HKM - Checked for Transition
   */
  PhaseCombo_Interaction::PhaseCombo_Interaction() :
    GibbsExcessVPSSTP(),
    numBinaryInteractions_(0),
    formMargules_(0),
    formTempModel_(0)
  {
  }
  //====================================================================================================================
  /*
   * Working constructors
   *
   *  The two constructors below are the normal way
   *  the phase initializes itself. They are shells that call\
   *  the routine initThermo(), with a reference to the
   *  XML database to get the info for the phase.
   *
   * HKM - Checked for Transition
   */
  PhaseCombo_Interaction::PhaseCombo_Interaction(std::string inputFile, std::string id) :
    GibbsExcessVPSSTP(),
    numBinaryInteractions_(0),
    formMargules_(0),
    formTempModel_(0)
  {
    constructPhaseFile(inputFile, id);
  }
  //====================================================================================================================
  //
  /*
   *
   * HKM - Checked for Transition
   */
  PhaseCombo_Interaction::PhaseCombo_Interaction(XML_Node& phaseRoot, std::string id) :
    GibbsExcessVPSSTP(),
    numBinaryInteractions_(0),
    formMargules_(0),
    formTempModel_(0)
  {
    constructPhaseXML(phaseRoot, id);
  }

  //====================================================================================================================
  /*
   * Copy Constructor:
   *
   *  Note this stuff will not work until the underlying phase
   *  has a working copy constructor
   *
   * HKM - Checked for Transition
   */
  PhaseCombo_Interaction::PhaseCombo_Interaction(const PhaseCombo_Interaction &b) :
    GibbsExcessVPSSTP()
  {
    PhaseCombo_Interaction::operator=(b);
  }
  //====================================================================================================================
  /*
   * operator=()
   *
   *  Note this stuff will not work until the underlying phase
   *  has a working assignment operator
   *
   * HKM - Checked for Transition
   */
  PhaseCombo_Interaction& PhaseCombo_Interaction::
  operator=(const PhaseCombo_Interaction &b) {
    if (&b == this) {
      return *this;
    }
   
    GibbsExcessVPSSTP::operator=(b);
    
    numBinaryInteractions_      = b.numBinaryInteractions_ ;
    m_HE_b_ij                   = b.m_HE_b_ij;
    m_HE_c_ij                   = b.m_HE_c_ij;
    m_HE_d_ij                   = b.m_HE_d_ij;
    m_SE_b_ij                   = b.m_SE_b_ij;
    m_SE_c_ij                   = b.m_SE_c_ij;
    m_SE_d_ij                   = b.m_SE_d_ij;
    m_VHE_b_ij                  = b.m_VHE_b_ij;
    m_VHE_c_ij                  = b.m_VHE_c_ij;
    m_VHE_d_ij                  = b.m_VHE_d_ij;
    m_VSE_b_ij                  = b.m_VSE_b_ij;
    m_VSE_c_ij                  = b.m_VSE_c_ij;
    m_VSE_d_ij                  = b.m_VSE_d_ij;
    m_pSpecies_A_ij             = b.m_pSpecies_A_ij;
    m_pSpecies_B_ij             = b.m_pSpecies_B_ij;
    formMargules_               = b.formMargules_;
    formTempModel_              = b.formTempModel_;
    
    return *this;
  }
  //====================================================================================================================
  /**
   *
   * ~PhaseCombo_Interaction():   (virtual)
   *
   * Destructor: does nothing:
   *  
   * HKM - Checked for Transition
   */
  PhaseCombo_Interaction::~PhaseCombo_Interaction() {
  }
  //====================================================================================================================
  /*
   * This routine duplicates the current object and returnsa pointer to ThermoPhase.
   *
   * HKM - Checked for Transition
   */
  ThermoPhase* 
  PhaseCombo_Interaction::duplMyselfAsThermoPhase() const {
    PhaseCombo_Interaction* mtp = new PhaseCombo_Interaction(*this);
    return (ThermoPhase *) mtp;
  }
  //====================================================================================================================
  // Special constructor for a hard-coded problem
  /*
   *
   *   LiKCl treating the PseudoBinary layer as passthrough.
   *   -> test to predict the eutectic and liquidus correctly.
   *
   */
  PhaseCombo_Interaction::PhaseCombo_Interaction(int testProb)  :
    GibbsExcessVPSSTP(),
    numBinaryInteractions_(0),
    formMargules_(0),
    formTempModel_(0)
  {
  

    constructPhaseFile("PhaseCombo_Interaction.xml", "");


    numBinaryInteractions_ = 1;

    m_HE_b_ij.resize(1);
    m_HE_c_ij.resize(1);
    m_HE_d_ij.resize(1);

    m_SE_b_ij.resize(1);
    m_SE_c_ij.resize(1);
    m_SE_d_ij.resize(1);

    m_VHE_b_ij.resize(1);
    m_VHE_c_ij.resize(1);
    m_VHE_d_ij.resize(1);

    m_VSE_b_ij.resize(1);
    m_VSE_c_ij.resize(1);
    m_VSE_d_ij.resize(1);
    
    m_pSpecies_A_ij.resize(1);
    m_pSpecies_B_ij.resize(1);


    
    m_HE_b_ij[0] = -17570E3;
    m_HE_c_ij[0] = -377.0E3;
    m_HE_d_ij[0] = 0.0;
      
    m_SE_b_ij[0] = -7.627E3;
    m_SE_c_ij[0] =  4.958E3;
    m_SE_d_ij[0] =  0.0;
    

    int iLiT = speciesIndex("LiTFe1S2(S)");
    if (iLiT < 0) {
      throw CanteraError("PhaseCombo_Interaction test1 constructor",
			 "Unable to find LiTFe1S2(S)");
    }
    m_pSpecies_A_ij[0] = iLiT;


    int iLi2 = speciesIndex("Li2Fe1S2(S)");
    if (iLi2 < 0) {
      throw CanteraError("PhaseCombo_Interaction test1 constructor",
			 "Unable to find Li2Fe1S2(S)");
    }
    m_pSpecies_B_ij[0] = iLi2;
    throw CanteraError("", "unimplemented");
  }
  //====================================================================================================================

  /*
   *  -------------- Utilities -------------------------------
   */

 
  // Equation of state type flag.
  /*
   * The ThermoPhase base class returns
   * zero. Subclasses should define this to return a unique
   * non-zero value. Known constants defined for this purpose are
   * listed in mix_defs.h. The PhaseCombo_Interaction class also returns
   * zero, as it is a non-complete class.
   */
  int PhaseCombo_Interaction::eosType() const { 
    return cPhaseCombo_Interaction;
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
   *
   * HKM - Checked for Transition
   */
  void PhaseCombo_Interaction::constructPhaseFile(std::string inputFile, std::string id) {

    if ((int) inputFile.size() == 0) {
      throw CanteraError("PhaseCombo_Interaction:constructPhaseFile",
                         "input file is null");
    }
    string path = findInputFile(inputFile);
    std::ifstream fin(path.c_str());
    if (!fin) {
      throw CanteraError("PhaseCombo_Interaction:constructPhaseFile",
			 "Could not open " +path+" for reading.");
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
      throw CanteraError("PhaseCombo_Interaction:constructPhaseFile",
                         "ERROR: Can not find phase named " + id + " in file named " + inputFile);
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
   *
   * HKM - Checked for Transition
   */
  void PhaseCombo_Interaction::constructPhaseXML(XML_Node& phaseNode, std::string id) {
    string stemp;
    if ((int) id.size() > 0) {
      string idp = phaseNode.id();
      if (idp != id) {
	throw CanteraError("PhaseCombo_Interaction::constructPhaseXML", 
			   "phasenode and Id are incompatible");
      }
    }

    /*
     * Find the Thermo XML node
     */
    if (!phaseNode.hasChild("thermo")) {
      throw CanteraError("PhaseCombo_Interaction::constructPhaseXML",
			 "no thermo XML node");
    }
    XML_Node& thermoNode = phaseNode.child("thermo");

    /*
     * Make sure that the thermo model is PhaseCombo_Interaction
     */ 
    stemp = thermoNode.attrib("model");
    string formString = lowercase(stemp);
    if (formString != "phasecombo_interaction") {
      throw CanteraError("PhaseCombo_Interaction::constructPhaseXML",
			 "model name isn't PhaseCombo_Interaction: " + formString);
    }

    /*
     * Call the Cantera importPhase() function. This will import
     * all of the species into the phase. This will also handle
     * all of the species standard states
     */
    bool m_ok = importPhase(phaseNode, this);
    if (!m_ok) {
      throw CanteraError("PhaseCombo_Interaction::constructPhaseXML","importPhase failed "); 
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
  void PhaseCombo_Interaction::getActivityConcentrations(doublereal* c) const {
    getActivities(c);
  }
  //====================================================================================================================
  doublereal PhaseCombo_Interaction::standardConcentration(int k) const {
    //err("standardConcentration");
    //return -1.0;
    return 1.0;
  }
  //====================================================================================================================
  doublereal PhaseCombo_Interaction::logStandardConc(int k) const {
    //err("logStandardConc");
    //return -1.0;
    return 0.0;
  }
  //====================================================================================================================
  // Get the array of non-dimensional molar-based activity coefficients at
  // the current solution temperature, pressure, and solution concentration.
  /*
   * @param ac Output vector of activity coefficients. Length: m_kk.
   */
  void PhaseCombo_Interaction::getActivityCoefficients(doublereal* ac) const {
    /*
     * Update the activity coefficients
     */
    s_update_lnActCoeff();

    /*
     * take the exp of the internally storred coefficients.
     */
    for (int k = 0; k < m_kk; k++) {
      ac[k] = exp(lnActCoeff_Scaled_[k]); 
    }
  }

  /*
   * ------------ Partial Molar Properties of the Solution ------------
   */

  //====================================================================================================================

  void PhaseCombo_Interaction::getElectrochemPotentials(doublereal* mu) const {
    getChemPotentials(mu);
    double ve = Faraday * electricPotential();
    for (int k = 0; k < m_kk; k++) {
      mu[k] += ve*charge(k);
    }
  }

  //====================================================================================================================
  void PhaseCombo_Interaction::getChemPotentials(doublereal* mu) const {
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
  // Molar enthalpy. Units: J/kmol. 
  doublereal PhaseCombo_Interaction::enthalpy_mole() const {
    int kk = nSpecies();
    double hbar[kk], h = 0;
    getPartialMolarEnthalpies(hbar);
    for (int i = 0; i < kk; i++){
      h += moleFractions_[i]*hbar[i];
    }
    return h;
  }
  //====================================================================================================================
  // Molar entropy. Units: J/kmol. 
  doublereal PhaseCombo_Interaction::entropy_mole() const {
    int kk = nSpecies();
    double sbar[kk], s = 0;
    getPartialMolarEntropies(sbar);
    for (int i = 0; i < kk; i++){
      s += moleFractions_[i]*sbar[i];
    }
    return s;
  }
  //====================================================================================================================
  // Molar heat capacity at constant pressure. Units: J/kmol/K. 
  doublereal PhaseCombo_Interaction::cp_mole() const {
    int kk = nSpecies();
    double cpbar[kk], cp = 0;
    getPartialMolarCp(cpbar);
    for (int i = 0; i < kk; i++){
      cp += moleFractions_[i]*cpbar[i];
    }
    return cp;
  }
  //====================================================================================================================
  // Molar heat capacity at constant volume. Units: J/kmol/K. 
  doublereal PhaseCombo_Interaction::cv_mole() const {
    return cp_mole() - GasConstant;
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
  void PhaseCombo_Interaction::getPartialMolarEnthalpies(doublereal* hbar) const {
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
  // Returns an array of partial molar heat capacities for the species in the mixture.
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
  void PhaseCombo_Interaction::getPartialMolarCp(doublereal* cpbar) const {
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
   *         \bar s_k(T,P) = s^o_k(T,P) - R T^2 \frac{d \ln(\gamma_k)}{dT}
   * \f]
   *
   */
  void PhaseCombo_Interaction::getPartialMolarEntropies(doublereal* sbar) const {
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
      sbar[k] += - lnActCoeff_Scaled_[k] - log(xx) - T * dlnActCoeffdT_Scaled_[k];
    }  
    /*
     * dimensionalize it.
     */
   for (int k = 0; k < m_kk; k++) {
      sbar[k] *= GasConstant;
    }
  }
  //====================================================================================================================
  /*
   * ------------ Partial Molar Properties of the Solution ------------
   */

  // Return an array of partial molar volumes for the species in the mixture. Units: m^3/kmol.
  /*
   *  Frequently, for this class of thermodynamics representations,
   *  the excess Volume due to mixing is zero. Here, we set it as
   *  a default. It may be overriden in derived classes.
   *
   *  @param vbar   Output vector of speciar partial molar volumes.
   *                Length = m_kk. units are m^3/kmol.
   */
  void PhaseCombo_Interaction::getPartialMolarVolumes(doublereal* vbar) const {

    int iA, iB, iK, delAK, delBK;
    double XA, XB, XK, g0 , g1;
    double T = temperature();

    /*
     * Get the standard state values in m^3 kmol-1
     */
    getStandardVolumes(vbar);
 
    for ( iK = 0; iK < m_kk; iK++ ){
      delAK = 0;
      delBK = 0;
      XK = moleFractions_[iK];  
      for (int i = 0; i <  numBinaryInteractions_; i++) {
    
	iA =  m_pSpecies_A_ij[i];    
	iB =  m_pSpecies_B_ij[i];

	if (iA==iK) delAK = 1;
	else if (iB==iK) delBK = 1;
	
	XA = moleFractions_[iA];
	XB = moleFractions_[iB];
	
	g0 = (m_VHE_b_ij[i] - T * m_VSE_b_ij[i]);
	g1 = (m_VHE_c_ij[i] - T * m_VSE_c_ij[i]);
	
	vbar[iK] += XA*XB*(g0+g1*XB)+((delAK-XA)*XB+XA*(delBK-XB))*(g0+g1*XB)+XA*XB*(delBK-XB)*g1;
      }
    }
  }  
  //====================================================================================================================
  doublereal PhaseCombo_Interaction::err(std::string msg) const {
    throw CanteraError("PhaseCombo_Interaction","Base class method "
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
  void PhaseCombo_Interaction::initThermo() {
    initLengths();
    GibbsExcessVPSSTP::initThermo();
  }

  //====================================================================================================================
  //   Initialize lengths of local variables after all species have
  //   been identified.
  void  PhaseCombo_Interaction::initLengths() {
    m_kk = nSpecies();
    dlnActCoeffdlnN_.resize(m_kk, m_kk);
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
  void PhaseCombo_Interaction::initThermoXML(XML_Node& phaseNode, std::string id) {
    string subname = "PhaseCombo_Interaction::initThermoXML";
    string stemp;

    /*
     * Check on the thermo field. Must have:
     * <thermo model="IdealSolidSolution" />
     */
  
    XML_Node& thermoNode = phaseNode.child("thermo");
    string mStringa = thermoNode.attrib("model");
    string mString = lowercase(mStringa);
    if (mString != "phasecombo_interaction") {
      throw CanteraError(subname.c_str(), "Unknown thermo model: " + mStringa);
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
      string mStringa = acNode.attrib("model");
      string mString = lowercase(mStringa);
      if (mString != "margules") {
	throw CanteraError(subname.c_str(),
			   "Unknown activity coefficient model: " + mStringa);
      }
      int n = acNodePtr->nChildren();
      for (int i = 0; i < n; i++) {
	XML_Node &xmlACChild = acNodePtr->child(i);
	stemp = xmlACChild.name();
	string nodeName = lowercase(stemp);
	/*
	 * Process a binary salt field, or any of the other XML fields
	 * that make up the Pitzer Database. Entries will be ignored
	 * if any of the species in the entry isn't in the solution.
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
  //===================================================================================================================
  // Update the activity coefficients
  /*
   * This function will be called to update the internally storred
   * natural logarithm of the activity coefficients
   *
   *   he = X_A X_B(B + C X_B)
   *
   *  HKM - Checked for Transition
   */
    void PhaseCombo_Interaction::s_update_lnActCoeff() const {
    int iA, iB, iK, delAK, delBK;
    doublereal XA, XB, g0 , g1;
    doublereal xx;
    doublereal T = temperature();
    doublereal RT = GasConstant*T;
    fvo_zero_dbl_1(lnActCoeff_Scaled_, m_kk);

    for (iK = 0; iK < m_kk; iK++) {
      /*
       *  We never sample the end of the mole fraction domains
       */
      xx = fmaxx(moleFractions_[iK], xxSmall);
      /*
       *  First wipe out the ideal solution mixing term
       */  
      lnActCoeff_Scaled_[iK] = - log(xx);

      /*
       *  Then add in the Margules interaction terms. that's it!
       */
      for (int i = 0; i <  numBinaryInteractions_; i++) {
	iA =  m_pSpecies_A_ij[i];    
	iB =  m_pSpecies_B_ij[i];
	delAK = 0;
	delBK = 0;
	if      (iA==iK) delAK = 1;
	else if (iB==iK) delBK = 1;
	XA = moleFractions_[iA];
	XB = moleFractions_[iB];
	g0 = (m_HE_b_ij[i] - T * m_SE_b_ij[i]) / RT;
	g1 = (m_HE_c_ij[i] - T * m_SE_c_ij[i]) / RT;
	lnActCoeff_Scaled_[iK] += (delAK * XB + XA * delBK - XA * XB) * (g0 + g1 * XB) + XA * XB * (delBK - XB) * g1;
      }
    }
  }
  //===================================================================================================================
  // Update the derivative of the log of the activity coefficients wrt T
  /*
   * This function will be called to update the internally storred
   * natural logarithm of the activity coefficients
   *
   *   he = X_A X_B(B + C X_B)
   *
   *  HKM - Checked for Transition
   */
  void PhaseCombo_Interaction::s_update_dlnActCoeff_dT() const {
    int iA, iB, iK, delAK, delBK;
    doublereal XA, XB, g0, g1;
    doublereal T = temperature();
    doublereal RTT = GasConstant*T*T;
    fvo_zero_dbl_1(dlnActCoeffdT_Scaled_, m_kk);
    fvo_zero_dbl_1(d2lnActCoeffdT2_Scaled_, m_kk);
    for (iK = 0; iK < m_kk; iK++) {
      for (int i = 0; i <  numBinaryInteractions_; i++) {
	iA =  m_pSpecies_A_ij[i];    
	iB =  m_pSpecies_B_ij[i];
	delAK = 0;
	delBK = 0;
	if      (iA==iK) delAK = 1;
	else if (iB==iK) delBK = 1;
	XA = moleFractions_[iA];
	XB = moleFractions_[iB];
	g0 = -m_HE_b_ij[i] / RTT;
	g1 = -m_HE_c_ij[i] / RTT;
	double temp = (delAK * XB + XA * delBK - XA * XB) * (g0 + g1 * XB) + XA * XB * (delBK - XB) * g1;
	dlnActCoeffdT_Scaled_[iK] += temp;
	d2lnActCoeffdT2_Scaled_[iK] -= 2.0 * temp / T;
      }
    }
  }
  //====================================================================================================================
  //
  /*
   * HKM - Checked for Transition
   */
  void PhaseCombo_Interaction::getdlnActCoeffdT(doublereal *dlnActCoeffdT) const {
    s_update_dlnActCoeff_dT();
    for (int k = 0; k < m_kk; k++) {
      dlnActCoeffdT[k] = dlnActCoeffdT_Scaled_[k];
    }
  }
  //====================================================================================================================
  //
  /*
   * HKM - Checked for Transition
   */
  void PhaseCombo_Interaction::getd2lnActCoeffdT2(doublereal *d2lnActCoeffdT2) const {
    s_update_dlnActCoeff_dT();
    for (int k = 0; k < m_kk; k++) {
      d2lnActCoeffdT2[k] = d2lnActCoeffdT2_Scaled_[k];
    }
  }
  //====================================================================================================================

  // Get the change in activity coefficients w.r.t. change in state (temp, mole fraction, etc.) along
  // a line in parameter space or along a line in physical space
  /*
   *
   * @param dTds           Input of temperature change along the path
   * @param dXds           Input vector of changes in mole fraction along the path. length = m_kk
   *                       Along the path length it must be the case that the mole fractions sum to one.
   * @param dlnActCoeffds  Output vector of the directional derivatives of the 
   *                       log Activity Coefficients along the path. length = m_kk
   *  units are 1/units(s). if s is a physical coordinate then the units are 1/m.
   *
   * HKM - Checked for Transition
   */
  void  PhaseCombo_Interaction::getdlnActCoeffds(const doublereal dTds, const doublereal * const dXds,
					 doublereal *dlnActCoeffds) const {
  

    int iA, iB, iK, delAK, delBK;
    doublereal XA, XB, XK, g0 , g1, dXA, dXB;
    doublereal T = temperature();
    doublereal RT = GasConstant*T;
    doublereal xx;

    //fvo_zero_dbl_1(dlnActCoeff, m_kk);
    s_update_dlnActCoeff_dT();

    for (iK = 0; iK < m_kk; iK++) {

      XK = moleFractions_[iK];  
 
      /*
       *  We never sample the end of the mole fraction domains
       */
      xx = fmaxx(moleFractions_[iK], xxSmall);
      /*
       *  First wipe out the ideal solution mixing term
       */  
      if (xx > xxSmall) {
	dlnActCoeffds[iK] += - 1.0 / xx;
      }

      for (int i = 0; i <  numBinaryInteractions_; i++) {
    
	iA =  m_pSpecies_A_ij[i];    
	iB =  m_pSpecies_B_ij[i];

	delAK = 0;
	delBK = 0;

	if (iA==iK) delAK = 1;
	else if (iB==iK) delBK = 1;
	
	XA = moleFractions_[iA];
	XB = moleFractions_[iB];

	dXA = dXds[iA];
	dXB = dXds[iB];
	
	g0 = (m_HE_b_ij[i] - T * m_SE_b_ij[i]) / RT;
	g1 = (m_HE_c_ij[i] - T * m_SE_c_ij[i]) / RT;
	
	dlnActCoeffds[iK] += ((delBK-XB)*dXA + (delAK-XA)*dXB)*(g0+2*g1*XB) + (delBK-XB)*2*g1*XA*dXB 
	  + dlnActCoeffdT_Scaled_[iK]*dTds;
      }
    }
  }
  //====================================================================================================================
  // Update the derivative of the log of the activity coefficients wrt the log of the corresponding species number density
  /*
   * This function will be called to update the internally stored gradients of the 
   * logarithm of the activity coefficients.  These are used in the determination 
   * of the diffusion coefficients.
   *
   *   he = X_A X_B(B + C X_B)
   *
   *  This function only carries out the diagonal calculation
   *
   * HKM - Checked for Transition
   */
  void PhaseCombo_Interaction::s_update_dlnActCoeff_dlnN_diag() const {
    int iA, iB, iK, delAK, delBK;
    doublereal XA, XB, XK, g0 , g1;
    doublereal T = temperature();
    doublereal RT = GasConstant*T;
    doublereal xx;

    fvo_zero_dbl_1(dlnActCoeffdlnN_diag_, m_kk);
    
    for (iK = 0; iK < m_kk; iK++) {

      XK = moleFractions_[iK];  
      /*
       *  We never sample the end of the mole fraction domains
       */
      xx = fmaxx(moleFractions_[iK], xxSmall);
      /*
       *  First wipe out the ideal solution mixing term
       */  
      // lnActCoeff_Scaled_[iK] = - log(xx);
      if (xx > xxSmall) {
	dlnActCoeffdlnN_diag_[iK] = - 1.0 + xx;
      }

      for (int i = 0; i <  numBinaryInteractions_; i++) {
    
	iA =  m_pSpecies_A_ij[i];    
	iB =  m_pSpecies_B_ij[i];

	delAK = 0;
	delBK = 0;

	if (iA==iK) delAK = 1;
	else if (iB==iK) delBK = 1;
	
	XA = moleFractions_[iA];
	XB = moleFractions_[iB];
	
	g0 = (m_HE_b_ij[i] - T * m_SE_b_ij[i]) / RT;
	g1 = (m_HE_c_ij[i] - T * m_SE_c_ij[i]) / RT;
	
	dlnActCoeffdlnN_diag_[iK] += 2*(delBK-XB)*(g0*(delAK-XA)+g1*(2*(delAK-XA)*XB+XA*(delBK-XB)));
      }
      dlnActCoeffdlnN_diag_[iK] = XK*dlnActCoeffdlnN_diag_[iK];
    }
   
  }
  //====================================================================================================================
  // Update the derivative of the log of the activity coefficients wrt ln N_k
  /*
   * This function will be called to update the internally storred gradients of the 
   * logarithm of the activity coefficients.  These are used in the determination 
   * of the diffusion coefficients.
   *
   * HKM - Checked for Transition
   */
  void PhaseCombo_Interaction::s_update_dlnActCoeff_dlnN() const {
    int iA, iB;
    doublereal delAK, delBK;
    double XA, XB, g0 , g1, XK, XM;
    double xx , delKM;
    double T = temperature();
    double RT = GasConstant*T;
 
    doublereal delAM, delBM;

    dlnActCoeffdlnN_.zero();
    
    /*
     *  Loop over the activity coefficient gamma_k
     */
    for (int iK = 0; iK < m_kk; iK++) {
      XK = moleFractions_[iK];
      /*
       *  We never sample the end of the mole fraction domains
       */
      xx = fmaxx(moleFractions_[iK], xxSmall);

      for (int iM = 0; iM < m_kk; iM++) {
	XM = moleFractions_[iM];

	if (xx > xxSmall) {
	  delKM = 0.0;
	  if (iK == iM) delKM = 1.0;
	  // this gets multiplied by XM at the bottom
	  dlnActCoeffdlnN_(iK,iM) += - delKM/XM + 1.0;
	}


	for (int i = 0; i <  numBinaryInteractions_; i++) {
    
	  iA =  m_pSpecies_A_ij[i];    
	  iB =  m_pSpecies_B_ij[i];

	  delAK = 0.0;
	  delBK = 0.0;
	  delAM = 0.0;
	  delBM = 0.0;
	  if      (iA==iK) delAK = 1.0;
	  else if (iB==iK) delBK = 1.0;
	  if      (iA==iM) delAM = 1.0;
	  else if (iB==iM) delBM = 1.0;
	
	  XA = moleFractions_[iA];
	  XB = moleFractions_[iB];
	
	  g0 = (m_HE_b_ij[i] - T * m_SE_b_ij[i]) / RT;
	  g1 = (m_HE_c_ij[i] - T * m_SE_c_ij[i]) / RT;

	  dlnActCoeffdlnN_(iK,iM) += g0*((delAM-XA)*(delBK-XB)+(delAK-XA)*(delBM-XB));
	  dlnActCoeffdlnN_(iK,iM) += 2*g1*((delAM-XA)*(delBK-XB)*XB+(delAK-XA)*(delBM-XB)*XB+(delBM-XB)*(delBK-XB)*XA);

	}
	dlnActCoeffdlnN_(iK,iM) = XM * dlnActCoeffdlnN_(iK,iM);
      }
    }
  }
  //====================================================================================================================
  void PhaseCombo_Interaction::s_update_dlnActCoeff_dlnX_diag() const {

    int iA, iB;
    doublereal XA, XB, g0 , g1;
    doublereal T = temperature();

    fvo_zero_dbl_1(dlnActCoeffdlnX_diag_, m_kk);

    doublereal RT = GasConstant * T;
    
    
    for (int i = 0; i <  numBinaryInteractions_; i++) {
      
      iA =  m_pSpecies_A_ij[i];    
      iB =  m_pSpecies_B_ij[i];
      
      XA = moleFractions_[iA];
      XB = moleFractions_[iB];
      
      g0 = (m_HE_b_ij[i] - T * m_SE_b_ij[i]) / RT;
      g1 = (m_HE_c_ij[i] - T * m_SE_c_ij[i]) / RT;
      
      dlnActCoeffdlnX_diag_[iA] += XA*XB*(2*g1*-2*g0-6*g1*XB);
      dlnActCoeffdlnX_diag_[iB] += XA*XB*(2*g1*-2*g0-6*g1*XB);
    }  
    throw CanteraError("", "unimplemented");
  }
  
  //====================================================================================================================
  //
  /*
   * HKM - Checked for Transition
   */
  void PhaseCombo_Interaction::getdlnActCoeffdlnN_diag(doublereal *dlnActCoeffdlnN_diag) const {
    s_update_dlnActCoeff_dlnN_diag();
    for (int k = 0; k < m_kk; k++) {
      dlnActCoeffdlnN_diag[k] = dlnActCoeffdlnN_diag_[k];
    }
  }
  //====================================================================================================================
  //
  /*
   * HKM - Checked for Transition
   */
  void PhaseCombo_Interaction::getdlnActCoeffdlnX_diag(doublereal *dlnActCoeffdlnX_diag) const {
    s_update_dlnActCoeff_dlnX_diag();
    for (int k = 0; k < m_kk; k++) {
      dlnActCoeffdlnX_diag[k] = dlnActCoeffdlnX_diag_[k];
    }
  }
  //====================================================================================================================
  //
  /*
   * HKM - Checked for Transition
   */
  void PhaseCombo_Interaction::getdlnActCoeffdlnN(const int ld, doublereal *dlnActCoeffdlnN) const {
    s_update_dlnActCoeff_dlnN();
    double *data =  & dlnActCoeffdlnN_(0,0);
    for (int k = 0; k < m_kk; k++) {
      for (int m = 0; m < m_kk; m++) {
	dlnActCoeffdlnN[ld * k + m] = data[m_kk * k + m];
      } 
    }
  }
  //====================================================================================================================
  //
  /*
   * HKM - Checked for Transition
   */
  void PhaseCombo_Interaction::resizeNumInteractions(const int num) {
    numBinaryInteractions_ = num;
    m_HE_b_ij.resize(num, 0.0);
    m_HE_c_ij.resize(num, 0.0);
    m_HE_d_ij.resize(num, 0.0);
    m_SE_b_ij.resize(num, 0.0);
    m_SE_c_ij.resize(num, 0.0);
    m_SE_d_ij.resize(num, 0.0);
    m_VHE_b_ij.resize(num, 0.0);
    m_VHE_c_ij.resize(num, 0.0);
    m_VHE_d_ij.resize(num, 0.0);
    m_VSE_b_ij.resize(num, 0.0);
    m_VSE_c_ij.resize(num, 0.0);
    m_VSE_d_ij.resize(num, 0.0);

    m_pSpecies_A_ij.resize(num, -1);
    m_pSpecies_B_ij.resize(num, -1);
   throw CanteraError("", "unimplemented");
  }
  //====================================================================================================================

  /*
   * Process an XML node called "binaryNeutralSpeciesParameters"
   * This node contains all of the parameters necessary to describe
   * the Margules Interaction for a single binary interaction
   * This function reads the XML file and writes the coefficients
   * it finds to an internal data structures.
   */
  void PhaseCombo_Interaction::readXMLBinarySpecies(XML_Node &xmLBinarySpecies) {
    string xname = xmLBinarySpecies.name();
    if (xname != "binaryNeutralSpeciesParameters") {
      throw CanteraError("PhaseCombo_Interaction::readXMLBinarySpecies",
                         "Incorrect name for processing this routine: " + xname);
    }
    double *charge = DATA_PTR(m_speciesCharge);
    string stemp;
    int nParamsFound;
    vector_fp vParams;
    string iName = xmLBinarySpecies.attrib("speciesA");
    if (iName == "") {
      throw CanteraError("PhaseCombo_Interaction::readXMLBinarySpecies", "no speciesA attrib");
    }
    string jName = xmLBinarySpecies.attrib("speciesB");
    if (jName == "") {
      throw CanteraError("PhaseCombo_Interaction::readXMLBinarySpecies", "no speciesB attrib");
    }
    /*
     * Find the index of the species in the current phase. It's not
     * an error to not find the species
     */
    int iSpecies = speciesIndex(iName);
    if (iSpecies < 0) {
      return;
    }
    string ispName = speciesName(iSpecies);
    if (charge[iSpecies] != 0) {
      throw CanteraError("PhaseCombo_Interaction::readXMLBinarySpecies", "speciesA charge problem");
    }
    int jSpecies = speciesIndex(jName);
    if (jSpecies < 0) {
      return;
    }
    string jspName = speciesName(jSpecies);
    if (charge[jSpecies] != 0) {
      throw CanteraError("PhaseCombo_Interaction::readXMLBinarySpecies", "speciesB charge problem");
    }

    resizeNumInteractions(numBinaryInteractions_ + 1);
    int iSpot = numBinaryInteractions_ - 1;
    m_pSpecies_A_ij[iSpot] = iSpecies;
    m_pSpecies_B_ij[iSpot] = jSpecies;
  
    int num = xmLBinarySpecies.nChildren();
    for (int iChild = 0; iChild < num; iChild++) {
      XML_Node &xmlChild = xmLBinarySpecies.child(iChild);
      stemp = xmlChild.name();
      string nodeName = lowercase(stemp);
      /*
       * Process the binary species interaction child elements
       */
      if (nodeName == "excessenthalpy") {
        /*
         * Get the string containing all of the values
         */
        getFloatArray(xmlChild, vParams, true, "toSI", "excessEnthalpy");
        nParamsFound = vParams.size();
       
	if (nParamsFound != 2) {
	  throw CanteraError("PhaseCombo_Interaction::readXMLBinarySpecies::excessEnthalpy for " + ispName
			     + "::" + jspName,
			     "wrong number of params found");
	}
	m_HE_b_ij[iSpot] = vParams[0];
	m_HE_c_ij[iSpot] = vParams[1];
      }

      if (nodeName == "excessentropy") {
        /*
         * Get the string containing all of the values
         */
        getFloatArray(xmlChild, vParams, true, "toSI", "excessEntropy");
        nParamsFound = vParams.size();
       
	if (nParamsFound != 2) {
	  throw CanteraError("PhaseCombo_Interaction::readXMLBinarySpecies::excessEntropy for " + ispName
			     + "::" + jspName,
			     "wrong number of params found");
	}
	m_SE_b_ij[iSpot] = vParams[0];
	m_SE_c_ij[iSpot] = vParams[1];
      }

      if (nodeName == "excessvolume_enthalpy") {
        /*
         * Get the string containing all of the values
         */
        getFloatArray(xmlChild, vParams, true, "toSI", "excessVolume_Enthalpy");
        nParamsFound = vParams.size();
       
	if (nParamsFound != 2) {
	  throw CanteraError("PhaseCombo_Interaction::readXMLBinarySpecies::excessVolume_Enthalpy for " + ispName
			     + "::" + jspName,
			     "wrong number of params found");
	}
	m_VHE_b_ij[iSpot] = vParams[0];
	m_VHE_c_ij[iSpot] = vParams[1];
      }

      if (nodeName == "excessvolume_entropy") {
        /*
         * Get the string containing all of the values
         */
        getFloatArray(xmlChild, vParams, true, "toSI", "excessVolume_Entropy");
        nParamsFound = vParams.size();
       
	if (nParamsFound != 2) {
	  throw CanteraError("PhaseCombo_Interaction::readXMLBinarySpecies::excessVolume_Entropy for " + ispName
			     + "::" + jspName,
			     "wrong number of params found");
	}
	m_VSE_b_ij[iSpot] = vParams[0];
	m_VSE_c_ij[iSpot] = vParams[1];
      }


    } 
  }
  //====================================================================================================================
}
//======================================================================================================================
