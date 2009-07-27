/*!
 * @file vcs_solve.h
 *    Header file for the internal class that holds the problem.
 */
/*
 * $Id: vcs_solve.cpp,v 1.60 2008/12/17 16:56:43 hkmoffa Exp $
 */
/*
 * Copywrite (2005) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */


#include "vcs_solve.h"
#include "vcs_Exception.h"
#include "vcs_internal.h"
#include "vcs_prob.h"

#include "vcs_VolPhase.h"
#include "vcs_SpeciesProperties.h"
#include "vcs_species_thermo.h"

#include "clockWC.h"

#include <string>
#include "math.h"
using namespace std;

namespace VCSnonideal {

  int vcs_timing_print_lvl = 1;

  VCS_SOLVE::VCS_SOLVE() :
    NSPECIES0(0),
    NPHASE0(0),
    m_numSpeciesTot(0),
    m_numElemConstraints(0),
    m_numComponents(0),
    m_numRxnTot(0),
    m_numSpeciesRdc(0),
    m_numRxnMinorZeroed(0),
    m_numPhases(0),
    m_doEstimateEquil(0),
    m_totalMolNum(0.0),
    m_temperature(0.0),
    m_pressurePA(0.0),
    m_tolmaj(0.0),
    m_tolmin(0.0),
    m_tolmaj2(0.0),
    m_tolmin2(0.0),
    m_unitsState(VCS_DIMENSIONAL_G),
    m_totalMoleScale(1.0),
    m_useActCoeffJac(0),
    m_totalVol(0.0),
    m_Faraday_dim(1.602e-19 * 6.022136736e26),
    m_VCount(0),
    m_debug_print_lvl(0),
    m_timing_print_lvl(1),
    m_VCS_UnitsFormat(VCS_UNITS_UNITLESS)
  {
  }

  //   Initialize the sizes within the VCS_SOLVE object

