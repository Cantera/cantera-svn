/**
 * @file vcs_VolPhase.cpp
 */

/* $Id$ */

/*
 * Copywrite (2005) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */
#include "vcs_VolPhase.h"
#include "vcs_internal.h"
#include "vcs_SpeciesProperties.h"
#include "vcs_species_thermo.h"
#include "vcs_solve.h"

#include "ThermoPhase.h"
#include "mix_defs.h"

#include <string>
#include <cstdio>
#include <cstdlib>

namespace VCSnonideal {

  /*
   * 
   *  vcs_VolPhase():
   *
   *    Constructor for the VolPhase object.
   */
  vcs_VolPhase::vcs_VolPhase(VCS_SOLVE * owningSolverObject) :
    m_owningSolverObject(0),
    VP_ID_(-1),
    Domain_ID(-1),
    m_singleSpecies(true),
    m_gasPhase(false),
    m_eqnState(VCS_EOS_CONSTANT),
    ChargeNeutralityElement(-1),
    p_VCS_UnitsFormat(VCS_UNITS_MKS),
    p_activityConvention(0),
    m_numElemConstraints(0),
    m_elemGlobalIndex(0),
    m_numSpecies(0),
    m_totalMolesInert(0.0),
    m_isIdealSoln(false),
    m_existence(VCS_PHASE_EXIST_NO),
    m_MFStartIndex(0),
    IndSpecies(0),
    m_useCanteraCalls(false),
    TP_ptr(0),
    v_totalMoles(0.0),
    m_phiVarIndex(-1),
    m_totalVol(0.0),
    m_vcsStateStatus(VCS_STATECALC_OLD),
    m_phi(0.0),
    m_UpToDate(false),
    m_UpToDate_AC(false),
    m_UpToDate_VolStar(false),
    m_UpToDate_VolPM(false),
    m_UpToDate_GStar(false),
    m_UpToDate_G0(false),
    Temp(273.15),
    Pres(1.01325E5),
    RefPres(1.01325E5)
  {
    m_owningSolverObject = owningSolverObject;
  }
  /***************************************************************************/

  /*
   * 
   *  ~vcs_VolPhase():
   *
   *   Destructor for the VolPhase object.
   */
  vcs_VolPhase::~vcs_VolPhase() {
    for (int k = 0; k < m_numSpecies; k++) {
      vcs_SpeciesProperties *sp = ListSpeciesPtr[k];
      delete sp;
      sp = 0;
    }
  }
  /************************************************************************************/

  /*
   * 
   *  Copy Constructor():
   *
   *  Objects that are owned by this object are deep copied here, except
   *  for the ThermoPhase object.
   *  The assignment operator does most of the work.
   */
  vcs_VolPhase::vcs_VolPhase(const vcs_VolPhase& b) :
    m_owningSolverObject(b.m_owningSolverObject),
    VP_ID_(b.VP_ID_),
    Domain_ID(b.Domain_ID),
    m_singleSpecies(b.m_singleSpecies),
    m_gasPhase(b.m_gasPhase),
    m_eqnState(b.m_eqnState),
    ChargeNeutralityElement(b.ChargeNeutralityElement),
    p_VCS_UnitsFormat(b.p_VCS_UnitsFormat),
    p_activityConvention(b.p_activityConvention),
    m_numElemConstraints(b.m_numElemConstraints),
    m_numSpecies(b.m_numSpecies),
    m_totalMolesInert(b.m_totalMolesInert),
    m_isIdealSoln(b.m_isIdealSoln),
    m_existence(b.m_existence),
    m_MFStartIndex(b.m_MFStartIndex),
    m_useCanteraCalls(b.m_useCanteraCalls),
    TP_ptr(b.TP_ptr),
    v_totalMoles(b.v_totalMoles),
    m_phiVarIndex(-1),
    m_totalVol(b.m_totalVol),
    m_vcsStateStatus(VCS_STATECALC_OLD),
    m_phi(b.m_phi),
    m_UpToDate(false),
    m_UpToDate_AC(false),
    m_UpToDate_VolStar(false),
    m_UpToDate_VolPM(false),
    m_UpToDate_GStar(false),
    m_UpToDate_G0(false),
    Temp(b.Temp),
    Pres(b.Pres)
  {
    /*
     * Call the Assignment operator to do the heavy
     * lifting.
     */
    *this = b;
  }
  /***************************************************************************/
  
  /*
   * Assignment operator()
   *
   *   (note, this is used, so keep it current!)
   */
  vcs_VolPhase& vcs_VolPhase::operator=(const vcs_VolPhase& b) {
    int k;
    if (&b != this) {
      int old_num = m_numSpecies;

      //  Note: we comment this out for the assignment operator
      //        specifically, because it isn't true for the assignment
      //        operator but is true for a copy constructor
      // m_owningSolverObject = b.m_owningSolverObject;

      VP_ID_               = b.VP_ID_;
      Domain_ID           = b.Domain_ID;
      m_singleSpecies     = b.m_singleSpecies;
      m_gasPhase            = b.m_gasPhase;
      m_eqnState            = b.m_eqnState;
 
      m_numSpecies         = b.m_numSpecies;
      m_numElemConstraints    = b.m_numElemConstraints;
      ChargeNeutralityElement = b.ChargeNeutralityElement;


      m_elementNames.resize(b.m_numElemConstraints);
      for (int e = 0; e < b.m_numElemConstraints; e++) {
	m_elementNames[e] = b.m_elementNames[e];
      }
 
      m_elementActive = b.m_elementActive;
      m_elementType = b.m_elementType;
  
      m_formulaMatrix.resize(m_numElemConstraints, m_numSpecies, 0.0);
      for (int e = 0; e < m_numElemConstraints; e++) {
	for (int k = 0; k < m_numSpecies; k++) {
	  m_formulaMatrix[e][k] = b.m_formulaMatrix[e][k];
	}
      }

      m_speciesUnknownType = b.m_speciesUnknownType;
      m_elemGlobalIndex    = b.m_elemGlobalIndex;
      m_numSpecies         = b.m_numSpecies;
      PhaseName           = b.PhaseName;
      m_totalMolesInert   = b.m_totalMolesInert;
      p_activityConvention= b.p_activityConvention;
      m_isIdealSoln       = b.m_isIdealSoln;
      m_existence         = b.m_existence;
      m_MFStartIndex      = b.m_MFStartIndex;

      /*
       * Do a shallow copy because we haven' figured this out.
       */
      IndSpecies = b.IndSpecies;
      //IndSpeciesContig = b.IndSpeciesContig;

      for (k = 0; k < old_num; k++) {
	if ( ListSpeciesPtr[k]) {
	  delete  ListSpeciesPtr[k];
	  ListSpeciesPtr[k] = 0;
	}
      }
      ListSpeciesPtr.resize(m_numSpecies, 0);
      for (k = 0; k < m_numSpecies; k++) {
	ListSpeciesPtr[k] = 
	  new vcs_SpeciesProperties(*(b.ListSpeciesPtr[k]));
      }
    
      p_VCS_UnitsFormat   = b.p_VCS_UnitsFormat;
      m_useCanteraCalls   = b.m_useCanteraCalls;
      /*
       * Do a shallow copy of the ThermoPhase object pointer.
       * We don't duplicate the object.
       *  Um, there is no reason we couldn't do a 
       *  duplicateMyselfAsThermoPhase() call here. This will
       *  have to be looked into.
       */
      TP_ptr              = b.TP_ptr;
      v_totalMoles              = b.v_totalMoles;
 
      Xmol = b.Xmol;
      fractionCreationDelta_ = b.fractionCreationDelta_;

      m_phi               = b.m_phi;
      m_phiVarIndex       = b.m_phiVarIndex;
 
      SS0ChemicalPotential = b.SS0ChemicalPotential;
      StarChemicalPotential = b.StarChemicalPotential;

      StarMolarVol = b.StarMolarVol;
      PartialMolarVol = b.PartialMolarVol; 
      ActCoeff = b.ActCoeff;

      dLnActCoeffdMolNumber = b.dLnActCoeffdMolNumber;

      m_UpToDate            = false;
      m_vcsStateStatus      = b.m_vcsStateStatus;
      m_UpToDate_AC         = false;
      m_UpToDate_VolStar    = false;
      m_UpToDate_VolPM      = false;
      m_UpToDate_GStar      = false;
      m_UpToDate_G0         = false;
      Temp                = b.Temp;
      Pres                = b.Pres;
      setState_TP(Temp, Pres);
      _updateMoleFractionDependencies();
    }
    return *this;
  }
  /***************************************************************************/

