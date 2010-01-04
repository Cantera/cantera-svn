/**
 *  @file vcs_prep.cpp
 *    This file contains some prepatory functions.
 */

/* $RCSfile: vcs_prep.cpp,v $ */
/* $Author$ */
/* $Date$ */
/* $Revision$ */
/*
 * Copywrite (2005) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */

#include "vcs_solve.h"
#include "vcs_internal.h"
#include "vcs_prob.h"
#include "vcs_VolPhase.h"
#include "vcs_SpeciesProperties.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>

namespace VCSnonideal { 

  
  //  Calculate the status of single species phases.
  void VCS_SOLVE::vcs_SSPhase() {
    int kspec, iph;
    vcs_VolPhase *Vphase;

    std::vector<int> numPhSpecies(m_numPhases, 0);

    for (kspec = 0; kspec < m_numSpeciesTot; ++kspec) {
      numPhSpecies[m_phaseID[kspec]]++;
    }   
    /*
     *           Handle the special case of a single species in a phase that
     *           has been earmarked as a multispecies phase.
     *           Treat that species as a single-species phase
     */
    for (iph = 0; iph < m_numPhases; iph++) {
      Vphase = m_VolPhaseList[iph];
      Vphase->m_singleSpecies = false;
      if (TPhInertMoles[iph] > 0.0) {
	Vphase->setExistence(2);
      }
      if (numPhSpecies[iph] <= 1) {
	if (TPhInertMoles[iph] == 0.0) {
	  Vphase->m_singleSpecies = true;
	}
      }
    }  

    /*
     *  Fill in some useful arrays here that have to do with the
     *  static information concerning the phase ID of species.
     *       SSPhase = Boolean indicating whether a species is in a 
     *                 single species phase or not.
     */
    for (kspec = 0; kspec < m_numSpeciesTot; kspec++) {
      iph = m_phaseID[kspec];
      Vphase = m_VolPhaseList[iph];
      if (Vphase->m_singleSpecies)  m_SSPhase[kspec] = TRUE;
      else                        m_SSPhase[kspec] = FALSE;
    }
  }
  /*****************************************************************************/

  //  This routine is mostly concerned with changing the private data  
  //  to be consistent with what's needed for solution. It is called one 
  //  time for each new problem structure definition.
  /*
   *  This routine is always followed by vcs_prep(). Therefore, tasks
   *  that need to be done for every call to vcsc() should be placed in
   *  vcs_prep() and not in this routine.
   *
   *  The problem structure refers to:
   *
   *     the number and identity of the species.
   *     the formula matrix and thus the number of components.
   *     the number and identity of the phases.
   *     the equation of state
   *     the method and parameters for determining the standard state
   *     The method and parameters for determining the activity coefficients.
   *
   * Tasks:
   *    0) Fill in the SSPhase[] array.
   *    1) Check to see if any multispecies phases actually have only one
   *       species in that phase. If true, reassign that phase and species
   *       to be a single-species phase.
   *    2) Determine the number of components in the problem if not already
   *       done so. During this process the order of the species is changed
   *       in the private data structure. All references to the species
   *       properties must employ the ind[] index vector. 
   *
   *  @param printLvl Print level of the routine
   *
   *  @return the return code
   *        VCS_SUCCESS = everything went OK
   *
   */
  int VCS_SOLVE::vcs_prep_oneTime(int printLvl) {
    int kspec, i, conv, retn = VCS_SUCCESS; 
    double pres, test;
    double *aw, *sa, *sm, *ss;
    bool modifiedSoln = false;

    m_debug_print_lvl = printLvl;


    /*
     *  Calculate the Single Species status of phases
     *  Also calculate the number of species per phase
     */
    vcs_SSPhase();

    /*
     *      Set an initial estimate for the number of noncomponent species
     *      equal to nspecies - nelements. This may be changed below
     */
    m_numRxnTot = m_numSpeciesTot - m_numElemConstraints;
    m_numRxnRdc = m_numRxnTot;
    m_numSpeciesRdc = m_numSpeciesTot;
    for (i = 0; i < m_numRxnRdc; ++i) {
      m_indexRxnToSpecies[i] = m_numElemConstraints + i;
    }
  
    for (kspec = 0; kspec < m_numSpeciesTot; ++kspec) {
      int pID = m_phaseID[kspec];
      int spPhIndex = m_speciesLocalPhaseIndex[kspec];
      vcs_VolPhase *vPhase =  m_VolPhaseList[pID];
      vcs_SpeciesProperties *spProp = vPhase->speciesProperty(spPhIndex);
      double sz = 0.0;
      int eSize =  spProp->FormulaMatrixCol.size();
      for (int e = 0; e < eSize; e++) {
	sz += fabs(spProp->FormulaMatrixCol[e]);
      }
      if (sz > 0.0) {
	m_spSize[kspec] = sz;
      } else {
	m_spSize[kspec] = 1.0;
      }
    }

    /* ***************************************************** */
    /* **** DETERMINE THE NUMBER OF COMPONENTS ************* */
    /* ***************************************************** */
   
    /* 
     *       Obtain a valid estimate of the mole fraction. This will
     *       be used as an initial ordering vector for prioritizing
     *       which species are defined as components.
     *
     *       If a mole number estimate was supplied from the 
     *       input file, use that mole number estimate.
     *
     *       If a solution estimate wasn't supplied from the input file,
     *       supply an initial estimate for the mole fractions 
     *       based on the relative reverse ordering of the
     *       chemical potentials.
     *
     *       For voltage unknowns, set these to zero for the moment.
     */
    test = -1.0e-10;
    if (m_doEstimateEquil < 0) {
      double sum  = 0.0;
      for (kspec = 0; kspec < m_numSpeciesTot; ++kspec) {
	if (m_speciesUnknownType[kspec] == VCS_SPECIES_TYPE_MOLNUM) {
	  sum += fabs(m_molNumSpecies_old[kspec]);
	}
      }
      if (fabs(sum) < 1.0E-6) {
	modifiedSoln = true;
	if (m_pressurePA <= 0.0)  pres = 1.01325E5;
	else                      pres = m_pressurePA;
	retn = vcs_evalSS_TP(0, 0, m_temperature, pres);
	for (kspec = 0; kspec < m_numSpeciesTot; ++kspec) {
	  if (m_speciesUnknownType[kspec] == VCS_SPECIES_TYPE_MOLNUM) {
	    m_molNumSpecies_old[kspec] = - m_SSfeSpecies[kspec];
	  } else {
	    m_molNumSpecies_old[kspec] = 0.0;
	  }
	}
      }
      test = -1.0e20;
    }

    /*
     *      NC = number of components is in the vcs.h common block 
     *     This call to BASOPT doesn't calculate the stoichiometric
     *     reaction matrix.
     */
    std::vector<double> awSpace( m_numSpeciesTot + (m_numElemConstraints + 2)*(m_numElemConstraints), 0.0); 
    aw = VCS_DATA_PTR(awSpace);
    if (aw == NULL) {
      plogf("vcs_prep_oneTime: failed to get memory: global bailout\n");
      return VCS_NOMEMORY;
    }
    sa = aw + m_numSpeciesTot;
    sm = sa + m_numElemConstraints;
    ss = sm + (m_numElemConstraints)*(m_numElemConstraints);
    retn = vcs_basopt(TRUE, aw, sa, sm, ss, test, &conv);
    if (retn != VCS_SUCCESS) {
      plogf("vcs_prep_oneTime:");
      plogf(" Determination of number of components failed: %d\n",
	    retn);
      plogf("          Global Bailout!\n");
      return retn;
    }  
   
    if (m_numElemConstraints != m_numComponents) {
      m_numRxnTot = m_numRxnRdc = m_numSpeciesTot - m_numComponents;
      for (i = 0; i < m_numRxnRdc; ++i) {
	m_indexRxnToSpecies[i] = m_numComponents + i;
      }
    }
    
    /*
     *   The elements might need to be rearranged.
     */
    awSpace.resize(m_numElemConstraints + (m_numElemConstraints + 2)*(m_numElemConstraints), 0.0); 
    aw = VCS_DATA_PTR(awSpace);
    sa = aw + m_numElemConstraints;
    sm = sa + m_numElemConstraints;
    ss = sm + (m_numElemConstraints)*(m_numElemConstraints);
    retn = vcs_elem_rearrange(aw, sa, sm, ss);
    if (retn != VCS_SUCCESS) {
      plogf("vcs_prep_oneTime:");
      plogf(" Determination of element reordering failed: %d\n",
	    retn);
      plogf("          Global Bailout!\n");
      return retn;
    }

    // If we mucked up the solution unknowns because they were all
    // zero to start with, set them back to zero here
    if (modifiedSoln) {
      for (kspec = 0; kspec < m_numSpeciesTot; ++kspec) {
	m_molNumSpecies_old[kspec] = 0.0;
      }
    }
    return VCS_SUCCESS;  
  }
 