  /*
   *    This resizes all of the internal arrays within the object. This routine
   *    operates in two modes. If all of the parameters are the same as it
   *    currently exists in the object, nothing is done by this routine; a quick
   *    exit is carried out and all of the data in the object persists.
   *
   *    IF any of the parameters are different than currently exists in the
   *    object, then all of the data in the object must be redone. It may not
   *    be zeroed, but it must be redone.
   *
   *  @param nspecies0     Number of species within the object
   *  @param nelements     Number of element constraints within the problem
   *  @param nphase0       Number of phases defined within the problem.
   *
   */
  void VCS_SOLVE::vcs_initSizes(const int nspecies0, const int nelements, 
				const int nphase0) {

    if (NSPECIES0 != 0) {
      if ((nspecies0 != NSPECIES0) || (nelements != m_numElemConstraints) || (nphase0 != NPHASE0)){
	vcs_delete_memory();
      } else {
	return;
      }
    }

    NSPECIES0 = nspecies0;
    NPHASE0 = nphase0;
    m_numSpeciesTot = nspecies0;
    m_numElemConstraints = nelements;
    m_numComponents = nelements;

    int iph;
    string ser = "VCS_SOLVE: ERROR:\n\t";
    if (nspecies0 <= 0) {
      plogf("%s Number of species is nonpositive\n", ser.c_str());
      throw vcsError("VCS_SOLVE()", 
		     ser + " Number of species is nonpositive\n",
		     VCS_PUB_BAD);
    }
    if (nelements <= 0) {
      plogf("%s Number of elements is nonpositive\n", ser.c_str());
      throw vcsError("VCS_SOLVE()", 
		     ser + " Number of species is nonpositive\n",
		     VCS_PUB_BAD);
    }
    if (nphase0 <= 0) {
      plogf("%s Number of phases is nonpositive\n", ser.c_str());
      throw vcsError("VCS_SOLVE()", 
		     ser + " Number of species is nonpositive\n",
		     VCS_PUB_BAD);
    }

    //vcs_priv_init(this);
    m_VCS_UnitsFormat = VCS_UNITS_UNITLESS;
   
    /*
     * We will initialize sc[] to note the fact that it needs to be
     * filled with meaningful information.
     */
    m_stoichCoeffRxnMatrix.resize(nspecies0, nelements, 0.0);

    m_scSize.resize(nspecies0, 0.0);
    m_spSize.resize(nspecies0, 1.0);

    m_SSfeSpecies.resize(nspecies0, 0.0);
    m_feSpecies_new.resize(nspecies0, 0.0);
    m_molNumSpecies_old.resize(nspecies0, 0.0);

    m_speciesUnknownType.resize(nspecies0, VCS_SPECIES_TYPE_MOLNUM);

    m_deltaMolNumPhase.resize(nspecies0, nphase0, 0.0);
    m_phaseParticipation.resize(nspecies0, nphase0, 0);
    m_phasePhi.resize(nphase0, 0.0);

    m_molNumSpecies_new.resize(nspecies0, 0.0);

    m_deltaGRxn_new.resize(nspecies0, 0.0);
    m_deltaGRxn_old.resize(nspecies0, 0.0);
    m_deltaGRxn_Deficient.resize(nspecies0, 0.0);
    m_deltaGRxn_tmp.resize(nspecies0, 0.0);
    m_deltaMolNumSpecies.resize(nspecies0, 0.0);

    m_feSpecies_old.resize(nspecies0, 0.0);
    m_elemAbundances.resize(nelements, 0.0);
    m_elemAbundancesGoal.resize(nelements, 0.0);

    m_tPhaseMoles_old.resize(nphase0, 0.0);
    m_tPhaseMoles_new.resize(nphase0, 0.0);
    m_deltaPhaseMoles.resize(nphase0, 0.0);
    m_TmpPhase.resize(nphase0, 0.0);
    m_TmpPhase2.resize(nphase0, 0.0);
  
    m_formulaMatrix.resize(nelements, nspecies0);

    TPhInertMoles.resize(nphase0, 0.0);

    /*
     * ind[] is an index variable that keep track of solution vector
     * rotations.
     */
    m_speciesMapIndex.resize(nspecies0, 0);
    m_speciesLocalPhaseIndex.resize(nspecies0, 0);
    /*
     *    IndEl[] is an index variable that keep track of element vector
     *    rotations.
     */
    m_elementMapIndex.resize(nelements, 0);

    /*
     *   ir[] is an index vector that keeps track of the irxn to species
     *   mapping. We can't fill it in until we know the number of c
     *   components in the problem
     */
    m_indexRxnToSpecies.resize(nspecies0, 0);
   
    /* Initialize all species to be major species */
    //m_rxnStatus.resize(nspecies0, 1);
    m_speciesStatus.resize(nspecies0, 1);
   
    m_SSPhase.resize(2*nspecies0, 0);
    m_phaseID.resize(nspecies0, 0);

    m_numElemConstraints  = nelements;

    m_elementName.resize(nelements, std::string(""));
    m_speciesName.resize(nspecies0, std::string(""));

    m_elType.resize(nelements, VCS_ELEM_TYPE_ABSPOS);

    m_elementActive.resize(nelements,  1);
    /*
     *    Malloc space for activity coefficients for all species
     *    -> Set it equal to one.
     */
    m_actConventionSpecies.resize(nspecies0, 0);
    m_phaseActConvention.resize(nphase0, 0);
    m_lnMnaughtSpecies.resize(nspecies0, 0.0);
    m_actCoeffSpecies_new.resize(nspecies0, 1.0);
    m_actCoeffSpecies_old.resize(nspecies0, 1.0);
    m_wtSpecies.resize(nspecies0, 0.0);
    m_chargeSpecies.resize(nspecies0, 0.0);
    m_speciesThermoList.resize(nspecies0, (VCS_SPECIES_THERMO *)0);
   
    /*
     *    Malloc Phase Info
     */
    m_VolPhaseList.resize(nphase0, 0);
    for (iph = 0; iph < nphase0; iph++) {
      m_VolPhaseList[iph] = new vcs_VolPhase(this);
    }   
   
    /*
     *        For Future expansion
     */
    m_useActCoeffJac = true;
    if (m_useActCoeffJac) {
      m_dLnActCoeffdMolNum.resize(nspecies0, nspecies0, 0.0);
    }
   
    m_PMVolumeSpecies.resize(nspecies0, 0.0);
   
    /*
     *        Malloc space for counters kept within vcs
     *
     */
    m_VCount    = new VCS_COUNTERS();
    vcs_counters_init(1);

    if (vcs_timing_print_lvl == 0) {
      m_timing_print_lvl = 0;
    }

    return;

  }
  /****************************************************************************/

  // Destructor
  /*
   *
   */
  VCS_SOLVE::~VCS_SOLVE()
  {
    vcs_delete_memory();
  }
  /*****************************************************************************/

  // Delete memory that isn't just resizeable STL containers
  /*
   * This gets called by the destructor or by InitSizes().
   */
  void VCS_SOLVE::vcs_delete_memory() {
    int j;
    int nspecies = m_numSpeciesTot;
   
    for (j = 0; j < m_numPhases; j++) {
      delete m_VolPhaseList[j];
      m_VolPhaseList[j] = 0;
    }

    for (j = 0; j < nspecies; j++) {
      delete m_speciesThermoList[j];
      m_speciesThermoList[j] = 0;
    } 
   
    delete m_VCount; 
    m_VCount = 0;

    NSPECIES0 = 0;
    NPHASE0 = 0;
    m_numElemConstraints = 0;
    m_numComponents = 0;
  }
  /*****************************************************************************/

