/*
 * $Id$
 */

/*
 * Copywrite (2005) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */

#ifndef VCS_SPECIES_THERMO_H
#define VCS_SPECIES_THERMO_H

//#include <vector>

namespace VCSnonideal {

class vcs_VolPhase;

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*
 *     Models for the species standard state Naught temperature 
 *     dependence
 */
#define VCS_SS0_NOTHANDLED    -1
#define VCS_SS0_CONSTANT       0
//#define VCS_SS0_NASA_POLY      1
#define VCS_SS0_CONSTANT_CP    2


/*
 *     Models for the species standard state extra pressure dependence
 *
 */
#define VCS_SSSTAR_NOTHANDLED    -1
#define VCS_SSSTAR_CONSTANT       0
#define VCS_SSSTAR_IDEAL_GAS      1

/*
 *  Identifies the thermo model for the species
 *  This structure is shared by volumetric and surface species. However,
 *  each will have its own types of thermodynamic models. These
 *  quantities all have appropriate units. The units are specified by 
 *  VCS_UnitsFormat.
 */
class VCS_SPECIES_THERMO {
  /*
   * All objects are public for ease of development
   */
public:
  /**
   * Index of the phase that this species belongs to.
   */
  int IndexPhase;
    
  /**
   * Index of this species in the current phase.
   */
  int IndexSpeciesPhase;

  /**
   * Pointer to the owning phase object.
   */
  vcs_VolPhase *OwningPhase;

  /**
   * Integer representing the models for the species standard state
   * Naught temperature dependence. They are listed above and start
   * with VCS_SS0_...
   */
  int    SS0_Model;

  /**
   * Internal storage of the last calculation of the reference
   * naught Gibbs free energy at SS0_TSave.
   * (always in units of Kelvin)
   */
  double SS0_feSave;

  /**
   * Internal storage of the last temperature used in the
   * calculation of the reference naught Gibbs free energy.
   * units = kelvin
   */
  double SS0_TSave;

  /**
   * Base temperature used in the VCS_SS0_CONSTANT_CP 
   * model
   */
  double SS0_T0;

  /**
   * Base enthalpy used in the VCS_SS0_CONSTANT_CP 
   * model
   */
  double SS0_H0;

  /**
   * Base entropy used in the VCS_SS0_CONSTANT_CP 
   * model
   */
  double SS0_S0;

  /**
   * Base heat capacity used in the VCS_SS0_CONSTANT_CP 
   * model
   */
  double SS0_Cp0;

  /**
   * Value of the pressure for the reference state.
   *  defaults to 1.01325E5 = 1 atm
   */
  double SS0_Pref;
  /**
   * Pointer to a list of parameters that is malloced for
   * complicated reference state calculation.
   */
  void *SS0_Params;
  /**
   * Integer value representing the star state model.
   */
  int SSStar_Model;

  /**
   * Pointer to a list of parameters that is malloced for
   * complicated reference star state calculation.
   */
  void *SSStar_Params;
 
  /**
   * Integer value representing the activity coefficient model
   * These are defined in vcs_VolPhase.h and start with 
   * VCS_AC_...
   */
  int Activity_Coeff_Model;

  /**
   * Pointer to a list of parameters that is malloced for
   * activity coefficient models.
   */
  void *Activity_Coeff_Params;

  /**
   * Models for the standard state volume of each species
   */
  int    SSStar_Vol_Model;

  /**
   * Pointer to a list of parameters that is malloced for
   * volume models
   */
  void *SSStar_Vol_Params;

  /**
   * parameter that is used int eh VCS_SSVOL_CONSTANT model.
   */
  double SSStar_Vol0;

  /**
   * If true, this object will call Cantera to do its member
   * calculations.
   */
  bool UseCanteraCalls;

  int m_VCS_UnitsFormat;
  /*
   * constructor and destructor
   */
  VCS_SPECIES_THERMO(int indexPhase, int indexSpeciesPhase);
  virtual ~VCS_SPECIES_THERMO();

  /*
   * Copy constructor and assignment operator
   */
  VCS_SPECIES_THERMO(const VCS_SPECIES_THERMO& b);
  VCS_SPECIES_THERMO& operator=(const VCS_SPECIES_THERMO& b);

  /*
   * Duplication function for inherited classes.
   */
  virtual VCS_SPECIES_THERMO* duplMyselfAsVCS_SPECIES_THERMO();

  /**
   * This function calculates the standard state Gibbs free energy
   *  for species, kspec, at the temperature TKelvin and pressure, Pres.
   *  
   *
   *  Input
   *   TKelvin = Temperature in Kelvin
   *   pres = pressure is given in units specified by if__ variable.
   *
   *
   * Output
   *    return value = standard state free energy in units of Kelvin.
   */
  virtual double GStar_R_calc(int kspec, double TKelvin, double pres);
    
  /**
   *
   * G0_calc:
   *
   *  This function calculates the standard state Gibbs free energy
   *  for species, kspec, at the temperature TKelvin
   *
   *  Input
   *
   *
   * Output
   *    return value = standard state free energy in Kelvin.
   */
  virtual double G0_R_calc(int kspec, double TKelvin);

  /**
   * cpc_ts_VStar_calc:
   *
   *    This function calculates the standard state molar volume
   *  for species, kspec, at the temperature TKelvin and pressure, Pres,
   * 
   *
   *  Input
   *
   *
   * Output
   *    return value = standard state volume in cm**3 per mol.
   *                   (if__=3)                     m**3 / kmol
   */
  virtual double VolStar_calc(int kglob, double TKelvin, double Pres);

  /**
   *  This function evaluates the activity coefficient
   *  for species, kspec
   *
   *  Input
   *      kspec -> integer value of the species in the global 
   *            species list within VCS_SOLVE. Phase and local species id
   *             can be looked up within object.
   * 
   *   Note, T, P and mole fractions are obtained from the
   *   single private instance of VCS_SOLVE
   *   
   *
   *
   * Output
   *    return value = activity coefficient for species kspec
   */
  virtual double eval_ac(int kspec);

  /**
   *  Get the pointer to the vcs_VolPhase object for this species. 
   */
    
};

/* Externals for vcs_species_thermo.c */

//extern double vcs_Gxs_phase_calc(vcs_VolPhase *, double *);
//extern double vcs_Gxs_calc(int iphase);

}

#endif
