/**
 * @file InterfaceKinetics.h
 *
 * @ingroup chemkinetics
 */
// Copyright 2001  California Institute of Technology

#ifndef CT_IFACEKINETICS_H
#define CT_IFACEKINETICS_H

#include <fstream>
#include <map>

#include "mix_defs.h"
#include "Kinetics.h"

#include "utilities.h"
#include "RateCoeffMgr.h"
#include "ReactionStoichMgr.h"

#include <cmath>
#include <cstdlib>

namespace Cantera {

    // forward references

  class ReactionData;
  class InterfaceKineticsData;
  class ThermoPhase;
  class SurfPhase;
  class ImplicitSurfChem;


  //! This class holds mechanism-specific data.
  /*!
   *
   */
  class InterfaceKineticsData {
  public:
    InterfaceKineticsData() :
      m_logp0(0.0),
      m_logc0(0.0),
      m_ROP_ok(false),
      m_temp(0.0), 
      m_logtemp(0.0)
    {}
 
    //! Virtual destructor
    virtual ~InterfaceKineticsData() {
    }

    doublereal m_logp0;
    doublereal m_logc0;
    array_fp m_ropf;
    array_fp m_ropr;
    array_fp m_ropnet;
   
    bool m_ROP_ok;

    //! Current temperature of the data
    doublereal m_temp;
    //! Current log of the temperature
    doublereal m_logtemp;
    vector_fp m_rfn;
    vector_fp m_rkcn;
  };


  ///
  ///  A kinetics manager for heterogeneous reaction mechanisms. The
  ///  reactions are assumed to occur at a 2D interface between two
  ///  3D phases.
  ///
  ///  @ingroup chemkinetics
  class InterfaceKinetics : public Kinetics {

  public:

   
    //! Constructor
    /*!
     * @param thermo The optional parameter may be used to initialize
     *               the object with one ThermoPhase object.
     *               HKM Note -> Since the interface kinetics
     *               object will probably require multiple thermophase
     *               objects, this is probably not a good idea
     *               to have this parameter.
     */
    InterfaceKinetics(thermo_t* thermo = 0);


    /// Destructor.
    virtual ~InterfaceKinetics();

    //! Copy Constructor for the %Kinetics object.
    /*!
     * Currently, this is not fully implemented. If called it will
     * throw an exception.
     */
    InterfaceKinetics(const InterfaceKinetics &right);

    //! Assignment operator
    /*!
     *  This is NOT a virtual function.
     *
     * @param right    Reference to %Kinetics object to be copied into the
     *                 current one.
     */
    InterfaceKinetics& operator=(const InterfaceKinetics &right);


    //! Duplication routine for objects which inherit from
    //! Kinetics
    /*!
     *  This virtual routine can be used to duplicate %InterfaceKinetics objects
     *  inherited from %Kinetics even if the application only has
     *  a pointer to %Kinetics to work with.
     *
     *  These routines are basically wrappers around the derived copy
     *  constructor.
     */
    virtual Kinetics *duplMyselfAsKinetics() const;

    //! Return the ID of the kinetics object
    virtual int ID() const;

    //! Retunr the type of the kinetics object
    virtual int type() const;

    //! Set the electric potential in the nth phase
    /*!
     * @param n phase Index in this kinetics object.
     * @param V Electric potential (volts)
     */
    void setElectricPotential(int n, doublereal V);


    ///
    ///  @name Reaction Rates Of Progress
    ///
    //@{

    //! Return the forward rates of progress for each reaction
    /*!
     * @param fwdROP vector of rates of progress.
     *        length = number of reactions, Units are kmol m-2 s-1.
     */
    virtual void getFwdRatesOfProgress(doublereal* fwdROP) {
      updateROP();
      std::copy(m_kdata->m_ropf.begin(), m_kdata->m_ropf.end(), fwdROP);
    }

    //! Return the reverse rates of progress for each reaction
    /*!
     * @param revROP vector of rates of progress.
     *        length = number of reactions, Units are kmol m-2 s-1.
     */
    virtual void getRevRatesOfProgress(doublereal* revROP) {
      updateROP();
      std::copy(m_kdata->m_ropr.begin(), m_kdata->m_ropr.end(), revROP);
    }

    //! Return the net rates of progress for each reaction
    /*!
     * @param netROP vector of rates of progress.
     *        length = number of reactions, Units are kmol m-2 s-1.
     */
    virtual void getNetRatesOfProgress(doublereal* netROP) {
      updateROP();
      std::copy(m_kdata->m_ropnet.begin(), m_kdata->m_ropnet.end(), netROP);
    }


