/**
 *  @file State.h Header for the class State, that manages the
 * independent variables of temperature, mass density, and species
 * mass/mole fraction that define the thermodynamic state (see \ref
 * phases and class \link Cantera::State State\endlink).
 */

/*
 *  $Date$
 *  $Revision$
 *
 *  Copyright 2001-2003 California Institute of Technology
 *  See file License.txt for licensing information
 *
 */

#ifndef CT_STATE2_H
#define CT_STATE2_H

#include "ct_defs.h"
#include "utilities.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

namespace Cantera {

    
  //! Manages the independent variables of temperature, mass density,
  //! and species mass/mole fraction that define the thermodynamic
  //! state.
  /*!
   * Class State stores just enough information about a
   * multicomponent solution to specify its intensive thermodynamic
   * state.  It stores values for the temperature, mass density, and
   * an array of species mass fractions. It also stores an array of
   * species molecular weights, which are used to convert between
   * mole and mass representations of the composition. These are the
   * \e only properties of the species that class State knows about.
   * For efficiency in mass/mole conversion, the vector of mass
   * fractions divided by molecular weight \f$ Y_k/M_k \f$ is also
   * stored.
   *
   * Class State is not usually used directly in application
   * programs. Its primary use is as a base class for class
   * Phase. Class State has no virtual methods, and none of its
   * methods are meant to be overloaded. However, this is one exception.
   *  If the phase is incompressible, then the density must be replaced
   *  by the pressure as the independent variable. In this case, functions
   *  such as setMassFraction within the class %State must actually now
   *  calculate the density (at constant T and P) instead of leaving
   *  it alone as befits an independent variable. Threfore, these type
   *  of functions are virtual functions and need to be overloaded 
   *  for incompressible phases. Note, for almost incompressible phases
   *  (or phases which utilize standard states based on a T and P) this
   *  may be advantageous as well, and they need to overload these functions
   *  too. 
   *
   * @ingroup phases
   */
  class State {

  public:

    /**
     * Constructor. 
     */
    State();
            
    /**
     * Destructor. Since no memory is allocated by methods of this
     * class, the destructor does nothing.
     */
    virtual ~State();

    /**
     * Copy Constructor for the State Class
     *
     * @param right   Reference to the class to be copied.
     */
    State(const State& right);

    /**
     * Assignment operator for the state class.
     *
     * @param right   Reference to the class to be copied.
     */
    State& operator=(const State& right);


    /// @name Species Information
    ///
    /// The only thing class State knows about the species is their
    /// molecular weights.
    //@{        

    /// Return a read-only reference to the array of molecular
    /// weights.
    const array_fp& molecularWeights() const { return m_molwts; }


    //@}
    /// @name Composition
    //@{

	
    //! Get the species mole fraction vector.
    /*!
     * @param x On return, x contains the mole fractions. Must have a
     *          length greater than or equal to the number of species.
     */
    void getMoleFractions(doublereal* const x) const;


    //! The mole fraction of species k.
    /*!
     *   If k is ouside the valid
     *   range, an exception will be thrown. Note that it is
     *   somewhat more efficent to call getMoleFractions if the
     *   mole fractions of all species are desired.
     *   @param k species index
     */
    doublereal moleFraction(const int k) const;
    
    //! Set the mole fractions to the specified values, and then 
    //! normalize them so that they sum to 1.0.
    /*!
     * @param x Array of unnormalized mole fraction values (input). 
     *          Must have a length greater than or equal to the number of
     *          species, m_kk. There is no restriction
     *          on the sum of the mole fraction vector. Internally,
     *          the State object will normalize this vector before
     *          storring its contents.
     */
    virtual void setMoleFractions(const doublereal* const x);

    /**
     * Set the mole fractions to the specified values without
     * normalizing. This is useful when the normalization
     * condition is being handled by some other means, for example
     * by a constraint equation as part of a larger set of
     * equations.
     *
     * @param x  Input vector of mole fractions.
     *           Length is m_kk.
     */
    virtual void setMoleFractions_NoNorm(const doublereal* const x);

