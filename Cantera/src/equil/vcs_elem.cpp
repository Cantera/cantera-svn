/**
 * @file vcs_elem.cpp
 *  This file contains the algorithm for checking the satisfaction of the
 *  element abundances constraints and the algorithm for fixing violations
 *  of the element abundances constraints.
 */
#include "vcs_solve.h"
#include "vcs_internal.h" 
#include "math.h"

namespace VCSnonideal {


  //! Computes the current elemental abundances vector
  /*!
   *   Computes the elemental abundances vector, m_elemAbundances[], and stores it
   *   back into the global structure
   */
  void VCS_SOLVE::vcs_elab() {
    for (int j = 0; j < m_numElemConstraints; ++j) {
      m_elemAbundances[j] = 0.0;
      for (int i = 0; i < m_numSpeciesTot; ++i) {
	if (m_speciesUnknownType[i] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	  m_elemAbundances[j] += m_formulaMatrix[j][i] * m_molNumSpecies_old[i];
	}
      }
    }
  }


  /*
   *
   * vcs_elabcheck:
   *
   *   This function checks to see if the element abundances are in 
   *   compliance. If they are, then TRUE is returned. If not, 
   *   FALSE is returned. Note the number of constraints checked is 
   *   usually equal to the number of components in the problem. This 
   *   routine can check satisfaction of all of the constraints in the
   *   problem, which is equal to ne. However, the solver can't fix
   *   breakage of constraints above nc, because that nc is the
   *   range space by definition. Satisfaction of extra constraints would
   *   have had to occur in the problem specification. 
   *
   *   The constraints should be broken up into 2  sections. If 
   *   a constraint involves a formula matrix with positive and 
   *   negative signs, and eaSet = 0.0, then you can't expect that the
   *   sum will be zero. There may be roundoff that inhibits this.
   *   However, if the formula matrix is all of one sign, then
   *   this requires that all species with nonzero entries in the
   *   formula matrix be identically zero. We put this into
   *   the logic below.
   *
   * Input
   * -------
   *   ibound = 1 : Checks constraints up to the number of elements
   *            0 : Checks constraints up to the number of components.
   *
   */
  int VCS_SOLVE::vcs_elabcheck(int ibound) {
    int i; 
    int top = m_numComponents;
    double eval, scale;
    int numNonZero;
    bool multisign = false;
    if (ibound) {
      top = m_numElemConstraints;  
    }
    /*
     * Require 12 digits of accuracy on non-zero constraints.
     */
    for (i = 0; i < top; ++i) {
      if (m_elementActive[i]) {
	if (fabs(m_elemAbundances[i] - m_elemAbundancesGoal[i]) > (fabs(m_elemAbundancesGoal[i]) * 1.0e-12)) {
	  /*
	   * This logic is for charge neutrality condition
	   */
	  if (m_elType[i] == VCS_ELEM_TYPE_CHARGENEUTRALITY) {
	    AssertThrowVCS(m_elemAbundancesGoal[i] == 0.0, "vcs_elabcheck");
	  }
	  if (m_elemAbundancesGoal[i] == 0.0 || (m_elType[i] == VCS_ELEM_TYPE_ELECTRONCHARGE)) {
	    scale = VCS_DELETE_MINORSPECIES_CUTOFF;
	    /*
	     * Find out if the constraint is a multisign constraint.
	     * If it is, then we have to worry about roundoff error
	     * in the addition of terms. We are limited to 13
	     * digits of finite arithmetic accuracy.
	     */
	    numNonZero = 0;
	    multisign = false;
	    for (int kspec = 0; kspec < m_numSpeciesTot; kspec++) {
	      eval = m_formulaMatrix[i][kspec];
	      if (eval < 0.0) {
		multisign = true;
	      }
	      if (eval != 0.0) {
		scale = MAX(scale, fabs(eval * m_molNumSpecies_old[kspec]));
		numNonZero++;
	      }
	    }
	    if (multisign) {
	      if (fabs(m_elemAbundances[i] - m_elemAbundancesGoal[i]) > 1e-11 * scale) {
		return FALSE;
	      }
	    } else {
	      if (fabs(m_elemAbundances[i] - m_elemAbundancesGoal[i]) > VCS_DELETE_MINORSPECIES_CUTOFF) {
		return FALSE;
	      }
	    }
	  } else {
	    /*
	     * For normal element balances, we require absolute compliance
	     * even for rediculously small numbers.
	     */
	    if (m_elType[i] == VCS_ELEM_TYPE_ABSPOS) {
	      return FALSE;
	    } else {
	      return FALSE;
	    }
	  }
	}
      }
    }
    return TRUE;
  } /* vcs_elabcheck() *********************************************************/