  void vcs_VolPhase::resize(const int phaseNum, const int nspecies, 
			    const int numElem, const char * const phaseName,
			    const double molesInert) {
#ifdef DEBUG_MODE
    if (nspecies <= 0) {
      plogf("nspecies Error\n");
      exit(EXIT_FAILURE);
    }
    if (phaseNum < 0) {
      plogf("phaseNum should be greater than 0\n");
      exit(EXIT_FAILURE);
    }
#endif
    setTotalMolesInert(molesInert);
    m_phi = 0.0;
    m_phiVarIndex = -1;

    if (phaseNum == VP_ID_) {
      if (strcmp(PhaseName.c_str(), phaseName)) {
	plogf("Strings are different: %s %s :unknown situation\n",
	      PhaseName.c_str(), phaseName);
	exit(EXIT_FAILURE);
      }
    } else {
      VP_ID_ = phaseNum;
      if (!phaseName) {
	char itmp[40];
	sprintf(itmp, "Phase_%d", VP_ID_);
	PhaseName = itmp;
      } else {
	PhaseName = phaseName;
      }
    }
    if (nspecies > 1) {
      m_singleSpecies = false;
    } else {
      m_singleSpecies = true;
    }

    if (m_numSpecies == nspecies && numElem == m_numElemConstraints) {
      return;
    }
 
    m_numSpecies = nspecies;
    if (nspecies > 1) {
      m_singleSpecies = false;
    }


    IndSpecies.resize(nspecies, -1);

    if ((int) ListSpeciesPtr.size() >= m_numSpecies) {
      for (int i = 0; i < m_numSpecies; i++) {
	if (ListSpeciesPtr[i]) {
	  delete ListSpeciesPtr[i]; 
	  ListSpeciesPtr[i] = 0;
	}
      }
    }
    ListSpeciesPtr.resize(nspecies, 0);
    for (int i = 0; i < nspecies; i++) {
      ListSpeciesPtr[i] = new vcs_SpeciesProperties(phaseNum, i, this);
    }

    Xmol.resize(nspecies, 0.0);
    fractionCreationDelta_.resize(nspecies, 0.0);
    for (int i = 0; i < nspecies; i++) {
      Xmol[i] = 1.0/nspecies;
      fractionCreationDelta_[i] = 1.0/nspecies;
    }

    SS0ChemicalPotential.resize(nspecies, -1.0);
    StarChemicalPotential.resize(nspecies, -1.0);
    StarMolarVol.resize(nspecies, -1.0);
    PartialMolarVol.resize(nspecies, -1.0);
    ActCoeff.resize(nspecies, 1.0);
    dLnActCoeffdMolNumber.resize(nspecies, nspecies, 0.0);
 

    m_speciesUnknownType.resize(nspecies, VCS_SPECIES_TYPE_MOLNUM);
    m_UpToDate            = false;
    m_vcsStateStatus      = VCS_STATECALC_OLD;
    m_UpToDate_AC         = false;
    m_UpToDate_VolStar    = false;
    m_UpToDate_VolPM      = false;
    m_UpToDate_GStar      = false;
    m_UpToDate_G0         = false;


    elemResize(numElem);

  }
  /***************************************************************************/

  void vcs_VolPhase::elemResize(const int numElemConstraints) {

    m_elementNames.resize(numElemConstraints);

    m_elementActive.resize(numElemConstraints+1, 1);
    m_elementType.resize(numElemConstraints, VCS_ELEM_TYPE_ABSPOS);
    m_formulaMatrix.resize(numElemConstraints, m_numSpecies, 0.0);

    m_elementNames.resize(numElemConstraints, "");
    m_elemGlobalIndex.resize(numElemConstraints, -1);

    m_numElemConstraints = numElemConstraints;
  }
  /***************************************************************************/

  //! Evaluate activity coefficients
  /*!
   *   We carry out a calculation whenever UpTODate_AC is false. Specifically
   *   whenever a phase goes zero, we do not carry out calculations on it.
   * 
   * (private)
   */
  void vcs_VolPhase::_updateActCoeff() const {
    if (m_isIdealSoln) {
      m_UpToDate_AC = true;
      return;
    }
    if (m_useCanteraCalls) {
      TP_ptr->getActivityCoefficients(VCS_DATA_PTR(ActCoeff));
    }
    m_UpToDate_AC = true;
  }
  /***************************************************************************/

  /*
   *
   * Evaluate one activity coefficients.
   *
   *   return one activity coefficient. Have to recalculate them all to get
   *   one.
   */
  double vcs_VolPhase::AC_calc_one(int kspec) const {
    if (! m_UpToDate_AC) {
      _updateActCoeff();
    }
    return(ActCoeff[kspec]);
  }
  /***************************************************************************/

  // Gibbs free energy calculation at a temperature for the reference state
  // of each species
  /*
   */
  void vcs_VolPhase::_updateG0() const {
    if (m_useCanteraCalls) {
      TP_ptr->getGibbs_ref(VCS_DATA_PTR(SS0ChemicalPotential));
    } else {
      double R = vcsUtil_gasConstant(p_VCS_UnitsFormat);
      for (int k = 0; k < m_numSpecies; k++) {
	int kglob = IndSpecies[k];
	vcs_SpeciesProperties *sProp = ListSpeciesPtr[k];
	VCS_SPECIES_THERMO *sTherm = sProp->SpeciesThermo;
	SS0ChemicalPotential[k] =
	  R * (sTherm->G0_R_calc(kglob, Temp));
      }
    }
    m_UpToDate_G0 = true;
  }
  /***************************************************************************/

