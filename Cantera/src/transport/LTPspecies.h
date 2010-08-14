/**
 *  @file LTPspecies.h
 *  Header file defining class LTPspecies and its child classes
 */
/*
 *  $Author: christopher.lueth@gmail.com $
 *  $Date: 2010-03-30 12:30:39 -0600 (Tue, 30 Mar 2010) $
 *  $Revision: 427 $

 */

#ifndef CT_LTPSPECIES_H
#define CT_LTPSPECIES_H


// Cantera includes
#include "ct_defs.h"
#include "TransportBase.h"
#include "FactoryBase.h"

// STL includes
#include <vector>
#include <string>


namespace Cantera {

  
  //! Enumeration of the types of transport properties that can be 
  //! handled by the variables in the various Transport classes.
  /*!
   * Not all of these are handled by each class and each class
   * should handle exceptions where the transport property is not handled.
   *
   * Tranport properties currently on the list
   *
   *    0  - viscosity
   *    1  - Ionic conductivity
   *    2  - Mobility Ratio
   *    3  - Self Diffusion coefficient
   *    4  - Thermal conductivity
   *    5  - species diffusivity
   *    6  - hydrodynamic radius
   *    7  - electrical conductivity
   */
  enum TransportPropertyType {
    TP_UNKNOWN = -1,
    TP_VISCOSITY = 0,
    TP_IONCONDUCTIVITY,
    TP_MOBILITYRATIO,
    TP_SELFDIFFUSION,
    TP_THERMALCOND,
    TP_DIFFUSIVITY,
    TP_HYDRORADIUS,
    TP_ELECTCOND
  };

  //! Temperature dependence type for pure (liquid) species properties
  /*!
   *  Types of temperature dependencies:
   *     0  - Independent of temperature 
   *     1  - extended arrhenius form
   *     2  - polynomial in temperature form
   */
  enum LiquidTR_Model {
    LTR_MODEL_NOTSET=-1,
    LTR_MODEL_CONSTANT, 
    LTR_MODEL_ARRHENIUS,
    LTR_MODEL_POLY,
    LTR_MODEL_EXPT
  };

  //====================================================================================================================
  //! Class LTPspecies holds transport parameters for a specific liquid-phase species.
  /*!
   * Subclasses handle different means of specifying transport properties
   * like constant, %Arrhenius or polynomial fits.  In its current state, 
   * it is primarily suitable for specifying temperature dependence, but 
   * the adjustCoeffsForComposition() method can be implemented to 
   * adjust for composition dependence.  
   *
   * Mixing rules for computing mixture transport properties are handled 
   * separately in LiquidTranInteraction subclasses.
   */
  class LTPspecies {

  public:

    //! Construct an LTPspecies object for a liquid tranport property.
    /*!
     *    The transport property is constructed from the XML node, 
     *    \verbatim <propNode>, \endverbatim that is a child of the
     *    \verbatim <transport> \endverbatim node and specifies a type of
     *    transport property (like viscosity)
     */ 
    LTPspecies(const XML_Node &propNode = 0, 
	       std::string name = "-", 
	       TransportPropertyType tp_ind = TP_UNKNOWN, 
	       const thermo_t* const thermo = 0) :
      m_speciesName(name), 
      m_model(LTR_MODEL_NOTSET),
      m_property(tp_ind),
      m_thermo(thermo),
      m_mixWeight(1.0)
    {
      if (propNode.hasChild("mixtureWeighting") ) {
	m_mixWeight = getFloat(propNode, "mixtureWeighting");
      }
    }
    
    //! Copy constructor
    LTPspecies(const LTPspecies &right); 

    //! Assignment operator
    LTPspecies&  operator=(const LTPspecies& right);

    virtual ~LTPspecies( ) { }

    //! duplication routine 
    /*!
     *  @return  Returns a copy of this routine as a pointer to LTPspecies
     */
    virtual LTPspecies * duplMyselfAsLTPspecies() const;

    //! Returns the vector of pure species tranport property
    /*!
     *  The pure species transport property (i.e. pure species viscosity)
     *  is returned.  Any temperature and composition dependence will be 
     *  adjusted internally according to the information provided by the 
     *  subclass object. 
     */
    virtual doublereal getSpeciesTransProp( ) { return 0.0; }