  // Solve an equilibrium problem
  /*
   *  This is the main interface routine to the equilibrium solver
   *
   * Input:
   *   @param vprob Object containing the equilibrium Problem statement
   *   
   *   @param ifunc Determines the operation to be done: Valid values:
   *            0 -> Solve a new problem by initializing structures
   *                 first. An initial estimate may or may not have
   *                 been already determined. This is indicated in the
   *                 VCS_PROB structure.
   *            1 -> The problem has already been initialized and 
   *                 set up. We call this routine to resolve it 
   *                 using the problem statement and
   *                 solution estimate contained in 
   *                 the VCS_PROB structure.
   *            2 -> Don't solve a problem. Destroy all the private 
   *                 structures.
   *
   *  @param ipr Printing of results
   *     ipr = 1 -> Print problem statement and final results to 
   *                standard output 
   *           0 -> don't report on anything 
   *  @param ip1 Printing of intermediate results
   *     ip1 = 1 -> Print intermediate results. 
   *         = 0 -> No intermediate results printing
   *
   *  @param maxit  Maximum number of iterations for the algorithm 
   *
   * Output:
   *
   *    @return
   *       nonzero value: failure to solve the problem at hand.
   *       zero : success
   */
  int VCS_SOLVE::vcs(VCS_PROB *vprob, int ifunc, int ipr, int ip1, int maxit) {
    int retn = 0;
    int iconv = 0, nspecies0, nelements0, nphase0;
    Cantera::clockWC tickTock;
    
    int  iprintTime = MAX(ipr, ip1);
    if (m_timing_print_lvl < iprintTime) {
      iprintTime = m_timing_print_lvl ;
    }
    
    if (ifunc < 0 || ifunc > 2) {
      plogf("vcs: Unrecognized value of ifunc, %d: bailing!\n",
	    ifunc);
      return VCS_PUB_BAD;
    }
  
    if (ifunc == 0) {
      /*
       *         This function is called to create the private data
       *         using the public data.
       */
      nspecies0 = vprob->nspecies + 10;
      nelements0 = vprob->ne;
      nphase0 = vprob->NPhase;
    
      vcs_initSizes(nspecies0, nelements0, nphase0);
     
      if (retn != 0) {
	plogf("vcs_priv_alloc returned a bad status, %d: bailing!\n",
	      retn);
	return retn;
      }
      /*
       *         This function is called to copy the public data
       *         and the current problem specification
       *         into the current object's data structure.
       */
      retn = vcs_prob_specifyFully(vprob);
      if (retn != 0) {
	plogf("vcs_pub_to_priv returned a bad status, %d: bailing!\n",
	      retn);
	return retn;
      }
      /*
       *        Prep the problem data
       *           - adjust the identity of any phases
       *           - determine the number of components in the problem
       */
      retn = vcs_prep_oneTime(ip1);
      if (retn != 0) {
	plogf("vcs_prep_oneTime returned a bad status, %d: bailing!\n",
	      retn);
	return retn;
      }      
    }
    if (ifunc == 1) {
      /*
       *         This function is called to copy the current problem
       *         into the current object's data structure.
       */
      retn = vcs_prob_specify(vprob);
      if (retn != 0) {
	plogf("vcs_prob_specify returned a bad status, %d: bailing!\n",
	      retn);
	return retn;
      }
    }
    if (ifunc != 2) {
      /*
       *        Prep the problem data for this particular instantiation of
       *        the problem
       */
      retn = vcs_prep();
      if (retn != VCS_SUCCESS) {
	plogf("vcs_prep returned a bad status, %d: bailing!\n", retn);
	return retn;
      }

      /*
       *        Check to see if the current problem is well posed.
       */
      if (!vcs_wellPosed(vprob)) {
	plogf("vcs has determined the problem is not well posed: Bailing\n");
	return VCS_PUB_BAD;
      }
       
      /*
       *      Once we have defined the global internal data structure defining
       *      the problem, then we go ahead and solve the problem.
       *
       *   (right now, all we do is solve fixed T, P problems.
       *    Methods for other problem types will go in at this level.
       *    For example, solving for fixed T, V problems will involve
       *    a 2x2 Newton's method, using loops over vcs_TP() to
       *    calculate the residual and Jacobian)
       */
    
      iconv = vcs_TP(ipr, ip1, maxit, vprob->T, vprob->PresPA);

	     
      /*
       *        If requested to print anything out, go ahead and do so;
       */
      if (ipr > 0) vcs_report(iconv);
      /*
       *        Copy the results of the run back to the VCS_PROB structure,
       *        which is returned to the user.
       */
      vcs_prob_update(vprob);
    }

    /*
     * Report on the time if requested to do so
     */
    double te = tickTock.secondsWC();
    m_VCount->T_Time_vcs += te;
    if (iprintTime > 0) {
      vcs_TCounters_report(m_timing_print_lvl);
    }
    /*
     *        Now, destroy the private data, if requested to do so
     *
     * FILL IN
     */
   
    if (iconv < 0) {
      plogf("ERROR: FAILURE its = %d!\n", m_VCount->Its);
    } else if (iconv == 1) {
      plogf("WARNING: RANGE SPACE ERROR encountered\n");
    }
    return iconv;
  }
  /*****************************************************************************/