  // Gibbs free energy calculation at a temperature for the reference state
  // of a species, return a value for one species
  /*
   *  @param kspec   species index
   *  @param TKelvin temperature
   *
   *  @return return value of the gibbs free energy
   */
  double vcs_VolPhase::G0_calc_one(int kspec) const {
    if (!m_UpToDate_G0) {
      _updateG0();
    }
    return SS0ChemicalPotential[kspec];
  }
  /***************************************************************************/

  // Gibbs free energy calculation for standard states
  /*
   * Calculate the Gibbs free energies for the standard states
   * The results are held internally within the object.
   *
   * @param TKelvin Current temperature
   * @param pres    Current pressure (pascal)
   */
  void vcs_VolPhase::_updateGStar() const {
    if (m_useCanteraCalls) {
      TP_ptr->getStandardChemPotentials(VCS_DATA_PTR(StarChemicalPotential));
    } else {
      double R = vcsUtil_gasConstant(p_VCS_UnitsFormat);
      for (int k = 0; k < m_numSpecies; k++) {
	int kglob = IndSpecies[k];
	vcs_SpeciesProperties *sProp = ListSpeciesPtr[k];
	VCS_SPECIES_THERMO *sTherm = sProp->SpeciesThermo;
	StarChemicalPotential[k] =
	  R * (sTherm->GStar_R_calc(kglob, Temp, Pres));
      }
    }
    m_UpToDate_GStar = true;
  }
  /***************************************************************************/

  // Gibbs free energy calculation for standard state of one species
  /*
   * Calculate the Gibbs free energies for the standard state
   * of the kth species.
   * The results are held internally within the object.
   * The kth species standard state G is returned
   *
   * @param kspec   Species number (within the phase)
   *
   * @return Gstar[kspec] returns the gibbs free energy for the
   *         standard state of the kspec species.
   */
  double vcs_VolPhase::GStar_calc_one(int kspec) const {
    if (!m_UpToDate_GStar) {
      _updateGStar();
    }
    return StarChemicalPotential[kspec];
  }
  /***************************************************************************/

  // Set the mole fractions from a conventional mole fraction vector
  /*
   *
   * @param xmol Value of the mole fractions for the species
   *             in the phase. These are contiguous. 
   */
  void vcs_VolPhase::setMoleFractions(const double * const xmol) {
    double sum = -1.0;
    for (int k = 0; k < m_numSpecies; k++) {
      Xmol[k] = xmol[k];
      sum+= xmol[k];
    }
    if (std::fabs(sum) > 1.0E-13) {
      for (int k = 0; k < m_numSpecies; k++) {
	Xmol[k] /= sum;
      }
    }
    _updateMoleFractionDependencies();
    m_UpToDate = false;
    m_vcsStateStatus = VCS_STATECALC_TMP;
  }
  /***************************************************************************/

  // Updates the mole fractions in subobjects
  /*
   *  Whenever the mole fractions change, this routine
   *  should be called.
   */
  void vcs_VolPhase::_updateMoleFractionDependencies() {
    if (m_useCanteraCalls) {
      if (TP_ptr) {
	TP_ptr->setState_PX(Pres, &(Xmol[m_MFStartIndex]));
      }
    }
    if (!m_isIdealSoln) {
      m_UpToDate_AC = false;
      m_UpToDate_VolPM = false;
    }
  }
  /***************************************************************************/

  // Return a const reference to the mole fraction vector in the phase
  const std::vector<double> & vcs_VolPhase::moleFractions() const {
    return Xmol;
  }
  /***************************************************************************/

  // Set the moles and/or mole fractions within the phase
  /*
   *
   *
   */
  void vcs_VolPhase::setMoleFractionsState(const double totalMoles,
					   const double * const moleFractions,
					   const int vcsStateStatus) {

    if (totalMoles != 0.0) {
      // There are other ways to set the mole fractions when VCS_STATECALC
      // is set to a normal settting.
      if (vcsStateStatus != VCS_STATECALC_TMP) {
	printf("vcs_VolPhase::setMolesFractionsState: inappropriate usage\n");
	exit(EXIT_FAILURE);
      }
      m_UpToDate = false;
      m_vcsStateStatus = VCS_STATECALC_TMP;
      if (m_existence == VCS_PHASE_EXIST_ZEROEDPHASE ) {
	printf("vcs_VolPhase::setMolesFractionsState: inappropriate usage\n");
	exit(EXIT_FAILURE);
      }
      m_existence = VCS_PHASE_EXIST_YES;
    } else {
      m_UpToDate = true;
      m_vcsStateStatus = vcsStateStatus;
      if (m_existence > VCS_PHASE_EXIST_NO ) {
	m_existence = VCS_PHASE_EXIST_NO;
      }
    }
    v_totalMoles = totalMoles;
    double sum = 0.0;
    for (int k = 0; k < m_numSpecies; k++) {
      Xmol[k] = moleFractions[k];
      sum += moleFractions[k];
    }
    if (sum == 0.0) {
      printf("vcs_VolPhase::setMolesFractionsState: inappropriate usage\n");
      exit(EXIT_FAILURE);
    }
    if (sum  != 1.0) {
      for (int k = 0; k < m_numSpecies; k++) {
	Xmol[k] /= sum;
      }
    }
    _updateMoleFractionDependencies();
 
  }
  /***************************************************************************/