    virtual bool checkPositive( ) { return ( m_coeffs[0] > 0 ); }

    doublereal getMixWeight( ) {
      return m_mixWeight; 
    }

  protected:
    //! Species Name
    std::string m_speciesName;
   
    //! Model type for the temperature dependence
    LiquidTR_Model m_model;

    //! enum indicating which property this is (i.e viscosity)
    TransportPropertyType m_property;

    //! Model temperature-dependence ceofficients
    vector_fp m_coeffs;

    //! Pointer to a const thermo object to get current temperature
    const thermo_t * m_thermo;

    //! Weighting used for mixing.  
    /*!
     * This weighting can be employed to allow salt transport 
     * properties to be represented by specific ions.  
     * For example, to have Li+ and Ca+ represent the mixing 
     * transport properties of LiCl and CaCl2, the weightings for
     * Li+ would be 2.0, for K+ would be 3.0 and for Cl- would be 0.0.
     * The tranport properties for Li+ would be those for LiCl and 
     * the tranport properties for Ca+ would be those for CaCl2. 
     * The transport properties for Cl- should be something innoccuous like 
     * 1.0--note that 0.0 is not innocuous if there are logarithms involved.
     */
    doublereal m_mixWeight;

    //! Internal model to adjust species-specific properties for composition.
    /*!
     *  Currently just a place holder, but this method could take 
     *  the composition from the thermo object and adjust coefficients 
     *  accoding to some unspecified model.
     */
    virtual void adjustCoeffsForComposition() { }
  };


  //====================================================================================================================
  //! Class LTPspecies_Const holds transport parameters for a 
  //! specific liquid-phase species (LTPspecies) when the 
  //! transport property is just a constant value.  
  /*!
   * As an example of the input required for LTPspecies_Const
   * consider the following XML fragment
   *
   * \verbatim
   *    <species>
   *      <!-- thermodynamic properties -->
   *      <transport> 
   *        <hydrodynamicRadius model="Constant" units="A">
   *            1.000
   *        </hydrodynamicRadius>
   *        <!-- other tranport properties -->
   *      </transport> 
   *    </species>
   * \endverbatim
   */
  class LTPspecies_Const : public  LTPspecies {

  public:

    //! Construct an LTPspecies object for a liquid tranport property 
    //! expressed as a constant value.
    /** The transport property is constructed from the XML node, 
     *  \verbatim <propNode>, \endverbatim that is a child of the
     *  \verbatim <transport> \endverbatim node and specifies a type of
     *  transport property (like viscosity)
     */ 
    LTPspecies_Const(const XML_Node &propNode, 
		     std::string name, 
		     TransportPropertyType tp_ind, 
		     thermo_t* thermo);
    
    //! Copy constructor
    LTPspecies_Const(const LTPspecies_Const &right); 

    //! Assignment operator
    LTPspecies_Const&  operator=(const LTPspecies_Const& right );

    virtual ~LTPspecies_Const( ) { }

    //! duplication routine 
    /*!
     *  @return  Returns a copy of this routine as a pointer to LTPspecies
     */
    virtual LTPspecies * duplMyselfAsLTPspecies() const;

    //! Returns the pure species tranport property
    /*!
     *  The pure species transport property (i.e. pure species viscosity)
     *  is returned.  Any temperature and composition dependence will be 
     *  adjusted internally according to the information provided.
     */
    doublereal getSpeciesTransProp( );

  protected:

    //! Internal model to adjust species-specific properties for composition.
    /** Currently just a place holder, but this method could take 
     * the composition from the thermo object and adjust coefficients 
     * accoding to some unspecified model.
     */
    void adjustCoeffsForComposition( ) { }
  };