  // Fully specify the problem to be solved using VCS_PROB
  /*
   *  Use the contents of the VCS_PROB to specify the contents of the
   *  private data, VCS_SOLVE.
   *
   *  @param pub  Pointer to VCS_PROB that will be used to
   *              initialize the current equilibrium problem
   */
  int VCS_SOLVE::vcs_prob_specifyFully(const VCS_PROB *pub) {
    int i, j,  kspec;
    int iph;
    vcs_VolPhase *Vphase = 0;
    const char *ser =
      "vcs_pub_to_priv ERROR :ill defined interface -> bailout:\n\t";

    /*
     *  First Check to see whether we have room for the current problem
     *  size
     */
    int nspecies  = pub->nspecies;
    if (NSPECIES0 < nspecies) {
      plogf("%sPrivate Data is dimensioned too small\n", ser);
      return VCS_PUB_BAD;
    }
    int nph = pub->NPhase;
    if (NPHASE0 < nph) {
      plogf("%sPrivate Data is dimensioned too small\n", ser);
      return VCS_PUB_BAD;
    }
    int nelements = pub->ne;
    if (m_numElemConstraints < nelements) {
      plogf("%sPrivate Data is dimensioned too small\n", ser);
      return VCS_PUB_BAD;
    }

    /*
     * OK, We have room. Now, transfer the integer numbers
     */
    m_numElemConstraints     = nelements;
    m_numSpeciesTot = nspecies;
    m_numSpeciesRdc = m_numSpeciesTot;
    /*
     *  nc = number of components -> will be determined later. 
     *       but set it to its maximum possible value here.
     */
    m_numComponents     = nelements;
    /*
     *   m_numRxnTot = number of noncomponents, also equal to the
     *                 number of reactions
     */
    m_numRxnTot = MAX(nspecies - nelements, 0);
    m_numRxnRdc = m_numRxnTot;
    /*
     *  number of minor species rxn -> all species rxn are major at the start.
     */
    m_numRxnMinorZeroed = 0;
    /*
     *  NPhase = number of phases
     */
    m_numPhases = nph;

#ifdef DEBUG_MODE
    m_debug_print_lvl = pub->vcs_debug_print_lvl;
#else
    m_debug_print_lvl = MIN(2, pub->vcs_debug_print_lvl);
#endif

    /*
     * FormulaMatrix[] -> Copy the formula matrix over
     */
    for (i = 0; i < nspecies; i++) {
      for (j = 0; j < nelements; j++) {
	m_formulaMatrix[j][i] = pub->FormulaMatrix[j][i];
      }
    }
  
    /*
     *  Copy over the species molecular weights
     */
    vcs_vdcopy(m_wtSpecies, pub->WtSpecies, nspecies);

    /*
     * Copy over the charges
     */
    vcs_vdcopy(m_chargeSpecies, pub->Charge, nspecies);

    /*
     * Malloc and Copy the VCS_SPECIES_THERMO structures
     * 
     */
    for (kspec = 0; kspec < nspecies; kspec++) {
      if (m_speciesThermoList[kspec] != NULL) {
	delete m_speciesThermoList[kspec];
      }
      VCS_SPECIES_THERMO *spf = pub->SpeciesThermo[kspec];
      m_speciesThermoList[kspec] = spf->duplMyselfAsVCS_SPECIES_THERMO();
      if (m_speciesThermoList[kspec] == NULL) {
	plogf(" duplMyselfAsVCS_SPECIES_THERMO returned an error!\n");
	return VCS_PUB_BAD;
      }
    }

    /*
     * Copy the species unknown type
     */
    vcs_icopy(VCS_DATA_PTR(m_speciesUnknownType), 
	      VCS_DATA_PTR(pub->SpeciesUnknownType), nspecies);

    /*
     *  iest => Do we have an initial estimate of the species mole numbers ?
     */
    m_doEstimateEquil = pub->iest;

    /*
     * w[] -> Copy the equilibrium mole number estimate if it exists.
     */
    if (pub->w.size() != 0) {
      vcs_vdcopy(m_molNumSpecies_old, pub->w, nspecies);
    } else {
      m_doEstimateEquil = -1;
      vcs_dzero(VCS_DATA_PTR(m_molNumSpecies_old), nspecies);
    }

    /*
     * Formulate the Goal Element Abundance Vector
     */
    if (pub->gai.size() != 0) {
      for (i = 0; i < nelements; i++) m_elemAbundancesGoal[i] = pub->gai[i];
    } else {
      if (m_doEstimateEquil == 0) {
	for (j = 0; j < nelements; j++) {
	  m_elemAbundancesGoal[j] = 0.0;
	  for (kspec = 0; kspec < nspecies; kspec++) {
	    if (m_speciesUnknownType[kspec] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	      m_elemAbundancesGoal[j] += m_formulaMatrix[j][kspec] * m_molNumSpecies_old[kspec];
	    }
	  }
	}
      } else {
	plogf("%sElement Abundances, m_elemAbundancesGoal[], not specified\n", ser);
	return VCS_PUB_BAD;
      }
    }

    /*
     * zero out values that will be filled in later
     */
    /*
     * TPhMoles[]      -> Untouched here. These will be filled in vcs_prep.c
     * TPhMoles1[]
     * DelTPhMoles[]
     *
     *
     * T, Pres, copy over here
     */
    if (pub->T  > 0.0)  m_temperature = pub->T;
    else                m_temperature = 293.15;
    if (pub->PresPA > 0.0) m_pressurePA = pub->PresPA;
    else                   m_pressurePA = Cantera::OneAtm;
    /*
     *   TPhInertMoles[] -> must be copied over here
     */
    for (iph = 0; iph < nph; iph++) {
      Vphase = pub->VPhaseList[iph];
      TPhInertMoles[iph] = Vphase->totalMolesInert();
    }

    /*
     *  if__ : Copy over the units for the chemical potential
     */
    m_VCS_UnitsFormat = pub->m_VCS_UnitsFormat;

    /*
     *      tolerance requirements -> copy them over here and later
     */
    m_tolmaj = pub->tolmaj;
    m_tolmin = pub->tolmin;
    m_tolmaj2 = 0.01 * m_tolmaj;
    m_tolmin2 = 0.01 * m_tolmin;

    /*
     * m_speciesIndexVector[] is an index variable that keep track 
     * of solution vector rotations.
     */
    for (i = 0; i < nspecies; i++) {
      m_speciesMapIndex[i] = i;
    }

    /*
     *   IndEl[] is an index variable that keep track of element vector
     *   rotations.
     */
    for (i = 0; i < nelements; i++)   m_elementMapIndex[i] = i;

    /*
     *  Define all species to be major species, initially.
     */
    for (i = 0; i < nspecies; i++) {
      // m_rxnStatus[i] = VCS_SPECIES_MAJOR;
      m_speciesStatus[i] = VCS_SPECIES_MAJOR;
    }
    /*
     * PhaseID: Fill in the species to phase mapping
     *             -> Check for bad values at the same time.
     */
    if (pub->PhaseID.size() != 0) {
      std::vector<int> numPhSp(nph, 0);
      for (kspec = 0; kspec < nspecies; kspec++) {
	iph = pub->PhaseID[kspec];
	if (iph < 0 || iph >= nph) {
	  plogf("%sSpecies to Phase Mapping, PhaseID, has a bad value\n",
		ser);
	  plogf("\tPhaseID[%d] = %d\n", kspec, iph);
	  plogf("\tAllowed values: 0 to %d\n", nph - 1);
	  return VCS_PUB_BAD;	    
	}
	m_phaseID[kspec] = pub->PhaseID[kspec];
	m_speciesLocalPhaseIndex[kspec] = numPhSp[iph];
	numPhSp[iph]++;
      }
      for (iph = 0; iph < nph; iph++) {
	Vphase = pub->VPhaseList[iph];
	if (numPhSp[iph] != Vphase->nSpecies()) {
	  plogf("%sNumber of species in phase %d, %s, doesn't match\n",
		ser, iph, Vphase->PhaseName.c_str());
	  return VCS_PUB_BAD;
	}
      }
    } else {
      if (m_numPhases == 1) {
	for (kspec = 0; kspec < nspecies; kspec++) {
	  m_phaseID[kspec] = 0;
	  m_speciesLocalPhaseIndex[kspec] = kspec;
	}
      } else {
	plogf("%sSpecies to Phase Mapping, PhaseID, is not defined\n", ser);
	return VCS_PUB_BAD;	 
      }
    }

    /*
     *  Copy over the element types
     */
    m_elType.resize(nelements, VCS_ELEM_TYPE_ABSPOS);
    m_elementActive.resize(nelements, 1);
  
    /*
     *      Copy over the element names and types
     */
    for (i = 0; i < nelements; i++) {
      m_elementName[i] = pub->ElName[i];
      m_elType[i] = pub->m_elType[i];
      m_elementActive[i] = pub->ElActive[i];
      if (!strncmp(m_elementName[i].c_str(), "cn_", 3)) {
	m_elType[i] = VCS_ELEM_TYPE_CHARGENEUTRALITY;
	if (pub->m_elType[i] != VCS_ELEM_TYPE_CHARGENEUTRALITY) {
	  plogf("we have an inconsistency!\n");
	  exit(EXIT_FAILURE);
	}
      }
    }

    for (i = 0; i < nelements; i++) {
      if (m_elType[i] ==  VCS_ELEM_TYPE_CHARGENEUTRALITY) {
	if (m_elemAbundancesGoal[i] != 0.0) {
	  if (fabs(m_elemAbundancesGoal[i]) > 1.0E-9) {
	    plogf("Charge neutrality condition %s is signicantly nonzero, %g. Giving up\n",
		  m_elementName[i].c_str(), m_elemAbundancesGoal[i]);
	    exit(EXIT_FAILURE);
	  } else {
	    plogf("Charge neutrality condition %s not zero, %g. Setting it zero\n",
		  m_elementName[i].c_str(), m_elemAbundancesGoal[i]);
	    m_elemAbundancesGoal[i] = 0.0;
	  }
	  
	}
      }
    }
 
    /*
     *      Copy over the species names
     */
    for (i = 0; i < nspecies; i++) {
      m_speciesName[i] = pub->SpName[i];
    }
    /*
     *  Copy over all of the phase information
     *  Use the object's assignment operator
     */
    for (iph = 0; iph < nph; iph++) {
      *(m_VolPhaseList[iph]) = *(pub->VPhaseList[iph]);
      /*
       * Fix up the species thermo pointer in the vcs_SpeciesThermo object
       * It should point to the species thermo pointer in the private
       * data space.
       */
      Vphase = m_VolPhaseList[iph];
      for (int k = 0; k < Vphase->nSpecies(); k++) {
	vcs_SpeciesProperties *sProp = Vphase->speciesProperty(k);
	int kT = Vphase->spGlobalIndexVCS(k);
	sProp->SpeciesThermo = m_speciesThermoList[kT];
      }
    }

    /*
     * Specify the Activity Convention information
     */
    for (iph = 0; iph < nph; iph++) {
      Vphase = m_VolPhaseList[iph];
      m_phaseActConvention[iph] = Vphase->p_activityConvention;
      if (Vphase->p_activityConvention != 0) {
	/*
	 * We assume here that species 0 is the solvent.
	 * The solvent isn't on a unity activity basis
	 * The activity for the solvent assumes that the 
	 * it goes to one as the species mole fraction goes to
	 * one; i.e., it's really on a molarity framework.
	 * So SpecLnMnaught[iSolvent] = 0.0, and the
	 * loop below starts at 1, not 0.
	 */
	int iSolvent = Vphase->spGlobalIndexVCS(0);
	double mnaught = m_wtSpecies[iSolvent] / 1000.;
	for (int k = 1; k < Vphase->nSpecies(); k++) {
	  int kspec = Vphase->spGlobalIndexVCS(k);
	  m_actConventionSpecies[kspec] = Vphase->p_activityConvention;
	  m_lnMnaughtSpecies[kspec] = log(mnaught);
	}
      }
    }  
   
    /*
     *      Copy the title info
     */
    if (pub->Title.size() == 0) {
      m_title = "Unspecified Problem Title";
    } else {
      m_title = pub->Title;
    }

    /*
     *  Copy the volume info
     */
    m_totalVol = pub->Vol; 
    if (m_PMVolumeSpecies.size() != 0) {
      vcs_dcopy(VCS_DATA_PTR(m_PMVolumeSpecies), VCS_DATA_PTR(pub->VolPM), nspecies);
    }
   
    /*
     *      Return the success flag
     */
    return VCS_SUCCESS;
  }
  /*****************************************************************************/

