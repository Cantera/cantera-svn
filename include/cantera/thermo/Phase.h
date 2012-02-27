/**
 *  @file Phase.h
 *
 *   Header file for class, Phase, which contains functions for
 *   setting the state of a phase, and for referencing species by
 *   name, and also contains text for the module phases (see \ref
 *   phases and class \link Cantera::Phase Phase\endlink).
 */
// Copyright 2001  California Institute of Technology

#ifndef CT_PHASE_H
#define CT_PHASE_H

#include "State.h"
#include "Constituents.h"
#include "cantera/base/vec_functions.h"

#include "cantera/base/ctml.h"

namespace Cantera
{


/**
 * @defgroup phases Models of Phases of Matter
 *
 *  These classes are used to represent the composition and state of a
 *  single phase of matter.
 *   Together  these classes form the basis for describing the species and
 *  element compositions of a phase as well as the stoichiometry
 *  of each species, and for describing the current state of the
 *  phase. They do not in themselves contain Thermodynamic equation of
 *  state information. However, they do comprise all of the necessary
 *  background functionality to support thermodynamic calculations, and the
 *  class ThermoPhase inherits from the class Phase (see \ref thermoprops).
 *
 *  Class Elements manages the elements that are part of a
 *  chemistry specification for a phase.  This class may support calculations
 *  employing Multiple phases. In this case, a single Elements object may
 *  be shared by more than one Constituents class. Reactions between
 *  the phases may then be described using stoichiometry base on the
 *  same Elements class object.
 *
 *  The member functions of class %Elements return information about
 *  the elements described in a particular instantiation of the
 *  class.
 *
 *  Class %Constituents is designed to provide information
 *  about the elements and species in a phase - names, index
 *  numbers (location in arrays), atomic or molecular weights,
 *  etc. No computations are performed by the methods of this
 *  class. The set of elements must include all those that compose
 *  the species, but may include additional elements.
 *
 *  %Constituents contains a pointer to the Elements object, and
 *  it contains wrapper functions for all of the functionality
 *  of the %Elements object, i.e., atomic weights, number and identity
 *  of the elements. %Elements may be added to a phase by using
 *  the function Constituents::addUniqueElement(). The %Elements
 *  object may be shared amongst different Phases.
 *
 *  %Constituents also contains utilities retrieving the index of
 *  a species in the phase given its name, Constituents::speciesIndex().
 *
 *  Class State manages the independent variables of temperature,
 *  mass density, and species mass/mole fraction that define the
 *  thermodynamic state.
 *
 *  Class %State stores just enough information about a
 *  multicomponent solution to specify its intensive thermodynamic
 *  state.  It stores values for the temperature, mass density, and
 *  an array of species mass fractions. It also stores an array of
 *  species molecular weights, which are used to convert between
 *  mole and mass representations of the composition. These are the
 *  \e only properties of the species that class %State knows about.
 *
 *  Class %State is not usually used directly in application
 *  programs. Its primary use is as a base class for class
 *  Phase. Class %State has no virtual methods, and none of its
 *  methods are meant to be overloaded. However, this is one
 *  exception.  If the phase is incompressible, then the density
 *  must be replaced by the pressure as the independent variable. In
 *  this case, functions such as State::setMassFractions() within
 *  the class %State must actually now calculate the density (at
 *  constant <I>T</I> and <I>P</I>) instead of leaving it alone as
 *  befits an independent variable. Therefore, these types of
 *  functions are virtual functions and need to be overloaded for
 *  incompressible phases. Note, for nearly incompressible phases
 *  (or phases which utilize standard states based on a <I>T</I> and
 *  <I>P</I>) this change in independent variables may be
 *  advantageous as well, and these functions in %State need to
 *  overload as well so that the stored density within State
 *  doesn't become out of date.
 *
 *  Class Phase derives from both clases
 *  Constituents and State. In addition to the methods of those two
 *  classes, it implements methods that allow referencing a species
 *  by name. And, it contains a lot of utility functions that will
 *  set the %State of the phase in its entirety, by first setting
 *  the composition, then the temperature and then the density.
 *  An example of this is the function,
 *    Phase::setState_TRY(doublereal t, doublereal dens, const doublereal* y).
 *
 *  Class Phase contains method for saving and restoring the
 *  full internal states of each phase. These are called Phase::saveState()
 *  and Phase::restoreState(). These functions operate on a state
 *  vector, which is in general of length (2 + nSpecies()). The first
 *  two entries of the state vector is temperature and density.
 *
 */


//! Base class for phases of matter
/*!
 * Base class for phases of matter. Class Phase derives from both
 * Constituents and State. In addition to the methods of those two
 * classes, it implements methods that allow referencing a species
 * by name.
 *
 *  Class Phase derives from both classes
 *  Constituents and State. In addition to the methods of those two
 *  classes, it implements methods that allow referencing a species
 *  by name. And, it contains a lot of utility functions that will
 *  set the %State of the phase in its entirety, by first setting
 *  the composition, then the temperature and then the density.
 *  An example of this is the function,
 *    Phase::setState_TRY(doublereal t, doublereal dens, const doublereal* y).
 *
 *  Class Phase contains method for saving and restoring the
 *  full internal states of each phase. These are called Phase::saveState()
 *  and Phase::restoreState(). These functions operate on a state
 *  vector, which is in general of length (2 + nSpecies()). The first
 *  two entries of the state vector is temperature and density.
 *
 *  The class Phase contains two strings that identify a phase.
 *  The string id() is the value of the ID attribute of the XML phase node
 *  that is used to initialize a phase when it is read it.
 *  The id() field will stay that way even if the name is changed.
 *  The name field is also set to the value of the ID attribute of
 *  the XML phase node.
 *
 *  However, the name field  may be changed to another value during the course of a calculation.
 *  For example, if a phase is located in two places, but has the same
 *  constitutive input, the id's of the two phases will be the same,
 *  but the names of the two phases may be different.
 *
 *  The name of a phase can be the same as the id of that same phase.
 *  Actually, this is the default and normal condition to have the name and
 *  the id for each phase to be the same. However, it is expected that
 *  it's an error to have two phases in a single problem with the same name.
 *  or the same id (or the name from one phase being the same as the id
 *  of another phase).
 *  Thus, it is expected that there is a 1-1 correspondence between
 *  names and unique phases within a Cantera problem.
 *
 *  A species name may be referred to via three methods:
 *
 *    -   "speciesName"
 *    -   "PhaseId:speciesName"
 *    -   "phaseName:speciesName"
 *    .
 *
 *  The first two methods of naming may not yield a unique species within
 *  complicated assemblies of Cantera Phases.
 *
 *
 *  @todo
 *   Make the concept of saving state vectors more general, so that
 *   it can handle other cases where there are additional internal state
 *   variables, such as the voltage, a potential energy, or a strain field.
 *
 * @ingroup phases
 */
class Phase : public Constituents, public State
{

public:

    /// Default constructor.
    Phase();

    /// Destructor.
    virtual ~Phase();

    /**
     * Copy Constructor
     *
     * @param  right       Reference to the class to be used in the copy
     */
    Phase(const Phase& right);

    /**
     * Assignment operator
     *
     * @param  right      Reference to the class to be used in the copy
     */
    Phase& operator=(const Phase& right);

    //! Returns a reference to the XML_Node stored for the phase
    /*!
     *  The XML_Node for the phase contains all of the input data used
     *  to set up the model for the phase, during its initialization.
     */
    XML_Node& xml();

    //! Return the string id for the phase
    /*!
     * Returns the id of the phase. The ID of the phase
     * is set to the string name of the phase within the XML file
     * Generally, it refers to the individual model name that
     * denotes the species, the thermo, and the reaction rate info.
     */
    std::string id() const;

    //! Set the string id for the phase
    /*!
     * Sets the id of the phase. The ID of the phase
     * is originally set to the string name of the phase within the XML file.
     * Generally, it refers to the individual model name that
     * denotes the species, the thermo, and the reaction rate info.
     *
     * @param id String id of the phase
     */
    void setID(std::string id);

    //! Return the name of the phase
    /*!
     * Returns the name of the phase. The name of the phase
     * is set to the string name of the phase within the XML file
     * Generally, it refers to the individual model name that
     * denotes the species, the thermo, and the reaction rate info.
     * It may also refer more specifically to a location within
     * the domain.
     */
    std::string name() const;

    //! Sets the string name for the phase
    /*!
     * Sets the name of the phase. The name of the phase
     * is originally set to the string name of the phase within the XML file.
     * Generally, it refers to the individual model name that
     * denotes the species, the thermo, and the reaction rate info.
     * It may also refer more specifically to a location within
     * the domain.
     *
     * @param nm String name of the phase
     */
    void setName(std::string nm);

    //! Returns the index of a species named 'name' within the Phase object
    /*!
     * The first species in the phase will have an index 0, and the last one in the
     * phase will have an index of nSpecies() - 1.
     *
     *  A species name may be referred to via three methods:
     *
     *    -   "speciesName"
     *    -   "PhaseId:speciesName"
     *    -   "phaseName:speciesName"
     *    .
     *
     *  The first two methods of naming may not yield a unique species within
     *  complicated assemblies of Cantera phases. The last method is guarranteed
     *  to be unique within a collection of Cantera phases.
     *
     * @param name String name of the species. It may also be the phase name
     *             species name combination, separated by a colon.
     * @return     Returns the index of the species. If the name is not found,
     *             the value of -1 is returned.
     */
    size_t speciesIndex(std::string name) const;