  //====================================================================================================================
  //! Class LTPspecies_Arrhenius holds transport parameters for a 
  //! specific liquid-phase species (LTPspecies) when the 
  //! transport property is expressed in Arrhenius form.  
  /*!
   * Used for pure species properties with equations of the form
   * \f[
   *      x = A T^b \exp( - E / RT )   
   * \f]
   * where A, b, and E are passed in the xml input file.
   *
   * As an example of the input required for LTPspecies_Arrhenius
   * consider the following XML fragment
   *
   * \verbatim
   *    <species>
   *      <!-- thermodynamic properties -->
   *      <transport> 
   *        <viscosity model="Arrhenius">
   *           <!-- Janz, JPCRD, 17, supplement 2, 1988 -->
   *           <A>6.578e-5</A>
   *           <b>0.0</b>
   *           <E units="J/kmol">23788.e3</E>
   *        </viscosity>
   *        <!-- other tranport properties -->
   *      </transport> 
   *    </species>
   * \endverbatim
   */
  class LTPspecies_Arrhenius : public  LTPspecies{

  public:

    //! Construct an LTPspecies object for a liquid tranport property 
    //! expressed in extended Arrhenius form.
    /*!
     *  The transport property is constructed from the XML node, 
     *  \verbatim <propNode>, \endverbatim that is a child of the
     *  \verbatim <transport> \endverbatim node and specifies a type of
     *  transport property (like viscosity)
     */ 
    LTPspecies_Arrhenius( const XML_Node &propNode, 
			  std::string name, 
			  TransportPropertyType tp_ind, 
			  thermo_t* thermo ); 
    
    //! Copy constructor
    LTPspecies_Arrhenius( const LTPspecies_Arrhenius &right ); 

    //! Assignment operator
    LTPspecies_Arrhenius&  operator=(const LTPspecies_Arrhenius& right );

    virtual ~LTPspecies_Arrhenius( ) { }

    //! duplication routine 
    /*!
     *  @return  Returns a copy of this routine as a pointer to LTPspecies
     */
    virtual LTPspecies * duplMyselfAsLTPspecies() const;

    //! Return the pure species value for this transport property evaluated 
    //! from the Arrhenius expression
    /*!
     * In general the Arrhenius expression is 
     *
     * \f[
     *      \mu = A T^n \exp( - E / R T ).
     * \f]
     *
     * Note that for viscosity, the convention is such that 
     * a positive activation energy corresponds to the typical 
     * case of a positive argument to the exponential so that 
     * the Arrhenius expression is
     *
     * \f[
     *      \mu = A T^n \exp( + E / R T ).
     * \f]
     *
     * Any temperature and composition dependence will be 
     *  adjusted internally according to the information provided.
     */
    doublereal getSpeciesTransProp( );

  protected:

    //! temperature from thermo object
    doublereal m_temp;

    //! logarithm of current temperature
    doublereal m_logt;

    //! most recent evaluation of transport property
    doublereal m_prop;

    //! logarithm of most recent evaluation of transport property
    doublereal m_logProp;

    //! Internal model to adjust species-specific properties for composition.
    /*!
     * Currently just a place holder, but this method could take 
     * the composition from the thermo object and adjust coefficients 
     * accoding to some unspecified model.
     */
    void adjustCoeffsForComposition( ) { }
  };

  //====================================================================================================================
  //! Class LTPspecies_Poly holds transport parameters for a 
  //! specific liquid-phase species (LTPspecies) when the transport 
  //! property is expressed as a polynomial in temperature.
  /*!
   * Used for pure species properties with equations of the form
   * \f[
   *    x = f[0] + f[1] T + ... + f[N] T^N   
   * \f]
   * where f[i] are elements of the float array passed in.
   *
   * As an example of the input required for LTPspecies_Poly
   * consider the following XML fragment
   *
   * \verbatim
   *    <species>
   *      <!-- thermodynamic properties -->
   *      <transport> 
   *        <thermalConductivity model="coeffs">
   *           <floatArray size="2">  0.6, -15.0e-5 </floatArray>
   *        </thermalConductivity>
   *        <!-- other tranport properties -->
   *      </transport> 
   *    </species>
   * \endverbatim
   */
  class LTPspecies_Poly : public  LTPspecies {

  public:

