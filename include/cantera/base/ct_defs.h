/**
 * @file ct_defs.h
 * This file contains definitions of terms that are used in internal
 * routines and are unlikely to need modifying (text for module physConstants (see \ref physConstants) is found here).
 * This file is included
 * in every file that is in the Cantera Namespace.
 *
 * All physical constants are stored here.
 * The module physConstants is defined here.
 */
// Copyright 2001  California Institute of Technology


#ifndef CT_DEFS_H
#define CT_DEFS_H

#include "config.h"

#include <cmath>

// STL includes
#include <vector>
#include <map>
#include <string>

// Warn about deprecated functions at compile time
#ifdef __GNUC__
#define DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(func) __declspec(deprecated) func
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED(func) func
#endif

/**
 * Namespace for the Cantera kernel.
 */
namespace Cantera
{

#undef CHEMKIN_COMPATIBILITY_MODE

//! Creates a pointer to the start of the raw data for a vector
#ifndef DATA_PTR
#define DATA_PTR(vec) &vec[0]
#endif

/*!
 * All physical constants are stored here.
 *
 * @defgroup physConstants Physical Constants
 * %Cantera uses the MKS system of units. The unit for moles
 * is defined to be the kmol. All values of physical constants
 * are consistent with the 2006 CODATA recommendations.
 * @ingroup globalData
 * @{
 */
//! Pi
const doublereal Pi = 3.1415926;
//! sqrt(Pi)
const doublereal SqrtPi = std::sqrt(Pi);


/*!
 *   @name Variations of the Gas Constant
 * %Cantera uses the MKS system of units. The unit for moles
 * is defined to be the kmol.
 */
//@{

//! Avogadro's Number
/*!
 *  Units are number/kmol
 */
const doublereal Avogadro = 6.02214179e26;

/// Universal Gas Constant. 2006 CODATA value.
const doublereal GasConstant = 8314.47215; // J/kmol/K

const doublereal logGasConstant = 9.025752908;

//! One atmosphere
/*!
 *  Units are Pa
 */
const doublereal OneAtm = 1.01325e5;

//! Universal gas constant in cal/mol/K
const doublereal GasConst_cal_mol_K = 1.987;

//! Boltzmann's constant
/*!
 *  Units are J/K
 */
const doublereal Boltzmann = GasConstant / Avogadro;

/// Planck's constant. Units of J-s
const doublereal Planck = 6.62606896e-34;           // J-s
const doublereal Planck_bar = 1.05457162853e-34;    // m2-kg/s

/// log(k/h)
const doublereal logBoltz_Planck = 23.7599032;  // ln(k_B/h)
/// Stefan-Boltzmann constant
const doublereal StefanBoltz = 5.6704004e-8;

//@}
/// @name Electron Properties
//@{
const doublereal ElectronCharge = 1.60217648740e-19; // C
const doublereal ElectronMass = 9.1093821545e-31; // kg
const doublereal Faraday = ElectronCharge* Avogadro;
//@}

/// @name Electromagnetism
/// %Cantera uses the MKS unit system.
//@{

/// Permittivity of free space \f$ \epsilon_0 \f$ in F/m.
const doublereal epsilon_0 = 8.85417817e-12;  // Farads/m = C^2/N/m^2

/// Permeability of free space \f$ \mu_0 \f$ in N/A^2.
const doublereal permeability_0 = 4.0e-7*Pi; // N/A^2

/// Speed of Light (m/s).
const doublereal lightSpeed = 1.0/std::sqrt(epsilon_0* permeability_0);

//@}
//@}

/*!
 * @name Thermodynamic Equilibrium Constraints
 * Integer numbers representing pairs of thermodynamic variables
 * which are held constant during equilibration.
 */
//@{
const int TV = 100, HP = 101, SP = 102, PV = 103, TP = 104, UV = 105,
          ST = 106, SV = 107, UP = 108, VH = 109, TH = 110, SH = 111,
          PX = 112, TX = 113;
const int VT = -100, PH = -101, PS = -102, VP = -103, PT = -104,
          VU = -105, TS = -106, VS = -107, PU = -108, HV = -109,
          HT = -110, HS = -111, XP = -112, XT = -113;
//@}

//! 1/3
const doublereal OneThird = 1.0/3.0;
//! 5/16
const doublereal FiveSixteenths = 5.0/16.0;
//! sqrt(10)
const doublereal SqrtTen = std::sqrt(10.0);
//! sqrt(8)
const doublereal SqrtEight = std::sqrt(8.0);
//! sqrt(2)
const doublereal SqrtTwo = std::sqrt(2.0);

//! smallest number to compare to zero.
const doublereal SmallNumber = 1.e-300;
//! largest number to compare to inf.
const doublereal BigNumber = 1.e300;
//! largest x such that exp(x) is valid
const doublereal MaxExp = 690.775527898;

//! Fairly random number to be used to initialize variables against
//! to see if they are subsequently defined.
const doublereal Undef = -999.1234;
//! Small number to compare differences of mole fractions against.
const doublereal Tiny = 1.e-20;

//! Map connecting a string name with a double.
/*!
 * This is used mostly to assign concentrations and mole fractions
 * to species.
 */
typedef std::map<std::string, doublereal> compositionMap;
//! Turn on the use of stl vectors for the basic array type within cantera
//! Vector of doubles.
typedef std::vector<double>        vector_fp;
//! Vector of ints
typedef std::vector<int>           vector_int;

//! A grouplist is a vector of groups of species
typedef std::vector<std::vector<size_t> > grouplist_t;

//! index returned by functions to indicate "no position"
const size_t npos = static_cast<size_t>(-1);

}  // namespace

#endif