    //! Get the equilibrium constants of all reactions, whether
    //! the reaction is reversible or not.
    /*!
     *  @param kc   Returns the concentration equation constant for the reaction.
     *              Length is the number of reactions
     */
    virtual void getEquilibriumConstants(doublereal* kc);

    void getExchangeCurrentQuantities();


    virtual void getDeltaGibbs( doublereal* deltaG);

    /**
     * Return the vector of values for the reactions change in
     * enthalpy.
     * These values depend upon the concentration
     * of the solution.
     *
     *  units = J kmol-1
     */
    virtual void getDeltaEnthalpy( doublereal* deltaH);
      
    //! Return the vector of values for the change in
    //! entropy due to each reaction
    /*!
     * These values depend upon the concentration
     * of the solution.
     *
     *  units = J kmol-1 Kelvin-1
     *
     * @param deltaS vector of Enthalpy changes 
     *        Length = m_ii, number of reactions
     *         
     */
    virtual void getDeltaEntropy(doublereal* deltaS);


    //! Return the vector of values for the reaction
    //! standard state gibbs free energy change.
    /*!
     *  These values don't depend upon the concentration
     *  of the solution.
     *
     *  @param deltaG vector of rxn SS free energy changes
     *                units = J kmol-1
     */
    virtual void getDeltaSSGibbs(doublereal* deltaG);
      
    //!   Return the vector of values for the change in the
    //!  standard state enthalpies of reaction.
    /*!
     * These values don't depend upon the concentration
     * of the solution.
     *
     *  @param deltaH vector of rxn SS enthalpy changes
     *            units = J kmol-1
     */
    virtual void getDeltaSSEnthalpy(doublereal* deltaH);
      
    //!   Return the vector of values for the change in the
    //!   standard state entropies for each reaction.
    /*!
     *  These values don't depend upon the concentration
     *  of the solution.
     *
     *  @param deltaS vector of rxn SS entropy changes
     *  units = J kmol-1 Kelvin-1
     */
    virtual void getDeltaSSEntropy(doublereal* deltaS);
      

    //@}
    /**
     * @name Species Production Rates
     */
    //@{
      
      
    //! Returns the Species creation rates [kmol/m^2/s]. 
    /*!
     *   Return the species
     * creation rates in array cdot, which must be
     * dimensioned at least as large as the total number of
     * species in all phases of the kinetics
     * model
     *  
     *  @param cdot Vector containing creation rates.
     *              length = m_kk. units = kmol/m^2/s
     */
    virtual void getCreationRates(doublereal* cdot);
      
    //!  Return the Species destruction rates [kmol/m^2/s].
    /*!
     *  Return the species destruction rates in array ddot, which must be
     *  dimensioned at least as large as the total number of
     *  species in all phases of the kinetics model
     *
     *  @param ddot Vector containing destruction rates.
     *              length = m_kk. units = kmol/m^2/s
     */
    virtual void getDestructionRates(doublereal* ddot);
      
    //! Return the species net production rates [kmol/m^2/s]. 
    /*!
     * Species net production rates [kmol/m^2/s]. Return the species
     * net production rates (creation - destruction) in array
     * wdot, which must be dimensioned at least as large as the
     * total number of species in all phases of the kinetics
     * model
     *
     * @param net  Vector of species production rates.
     *             units kmol m-d s-1, where d is dimension.
     */
    virtual void getNetProductionRates(doublereal* net);
      
    //@}
    /**
     * @name Reaction Mechanism Informational Query Routines
     */
    //@{

    /**
     * Stoichiometric coefficient of species k as a reactant in
     * reaction i.
     */
    virtual doublereal reactantStoichCoeff(int k, int i) const {
      return m_rrxn[k][i];
    }

    /**
     * Stoichiometric coefficient of species k as a product in
     * reaction i.
     */
    virtual doublereal productStoichCoeff(int k, int i) const {
      return m_prxn[k][i];
    }

    /**
     * Flag specifying the type of reaction. The legal values and
     * their meaning are specific to the particular kinetics
     * manager.
     */
    virtual int reactionType(int i) const {
      return m_index[i].first;
    }