  // Set the moles within the phase
  /*
   *  This function takes as input the mole numbers in vcs format, and
   *  then updates this object with their values. This is essentially
   *  a gather routine.
   *
   *  
   *  @param molesSpeciesVCS  array of mole numbers. Note, the indecises 
   *            for species in 
   *            this array may not be contiguous. IndSpecies[] is needed
   *            to gather the species into the local contiguous vector
   *            format. 
   */
  void vcs_VolPhase::setMolesFromVCS(const int stateCalc, 
				     const double * molesSpeciesVCS) {
    int kglob;
    double tmp;
    v_totalMoles = m_totalMolesInert;

    if (molesSpeciesVCS == 0) {
#ifdef DEBUG_MODE
      if (m_owningSolverObject == 0) {
	printf("vcs_VolPhase::setMolesFromVCS  shouldn't be here\n");
	exit(EXIT_FAILURE);
      }
#endif
      if (stateCalc == VCS_STATECALC_OLD) {
	molesSpeciesVCS = VCS_DATA_PTR(m_owningSolverObject->m_molNumSpecies_old);
      } else if (stateCalc == VCS_STATECALC_NEW) {
	molesSpeciesVCS = VCS_DATA_PTR(m_owningSolverObject->m_molNumSpecies_new);
      }
#ifdef DEBUG_MODE
      else {
	printf("vcs_VolPhase::setMolesFromVCS shouldn't be here\n");
	exit(EXIT_FAILURE);
      }
#endif
    }
#ifdef DEBUG_MODE
    else {
      if (m_owningSolverObject) {
        if (stateCalc == VCS_STATECALC_OLD) {
  	  if (molesSpeciesVCS != VCS_DATA_PTR(m_owningSolverObject->m_molNumSpecies_old)) {
	    printf("vcs_VolPhase::setMolesFromVCS shouldn't be here\n");
	    exit(EXIT_FAILURE);
          }
        } else if (stateCalc == VCS_STATECALC_NEW) {
          if (molesSpeciesVCS != VCS_DATA_PTR(m_owningSolverObject->m_molNumSpecies_new)) {
	    printf("vcs_VolPhase::setMolesFromVCS shouldn't be here\n");
	    exit(EXIT_FAILURE);
          }
        }
      }
    }
#endif

    for (int k = 0; k < m_numSpecies; k++) {
      if (m_speciesUnknownType[k] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	kglob = IndSpecies[k];
	v_totalMoles += MAX(0.0, molesSpeciesVCS[kglob]);
      }
    }
    if (v_totalMoles > 0.0) {
      for (int k = 0; k < m_numSpecies; k++) {
	if (m_speciesUnknownType[k] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	  kglob = IndSpecies[k];
	  tmp = MAX(0.0, molesSpeciesVCS[kglob]);
	  Xmol[k] = tmp / v_totalMoles;
	}
      }
      m_existence = VCS_PHASE_EXIST_YES;
    } else {
      // This is where we will start to store a better approximation 
      // for the mole fractions, when the phase doesn't exist.
      // This is currently unimplemented.
      //for (int k = 0; k < m_numSpecies; k++) {
      //	Xmol[k] = 1.0 / m_numSpecies;
      //}
      m_existence = VCS_PHASE_EXIST_NO;
    }
    /*
     * Update the electric potential if it is a solution variable
     * in the equation system
     */
    if (m_phiVarIndex >= 0) {
      kglob = IndSpecies[m_phiVarIndex];
      if (m_numSpecies == 1) {
	Xmol[m_phiVarIndex] = 1.0;
      } else {
	Xmol[m_phiVarIndex] = 0.0;
      }
      double phi = molesSpeciesVCS[kglob];
      setElectricPotential(phi);
      if (m_numSpecies == 1) {
	m_existence = VCS_PHASE_EXIST_YES;
      }
    }
    _updateMoleFractionDependencies();
    if (m_totalMolesInert > 0.0) {
      m_existence = VCS_PHASE_EXIST_ALWAYS;
    }

    /*
     * If stateCalc is old and the total moles is positive,
     * then we have a valid state. If the phase went away, it would
     * be a valid starting point for F_k's. So, save the state.
     */
    if (stateCalc == VCS_STATECALC_OLD) {
      if (v_totalMoles > 0.0) {
	fractionCreationDelta_ = Xmol;
      }
    }

    /*
     * Set flags indicating we are up to date with the VCS state vector.
     */
    m_UpToDate = true;
    m_vcsStateStatus = stateCalc; 
 
  }
  /***************************************************************************/

  // Set the moles within the phase
  /*
   *  This function takes as input the mole numbers in vcs format, and
   *  then updates this object with their values. This is essentially
   *  a gather routine.
   *
   *     @param vcsStateStatus  State calc value either VCS_STATECALC_OLD 
   *                         or  VCS_STATECALC_NEW. With any other value
   *                         nothing is done.
   *
   *  @param molesSpeciesVCS  array of mole numbers. Note, 
   *                          the indecises for species in 
   *            this array may not be contiguous. IndSpecies[] is needed
   *            to gather the species into the local contiguous vector
   *            format. 
   */
  void vcs_VolPhase::setMolesFromVCSCheck(const int vcsStateStatus,
					  const double * molesSpeciesVCS, 
					  const double * const TPhMoles) {
    setMolesFromVCS(vcsStateStatus, molesSpeciesVCS);
    /*
     * Check for consistency with TPhMoles[]
     */
    double Tcheck = TPhMoles[VP_ID_];
    if (Tcheck != v_totalMoles) {
      if (vcs_doubleEqual(Tcheck, v_totalMoles)) {
	Tcheck = v_totalMoles;
      } else {
	plogf("vcs_VolPhase::setMolesFromVCSCheck: "
	      "We have a consistency problem: %21.16g %21.16g\n",
	      Tcheck, v_totalMoles);
	exit(EXIT_FAILURE);
      }
    }
  }
  /***************************************************************************/

  // Update the moles within the phase, if necessary
  /*
   *  This function takes as input the stateCalc value, which 
   *  determines where within VCS_SOLVE to fetch the mole numbers.
   *  It then updates this object with their values. This is essentially
   *  a gather routine.
   *
   *  @param vcsStateStatus  State calc value either VCS_STATECALC_OLD 
   *                         or  VCS_STATECALC_NEW. With any other value
   *                         nothing is done.
   *
   */
  void vcs_VolPhase::updateFromVCS_MoleNumbers(const int vcsStateStatus) {
    if (!m_UpToDate || (vcsStateStatus != m_vcsStateStatus)) {
      if (vcsStateStatus == VCS_STATECALC_OLD || vcsStateStatus == VCS_STATECALC_NEW) {
	if (m_owningSolverObject) {
	  setMolesFromVCS(vcsStateStatus);
	}
      }
    }
  }
  /**************************************************************************/

  // Fill in an activity coefficients vector within a VCS_SOLVE object
  /*
   *  This routine will calculate the activity coefficients for the
   *  current phase, and fill in the corresponding entries in the
   *  VCS activity coefficients vector.
   *  
   * @param AC  vector of activity coefficients for all of the species
   *            in all of the phases in a VCS problem. Only the
   *            entries for the current phase are filled in.
   */
  void vcs_VolPhase::sendToVCS_ActCoeff(const int vcsStateStatus,
					double * const AC) {
    updateFromVCS_MoleNumbers(vcsStateStatus);
    if (!m_UpToDate_AC) {
      _updateActCoeff();
    }
    int kglob;
    for (int k = 0; k < m_numSpecies; k++) {
      kglob = IndSpecies[k];
      AC[kglob] = ActCoeff[k];
    }
  }
  /***************************************************************************/

  // Fill in the partial molar volume vector for VCS
  /*
   *  This routine will calculate the partial molar volumes for the
   *  current phase (if needed), and fill in the corresponding entries in the
   *  VCS partial molar volumes vector.
   *  
   * @param VolPM  vector of partial molar volumes for all of the species
   *            in all of the phases in a VCS problem. Only the
   *            entries for the current phase are filled in.
   */
  double vcs_VolPhase::sendToVCS_VolPM(double * const VolPM) const {
    if (!m_UpToDate_VolPM) {
      (void) _updateVolPM();
    }
    int kglob;
    for (int k = 0; k < m_numSpecies; k++) {
      kglob = IndSpecies[k];
      VolPM[kglob] = PartialMolarVol[k];
    }
    return m_totalVol;
  }
  /***************************************************************************/