    //! Get the species mass fractions.  
    /*!
     * @param y On return, y contains the mass fractions. Array \a y must have a length
     *          greater than or equal to the number of species.
     */
    void getMassFractions(doublereal* const y) const;

    //! Mass fraction of species k. 
    /*!
     *  If k is outside the valid range, an exception will be thrown. Note that it is
     *  somewhat more efficent to call getMassFractions if the mass fractions of all species are desired.
     * 
     * @param k    species index
     */
    doublereal massFraction(const int k) const;

    //! Set the mass fractions to the specified values, and then 
    //! normalize them so that they sum to 1.0.
    /*!
     * @param y  Array of unnormalized mass fraction values (input). 
     *           Must have a length greater than or equal to the number of species.
     *           Input vector of mass fractions. There is no restriction
     *           on the sum of the mass fraction vector. Internally,
     *           the State object will normalize this vector before
     *           storring its contents.
     *           Length is m_kk.
     */
    virtual void setMassFractions(const doublereal* const y);

    //! Set the mass fractions to the specified values without normalizing.
    /*!
     * This is useful when the normalization
     * condition is being handled by some other means, for example
     * by a constraint equation as part of a larger set of equations.
     *
     * @param y  Input vector of mass fractions.
     *           Length is m_kk.
     */
    virtual void setMassFractions_NoNorm(const doublereal* const y);

    /**
     * Get the species concentrations (kmol/m^3).  @param c On
     * return, \a c contains the concentrations for all species.
     * Array \a c must have a length greater than or equal to the
     * number of species.
     */
    void getConcentrations(doublereal* const c) const;

    /**
     * Concentration of species k. If k is outside the valid
     * range, an exception will be thrown.
     *
     * @param  k Index of species
     */
    doublereal concentration(const int k) const;
    
    //! Set the concentrations to the specified values within the
    //! phase. 
    /*!
     * We set the concentrations here and therefore we set the
     * overall density of the phase. We hold the temperature constant
     * during this operation. Therefore, we have possibly changed
     * the pressure of the phase by calling this routine.
     *
     * @param conc The input vector to this routine is in dimensional
     *          units. For volumetric phases c[k] is the
     *          concentration of the kth species in kmol/m3.
     *          For surface phases, c[k] is the concentration
     *          in kmol/m2. The length of the vector is the number
     *          of species in the phase.
     */
    virtual void setConcentrations(const doublereal* const conc);

    //! Returns a read-only pointer to the start of the
    //! massFraction array
    /*!
     *  The pointer returned is readonly
     *  @return  returns a pointer to a vector of doubles of length m_kk.
     */
    const doublereal* massFractions() const {
      return &m_y[0]; 
    }

    /**
     * Returns a read-only pointer to the start of the
     * moleFraction/MW array.  This array is the array of mole
     * fractions, each divided by the mean molecular weight.
     */
    const doublereal* moleFractdivMMW() const;

    //@}

    /// @name Mean Properties
    //@{
    /**
     * Evaluate the mole-fraction-weighted mean of Q:
     * \f[ \sum_k X_k Q_k. \f]
     * Array Q should contain pure-species molar property 
     * values.
     *
     * @param Q input vector of length m_kk that is to be averaged.
     * @return
     *   mole-freaction-weighted mean of Q
     */
    doublereal mean_X(const doublereal* const Q) const;

    /**
     * Evaluate the mass-fraction-weighted mean of Q:
     * \f[ \sum_k Y_k Q_k \f]
     *
     * @param Q  Array Q contains a vector of species property values in mass units.
     * @return
     *     Return value containing the  mass-fraction-weighted mean of Q.
     */        
    doublereal mean_Y(const doublereal* const Q) const;

    /**
     * The mean molecular weight. Units: (kg/kmol)
     */
    doublereal meanMolecularWeight() const {
      return m_mmw; 
    }

    //! Evaluate \f$ \sum_k X_k \log X_k \f$.
    /*!
     * @return
     *     returns the indicated sum. units are dimensionless.
     */
    doublereal sum_xlogx() const;