    //! Return the charge transfer rxn Beta parameter for the ith reaction
    /*!
     *  Returns the beta parameter for a charge transfer reaction. This 
     *  parameter is not important for non-charge transfer reactions.
     *  Note, the parameter defaults to zero. However, a value of 0.5
     *  should be supplied for every charge transfer reaction if
     *  no information is known, as a value of 0.5 pertains to a
     *  symmetric transition state. The value can vary between 0 to 1.
     *
     *
     *  @param irxn Reaction number in the kinetics mechanism
     *
     *  @return
     *    Beta parameter. This defaults to zero, even for charge transfer
     *    reactions.
     */
    doublereal electrochem_beta(int irxn) const;

    /**
     * True if reaction i has been declared to be reversible. If
     * isReversible(i) is false, then the reverse rate of progress
     * for reaction i is always zero.
     */
    virtual bool isReversible(int i) {
      if (std::find(m_revindex.begin(), m_revindex.end(), i)
	  < m_revindex.end()) return true;
      else return false;
    }

    /**
     * Return a string representing the reaction.
     */
    virtual std::string reactionString(int i) const {
      return m_rxneqn[i];
    }


    virtual void getFwdRateConstants(doublereal* kfwd);
    virtual void getRevRateConstants(doublereal* krev,
				     bool doIrreversible = false);


    virtual void getActivationEnergies(doublereal *E);

    //@}
    /**
     * @name Reaction Mechanism Construction
     */
    //@{
      
    //!  Add a phase to the kinetics manager object. 
    /*!
     * This must be done before the function init() is called or 
     * before any reactions are input.
     *
     * This function calls the Kinetics operator addPhase.
     * It also sets the following functions
     *
     *        m_phaseExists[]
     *
     * @param thermo    Reference to the ThermoPhase to be added.
     */
    virtual void addPhase(thermo_t& thermo);
	
    //! Prepare the class for the addition of reactions. 
    /*!
     *  This function must be called after instantiation of the class, but before
     *  any reactions are actually added to the mechanism.
     *  This function calculates m_kk the number of species in all
     *  phases participating in the reaction mechanism. We don't know
     *  m_kk previously, before all phases have been added.
     */
    virtual void init();

    //!  Add a single reaction to the mechanism.
    /*!
     * @param r Reference to a ReactionData object containing all of
     *          the info needed to describe the reaction.
     */
    virtual void addReaction(const ReactionData& r);

        
    //! Finish adding reactions and prepare for use.
    /*!
     * This function
     * must be called after all reactions are entered into the mechanism
     * and before the mechanism is used to calculate reaction rates.
     */
    virtual void finalize();

    virtual bool ready() const;

    //! Internal routine that updates the Rates of Progress of the reactions
    /*!
     *  This is actually the guts of the functionality of the object
     */
    void updateROP();


   
    //! Update properties that depend on temperature
    /*!
     *  This is called to update all of the properties that depend on temperature
     *
     *  Current objects that this function updates
     *       m_kdata->m_logtemp 
     *       m_kdata->m_rfn
     *       m_rates.
     *       updateKc();
     */
    void _update_rates_T();

    void _update_rates_phi();
    void _update_rates_C();
      
    //! Advance the surface coverages in time
    /*!
     * This method carries out a time-accurate advancement of the
     * surface coverages for a specified amount of time.
     *
     *  \f[
     *    \dot {\theta}_k = \dot s_k (\sigma_k / s_0)
     *  \f]
     *
     *
     * @param tstep  Time value to advance the surface coverages
     */
    void advanceCoverages(doublereal tstep);

    //! Solve for the pseudo steady-state of the surface problem
    /*!
     * Solve for the steady state of the surface problem. 
     * This is the same thing as the advanceCoverages() function,
     * but at infinite times.
     *
     * Note, a direct solve is carried out under the hood here,
     * to reduce the computational time.
     *
     * @param ifuncOverride 4 values are possible
     *                    1  SFLUX_INITIALIZE
     *                    2  SFLUX_RESIDUAL
     *                    3  SFLUX_JACOBIAN
     *                    4  SFLUX_TRANSIENT
     *         The default is -1, which means that the program
     *         will decide.
     * @param timeScaleOverride When a psuedo transient is
     *             selected this value can be used to override
     *             the default time scale for integration which
     *             is one.
     *             When SFLUX_TRANSIENT is used, this is equal to the
     *             time over which the equations are integrated.
     *             When SFLUX_INITIALIZE is used, this is equal to the
     *             time used in the initial transient algorithm,
     *             before the equation system is solved directly.
     */
    void solvePseudoSteadyStateProblem(int ifuncOverride = -1,
				       doublereal timeScaleOverride = 1.0);