  // Fill in the partial molar volume vector for VCS
  /*
   *  This routine will calculate the partial molar volumes for the
   *  current phase (if needed), and fill in the corresponding entries in the
   *  VCS partial molar volumes vector.
   *  
   * @param VolPM  vector of partial molar volumes for all of the species
   *            in all of the phases in a VCS problem. Only the
   *            entries for the current phase are filled in.
   */
  void vcs_VolPhase::sendToVCS_GStar(double * const gstar) const {  
    if (!m_UpToDate_GStar) {
      _updateGStar();
    }
    int kglob;
    for (int k = 0; k < m_numSpecies; k++) {
      kglob = IndSpecies[k];
      gstar[kglob] = StarChemicalPotential[k];
    }
  }
  /***************************************************************************/


  void vcs_VolPhase::setElectricPotential(const double phi) {
    m_phi = phi;
    if (m_useCanteraCalls) {
      TP_ptr->setElectricPotential(m_phi);
    }
    // We have changed the state variable. Set uptodate flags to false
    m_UpToDate_AC = false;
    m_UpToDate_VolStar = false;
    m_UpToDate_VolPM = false;
    m_UpToDate_GStar = false;
  }
  /***************************************************************************/

  double vcs_VolPhase::electricPotential() const {
    return m_phi;
  }
  /***************************************************************************/

  // Sets the temperature and pressure in this object and
  //  underlying objects
  /*
   *  Sets the temperature and pressure in this object and
   *  underlying objects. The underlying objects refers to the
   *  Cantera's ThermoPhase object for this phase.
   *
   *  @param temperature_Kelvin    (Kelvin)
   *  @param pressure_PA  Pressure (MKS units - Pascal)
   */
  void vcs_VolPhase::setState_TP(const double temp, const double pres)
  {
    if (Temp == temp) {
      if (Pres == pres) {
	return;
      }
    }
    if (m_useCanteraCalls) {
      TP_ptr->setElectricPotential(m_phi);
      TP_ptr->setState_TP(temp, pres);
    }
    Temp = temp;
    Pres = pres;
    m_UpToDate_AC      = false;
    m_UpToDate_VolStar = false;
    m_UpToDate_VolPM   = false;
    m_UpToDate_GStar   = false;
    m_UpToDate_G0      = false;
  }
  /***************************************************************************/

  // Sets the temperature in this object and
  // underlying objects
  /*
   *  Sets the temperature and pressure in this object and
   *  underlying objects. The underlying objects refers to the
   *  Cantera's ThermoPhase object for this phase.
   *
   *  @param temperature_Kelvin    (Kelvin)
   */
  void vcs_VolPhase::setState_T(const double temp) {
    setState_TP(temp, Pres);
  }
  /***************************************************************************/

  // Molar volume calculation for standard states
  /*
   * Calculate the molar volume for the standard states
   * The results are held internally within the object.
   *
   * @param TKelvin Current temperature
   * @param pres    Current pressure (pascal)
   *
   *  Calculations are in m**3 / kmol
   */
  void vcs_VolPhase::_updateVolStar() const {
    if (m_useCanteraCalls) {
      TP_ptr->getStandardVolumes(VCS_DATA_PTR(StarMolarVol));
    } else {
      for (int k = 0; k < m_numSpecies; k++) {
	int kglob = IndSpecies[k];
	vcs_SpeciesProperties *sProp = ListSpeciesPtr[k];
	VCS_SPECIES_THERMO *sTherm = sProp->SpeciesThermo;
	StarMolarVol[k] = (sTherm->VolStar_calc(kglob, Temp, Pres));
      }
    }
    m_UpToDate_VolStar = true;
  }
  /***************************************************************************/

  // Molar volume calculation for standard state of one species
  /*
   * Calculate the molar volume for the standard states
   * The results are held internally within the object.
   * Return the molar volume for one species
   *
   * @param kspec   Species number (within the phase)
   * @param TKelvin Current temperature
   * @param pres    Current pressure (pascal)
   *
   * @return molar volume of the kspec species's standard
   *         state
   */
  double vcs_VolPhase::VolStar_calc_one(int kspec) const {
    if (!m_UpToDate_VolStar) {
      _updateVolStar();
    }
    return StarMolarVol[kspec];
  }
  /***************************************************************************/

  // Calculate the partial molar volumes of all species and return the
  // total volume
  /*
   *  Calculates these quantitites internally and then stores them
   *
   * @return total volume  (m**3)
   */
  double vcs_VolPhase::_updateVolPM() const {
    int k, kglob;

    if (m_useCanteraCalls) {
      TP_ptr->getPartialMolarVolumes(VCS_DATA_PTR(PartialMolarVol));
    } else {
      for (k = 0; k < m_numSpecies; k++) {
	kglob = IndSpecies[k];
	vcs_SpeciesProperties *sProp = ListSpeciesPtr[k];
	VCS_SPECIES_THERMO *sTherm = sProp->SpeciesThermo;
	StarMolarVol[k] = (sTherm->VolStar_calc(kglob, Temp, Pres));
      }
      for (k = 0; k < m_numSpecies; k++) {
	PartialMolarVol[k] = StarMolarVol[k];
      }
    }

    m_totalVol = 0.0;
    for (k = 0; k < m_numSpecies; k++) {
      m_totalVol += PartialMolarVol[k] * Xmol[k];
    }
    m_totalVol *= v_totalMoles;

    if (m_totalMolesInert > 0.0) {
      if (m_gasPhase) {
	double volI = m_totalMolesInert * 8314.47215 * Temp / Pres;
	m_totalVol += volI;
      } else {
	printf("unknown situation\n");
	exit(EXIT_FAILURE);
      }
    }
    m_UpToDate_VolPM = true;
    return m_totalVol;
  }
  /***************************************************************************/