  //   Specify the problem to be solved using VCS_PROB, incrementally
  /*
   *  Use the contents of the VCS_PROB to specify the contents of the
   *  private data, VCS_SOLVE.
   *
   *  It's assumed we are solving the same problem.
   *
   *  @param pub  Pointer to VCS_PROdB that will be used to
   *              initialize the current equilibrium problem
   */
  int VCS_SOLVE::vcs_prob_specify(const VCS_PROB *pub) {
    int kspec, k, i, j, iph;
    string yo("vcs_prob_specify ERROR: ");
    int retn = VCS_SUCCESS;
    bool status_change = false;

    m_temperature = pub->T;
    m_pressurePA = pub->PresPA;
    m_VCS_UnitsFormat = pub->m_VCS_UnitsFormat;
    m_doEstimateEquil = pub->iest;

    m_totalVol = pub->Vol;

    m_tolmaj = pub->tolmaj;
    m_tolmin = pub->tolmin;
    m_tolmaj2 = 0.01 * m_tolmaj;
    m_tolmin2 = 0.01 * m_tolmin;

    for (kspec = 0; kspec < m_numSpeciesTot; ++kspec) {
      k = m_speciesMapIndex[kspec];
      m_molNumSpecies_old[kspec] = pub->w[k];
      m_molNumSpecies_new[kspec] = pub->mf[k];
      m_feSpecies_old[kspec] = pub->m_gibbsSpecies[k];
    }

    /*
     * Transfer the element abundance goals to the solve object
     */
    for (i = 0; i < m_numElemConstraints; i++) {
      j = m_elementMapIndex[i];
      m_elemAbundancesGoal[i] = pub->gai[j];
    }

    /*
     *  Try to do the best job at guessing at the title
     */
    if (pub->Title.size() == 0) {
      if (m_title.size() == 0) {
	m_title = "Unspecified Problem Title";
      }
    } else {
      m_title = pub->Title;
    }

    /*
     *   Copy over the phase information.
     *      -> For each entry in the phase structure, determine
     *         if that entry can change from its initial value
     *         Either copy over the new value or create an error
     *         condition.
     */

    for (iph = 0; iph < m_numPhases; iph++) {
      vcs_VolPhase *vPhase = m_VolPhaseList[iph];
      vcs_VolPhase *pub_phase_ptr = pub->VPhaseList[iph];

      if  (vPhase->VP_ID != pub_phase_ptr->VP_ID) {
	plogf("%sPhase numbers have changed:%d %d\n", yo.c_str(),
	      vPhase->VP_ID, pub_phase_ptr->VP_ID);
	retn = VCS_PUB_BAD;
      }

      if  (vPhase->m_singleSpecies != pub_phase_ptr->m_singleSpecies) {
	plogf("%sSingleSpecies value have changed:%d %d\n", yo.c_str(),
	      vPhase->m_singleSpecies,
	      pub_phase_ptr->m_singleSpecies);
	retn = VCS_PUB_BAD;
      }

      if  (vPhase->m_gasPhase != pub_phase_ptr->m_gasPhase) {
	plogf("%sGasPhase value have changed:%d %d\n", yo.c_str(),
	      vPhase->m_gasPhase,
	      pub_phase_ptr->m_gasPhase);
	retn = VCS_PUB_BAD;
      }

      vPhase->m_eqnState = pub_phase_ptr->m_eqnState;

      if  (vPhase->nSpecies() != pub_phase_ptr->nSpecies()) {
	plogf("%sNVolSpecies value have changed:%d %d\n", yo.c_str(),
	      vPhase->nSpecies(),
	      pub_phase_ptr->nSpecies());
	retn = VCS_PUB_BAD;
      }

      if (vPhase->PhaseName == pub_phase_ptr->PhaseName) {
	plogf("%sPhaseName value have changed:%s %s\n", yo.c_str(),
	      vPhase->PhaseName.c_str(),
	      pub_phase_ptr->PhaseName.c_str());
	retn = VCS_PUB_BAD;
      }

      if (vPhase->totalMolesInert() != pub_phase_ptr->totalMolesInert()) {
	status_change = true;
      }
      /*
       * Copy over the number of inert moles if it has changed.
       */
      TPhInertMoles[iph] = pub_phase_ptr->totalMolesInert();
      vPhase->setTotalMolesInert(pub_phase_ptr->totalMolesInert());
      if (TPhInertMoles[iph] > 0.0) {
	vPhase->setExistence(2);
	vPhase->m_singleSpecies = FALSE;
      }

      /*
       * Copy over the interfacial potential
       */
      double phi = pub_phase_ptr->electricPotential();
      vPhase->setElectricPotential(phi);
    }
 

    if (status_change) vcs_SSPhase();
    /*
     *   Calculate the total number of moles in all phases.
     */
    vcs_tmoles();

    return retn;
  }
  /*****************************************************************************/