    void setIOFlag(int ioFlag);

    void checkPartialEquil();


    int reactionNumber() const { return m_ii;}

    void addElementaryReaction(const ReactionData& r);
    void addGlobalReaction(const ReactionData& r);
    void installReagents(const ReactionData& r);
  
    void updateKc();

    //! Write values into m_index
    /*!
     * @param rxnNumber reaction number
     * @param type      reaction type
     * @param loc       location ??
     */
    void registerReaction(int rxnNumber, int type, int loc) {
      m_index[rxnNumber] = std::pair<int, int>(type, loc);
    }

    //! Apply corrections for interfacial charge transfer reactions
    /*!
     * For reactions that transfer charge across a potential difference,
     * the activation energies are modified by the potential difference.
     * (see, for example, ...). This method applies this correction.
     *
     * @param kf  Vector of forward reaction rate constants on which to have
     *            the correction applied
     */
    void applyButlerVolmerCorrection(doublereal* const kf);

    //! When an electrode reaction rate is optionally specified in terms of its
    //! exchange current density, extra vectors need to be precalculated
    /*!
     *
     */
    void applyExchangeCurrentDensityFormulation(doublereal* const kfwd);

    //! Set the existence of a phase in the reaction object
    /*!
     *    Tell the kinetics object whether a phase in the object exists.
     *    This is actually an extrinsic specification that must be carried out on top of the
     *    intrinsic calculation of the reaction rate
     *
     *  @param iphase  Index of the phase. This is the order within the internal thermo vector object
     *  @param exists  Boolean indicating whether the phase exists or not
     */
    void setPhaseExistence(const int iphase, const bool exists);
 
  protected:
     
    //! Temporary work vector of length m_kk
    vector_fp m_grt;

    //! m_kk is the number of species in all of the phases
    //! that participate in this kinetics mechanism.
    int m_kk;

    //! List of reactions numbers which are reversible reactions
    /*!
     *  This is a vector of reaction numbers. Each reaction
     *  in the list is reversible.
     *  Length = number of reversible reactions
     */
    vector_int m_revindex;

    Rate1<SurfaceArrhenius>                    m_rates;
    bool                                m_redo_rates;

    /**
     * Vector of information about reactions in the
     * mechanism.
     * The key is the reaction index (0 < i < m_ii).
     * The first pair is the reactionType of the reaction.
     * The second pair is ...
     */
    mutable std::map<int, std::pair<int, int> >   m_index;

    //! Vector of irreversible reaction numbers
    /*!
     * vector containing the reaction numbers of irreversible
     * reactions.
     */
    std::vector<size_t> m_irrev;

    //! Stoichiometric manager for the reaction mechanism
    /*!
     *    This is the manager for the kinetics mechanism that
     *    handles turning reaction extents into species
     *    production rates and also handles turning thermo
     *    properties into reaction thermo properties.
     */
    ReactionStoichMgr m_rxnstoich;

    //! Number of irreversible reactions in the mechanism
    size_t m_nirrev;

    //! Number of reversible reactions in the mechanism
    size_t m_nrev;

  	
    //!  m_rrxn is a vector of maps, containing the reactant
    //!  stochiometric coefficient information
    /*!
     *   m_rrxn has a length
     *  equal to the total number of species in the kinetics
     *  object. For each species, there exists a map, with the
     *  reaction number being the key, and the
     *  reactant stoichiometric coefficient for the species being the value.
     *  HKM -> mutable because search sometimes creates extra
     *         entries. To be fixed in future...
     */
    mutable std::vector<std::map<int, doublereal> >     m_rrxn;
      
    //!  m_prxn is a vector of maps, containing the reactant
    //!  stochiometric coefficient information
    /**
     *  m_prxn is a vector of maps. m_prxn has a length
     *  equal to the total number of species in the kinetics
     *  object. For each species, there exists a map, with the
     *  reaction number being the key, and the
     *  product stoichiometric coefficient for the species being the value.
     */
    mutable std::vector<std::map<int, doublereal> >     m_prxn;

    //! String expression for each rxn
    /*!
     * Vector of strings of length m_ii, the number of 
     * reactions, containing the
     * string expressions for each reaction
     * (e.g., reactants <=> product1 + product2)
     */
    std::vector<std::string> m_rxneqn;

    /**
     * Temporary data storage used in calculating the rates of
     * of reactions.
     */
    InterfaceKineticsData* m_kdata;