  /*
   * _updateLnActCoeffJac():
   *
   */
  void vcs_VolPhase::_updateLnActCoeffJac() {
    int k, j;
    double deltaMoles_j = 0.0;
 
    /*
     * Evaluate the current base activity coefficients if necessary
     */ 
    if (!m_UpToDate_AC) { 
      _updateActCoeff();
    }

    // Make copies of ActCoeff and Xmol for use in taking differences
    std::vector<double> ActCoeff_Base(ActCoeff);
    std::vector<double> Xmol_Base(Xmol);
    double TMoles_base = v_totalMoles;

    /*
     *  Loop over the columns species to be deltad
     */
    for (j = 0; j < m_numSpecies; j++) {
      /*
       * Calculate a value for the delta moles of species j
       * -> NOte Xmol[] and Tmoles are always positive or zero
       *    quantities.
       */
      double moles_j_base = v_totalMoles * Xmol_Base[j];
      deltaMoles_j = 1.0E-7 * moles_j_base + 1.0E-20 * v_totalMoles + 1.0E-150;
      /*
       * Now, update the total moles in the phase and all of the
       * mole fractions based on this.
       */
      v_totalMoles = TMoles_base + deltaMoles_j;      
      for (k = 0; k < m_numSpecies; k++) {
	Xmol[k] = Xmol_Base[k] * TMoles_base / v_totalMoles;
      }
      Xmol[j] = (moles_j_base + deltaMoles_j) / v_totalMoles;
 
      /*
       * Go get new values for the activity coefficients.
       * -> Note this calls setState_PX();
       */
      _updateMoleFractionDependencies();
      _updateActCoeff();
      /*
       * Calculate the column of the matrix
       */
      double * const lnActCoeffCol = dLnActCoeffdMolNumber[j];
      for (k = 0; k < m_numSpecies; k++) {
	lnActCoeffCol[k] = (ActCoeff[k] - ActCoeff_Base[k]) /
	  ((ActCoeff[k] + ActCoeff_Base[k]) * 0.5 * deltaMoles_j);
      }
      /*
       * Revert to the base case Xmol, v_totalMoles
       */
      v_totalMoles = TMoles_base;
      vcs_vdcopy(Xmol, Xmol_Base, m_numSpecies);
    }
    /*
     * Go get base values for the activity coefficients.
     * -> Note this calls setState_TPX() again;
     * -> Just wanted to make sure that cantera is in sync
     *    with VolPhase after this call.
     */
    setMoleFractions(VCS_DATA_PTR(Xmol_Base));
    _updateMoleFractionDependencies();
    _updateActCoeff();
  }
  /***************************************************************************/

  // Downloads the ln ActCoeff jacobian into the VCS version of the
  // ln ActCoeff jacobian.
  /*
   *
   *   This is essentially a scatter operation.
   *
   *   The Jacobians are actually d( lnActCoeff) / d (MolNumber);
   *   dLnActCoeffdMolNumber[j][k]
   * 
   *      j = id of the species mole number
   *      k = id of the species activity coefficient
   */
  void 
  vcs_VolPhase::sendToVCS_LnActCoeffJac(double * const * const LnACJac_VCS) {
    /*
     * update the Ln Act Coeff jacobian entries with respect to the
     * mole number of species in the phase -> we always assume that
     * they are out of date.
     */
    _updateLnActCoeffJac();

    /*
     *  Now copy over the values
     */
    int j, k, jglob, kglob;
    for (j = 0; j < m_numSpecies; j++) {
      jglob = IndSpecies[j];
      double * const lnACJacVCS_col = LnACJac_VCS[jglob];
      const double * const lnACJac_col = dLnActCoeffdMolNumber[j];
      for (k = 0; k < m_numSpecies; k++) {
	kglob = IndSpecies[k];
	lnACJacVCS_col[kglob] = lnACJac_col[k];
      }
    }
  }
  /***************************************************************************/

  // Set the pointer for Cantera's ThermoPhase parameter
  /*
   *  When we first initialize the ThermoPhase object, we read the
   *  state of the ThermoPhase into vcs_VolPhase object.
   *
   * @param tp_ptr Pointer to the ThermoPhase object corresponding
   *               to this phase.
   */
  void vcs_VolPhase::setPtrThermoPhase(Cantera::ThermoPhase *tp_ptr) {
    TP_ptr = tp_ptr;
    if (TP_ptr) {
      m_useCanteraCalls = true;
      Temp = TP_ptr->temperature();
      Pres = TP_ptr->pressure();
      setState_TP(Temp, Pres);
      p_VCS_UnitsFormat = VCS_UNITS_MKS;
      m_phi = TP_ptr->electricPotential();
      int nsp = TP_ptr->nSpecies();
      int nelem = TP_ptr->nElements();
      if (nsp !=  m_numSpecies) {
	if (m_numSpecies != 0) {
	  plogf("Warning Nsp != NVolSpeces: %d %d \n", nsp, m_numSpecies);
	}
	resize(VP_ID_, nsp, nelem, PhaseName.c_str());
      }
      TP_ptr->getMoleFractions(VCS_DATA_PTR(Xmol));
      fractionCreationDelta_ = Xmol;
      _updateMoleFractionDependencies();

      /*
       *  figure out ideal solution tag
       */
      if (nsp == 1) {
	m_isIdealSoln = true;
      } else {
	int eos = TP_ptr->eosType();
	switch (eos) {
	case Cantera::cIdealGas:
	case Cantera::cIncompressible:
	case Cantera::cSurf:
	case Cantera::cMetal:
	case Cantera::cStoichSubstance:
	case Cantera::cSemiconductor:
	case Cantera::cLatticeSolid:
	case Cantera::cLattice:
	case Cantera::cEdge:
	case Cantera::cIdealSolidSolnPhase:
	  m_isIdealSoln = true;
	  break;
	default:
	  m_isIdealSoln = false;
	};
      }
    } else {
      m_useCanteraCalls = false;
    }
  }
  /***************************************************************************/

  // Return a const ThermoPhase pointer corresponding to this phase
  /*
   *  @return pointer to the ThermoPhase.
   */
  const Cantera::ThermoPhase *vcs_VolPhase::ptrThermoPhase() const {
    return TP_ptr;
  }
  /***************************************************************************/

  double vcs_VolPhase::totalMoles() const {
    return v_totalMoles;
  }
  /***************************************************************************/

  double vcs_VolPhase::molefraction(int k) const {
    return Xmol[k];
  }
  /***************************************************************************/

  void vcs_VolPhase::setFractionCreationDeltas(const double * const F_k) {
    for (int k = 0; k < m_numSpecies; k++) {
      fractionCreationDelta_[k] = F_k[k];
    }
  }
  /***************************************************************************/

  const std::vector<double> & vcs_VolPhase::fractionCreationDeltas() const {
    return fractionCreationDelta_;
  }
  /***************************************************************************/

  // Sets the total moles in the phase
  /*   
   * We don't have to flag the internal state as changing here
   * because we have just changed the total moles.
   *
   *  @param totalMols   Total moles in the phase (kmol)
   */
  void vcs_VolPhase::setTotalMoles(const double totalMols)  {
    v_totalMoles = totalMols;
    if (m_totalMolesInert > 0.0) {
      m_existence = VCS_PHASE_EXIST_ALWAYS;
#ifdef DEBUG_MODE
      if (totalMols < m_totalMolesInert) {
      	printf(" vcs_VolPhase::setTotalMoles:: ERROR totalMoles "
	       "less than inert moles: %g %g\n", 
	       totalMols, m_totalMolesInert);
	exit(EXIT_FAILURE);
      }
#endif
    } else {
      if (m_singleSpecies && (m_phiVarIndex == 0)) {
	m_existence =  VCS_PHASE_EXIST_ALWAYS;
      } else {
      if (totalMols > 0.0) {
	m_existence = VCS_PHASE_EXIST_YES;
      } else {
	m_existence = VCS_PHASE_EXIST_NO;
      }
      }
    }
  }
  /***************************************************************************/