  /*****************************************************************************/

  // Prepare the object for re-solution
  /*
   *  This routine is mostly concerned with changing the private data  
   *  to be consistent with that needed for solution. It is called for
   *  every invocation of the vcs_solve() except for the cleanup invocation.
   *
   * Tasks:
   *  1)  Initialization of arrays to zero.
   *  2)  Calculate total number of moles in all phases
   *
   *  return code
   *     VCS_SUCCESS = everything went OK
   *     VCS_PUB_BAD = There is an irreconcilable difference in the 
   *                   public data structure from when the problem was
   *                   initially set up.
   */
  int VCS_SOLVE::vcs_prep() {
    /*
     *        Initialize various arrays in the data to zero
     */
    vcs_vdzero(m_feSpecies_old, m_numSpeciesTot);
    vcs_vdzero(m_feSpecies_new, m_numSpeciesTot);
    vcs_vdzero(m_molNumSpecies_new, m_numSpeciesTot);
    vcs_dzero(&(m_deltaMolNumPhase[0][0]), m_numSpeciesTot * m_numPhases);
    vcs_izero(&(m_phaseParticipation[0][0]), m_numSpeciesTot * m_numPhases);
    vcs_dzero(VCS_DATA_PTR(m_deltaPhaseMoles), m_numPhases);
    vcs_dzero(VCS_DATA_PTR(m_tPhaseMoles_new), m_numPhases);
    /*
     *   Calculate the total number of moles in all phases.
     */
    vcs_tmoles();
    return VCS_SUCCESS;
  }

  /*****************************************************************************/

  //  In this routine, we check for things that will cause the algorithm
  //  to fail.
  /*
   *  We check to see if the problem is well posed. If it is not, we return
   *  false and print out error conditions.
   *
   *  Current there is one condition. If all the element abundances are
   *  zero, the algorithm will fail
   *
   * @param vprob   VCS_PROB pointer to the definition of the equilibrium
   *                problem
   *
   * @return  If true, the problem is well-posed. If false, the problem
   *          is not well posed.
   */
  bool VCS_SOLVE::vcs_wellPosed(VCS_PROB *vprob) {
    double sum = 0.0;
    for (int e = 0; e < vprob->ne; e++) {
      sum = sum + vprob->gai[e];
    }
    if (sum < 1.0E-20) {
      plogf("vcs_wellPosed: Element abundance is close to zero\n");
      return false;
    }
    return true;
  }

  /*****************************************************************************/
}