    //! an array of generalized concentrations for each species
    /*!
     * An array of generalized concentrations
     * \f$ C_k \f$ that are defined such that \f$ a_k = C_k /
     * C^0_k, \f$ where \f$ C^0_k \f$ is a standard concentration/
     * These generalized concentrations are used
     * by this kinetics manager class to compute the forward and
     * reverse rates of elementary reactions. The "units" for the
     * concentrations of each phase depend upon the implementation
     * of kinetics within that phase.
     * The order of the species within the vector is based on
     * the order of listed ThermoPhase objects in the class, and the
     * order of the species within each ThermoPhase class.
     */
    vector_fp m_conc;

    //! Vector of standard state chemical potentials
    /*!
     * This vector contains a temporary vector of
     *  standard state chemical potentials
     * for all of the species in the kinetics object
     *
     * Length = m_k
     * units = J/kmol 
     */
    vector_fp m_mu0;

    //! Vector of phase electric potentials
    /*!
     * Temporary vector containing the potential of each phase
     * in the kinetics object
     *
     * length = number of phases
     * units = Volts
     */
    vector_fp m_phi;

    //! Vector of potential energies due to Voltages
    /*!
     *  Length is the number of species in kinetics mech. It's
     *  used to store the potential energy due to the voltage.
     */
    vector_fp m_pot;
      
    //! Vector temporary
    /*!
     * Length is number of reactions. it's used to store the
     * voltage contribution to the activation energy.
     */
    vector_fp m_rwork;

    //! Vector of raw activation energies for the reactions
    /*!
     * units are in Kelvin
     */
    vector_fp m_E;

    //! Pointer to the single surface phase
    SurfPhase* m_surf;

    //! Pointer to the Implicit surface chemistry object
    /*!
     * Note this object is owned by this InterfaceKinetics
     * object. It may only be used to solve this single
     * InterfaceKinetics objects's surface problem uncoupled
     * from other surface phases.
     */
    ImplicitSurfChem* m_integrator;

    vector_fp m_beta;

    //! Vector of reaction indexes specifying the id of the current transfer reactions
    //! in the mechanism
    /*!
     *  Vector of reaction indecices which involve current transfers. This provides
     *  an index into the m_beta array.
     *
     *        irxn = m_ctrxn[i] 
     */
    vector_int m_ctrxn;

    //! Vector of booleans indicating whether the charge transfer reaction may be 
    //! described by an exchange current density expression
    vector_int m_ctrxn_ecdf;

    vector_fp m_StandardConc;
    vector_fp m_deltaG0;
    vector_fp m_ProdStanConcReac;



    //! boolean indicating whether mechanism has been finalized
    bool m_finalized;

    //! Boolean flag indicating whether any reaction in the mechanism
    //! has a coverage dependent forward reaction rate
    /*!
     *   If this is true, then the coverage dependence is multiplied into
     *   the forward reaction rates constant
     */
    bool m_has_coverage_dependence;

    //! Boolean flag indicating whether any reaction in the mechanism
    //! has a beta electrochemical parameter.
    /*!
     *  If this is true, the the Butler-Volmer correction is applied
     *  to the forward reaction rate for those reactions.
     *
     *    fac = exp ( - beta * (delta_phi))
     */
    bool m_has_electrochem_rxns;

    //! Boolean flag indicating whether any reaction in the mechanism
    //! is described by an exchange current density expression
    /*!
     *  If this is true, the standard state gibbs free energy of the reaction and
     *  the product of the reactant standard concentrations must be precalculated
     *  in order to calculate the rate constant.
     */
    bool m_has_exchange_current_density_formulation;

    //! Int flag to indicate that some phases in the kinetics mechanism are
    //! non-existent.
    /*!
     *   We change the ROP vectors to make sure that non-existent phases are treated
     *   correctly in the kinetics operator. The value of this is equal to the number
     *   of phases which don't exist.
     */
    int m_phaseExistsCheck;

    //!  Vector of booleans indicating whether phases exist or not
    /*!
     *    Vector of booleans indicating whether a phase exists or not.
     *    We use this to set the ROP's so that unphysical things don't happen
     *
     *    length = number of phases in the object
     *    By default all phases exist.
     */
    std::vector<bool> m_phaseExists;

    std::vector<bool *> m_rxnPhaseIsReactant;
    std::vector<bool *> m_rxnPhaseIsProduct;

    int m_ioFlag;
  private:

  };
}

#endif