  // Sets the mole flag within the object to out of date
  /*
   *  This will trigger the object to go get the current mole numbers
   *  when it needs it.
   */
  void vcs_VolPhase::setMolesOutOfDate(int stateCalc) {
    m_UpToDate = false;
    if (stateCalc != -1) {
      m_vcsStateStatus = stateCalc;
    }
  }
  /***************************************************************************/

  // Sets the mole flag within the object to be current
  /*
   * 
   */
  void vcs_VolPhase::setMolesCurrent(int stateCalc) {
    m_UpToDate = true;
    m_vcsStateStatus = stateCalc;
  }
  /***************************************************************************/


  // Return a string representing the equation of state
  /* 
   * The string is no more than 16 characters. 
   *  @param EOSType : integer value of the equation of state
   *
   * @return returns a string representing the EOS
   */
  std::string string16_EOSType(int EOSType) {
    char st[32];
    st[16] = '\0';
    switch (EOSType) {
    case VCS_EOS_CONSTANT:
      sprintf(st,"Constant        ");
      break;
    case VCS_EOS_IDEAL_GAS:
      sprintf(st,"Ideal Gas       ");
      break;
    case  VCS_EOS_STOICH_SUB:
      sprintf(st,"Stoich Sub      ");
      break;
    case VCS_EOS_IDEAL_SOLN:
      sprintf(st,"Ideal Soln      ");
      break;
    case VCS_EOS_DEBEYE_HUCKEL:
      sprintf(st,"Debeye Huckel   ");
      break;
    case VCS_EOS_REDLICK_KWONG:
      sprintf(st,"Redlick_Kwong   ");
      break;
    case VCS_EOS_REGULAR_SOLN:
      sprintf(st,"Regular Soln    ");
      break;
    default:
      sprintf(st,"UnkType: %-7d", EOSType);
      break;
    }
    st[16] = '\0';
    std::string sss=st;
    return sss;
  }
  /***************************************************************************/

  // Returns whether the phase is an ideal solution phase
  bool vcs_VolPhase::isIdealSoln() const {
    return m_isIdealSoln;
  }
  /***************************************************************************/

  // Returns whether the phase uses Cantera calls
  bool vcs_VolPhase::usingCanteraCalls() const {
    return m_useCanteraCalls;
  }
  /***************************************************************************/

  int vcs_VolPhase::phiVarIndex() const {
    return m_phiVarIndex;
  }
  /***************************************************************************/


  void vcs_VolPhase::setPhiVarIndex(int phiVarIndex) {
    m_phiVarIndex = phiVarIndex;
    m_speciesUnknownType[m_phiVarIndex] = VCS_SPECIES_TYPE_INTERFACIALVOLTAGE;
    if (m_singleSpecies) {
      if (m_phiVarIndex == 0) {
	m_existence = VCS_PHASE_EXIST_ALWAYS;
      }
    }
  }
  /***************************************************************************/

  // Retrieve the kth Species structure for the species belonging to this phase
  /*
   * The index into this vector is the species index within the phase.
   *
   * @param kindex kth species index.
   */
  vcs_SpeciesProperties * vcs_VolPhase::speciesProperty(const int kindex) {
    return  ListSpeciesPtr[kindex];
  }
  /***************************************************************************/

  // Boolean indicating whether the phase exists or not
  int vcs_VolPhase::exists() const {
    return m_existence;
  }
 /**********************************************************************/

  // Set the existence flag in the object
  void vcs_VolPhase::setExistence(const int existence) {
    if (existence == VCS_PHASE_EXIST_NO || existence == VCS_PHASE_EXIST_ZEROEDPHASE) {
      if (v_totalMoles != 0.0) {
#ifdef DEBUG_MODE
	plogf("vcs_VolPhase::setExistence setting false existence for phase with moles");
	plogendl();
	exit(EXIT_FAILURE);
#else
	v_totalMoles = 0.0;
#endif
      }
    }
#ifdef DEBUG_MODE
    else { 
      if (m_totalMolesInert == 0.0) {
	if (v_totalMoles == 0.0) {
	  if (!m_singleSpecies  || m_phiVarIndex != 0) {
	    plogf("vcs_VolPhase::setExistence setting true existence for phase with no moles");
	    plogendl();
	    exit(EXIT_FAILURE);
	  }
	}
      }
    }
#endif
#ifdef DEBUG_MODE
    if (m_singleSpecies) {
      if (m_phiVarIndex == 0) {
	if (existence == VCS_PHASE_EXIST_NO || existence == VCS_PHASE_EXIST_ZEROEDPHASE) {
	  plogf("vcs_VolPhase::Trying to set existence of an electron phase to false");
	  plogendl();
	  exit(EXIT_FAILURE);
	}
      }
    }
#endif
    m_existence = existence;
  }
  /**********************************************************************/
  
  // Return the Global VCS index of the kth species in the phase
  /*
   *  @param spIndex local species index (0 to the number of species
   *                 in the phase)
   *
   * @return Returns the VCS_SOLVE species index of the that species
   *         This changes as rearrangements are carried out. 
   */
  int vcs_VolPhase::spGlobalIndexVCS(const int spIndex) const {
    return IndSpecies[spIndex];
  }
  /**********************************************************************/

  //! set the Global VCS index of the kth species in the phase
  /*!
   *  @param spIndex local species index (0 to the number of species
   *                 in the phase)
   *
   * @return Returns the VCS_SOLVE species index of the that species
   *         This changes as rearrangements are carried out. 
   */
  void vcs_VolPhase::setSpGlobalIndexVCS(const int spIndex, 
					 const int spGlobalIndex) {
    IndSpecies[spIndex] = spGlobalIndex;
  }
  /**********************************************************************/

  // Sets the total moles of inert in the phase
  /*
   * @param tMolesInert Value of the total kmols of inert species in the
   *        phase.
   */
  void vcs_VolPhase::setTotalMolesInert(const double tMolesInert) {
    if (m_totalMolesInert != tMolesInert) {
      m_UpToDate = false;
      m_UpToDate_AC = false;
      m_UpToDate_VolStar = false;
      m_UpToDate_VolPM = false;
      m_UpToDate_GStar = false;
      m_UpToDate_G0 = false;
      v_totalMoles += (tMolesInert - m_totalMolesInert);
      m_totalMolesInert = tMolesInert;
    }
    if (m_totalMolesInert > 0.0) {
      m_existence = VCS_PHASE_EXIST_ALWAYS;
    } else if (m_singleSpecies && (m_phiVarIndex == 0)) {
      m_existence = VCS_PHASE_EXIST_ALWAYS;
    } else {
      if (v_totalMoles > 0.0) {
	m_existence = VCS_PHASE_EXIST_YES;
      } else {
	m_existence = VCS_PHASE_EXIST_NO;
      }
    }
  }
  /**********************************************************************/

  // returns the value of the total kmol of inert in the phase
  double vcs_VolPhase::totalMolesInert() const {
    return m_totalMolesInert;
  }
  /**********************************************************************/