  /*****************************************************************************/
  /*****************************************************************************/
  /*****************************************************************************/

  void VCS_SOLVE::vcs_elabPhase(int iphase, double * const elemAbundPhase)
   
    /*************************************************************************
     *
     * vcs_elabPhase:
     *
     *  Computes the elemental abundances vector for a single phase,
     *  elemAbundPhase[], and returns it through the argument list.
     *  The mole numbers of species are taken from the current value
     *  in m_molNumSpecies_old[].
     *************************************************************************/
  {
    int i, j;
    for (j = 0; j < m_numElemConstraints; ++j) {
      elemAbundPhase[j] = 0.0;
      for (i = 0; i < m_numSpeciesTot; ++i) {
	if (m_speciesUnknownType[i] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	  if (m_phaseID[i] == iphase) {
	    elemAbundPhase[j] += m_formulaMatrix[j][i] * m_molNumSpecies_old[i];
	  }
	}
      }
    }
  }

  /*****************************************************************************/
  /*****************************************************************************/
  /*****************************************************************************/

  int VCS_SOLVE::vcs_elcorr(double aa[], double x[])
   
    /**************************************************************************
     *
     * vcs_elcorr:
     *
     * This subroutine corrects for element abundances. At the end of the 
     * surbroutine, the total moles in all phases are recalculated again,
     * because we have changed the number of moles in this routine.
     *
     * Input
     *  -> temporary work vectors:
     *     aa[ne*ne]
     *     x[ne]
     *
     * Return Values:
     *      0 = Nothing of significance happened,   
     *          Element abundances were and still are good.
     *      1 = The solution changed significantly; 
     *          The element abundances are now good.
     *      2 = The solution changed significantly,
     *          The element abundances are still bad.
     *      3 = The solution changed significantly,
     *          The element abundances are still bad and a component 
     *          species got zeroed out.
     *
     *  Internal data to be worked on::
     *
     *  ga    Current element abundances
     *  m_elemAbundancesGoal   Required elemental abundances
     *  m_molNumSpecies_old     Current mole number of species.
     *  m_formulaMatrix[][]  Formular matrix of the species
     *  ne    Number of elements
     *  nc    Number of components.
     *
     * NOTES:
     *  This routine is turning out to be very problematic. There are 
     *  lots of special cases and problems with zeroing out species.
     *
     *  Still need to check out when we do loops over nc vs. ne.
     *  
     *************************************************************************/
  {
    int i, j, retn = 0, kspec, goodSpec, its; 
    double xx, par, saveDir, dir;

#ifdef DEBUG_MODE
    double l2before = 0.0, l2after = 0.0;
    std::vector<double> ga_save(m_numElemConstraints, 0.0);
    vcs_dcopy(VCS_DATA_PTR(ga_save), VCS_DATA_PTR(m_elemAbundances), m_numElemConstraints);
    if (m_debug_print_lvl >= 2) {
      plogf("   --- vcsc_elcorr: Element abundances correction routine");
      if (m_numElemConstraints != m_numComponents) {
	plogf(" (m_numComponents != m_numElemConstraints)");
      }
      plogf("\n");
    }

    for (i = 0; i < m_numElemConstraints; ++i) {
      x[i] = m_elemAbundances[i] - m_elemAbundancesGoal[i];
    }
    l2before = 0.0;
    for (i = 0; i < m_numElemConstraints; ++i) {
      l2before += x[i] * x[i];
    }
    l2before = sqrt(l2before/m_numElemConstraints);
#endif

    /*
     * Special section to take out single species, single component,
     * moles. These are species which have non-zero entries in the
     * formula matrix, and no other species have zero values either.
     *
     */
    int numNonZero = 0;
    bool changed = false;
    bool multisign = false;
    for (i = 0; i < m_numElemConstraints; ++i) {
      numNonZero = 0;
      multisign = false;
      for (kspec = 0; kspec < m_numSpeciesTot; kspec++) {
	if (m_speciesUnknownType[kspec] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	  double eval = m_formulaMatrix[i][kspec];
	  if (eval < 0.0) {
	    multisign = true;
	  }
	  if (eval != 0.0) {
	    numNonZero++;
	  }
	}
      }
      if (!multisign) {
	if (numNonZero < 2) {
	  for (kspec = 0; kspec < m_numSpeciesTot; kspec++) {
	    if (m_speciesUnknownType[kspec] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	      double eval = m_formulaMatrix[i][kspec];
	      if (eval > 0.0) {
		m_molNumSpecies_old[kspec] = m_elemAbundancesGoal[i] / eval;
		changed = true;
	      }
	    }
	  }
	} else {
	  int numCompNonZero = 0;
	  int compID = -1;
	  for (kspec = 0; kspec < m_numComponents; kspec++) {
	    if (m_speciesUnknownType[kspec] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	      double eval = m_formulaMatrix[i][kspec];
	      if (eval > 0.0) {
		compID = kspec;
		numCompNonZero++;
	      }
	    }
	  }
	  if (numCompNonZero == 1) {
	    double diff = m_elemAbundancesGoal[i];
	    for (kspec = m_numComponents; kspec < m_numSpeciesTot; kspec++) {
	      if (m_speciesUnknownType[kspec] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
		double eval = m_formulaMatrix[i][kspec];
		diff -= eval * m_molNumSpecies_old[kspec];
	      }
	      m_molNumSpecies_old[compID] = MAX(0.0,diff/m_formulaMatrix[i][compID]);
	      changed = true;
	    }
	  }
	}
      }
    }
    if (changed) {
      vcs_elab();
    }

    /*
     *  Section to check for maximum bounds errors on all species
     *  due to elements.
     *    This may only be tried on element types which are VCS_ELEM_TYPE_ABSPOS.
     *    This is because no other species may have a negative number of these.
     *
     *  Note, also we can do this over ne, the number of elements, not just
     *  the number of components.
     */
    changed = false;
    for (i = 0; i < m_numElemConstraints; ++i) {
      int elType = m_elType[i];
      if (elType == VCS_ELEM_TYPE_ABSPOS) {
	for (kspec = 0; kspec < m_numSpeciesTot; kspec++) {
	  if (m_speciesUnknownType[kspec] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	    double atomComp = m_formulaMatrix[i][kspec];
	    if (atomComp > 0.0) {
	      double maxPermissible = m_elemAbundancesGoal[i] / atomComp;
	      if (m_molNumSpecies_old[kspec] > maxPermissible) {
	      
#ifdef DEBUG_MODE
		if (m_debug_print_lvl >= 3) {
		  plogf("  ---  vcs_elcorr: Reduced species %s from %g to %g "
			"due to %s max bounds constraint\n",
			m_speciesName[kspec].c_str(), m_molNumSpecies_old[kspec], 
			maxPermissible, m_elementName[i].c_str());
		}
#endif
		m_molNumSpecies_old[kspec] = maxPermissible;
		changed = true;
		if (m_molNumSpecies_old[kspec] < VCS_DELETE_MINORSPECIES_CUTOFF) {
		  m_molNumSpecies_old[kspec] = 0.0;
		  if (m_SSPhase[kspec]) {
		    m_speciesStatus[kspec] = VCS_SPECIES_ZEROEDSS;
		  } else {
		    m_speciesStatus[kspec] = VCS_SPECIES_ACTIVEBUTZERO;
		  } 
#ifdef DEBUG_MODE
		  if (m_debug_print_lvl >= 2) {
		    plogf("  ---  vcs_elcorr: Zeroed species %s and changed "
			  "status to %d due to max bounds constraint\n",
			  m_speciesName[kspec].c_str(), m_speciesStatus[kspec]);
		  }
#endif
		}
	      }
	    }
	  }
	}
      }
    }

    // Recalculate the element abundances if something has changed.
    if (changed) {
      vcs_elab();
    }

    /*
     * Ok, do the general case. Linear algebra problem is 
     * of length nc, not ne, as there may be degenerate rows when
     * nc .ne. ne.
     */
    for (i = 0; i < m_numComponents; ++i) {
      x[i] = m_elemAbundances[i] - m_elemAbundancesGoal[i];
      if (fabs(x[i]) > 1.0E-13) retn = 1;
      for (j = 0; j < m_numComponents; ++j) {
	aa[j + i*m_numElemConstraints] = m_formulaMatrix[j][i];
      }
    }
    i = vcsUtil_mlequ(aa, m_numElemConstraints, m_numComponents, x, 1);
    if (i == 1) {
      plogf("vcs_elcorr ERROR: mlequ returned error condition\n");
      return VCS_FAILED_CONVERGENCE;
    }
    /*
     * Now apply the new direction without creating negative species.
     */
    par = 0.5;
    for (i = 0; i < m_numComponents; ++i) {
      if (m_molNumSpecies_old[i] > 0.0) {
	xx = -x[i] / m_molNumSpecies_old[i];
	if (par < xx) par = xx;
      }
    }
    if (par > 100.0) {
      par = 100.0;
    }
    par = 1.0 / par;
    if (par < 1.0 && par > 0.0) {
      retn = 2;
      par *= 0.9999;
      for (i = 0; i < m_numComponents; ++i) {
	double tmp = m_molNumSpecies_old[i] + par * x[i];
	if (tmp > 0.0) {
	  m_molNumSpecies_old[i] = tmp;
	} else {
	  if (m_SSPhase[i]) {
	    m_molNumSpecies_old[i] =  0.0;
	  }  else {
	    m_molNumSpecies_old[i] = m_molNumSpecies_old[i] * 0.0001;
	  }
	}
      }
    } else {
      for (i = 0; i < m_numComponents; ++i) {
	double tmp = m_molNumSpecies_old[i] + x[i];
	if (tmp > 0.0) {
	  m_molNumSpecies_old[i] = tmp;
	} else { 
	  if (m_SSPhase[i]) {
	    m_molNumSpecies_old[i] =  0.0;
	  }  else {
	    m_molNumSpecies_old[i] = m_molNumSpecies_old[i] * 0.0001;
	  }
	}
      }
    }
   
    /*
     *   We have changed the element abundances. Calculate them again
     */
    vcs_elab();
    /*
     *   We have changed the total moles in each phase. Calculate them again
     */
    vcs_tmoles();
    
    /*
     *       Try some ad hoc procedures for fixing the problem
     */
    if (retn >= 2) {
      /*
       *       First find a species whose adjustment is a win-win
       *       situation.
       */
      for (kspec = 0; kspec < m_numSpeciesTot; kspec++) {
	if (m_speciesUnknownType[kspec] == VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
	  continue;
	}
	saveDir = 0.0;
	goodSpec = TRUE;
	for (i = 0; i < m_numComponents; ++i) {
	  dir = m_formulaMatrix[i][kspec] *  (m_elemAbundancesGoal[i] - m_elemAbundances[i]);
	  if (fabs(dir) > 1.0E-10) {
	    if (dir > 0.0) {
	      if (saveDir < 0.0) {
		goodSpec = FALSE;
		break;
	      }
	    } else {
	      if (saveDir > 0.0) {
		goodSpec = FALSE;
		break;
	      }		   
	    }
	    saveDir = dir;
	  } else {
	    if (m_formulaMatrix[i][kspec] != 0.) {
	      goodSpec = FALSE;
	      break;
	    }
	  }
	}
	if (goodSpec) {
	  its = 0;
	  xx = 0.0;
	  for (i = 0; i < m_numComponents; ++i) {
	    if (m_formulaMatrix[i][kspec] != 0.0) {
	      xx += (m_elemAbundancesGoal[i] - m_elemAbundances[i]) / m_formulaMatrix[i][kspec];
	      its++;
	    }
	  }
	  if (its > 0) xx /= its;
	  m_molNumSpecies_old[kspec] += xx;
	  m_molNumSpecies_old[kspec] = MAX(m_molNumSpecies_old[kspec], 1.0E-10);
	  /*
	   *   If we are dealing with a deleted species, then
	   *   we need to reinsert it into the active list.
	   */
	  if (kspec >= m_numSpeciesRdc) {
	    vcs_reinsert_deleted(kspec);	
	    m_molNumSpecies_old[m_numSpeciesRdc - 1] = xx;
	    vcs_elab();
	    goto L_CLEANUP;
	  }
	  vcs_elab();  
	}
      }     
    }
    if (vcs_elabcheck(0)) {
      retn = 1;
      goto L_CLEANUP;
    }
  
    for (i = 0; i < m_numElemConstraints; ++i) {
      if (m_elType[i] == VCS_ELEM_TYPE_CHARGENEUTRALITY ||
	  (m_elType[i] == VCS_ELEM_TYPE_ABSPOS && m_elemAbundancesGoal[i] == 0.0)) { 
	for (kspec = 0; kspec < m_numSpeciesRdc; kspec++) {
	  if (m_elemAbundances[i] > 0.0) {
	    if (m_formulaMatrix[i][kspec] < 0.0) {
	      m_molNumSpecies_old[kspec] -= m_elemAbundances[i] / m_formulaMatrix[i][kspec] ;
	      if (m_molNumSpecies_old[kspec] < 0.0) {
		m_molNumSpecies_old[kspec] = 0.0;
	      }
	      vcs_elab();
	      break;
	    }
	  }
	  if (m_elemAbundances[i] < 0.0) {
	    if (m_formulaMatrix[i][kspec] > 0.0) {
	      m_molNumSpecies_old[kspec] -= m_elemAbundances[i] / m_formulaMatrix[i][kspec];
	      if (m_molNumSpecies_old[kspec] < 0.0) {
		m_molNumSpecies_old[kspec] = 0.0;
	      }
	      vcs_elab();
	      break;
	    }
	  }
	}
      }
    }
    if (vcs_elabcheck(1)) {
      retn = 1;      
      goto L_CLEANUP;
    }

    /*
     *  For electron charges element types, we try positive deltas
     *  in the species concentrations to match the desired
     *  electron charge exactly.
     */
    for (i = 0; i < m_numElemConstraints; ++i) {
      double dev = m_elemAbundancesGoal[i] - m_elemAbundances[i];
      if (m_elType[i] == VCS_ELEM_TYPE_ELECTRONCHARGE && (fabs(dev) > 1.0E-300)) {
	bool useZeroed = true;
	for (kspec = 0; kspec < m_numSpeciesRdc; kspec++) {
	  if (dev < 0.0) {
	    if (m_formulaMatrix[i][kspec] < 0.0) {
	      if (m_molNumSpecies_old[kspec] > 0.0) {
		useZeroed = false;
	      }
	    }
	  } else {
	    if (m_formulaMatrix[i][kspec] > 0.0) {
	      if (m_molNumSpecies_old[kspec] > 0.0) {
		useZeroed = false;
	      }
	    }
	  }
	}
	for (kspec = 0; kspec < m_numSpeciesRdc; kspec++) {
	  if (m_molNumSpecies_old[kspec] > 0.0 || useZeroed) {
	    if (dev < 0.0) {
	      if (m_formulaMatrix[i][kspec] < 0.0) {
		double delta = dev / m_formulaMatrix[i][kspec] ;
		m_molNumSpecies_old[kspec] += delta;
		if (m_molNumSpecies_old[kspec] < 0.0) {
		  m_molNumSpecies_old[kspec] = 0.0;
		}
		vcs_elab();
		break;
	      }
	    }
	    if (dev > 0.0) {
	      if (m_formulaMatrix[i][kspec] > 0.0) {
		double delta = dev / m_formulaMatrix[i][kspec] ;
		m_molNumSpecies_old[kspec] += delta;
		if (m_molNumSpecies_old[kspec] < 0.0) {
		  m_molNumSpecies_old[kspec] = 0.0;
		}
		vcs_elab();
		break;
	      }
	    }
	  }
	}
      }
    }
    if (vcs_elabcheck(1)) {
      retn = 1;      
      goto L_CLEANUP;
    }

  L_CLEANUP: ;
    vcs_tmoles();
#ifdef DEBUG_MODE
    l2after = 0.0;
    for (i = 0; i < m_numElemConstraints; ++i) {
      l2after += SQUARE(m_elemAbundances[i] - m_elemAbundancesGoal[i]);
    }
    l2after = sqrt(l2after/m_numElemConstraints);
    if (m_debug_print_lvl >= 2) {
      plogf("   ---    Elem_Abund:  Correct             Initial  "
	    "              Final\n");
      for (i = 0; i < m_numElemConstraints; ++i) {
	plogf("   ---       "); plogf("%-2.2s", m_elementName[i].c_str());
	plogf(" %20.12E %20.12E %20.12E\n", m_elemAbundancesGoal[i], ga_save[i], m_elemAbundances[i]);
      }
      plogf("   ---            Diff_Norm:         %20.12E %20.12E\n",
	    l2before, l2after);
    }
#endif
    return retn;
  }

}