    //! Returns the expanded species name of a species, including the phase name
    /*!
     *  Returns the expanded phase name species name string.
     *  This is guarranteed to be unique within a Cantera problem.
     *
     * @param k  Species index within the phase
     * @return   Returns the "phaseName:speciesName" string
     */
    std::string speciesSPName(int k) const;

    //! Save the current internal state of the phase
    /*!
     * Write to vector 'state' the current internal state.
     *
     * @param state output vector. Will be resized to nSpecies() + 2 on return.
     */
    void saveState(vector_fp& state) const;

    //! Write to array 'state' the current internal state.
    /*!
     * @param lenstate length of the state array. Must be >= nSpecies() + 2
     * @param state    output vector. Must be of length  nSpecies() + 2 or
     *                 greater.
     */
    void saveState(size_t lenstate, doublereal* state) const;

    //!Restore a state saved on a previous call to saveState.
    /*!
     * @param state State vector containing the previously saved state.
     */
    void restoreState(const vector_fp& state);

    //! Restore the state of the phase from a previously saved state vector.
    /*!
     *  @param lenstate   Length of the state vector
     *  @param state      Vector of state conditions.
     */
    void restoreState(size_t lenstate, const doublereal* state);

    /**
     * Set the species mole fractions by name.
     * @param xMap map from species names to mole fraction values.
     * Species not listed by name in \c xMap are set to zero.
     */
    void setMoleFractionsByName(compositionMap& xMap);

    //! Set the mole fractions of a group of species by name
    /*!
     * The string x is in the form of a composition map
     * Species which are not listed by name in the composition
     * map are set to zero.
     *
     * @param x string x in the form of a composition map
     */
    void setMoleFractionsByName(const std::string& x);

    /**
     * Set the species mass fractions by name.
     * @param yMap map from species names to mass fraction values.
     * Species not listed by name in \c yMap are set to zero.
     */
    void setMassFractionsByName(compositionMap& yMap);


    //! Set the species mass fractions by name.
    /*!
     * Species not listed by name in \c x are set to zero.
     *
     * @param x  String containing a composition map
     */
    void setMassFractionsByName(const std::string& x);

    //! Set the internally stored temperature (K), density, and mole fractions.
    /*!
     * Note, the mole fractions are always set first, before the density
     *
     * @param t     Temperature in kelvin
     * @param dens  Density (kg/m^3)
     * @param x     vector of species mole fractions.
     *              Length is equal to m_kk
     */
    void setState_TRX(doublereal t, doublereal dens, const doublereal* x);


    //! Set the internally stored temperature (K), density, and mole fractions.
    /*!
     * Note, the mole fractions are always set first, before the density
     *
     * @param t     Temperature in kelvin
     * @param dens  Density (kg/m^3)
     * @param x     Composition Map containing the mole fractions.
     *              Species not included in the map are assumed to have
     *              a zero mole fraction.
     */
    void setState_TRX(doublereal t, doublereal dens, compositionMap& x);

    //! Set the internally stored temperature (K), density, and mass fractions.
    /*!
     * Note, the mass fractions are always set first, before the density
     *
     * @param t     Temperature in kelvin
     * @param dens  Density (kg/m^3)
     * @param y     vector of species mass fractions.
     *              Length is equal to m_kk
     */
    void setState_TRY(doublereal t, doublereal dens, const doublereal* y);

    //! Set the internally stored temperature (K), density, and mass fractions.
    /*!
     * Note, the mass fractions are always set first, before the density
     *
     * @param t     Temperature in kelvin
     * @param dens  Density (kg/m^3)
     * @param y     Composition Map containing the mass fractions.
     *              Species not included in the map are assumed to have
     *              a zero mass fraction.
     */
    void setState_TRY(doublereal t, doublereal dens, compositionMap& y);

    //! Set the internally stored temperature (K), molar density (kmol/m^3), and mole fractions.
    /*!
     * Note, the mole fractions are always set first, before the molar density
     *
     * @param t     Temperature in kelvin
     * @param n     molar density (kmol/m^3)
     * @param x     vector of species mole fractions.
     *              Length is equal to m_kk
     */
    void setState_TNX(doublereal t, doublereal n, const doublereal* x);

    //! Set the internally stored temperature (K) and density (kg/m^3)
    /*!
     * @param t     Temperature in kelvin
     * @param rho   Density (kg/m^3)
     */
    void setState_TR(doublereal t, doublereal rho);

    //! Set the internally stored temperature (K) and mole fractions.
    /*!
     * @param t   Temperature in kelvin
     * @param x   vector of species mole fractions.
     *            Length is equal to m_kk
     */
    void setState_TX(doublereal t, doublereal* x);

