/**
 *  @file PseudoBinaryVPSSTP.h
 *   Header for intermediate ThermoPhase object for phases which
 *   employ gibbs excess free energy based formulations
 *  (see \ref thermoprops 
 * and class \link Cantera::gibbsExcessVPSSTP gibbsExcessVPSSTP\endlink).
 *
 * Header file for a derived class of ThermoPhase that handles
 * variable pressure standard state methods for calculating
 * thermodynamic properties that are further based upon activities
 * based on the molality scale.  These include most of the methods for
 * calculating liquid electrolyte thermodynamics.
 */
/*
 * Copywrite (2006) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */
/*
 *  $Id$
 */

#ifndef CT_PSEUDOBINARYVPSSTP_H
#define CT_PSEUDOBINARYVPSSTP_H

#include "GibbsExcessVPSSTP.h"

namespace Cantera {

  /**
   * @ingroup thermoprops
   */

  /*!
   *  PseudoBinaryVPSSTP is a derived class of ThermoPhase
   *  GibbsExcessVPSSTP that handles
   *  variable pressure standard state methods for calculating
   *  thermodynamic properties that are further based on
   *  expressing the Excess Gibbs free energy as a function of
   *  the mole fractions (or pseudo mole fractions) of consitituents.
   *  This category is the workhorse for describing molten salts, 
   *  solid-phase mixtures of semiconductors, and mixtures of miscible
   *  and semi-miscible compounds.
   *
   * It includes 
   *   . regular solutions
   *   . Margueles expansions
   *   . NTRL equation
   *   . Wilson's equation
   *   . UNIQUAC equation of state.
   *
   *  This class adds additional functions onto the %ThermoPhase interface
   *  that handles the calculation of the excess Gibbs free energy. The %ThermoPhase
   *  class includes a member function, ThermoPhase::activityConvention() 
   *  that indicates which convention the activities are based on. The
   *  default is to assume activities are based on the molar convention.
   *  That default is used here. 
   *
   *  All of the Excess Gibbs free energy formulations in this area employ
   *  symmetrical formulations. 
   *
   *  This layer will massage the mole fraction vector to implement
   *  cation and anion based mole numbers in an optional manner
   *
   *  The way that it collects the cation and anion based mole numbers
   *  is via holding two extra ThermoPhase objects. These
   *  can include standard states for salts. 
   *        
   *
   */
  class PseudoBinaryVPSSTP : public GibbsExcessVPSSTP  {

  public:
        
    /// Constructors 
    /*!
     * This doesn't do much more than initialize constants with
     * default values for water at 25C. Water molecular weight 
     * comes from the default elements.xml file. It actually
     * differs slightly from the IAPWS95 value of 18.015268. However,
     * density conservation and therefore element conservation
     * is the more important principle to follow.
     */
    PseudoBinaryVPSSTP();

    //! Copy constructor
    /*!
     *  Note this stuff will not work until the underlying phase
     *  has a working copy constructor
     *
     * @param b class to be copied
     */
    PseudoBinaryVPSSTP(const  PseudoBinaryVPSSTP&b);

    /// Assignment operator
    /*!
     *
     * @param b class to be copied.
     */
    PseudoBinaryVPSSTP& operator=(const PseudoBinaryVPSSTP&b);

    /// Destructor. 
    virtual ~PseudoBinaryVPSSTP();

    //! Duplication routine for objects which inherit from  ThermoPhase.
    /*!
     *  This virtual routine can be used to duplicate thermophase objects
     *  inherited from ThermoPhase even if the application only has
     *  a pointer to ThermoPhase to work with.
     */
    virtual ThermoPhase *duplMyselfAsThermoPhase() const;
    
    /**
     *   
     * @name  Utilities  
     * @{
     */

   
    //! Equation of state type flag.
    /*!
     * The ThermoPhase base class returns
     * zero. Subclasses should define this to return a unique
     * non-zero value. Known constants defined for this purpose are
     * listed in mix_defs.h. The MolalityVPSSTP class also returns
     * zero, as it is a non-complete class.
     */
    virtual int eosType() const;

   

    /**
     * @} 
     * @name  Molar Thermodynamic Properties 
     * @{
     */


    /**
     * @}
     * @name Utilities for Solvent ID and Molality
     * @{
     */


 

    /**
     * @}
     * @name Mechanical Properties
     * @{
     */