  // Transfer the results of the equilibrium calculation back to VCS_PROB
  /*
   *   The VCS_PUB structure is  returned to the user.
   *
   *  @param pub  Pointer to VCS_PROB that will get the results of the
   *              equilibrium calculation transfered to it. 
   */
  int VCS_SOLVE::vcs_prob_update(VCS_PROB *pub) {
    int i, j, l;
    int k1 = 0;

    vcs_tmoles();
    m_totalVol = vcs_VolTotal(m_temperature, m_pressurePA, 
			      VCS_DATA_PTR(m_molNumSpecies_old), VCS_DATA_PTR(m_PMVolumeSpecies));

    for (i = 0; i < m_numSpeciesTot; ++i) {
      /*
       *         Find the index of I in the index vector, m_speciesIndexVector[]. 
       *         Call it K1 and continue. 
       */
      for (j = 0; j < m_numSpeciesTot; ++j) {
	l = m_speciesMapIndex[j];
	k1 = j;
	if (l == i) break;
      }
      /*
       * - Switch the species data back from K1 into I
       */
      if (pub->SpeciesUnknownType[i] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	pub->w[i] = m_molNumSpecies_old[k1];
      } else {
	pub->w[i] = 0.0;
	plogf("voltage species = %g\n", m_molNumSpecies_old[k1]);
      }
      //pub->mf[i] = m_molNumSpecies_new[k1];
      pub->m_gibbsSpecies[i] = m_feSpecies_old[k1];
      pub->VolPM[i] = m_PMVolumeSpecies[k1];
    } 
   
    pub->T    = m_temperature;
    pub->PresPA = m_pressurePA;
    pub->Vol  = m_totalVol;
    int kT = 0;
    for (int iph = 0; iph < pub->NPhase; iph++) {
      vcs_VolPhase *pubPhase = pub->VPhaseList[iph];
      vcs_VolPhase *vPhase = m_VolPhaseList[iph];
      pubPhase->setTotalMolesInert(vPhase->totalMolesInert());
      pubPhase->setTotalMoles(vPhase->totalMoles());
      pubPhase->setElectricPotential(vPhase->electricPotential());
      double sumMoles = pubPhase->totalMolesInert();
      pubPhase->setMoleFractionsState(vPhase->totalMoles(),
				      VCS_DATA_PTR(vPhase->moleFractions()),
				      VCS_STATECALC_TMP);
      const std::vector<double> & mfVector = pubPhase->moleFractions();
      for (int k = 0; k < pubPhase->nSpecies(); k++) {
	kT = pubPhase->spGlobalIndexVCS(k);
	pub->mf[kT] = mfVector[k];
	if (pubPhase->phiVarIndex() == k) {
	  k1 = vPhase->spGlobalIndexVCS(k);
	  double tmp = m_molNumSpecies_old[k1];
	  if (! vcs_doubleEqual( 	pubPhase->electricPotential() , tmp)) { 
	    plogf("We have an inconsistency in voltage, %g, %g\n",
		  pubPhase->electricPotential(), tmp);
	    exit(EXIT_FAILURE);
	  }
	}

	if (! vcs_doubleEqual( pub->mf[kT], vPhase->molefraction(k))) {
	  plogf("We have an inconsistency in mole fraction, %g, %g\n",
		pub->mf[kT], vPhase->molefraction(k));
	  exit(EXIT_FAILURE);
	}
	if (pubPhase->speciesUnknownType(k) != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	  sumMoles +=  pub->w[kT];
	}
      }
      if (! vcs_doubleEqual(sumMoles, vPhase->totalMoles())) {
      	plogf("We have an inconsistency in total moles, %g %g\n",
	      sumMoles, pubPhase->totalMoles());
	exit(EXIT_FAILURE);
      }

    }

    pub->m_Iterations            = m_VCount->Its;
    pub->m_NumBasisOptimizations = m_VCount->Basis_Opts;
   
    return VCS_SUCCESS;
  }
  /*****************************************************************************/

