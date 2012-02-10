/**
 * @file vcs_species_thermo.cpp
 *   Implementation for the VCS_SPECIES_THERMO object.
 */
/*
 * Copywrite (2005) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */



#include "vcs_solve.h"
#include "vcs_species_thermo.h"
#include "vcs_defs.h"
#include "vcs_VolPhase.h"

#include "vcs_Exception.h"
#include "vcs_internal.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>

using namespace std;

namespace VCSnonideal {


VCS_SPECIES_THERMO::VCS_SPECIES_THERMO(size_t indexPhase,
                                       size_t indexSpeciesPhase) :
    
  IndexPhase(indexPhase),
  IndexSpeciesPhase(indexSpeciesPhase),
  OwningPhase(0),
  SS0_Model(VCS_SS0_CONSTANT),
  SS0_feSave(0.0),   
  SS0_TSave(-90.0),
  SS0_T0(273.15),
  SS0_H0(0.0),
  SS0_S0(0.0),
  SS0_Cp0(0.0),
  SS0_Pref(1.01325E5),
  SS0_Params(0),
  SSStar_Model(VCS_SSSTAR_CONSTANT),
  SSStar_Params(0),
  Activity_Coeff_Model(VCS_AC_CONSTANT),
  Activity_Coeff_Params(0),
  SSStar_Vol_Model(VCS_SSVOL_IDEALGAS),
  SSStar_Vol_Params(0),
  SSStar_Vol0(-1.0),
  UseCanteraCalls(false),
  m_VCS_UnitsFormat(VCS_UNITS_UNITLESS)
{
  SS0_Pref = 1.01325E5;
}


/******************************************************************************
 *
 * destructor
 */
VCS_SPECIES_THERMO::~VCS_SPECIES_THERMO() 
{
}

/*****************************************************************************
 *
 * Copy Constructor VCS_SPECIES_THERMO
 */
VCS_SPECIES_THERMO::VCS_SPECIES_THERMO(const VCS_SPECIES_THERMO& b) :
  IndexPhase(b.IndexPhase),
  IndexSpeciesPhase(b.IndexSpeciesPhase),
  OwningPhase(b.OwningPhase),
  SS0_Model(b.SS0_Model),
  SS0_feSave(b.SS0_feSave),
  SS0_TSave(b.SS0_TSave),
  SS0_T0(b.SS0_T0),
  SS0_H0(b.SS0_H0),
  SS0_S0(b.SS0_S0),
  SS0_Cp0(b.SS0_Cp0),
  SS0_Pref(b.SS0_Pref),
  SS0_Params(0),
  SSStar_Model(b.SSStar_Model),
  SSStar_Params(0),
  Activity_Coeff_Model(b.Activity_Coeff_Model),
  Activity_Coeff_Params(0),
  SSStar_Vol_Model(b.SSStar_Vol_Model),
  SSStar_Vol_Params(0),
  SSStar_Vol0(b.SSStar_Vol0),
  UseCanteraCalls(b.UseCanteraCalls),
  m_VCS_UnitsFormat(b.m_VCS_UnitsFormat)
{
    
   SS0_Params = 0;
}

/*****************************************************************************
 *
 * Assignment operator for VCS_SPECIES_THERMO
 */
VCS_SPECIES_THERMO& 
VCS_SPECIES_THERMO::operator=(const VCS_SPECIES_THERMO& b)
{
  if (&b != this) {
    IndexPhase            = b.IndexPhase;
    IndexSpeciesPhase     = b.IndexSpeciesPhase;
    OwningPhase           = b.OwningPhase;
    SS0_Model             = b.SS0_Model;
    SS0_feSave            = b.SS0_feSave;
    SS0_TSave             = b.SS0_TSave;
    SS0_T0                = b.SS0_T0;
    SS0_H0                = b.SS0_H0;
    SS0_S0                = b.SS0_S0;
    SS0_Cp0               = b.SS0_Cp0;
    SS0_Pref              = b.SS0_Pref;
    SSStar_Model          = b.SSStar_Model;
    /*
     * shallow copy because function is undeveloped.
     */
    SSStar_Params         = b.SSStar_Params;
    Activity_Coeff_Model  = b.Activity_Coeff_Model;
    /*
     * shallow copy because function is undeveloped.
     */
    Activity_Coeff_Params = b.Activity_Coeff_Params;
    SSStar_Vol_Model      = b.SSStar_Vol_Model;
    /*
     * shallow copy because function is undeveloped.
     */
    SSStar_Vol_Params     = b.SSStar_Vol_Params; 
    SSStar_Vol0           = b.SSStar_Vol0;
    UseCanteraCalls       = b.UseCanteraCalls;
    m_VCS_UnitsFormat     = b.m_VCS_UnitsFormat;
  }
  return *this;
}

/******************************************************************************
 *
 * duplMyselfAsVCS_SPECIES_THERMO():                (virtual)
 *
 *    This routine can duplicate inherited objects given a base class
 *    pointer. It relies on valid copy constructors.
 */

VCS_SPECIES_THERMO* VCS_SPECIES_THERMO::duplMyselfAsVCS_SPECIES_THERMO() {
  VCS_SPECIES_THERMO* ptr = new VCS_SPECIES_THERMO(*this);
  return  ptr;
}


/**************************************************************************
 *
 * GStar_R_calc();
 *
 *  This function calculates the standard state Gibbs free energy
 *  for species, kspec, at the solution temperature TKelvin and
 *  solution pressure, Pres.
 *  
 *
 *  Input
 *   kglob = species global index.
 *   TKelvin = Temperature in Kelvin
 *   pres = pressure is given in units specified by if__ variable.
 *
 *
 * Output
 *    return value = standard state free energy in units of Kelvin.
 */
double VCS_SPECIES_THERMO::GStar_R_calc(size_t kglob, double TKelvin,
					double pres)
{
  char yo[] = "VCS_SPECIES_THERMO::GStar_R_calc ";
  double fe, T;
  fe = G0_R_calc(kglob, TKelvin);
  T = TKelvin;
  if (UseCanteraCalls) {
    AssertThrowVCS(m_VCS_UnitsFormat == VCS_UNITS_MKS, "Possible inconsistency");
    size_t kspec = IndexSpeciesPhase;
    OwningPhase->setState_TP(TKelvin, pres);
    fe = OwningPhase->GStar_calc_one(kspec);
    double R = vcsUtil_gasConstant(m_VCS_UnitsFormat);
    fe /= R;
  } else {
    double pref = SS0_Pref;
    switch(SSStar_Model) {
    case VCS_SSSTAR_CONSTANT:
      break;
    case VCS_SSSTAR_IDEAL_GAS:
      fe += T * log( pres/ pref );	 
      break;
    default:
      plogf("%sERROR: unknown SSStar model\n", yo);
      exit(EXIT_FAILURE);
    }
  }
  return fe;
}
   
/**************************************************************************
 *
 * VolStar_calc:
 *
 *  This function calculates the standard state molar volume
 *  for species, kspec, at the temperature TKelvin and pressure, Pres,
 * 
 *  Input
 *
 * Output
 *    return value = standard state volume in    m**3 per kmol.
 *                   (VCS_UNITS_MKS)  
 */
double VCS_SPECIES_THERMO::
VolStar_calc(size_t kglob, double TKelvin, double presPA)
{
  char yo[] = "VCS_SPECIES_THERMO::VStar_calc ";
  double vol, T;
   
  T = TKelvin;
  if (UseCanteraCalls) {
    AssertThrowVCS(m_VCS_UnitsFormat == VCS_UNITS_MKS, "Possible inconsistency");
    size_t kspec = IndexSpeciesPhase;
    OwningPhase->setState_TP(TKelvin, presPA);
    vol = OwningPhase->VolStar_calc_one(kspec);
  } else {
    switch(SSStar_Vol_Model) {
    case VCS_SSVOL_CONSTANT:
      vol = SSStar_Vol0;
      break;
    case VCS_SSVOL_IDEALGAS:
      // R J/kmol/K (2006 CODATA value)
      vol= 8314.47215  * T / presPA;
      break;
    default:     
      plogf("%sERROR: unknown SSVol model\n", yo);
      exit(EXIT_FAILURE);
    } 
  }
  return vol;
} 

/**************************************************************************
 *
 * G0_R_calc:
 *
 *  This function calculates the naught state Gibbs free energy
 *  for species, kspec, at the temperature TKelvin
 *
 *  Input
 *   kglob = species global index.
 *   TKelvin = Temperature in Kelvin
 *
 * Output
 *    return value = naught state free energy in Kelvin.
 */
double VCS_SPECIES_THERMO::G0_R_calc(size_t kglob, double TKelvin)
{
#ifdef DEBUG_MODE
  char yo[] = "VS_SPECIES_THERMO::G0_R_calc ";
#endif
  double fe, H, S;
  if (SS0_Model == VCS_SS0_CONSTANT) {
    fe = SS0_feSave;  
    return fe;
  }
  if (TKelvin == SS0_TSave) {
    fe = SS0_feSave;
    return fe;
  }
  if (UseCanteraCalls) {
    AssertThrowVCS(m_VCS_UnitsFormat == VCS_UNITS_MKS, "Possible inconsistency");
    size_t kspec = IndexSpeciesPhase;
    OwningPhase->setState_T(TKelvin);
    fe = OwningPhase->G0_calc_one(kspec);
    double R = vcsUtil_gasConstant(m_VCS_UnitsFormat);
    fe /= R;
  } else {
    switch (SS0_Model) {
    case VCS_SS0_CONSTANT:
      fe = SS0_feSave;
      break;
    case VCS_SS0_CONSTANT_CP:
      H  = SS0_H0 + (TKelvin - SS0_T0) * SS0_Cp0;
      S  = SS0_Cp0 + SS0_Cp0 * log((TKelvin / SS0_T0));
      fe = H - TKelvin * S;
      break;
    default:
#ifdef DEBUG_MODE
      plogf("%sERROR: unknown model\n", yo);
#endif
      exit(EXIT_FAILURE);
    }
  }
  SS0_feSave = fe;
  SS0_TSave = TKelvin;
  return fe;
} 

/**************************************************************************
 *
 * eval_ac:
 *
 *  This function evaluates the activity coefficient
 *  for species, kspec
 *
 *  Input
 *      kglob -> integer value of the species in the global 
 *            species list within VCS_GLOB. Phase and local species id
 *             can be looked up within object.
 * 
 *   Note, T, P and mole fractions are obtained from the
 *   single private instance of VCS_GLOB
 *   
 *
 * Output
 *    return value = activity coefficient for species kspec
 */
double VCS_SPECIES_THERMO::eval_ac(size_t kglob)
{
#ifdef DEBUG_MODE
  char yo[] = "VCS_SPECIES_THERMO::eval_ac ";
#endif
  double ac;
  /*
   *  Activity coefficients are frequently evaluated on a per phase
   *  basis. If they are, then the currPhAC[] boolean may be used
   *  to reduce repeated work. Just set currPhAC[iph], when the 
   *  activity coefficients for all species in the phase are reevaluated.
   */
  if (UseCanteraCalls) {
    size_t kspec = IndexSpeciesPhase;
    ac = OwningPhase->AC_calc_one(kspec);
  } else {
    switch (Activity_Coeff_Model) {
    case VCS_AC_CONSTANT:
      ac = 1.0;
      break;
    default:
#ifdef DEBUG_MODE
      plogf("%sERROR: unknown model\n", yo);
#endif
      exit(EXIT_FAILURE);
    }
  }
  return ac;
}

/*****************************************************************************/
}