  //! Construct an LTPspecies object for a liquid tranport property 
  //! expressed as a polynomial in temperature.
  /** The transport property is constructed from the XML node, 
   *  \verbatim <propNode>, \endverbatim that is a child of the
   *  \verbatim <transport> \endverbatim node and specifies a type of
   *  transport property (like viscosity)
   */ 
    LTPspecies_Poly( const XML_Node &propNode, 
		     std::string name, 
		     TransportPropertyType tp_ind, 
		     thermo_t* thermo ); 
    
    //! Copy constructor
    LTPspecies_Poly( const LTPspecies_Poly &right ); 

    //! Assignment operator
    LTPspecies_Poly&  operator=(const LTPspecies_Poly& right );

    virtual ~LTPspecies_Poly( ) { }

    //! Duplication routine 
    /*!
     *  @return  Returns a copy of this routine as a pointer to LTPspecies
     */
    virtual LTPspecies * duplMyselfAsLTPspecies() const;

    //! Returns the pure species tranport property
    /*!
     *  The pure species transport property (i.e. pure species viscosity)
     *  is returned.  Any temperature and composition dependence will be 
     *  adjusted internally according to the information provided.
     */
    doublereal getSpeciesTransProp( );

  protected:

    //! temperature from thermo object
    doublereal m_temp;

    //! most recent evaluation of transport property
    doublereal m_prop;

    //! Internal model to adjust species-specific properties for composition.
    /*!
     * Currently just a place holder, but this method could take 
     * the composition from the thermo object and adjust coefficients 
     * accoding to some unspecified model.
     */
    void adjustCoeffsForComposition( ){ }
  };

  //====================================================================================================================
  //! Class LTPspecies_ExpT holds transport parameters for a  specific liquid-phase species (LTPspecies) 
  //! when the transport property is expressed as a exponential in temperature.
  /**
   * Used for pure species properties with equations of the form
   *
   *    \f[
   *       x = f[0] \exp( f[1] T + ... + f[N] T^{N} )  
   *    \f]
   *
   * where f[i] are elements of the float array passed in.
   *
   * As an example of the input required for LTPspecies_ExpT
   * consider the following XML fragment
   *
   * \verbatim
   *    <species>
   *      <!-- thermodynamic properties -->
   *      <transport> 
   *        <thermalConductivity model="expT">
   *           <floatArray size="2">  0.6, -15.0e-5 </floatArray>
   *        </thermalConductivity>
   *        <!-- other tranport properties -->
   *      </transport> 
   *    </species>
   * \endverbatim
   */
  class LTPspecies_ExpT : public  LTPspecies {

  public:

  //! Construct an LTPspecies object for a liquid tranport property 
  //! expressed as an exponential in temperature.
  /*!
   *  The transport property is constructed from the XML node, \verbatim <propNode>, \endverbatim that is a child of the
   *  \verbatim <transport> \endverbatim node and specifies a type of transport property (like viscosity).
   *
   *  @param    propNode     
   */ 
    LTPspecies_ExpT(const XML_Node &propNode, 
		     std::string name, 
		     TransportPropertyType tp_ind, 
		     thermo_t* thermo ); 
    
    //! Copy constructor
    /*!
     *  @param right   Object to be copied
     */
    LTPspecies_ExpT(const LTPspecies_ExpT &right); 

    //! Assignment operator
    LTPspecies_ExpT&  operator=(const LTPspecies_ExpT& right );

    virtual ~LTPspecies_ExpT( ) { }

    //! duplication routine 
    /*!
     *  @return  Returns a copy of this routine as a pointer to LTPspecies
     */
    virtual LTPspecies * duplMyselfAsLTPspecies() const;

    //! Returns the pure species tranport property
    /*!
     *  The pure species transport property (i.e. pure species viscosity)
     *  is returned.  Any temperature and composition dependence will be 
     *  adjusted internally according to the information provided.
     */
    doublereal getSpeciesTransProp( );

  protected:

    //! temperature from thermo object
    doublereal m_temp;

    //! most recent evaluation of transport property
    doublereal m_prop;

    //! Internal model to adjust species-specific properties for composition.
    /** Currently just a place holder, but this method could take 
     * the composition from the thermo object and adjust coefficients 
     * accoding to some unspecified model.
     */
    void adjustCoeffsForComposition( ){ }
  };

  //====================================================================================================================

}
#endif