  // Returns the global index of the local element index for the phase
  int vcs_VolPhase::elemGlobalIndex(const int e) const {
    DebugAssertThrowVCS(e >= 0, " vcs_VolPhase::elemGlobalIndex") ;
    DebugAssertThrowVCS(e < m_numElemConstraints, " vcs_VolPhase::elemGlobalIndex") ;
    return m_elemGlobalIndex[e];
  }
  /**********************************************************************/

  // Returns the global index of the local element index for the phase
  void vcs_VolPhase::setElemGlobalIndex(const int eLocal, const int eGlobal) {
    DebugAssertThrowVCS(eLocal >= 0, "vcs_VolPhase::setElemGlobalIndex");
    DebugAssertThrowVCS(eLocal < m_numElemConstraints,
			"vcs_VolPhase::setElemGlobalIndex");
    m_elemGlobalIndex[eLocal] = eGlobal;
  }
  /**********************************************************************/
  
  int vcs_VolPhase::nElemConstraints() const {
    return m_numElemConstraints;
  }
  /**********************************************************************/
 
  std::string vcs_VolPhase::elementName(const int e) const {
    return m_elementNames[e];
  }
  /**********************************************************************/

  /*!
   *  This function decides whether a phase has charged species
   *  or not.
   */
  static bool hasChargedSpecies(const Cantera::ThermoPhase * const tPhase) {
    int nSpPhase = tPhase->nSpecies();
    for (int k = 0; k < nSpPhase; k++) {
      if (tPhase->charge(k) != 0.0) {
	return true;
      }
    }
    return false;
  }
  /**********************************************************************
   *
   * chargeNeutralityElement():
   *
   *  This utility routine decides whether a Cantera ThermoPhase needs
   *  a constraint equation representing the charge neutrality of the
   *  phase. It does this by searching for charged species. If it 
   *  finds one, and if the phase needs one, then it returns true.
   */
  static bool chargeNeutralityElement(const Cantera::ThermoPhase * const tPhase) {
    int hasCharge = hasChargedSpecies(tPhase);
    if (tPhase->chargeNeutralityNecessary()) {
      if (hasCharge) {
	return true;
      }
    }
    return false;
  }

  int vcs_VolPhase::transferElementsFM(const Cantera::ThermoPhase * const tPhase) {
    int e, k, eT;
    std::string ename; 
    int eFound = -2;
    /*
     *
     */
    int nebase = tPhase->nElements();
    int ne  = nebase;
    int ns = tPhase->nSpecies();

    /*
     * Decide whether we need an extra element constraint for charge
     * neutrality of the phase
     */
    bool cne = chargeNeutralityElement(tPhase);
    if (cne) {
      ChargeNeutralityElement = ne;
      ne++;
    }

    /*
     * Assign and malloc structures
     */
    elemResize(ne);


    if (ChargeNeutralityElement >= 0) {
      m_elementType[ChargeNeutralityElement] = VCS_ELEM_TYPE_CHARGENEUTRALITY;
    }

    if (hasChargedSpecies(tPhase)) {
      if (cne) {
	/*
	 * We need a charge neutrality constraint.
	 * We also have an Electron Element. These are
	 * duplicates of each other. To avoid trouble with
	 * possible range error conflicts, sometimes we eliminate
	 * the Electron condition. Flag that condition for elimination
	 * by toggling the ElActive variable. If we find we need it
	 * later, we will retoggle ElActive to true.
	 */
	for (eT = 0; eT < nebase; eT++) {
	  ename = tPhase->elementName(eT);
	  if (ename == "E") {
	    eFound = eT;
	    m_elementActive[eT] = 0;
	    m_elementType[eT] = VCS_ELEM_TYPE_ELECTRONCHARGE;
	  }
	}
      } else {
	for (eT = 0; eT < nebase; eT++) {
	  ename = tPhase->elementName(eT);
	  if (ename == "E") {
	    eFound = eT;
	    m_elementType[eT] = VCS_ELEM_TYPE_ELECTRONCHARGE;
	  }
	}
      }
      if (eFound == -2) {
	eFound = ne;
	m_elementType[ne] = VCS_ELEM_TYPE_ELECTRONCHARGE;
	m_elementActive[ne] = 0;
	std::string ename = "E";
	m_elementNames[ne] = ename;
	ne++;
	elemResize(ne);
      }

    }

    m_formulaMatrix.resize(ne, ns, 0.0);
    
    m_speciesUnknownType.resize(ns, VCS_SPECIES_TYPE_MOLNUM);
    
    elemResize(ne);
    
    e = 0;
    for (eT = 0; eT < nebase; eT++) {
      ename = tPhase->elementName(eT);
      m_elementNames[e] = ename;
      e++;
    }

    if (cne) {
      std::string pname = tPhase->id();
      if (pname == "") {
	char sss[50];
	sprintf(sss, "phase%d", VP_ID_);
	pname = sss;
      }
      ename = "cn_" + pname;
      e = ChargeNeutralityElement;
      m_elementNames[e] = ename;
    }
 
    double * const * const fm = m_formulaMatrix.baseDataAddr();
    for (k = 0; k < ns; k++) {
      e = 0;
      for (eT = 0; eT < nebase; eT++) {
	fm[e][k] = tPhase->nAtoms(k, eT);
	e++;
      }
      if (eFound >= 0) {
	fm[eFound][k] = - tPhase->charge(k);
      }
    }

    if (cne) {
      for (k = 0; k < ns; k++) {
	fm[ChargeNeutralityElement][k] = tPhase->charge(k);
      }
    }

 
    /*
     * Here, we figure out what is the species types are
     * The logic isn't set in stone, and is just for a particular type
     * of problem that I'm solving first.
     */
    if (ns == 1) {
      if (tPhase->charge(0) != 0.0) {
	m_speciesUnknownType[0] = VCS_SPECIES_TYPE_INTERFACIALVOLTAGE;
	setPhiVarIndex(0);
      }
    }

    return ne;
  }
  /***************************************************************************/

  // Type of the element constraint with index \c e.
  /*
   * @param e Element index.
   */
  int vcs_VolPhase::elementType(const int e) const {
    return m_elementType[e];
  }
  /***************************************************************************/

  // Set the element Type of the element constraint with index \c e.
  /*
   * @param e Element index
   * @param eType  type of the element.
   */
  void vcs_VolPhase::setElementType(const int e, const int eType) {
    m_elementType[e] = eType;
  }
  /***************************************************************************/

  double const * const * const vcs_VolPhase::getFormulaMatrix() const {
    double const * const * const fm = m_formulaMatrix.constBaseDataAddr();
    return fm;
  }
  /***************************************************************************/

  int vcs_VolPhase::speciesUnknownType(const int k) const {
    return m_speciesUnknownType[k];
  }
  /***************************************************************************/

  int vcs_VolPhase::elementActive(const int e) const {
    return m_elementActive[e];
  }
  /***************************************************************************/

  //! Return the number of species in the phase
  int vcs_VolPhase::nSpecies() const {
    return m_numSpecies;
  }
  /***************************************************************************/

}