  // Initialize the internal counters
  /*
   * Initialize the internal counters containing the subroutine call
   * values and times spent in the subroutines.
   *
   *  ifunc = 0     Initialize only those counters appropriate for the top of
   *                vcs_solve_TP().
   *        = 1     Initialize all counters.
   */
  void VCS_SOLVE::vcs_counters_init(int ifunc) {
    m_VCount->Its = 0;
    m_VCount->Basis_Opts = 0;
    m_VCount->Time_vcs_TP = 0.0;
    m_VCount->Time_basopt = 0.0;
    if (ifunc) {
      m_VCount->T_Its = 0;
      m_VCount->T_Basis_Opts = 0;
      m_VCount->T_Calls_Inest = 0;
      m_VCount->T_Calls_vcs_TP = 0;
      m_VCount->T_Time_vcs_TP = 0.0;
      m_VCount->T_Time_basopt = 0.0;
      m_VCount->T_Time_inest = 0.0;
      m_VCount->T_Time_vcs = 0.0;
    }
  }
  /**************************************************************************/

  //    Calculation of the total volume and the partial molar volumes
  /*
   *  This function calculates the partial molar volume
   *  for all species, kspec, in the thermo problem
   *  at the temperature TKelvin and pressure, Pres, pres is in atm.
   *  And, it calculates the total volume of the combined system.
   *
   * Input
   * ---------------
   *    @param tkelvin   Temperature in kelvin()
   *    @param pres      Pressure in Pascal
   *    @param w         w[] is thevector containing the current mole numbers
   *                     in units of kmol.
   *
   * Output
   * ----------------
   *    @param volPM[]    For species in all phase, the entries are the
   *                      partial molar volumes units of M**3 / kmol.
   *
   *    @return           The return value is the total volume of 
   *                      the entire system in units of m**3.
   */
  double VCS_SOLVE::vcs_VolTotal(const double tkelvin, const double pres, 
				 const double w[], double volPM[]) {
    double VolTot = 0.0;
    for (int iphase = 0; iphase < m_numPhases; iphase++) {
      vcs_VolPhase *Vphase = m_VolPhaseList[iphase];
      Vphase->setState_TP(tkelvin, pres);
      Vphase->setMolesFromVCS(VCS_STATECALC_OLD, w);
      double Volp = Vphase->sendToVCS_VolPM(volPM);
      VolTot += Volp;
    }
    return VolTot;
  }
  /****************************************************************************/


}