    /**
     * @} 
     * @name Potential Energy
     * 
     * Species may have an additional potential energy due to the
     * presence of external gravitation or electric fields. These
     * methods allow specifying a potential energy for individual
     * species.
     * @{
     */

    /**
     * @}
     * @name Activities, Standard States, and Activity Concentrations
     *
     * The activity \f$a_k\f$ of a species in solution is
     * related to the chemical potential by \f[ \mu_k = \mu_k^0(T)
     * + \hat R T \log a_k. \f] The quantity \f$\mu_k^0(T,P)\f$ is
     * the chemical potential at unit activity, which depends only
     * on temperature and pressure.
     * @{
     */

  
 

    /**
     * The standard concentration \f$ C^0_k \f$ used to normalize
     * the generalized concentration. In many cases, this quantity
     * will be the same for all species in a phase - for example,
     * for an ideal gas \f$ C^0_k = P/\hat R T \f$. For this
     * reason, this method returns a single value, instead of an
     * array.  However, for phases in which the standard
     * concentration is species-specific (e.g. surface species of
     * different sizes), this method may be called with an
     * optional parameter indicating the species.
     *
     * @param k species index. Defaults to zero.
     */
    virtual doublereal standardConcentration(int k=0) const;

    /**
     * Returns the natural logarithm of the standard 
     * concentration of the kth species
     *
     * @param k  species index
     */
    virtual doublereal logStandardConc(int k=0) const;

 
   
 
    //@}
    /// @name  Partial Molar Properties of the Solution 
    //@{


    /**
     * Get the species electrochemical potentials. 
     * These are partial molar quantities.
     * This method adds a term \f$ Fz_k \phi_k \f$ to the 
     * to each chemical potential.
     *
     * Units: J/kmol
     *
     * @param mu     output vector containing the species electrochemical potentials.
     *               Length: m_kk.
     */
    void getElectrochemPotentials(doublereal* mu) const;

 
    //@}
    /// @name  Properties of the Standard State of the Species in the Solution
    //@{

     

    //@}
    /// @name Thermodynamic Values for the Species Reference States
    //@{


    ///////////////////////////////////////////////////////
    //
    //  The methods below are not virtual, and should not
    //  be overloaded.
    //
    //////////////////////////////////////////////////////

    /**
     * @name Specific Properties
     * @{
     */


    /**
     * @name Setting the State
     *
     * These methods set all or part of the thermodynamic
     * state.
     * @{
     */

    //! Calculate pseudo binary mole fractions
    /*!
     *
     */
    virtual void calcPseudoBinaryMoleFractions() const;


    //@}

    /**
     * @name Chemical Equilibrium
     * Routines that implement the Chemical equilibrium capability
     * for a single phase, based on the element-potential method.
     * @{
     */

   

    //@}



    /// The following methods are used in the process of constructing
    /// the phase and setting its parameters from a specification in an 
    /// input file. They are not normally used in application programs.
    /// To see how they are used, see files importCTML.cpp and 
    /// ThermoFactory.cpp.


    /*!
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
    virtual void initThermo();


    /**
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
    void initThermoXML(XML_Node& phaseNode, std::string id);

 
    //! returns a summary of the state of the phase as a string
    /*!
     * @param show_thermo If true, extra information is printed out
     *                    about the thermodynamic state of the system.
     */
    virtual std::string report(bool show_thermo = true) const;


  private:
  

    //! Initialize lengths of local variables after all species have
    //! been identified.
    void initLengths();
            


  private:
    //! Error function
    /*!
     *  Print an error string and exit
     *
     * @param msg  Message to be printed
     */
    doublereal err(std::string msg) const;

  protected:

    int PBType_;

    //! Number of pseudo binary species
    int numPBSpecies_;

    //! index of special species
    int indexSpecialSpecies_;
    
    mutable std::vector<doublereal> PBMoleFractions_;

    std::vector<int> cationList_;
    int numCationSpecies_;

    std::vector<int>anionList_;
    int numAnionSpecies_;

    std::vector<int> passThroughList_;
    int numPassThroughSpecies_;
    int neutralPBindexStart;

    ThermoPhase *cationPhase_;

    ThermoPhase *anionPhase_;
   
    mutable std::vector<doublereal> moleFractionsTmp_;

  private:

  
  };

#define  PBTYPE_PASSTHROUGH        0
#define  PBTYPE_SINGLEANION        1
#define  PBTYPE_SINGLECATION       2
#define  PBTYPE_MULTICATIONANION   3



}
        
#endif