    //! Evaluate \f$ \sum_k X_k \log Q_k \f$.
    /*!
     *  @param Q Vector of length m_kk to take the log average of
     *  @return Returns the indicated sum.
     */
    doublereal sum_xlogQ(doublereal* const Q) const;
    //@}

    /// @name Thermodynamic Properties
    /// Class State only stores enough thermodynamic data to
    /// specify the state. In addition to composition information, 
    /// it stores the temperature and
    /// mass density. 
    //@{

    //! Temperature (K).
    /*!
     * @return  Returns the temperature of the phase
     */
    doublereal temperature() const { 
      return m_temp;
    }

    //! Density (kg/m^3).
    /*!
     * @return  Returns the density of the phase
     */
    virtual doublereal density() const { 
      return m_dens;
    }

    //! Molar density (kmol/m^3).
    /*!
     * @return  Returns the molar density of the phase
     */
    doublereal molarDensity() const;

    //! Molar volume (m^3/kmol).
    /*!
     * @return  Returns the molar volume of the phase
     */
    doublereal molarVolume() const;

    //! Set the internally storred density (kg/m^3) of the phase
    /*!
     * Note the density of a phase is an indepedent variable.
     * 
     * @param density Input density (kg/m^3).
     */
    virtual void setDensity(const doublereal density) {
      m_dens = density;
    }

    //! Set the internally storred molar density (kmol/m^3) of the phase.
    /*!
     * @param molarDensity   Input molar density (kmol/m^3).
     */
    virtual void setMolarDensity(const doublereal molarDensity);

    //! Set the temperature (K).
    /*!
     * This function sets the internally storred temperature of the phase.
     *
     * @param temp Temperature in kelvin
     */
    virtual void setTemperature(const doublereal temp) {
      m_temp = temp;
    }
    //@}

    //! True if the number species has been set
    bool ready() const;

    //! Every time the mole fractions have changed, this routine
    //! will increment the stateMFNumber
    /*!
     *  @param forceChange If this is true then the stateMFNumber always
     *                     changes. This defaults to false.
     */
    void stateMFChangeCalc(bool forceChange = false);

    //! Return the state number
    int stateMFNumber() const;

  protected:

    /**
     * @internal 
     * Initialize. Make a local copy of the vector of
     * molecular weights, and resize the composition arrays to
     * the appropriate size. The only information an instance of
     * State has about the species is their molecular weights. 
     *
     * @param mw Vector of molecular weights of the species.
     */
    void init(const array_fp& mw); //, density_is_independent = true);
      
    /**
     * m_kk is the number of species in the phase
     */
    int m_kk;

    //! Set the molecular weight of a single species to a given value
    /*!
     * @param k       id of the species
     * @param mw      Molecular Weight (kg kmol-1)
     */
    void setMolecularWeight(const int k, const double mw) {
      m_molwts[k] = mw;
      m_rmolwts[k] = 1.0/mw;
    }

  private:

    /**
     * Temperature. This is an independent variable 
     * units = Kelvin
     */
    doublereal m_temp;

    /**
     * Density.   This is an independent variable except in
     *            the incompressible degenerate case. Thus,
     *            the pressure is determined from this variable
     *            not the other way round.
     * units = kg m-3
     */
    doublereal m_dens;

    /**
     * m_mmw is the mean molecular weight of the mixture
     * (kg kmol-1)
     */
    doublereal m_mmw;
        
    /**
     *  m_ym[k] = mole fraction of species k divided by the
     *            mean molecular weight of mixture.
     */
    mutable array_fp m_ym;

    /**
     * m_y[k]  = mass fraction of species k
     */
    mutable array_fp m_y;

    /**
     * m_molwts[k] = molecular weight of species k (kg kmol-1)
     */
    array_fp m_molwts;

    /**
     *  m_rmolwts[k] = inverse of the molecular weight of species k
     *  units = kmol kg-1.
     */
    array_fp m_rmolwts;

    //! State Change variable
    /*!
     * Whenever the mole fraction vector changes, this int is 
     * incremented.
     */
    int m_stateNum;

  };

  //! Return the State Mole Fraction Number
  inline int State::stateMFNumber() const {
    return m_stateNum;
  }

}

#endif