    //! Set the internally stored temperature (K) and mass fractions.
    /*!
     * @param t   Temperature in kelvin
     * @param y   vector of species mass fractions.
     *            Length is equal to m_kk
     */
    void setState_TY(doublereal t, doublereal* y);

    //! Set the density (kg/m^3) and mole fractions.
    /*!
     * @param rho  Density (kg/m^3)
     * @param x    vector of species mole fractions.
     *             Length is equal to m_kk
     */
    void setState_RX(doublereal rho, doublereal* x);

    //! Set the density (kg/m^3) and mass fractions.
    /*!
     * @param rho  Density (kg/m^3)
     * @param y    vector of species mass fractions.
     *             Length is equal to m_kk
     */
    void setState_RY(doublereal rho, doublereal* y);

    /**
     * Copy the vector of molecular weights into vector weights.
     *
     * @param weights Output vector of molecular weights (kg/kmol)
     */
    void getMolecularWeights(vector_fp& weights) const;

    /**
     * Copy the vector of molecular weights into array weights.
     *
     * @param iwt      Unused.
     * @param weights  Output array of molecular weights (kg/kmol)
     *
     * @deprecated
     */
    DEPRECATED(void getMolecularWeights(int iwt, doublereal* weights) const);

    /**
     * Copy the vector of molecular weights into array weights.
     *
     * @param weights  Output array of molecular weights (kg/kmol)
     */
    void getMolecularWeights(doublereal* weights) const;

    /**
     * Return a const reference to the internal vector of
     * molecular weights.
     */
    const vector_fp& molecularWeights() const;

    /**
     * Get the mole fractions by name.
     *
     * @param x  Output composition map containing the
     *           species mole fractions.
     */
    void getMoleFractionsByName(compositionMap& x) const;

    //! Return the mole fraction of a single species
    /*!
     * @param  k  String name of the species
     *
     * @return Mole fraction of the species
     */
    doublereal moleFraction(size_t k) const;

    //! Return the mole fraction of a single species
    /*!
     * @param  name  String name of the species
     *
     * @return Mole fraction of the species
     */
    doublereal moleFraction(std::string name) const;

    //! Return the mass fraction of a single species
    /*!
     * @param  k  String name of the species
     *
     * @return Mass Fraction of the species
     */
    doublereal massFraction(size_t k) const;

    //! Return the mass fraction of a single species
    /*!
     * @param  name  String name of the species
     *
     * @return Mass Fraction of the species
     */
    doublereal massFraction(std::string name) const;

    /**
     * Charge density [C/m^3].
     */
    doublereal chargeDensity() const;

    /// Returns the number of spatial dimensions (1, 2, or 3)
    size_t nDim() const {
        return m_ndim;
    }

    //! Set the number of spatial dimensions (1, 2, or 3)
    /*!
     *  The number of spatial dimensions is used for vector involving
     *  directions.
     *
     * @param ndim   Input number of dimensions.
     */
    void setNDim(size_t ndim) {
        m_ndim = ndim;
    }

    /**
     *  Finished adding species, prepare to use them for calculation
     *  of mixture properties.
     */
    virtual void freezeSpecies();

    virtual bool ready() const;


protected:

    /**
     * m_kk = Number of species in the phase.  @internal m_kk is a
     * member of both the State and Constituents classes.
     * Therefore, to avoid multiple inheritance problems, we need
     * to restate it in here, so that the declarations in the two
     * base classes become hidden.
     */
    size_t m_kk;

    /**
     * m_ndim is the dimensionality of the phase.  Volumetric
     * phases have dimensionality 3 and surface phases have
     * dimensionality 2.
     */
    size_t m_ndim;

private:

    //! This stores the initial state of the system
    /*!
     * @deprecated
     *      This doesn't seem to be used much anymore.
     */
    vector_fp m_data;

    //! Pointer to the XML node containing the XML info for this phase
    XML_Node* m_xml;

    //! ID of the phase.
    /*!
     * This is the value of the ID attribute of the XML phase node.
     * The field will stay that way even if the name is changed.
     */
    std::string m_id;

    //! Name of the phase.
    /*!
     * Initially, this is the value of the ID attribute of the XML phase node.
     *
     * It may be changed to another value during the course of a calculation.
     * for example, if a phase is located in two places, but has the same
     * constituitive input, the id's of the two phases will be the same,
     * but the names of the two phases may be different.
     *
     * The name can be the same as the id, within a phase. However, besides
     * that case, it is expected that there is a 1-1 correspondence between
     * names and unique phases within a Cantera problem.
     */
    std::string m_name;
};

//! typedef for the base Phase class
typedef Phase phase_t;
}

#endif
