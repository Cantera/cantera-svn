/**
 *  @file TransportFactory.h
 *  Header file defining class TransportFactory
 *     (see \link Cantera::TransportFactory TransportFactory\endlink)
 */
/*
 *  $Author$
 *  $Date$
 *  $Revision$
 *
 *  Copyright 2001 California Institute of Technology
 *
 */

#ifndef CT_TRANFACTORY_H
#define CT_TRANFACTORY_H


// turn off warnings under Windows
#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

// STL includes
#include <vector>
#include <string>
#include <iostream>
#include <new>



// Cantera includes
#include "ct_defs.h"
#include "TransportBase.h"
#include "FactoryBase.h"
//#include "LiquidTransportData.h"
#include "LiquidTransportParams.h"

#if defined(THREAD_SAFE_CANTERA)
#include <boost/thread/mutex.hpp>
#endif

namespace Cantera {

  /**
   * Struct to hold data read from a transport property database file.
   */
  struct GasTransportData {
    GasTransportData() : speciesName("-"), 
			 geometry(-1), wellDepth(-1.0),
			 diameter(-1.0), 
			 dipoleMoment(-1.0), 
			 polarizability(-1.0),
			 rotRelaxNumber(-1.0) {}

    std::string speciesName;
    int geometry;
    doublereal wellDepth;
    doublereal diameter;
    doublereal dipoleMoment;
    doublereal polarizability;
    doublereal rotRelaxNumber;
  };


  // forward references
  class MMCollisionInt;
  class GasTransportParams; 
  class LiquidTransportParams;
  class XML_Node;

 
  //! The purpose of TransportFactory is to create new instances of
  //! 'transport managers', which are classes that provide transport
  //! properties and are derived from base class Transport.
  /*!
   * TransportFactory handles all initialization
   * required, including evaluation of collision integrals and
   * generating polynomial fits.  Transport managers can also be
   * created in other ways.
   *
   * @ingroup transportgroup
   * @ingroup transportProps
   */
  class TransportFactory : FactoryBase {

  public:

    /**
     * Return a pointer to a TransportFactory
     * instance. TransportFactory is implemented as a 'singleton',
     * which means that at most one instance may be created. The
     * constructor is private. When a TransportFactory instance is
     * required, call static method factory() to return a pointer
     * to the TransportFactory instance.
     *
     * @code
     * TransportFactory* f;
     * f = TransportFactory::factory();
     * @endcode
     */
    static TransportFactory* factory() {
#if defined(THREAD_SAFE_CANTERA)
      boost::mutex::scoped_lock   lock(transport_mutex) ;
#endif
      if (!s_factory) {
	s_factory = new TransportFactory();
      }
      return s_factory;
    }


    /**
     * This static function deletes the statically malloced instance.
     */
    virtual void deleteFactory();

    /*!
     * Destructor 
     *
     * We do not delete statically created single instance of this
     * class here, because it would create an infinite loop if
     * destructor is called for that single instance.  However, we do
     * have a pointer to m_integrals that does need to be
     * explicitly deleted.
     */
    virtual ~TransportFactory();


    /**
     *  make one of several transport models, and return a base class
     *  pointer to it.  This method operates at the level of a 
     *  single transport property as a function of temperature 
     *  and possibly composition.
     */
    virtual LTPspecies* 
      newLTP( const XML_Node &trNode, std::string &name, 
	      TransportPropertyList tp_ind, thermo_t* thermo) ;

    
    /**
     *  make one of several transport models, and return a base class
     *  pointer to it.  This method operates at the level of a 
     *  single mixture transport property.  Individual species 
     *  transport properties are addressed by the LTPspecies 
     *  returned by newLTP
     */
    virtual LiquidTranInteraction* newLTI( const XML_Node &trNode, 
					   TransportPropertyList tp_ind, 
					   LiquidTransportParams& trParam) ;

    
    //! Build a new transport manager using a transport manager
    //! that may not be the same as in the phase description
    //! and return a base class pointer to it
    /*!
     *  @param model     String name for the transport manager
     *  @param thermo    ThermoPhase object
     *  @param log_level log level
     */
    virtual Transport*
      newTransport(std::string model, thermo_t* thermo, int log_level=0);

    //! Build a new transport manager using the default transport manager
    //! in the phase description and return a base class pointer to it
    /*!
     *  @param thermo   ThermoPhase object
     *  @param log_level log level
     */ 
    virtual Transport*
    newTransport(thermo_t* thermo, int log_level=0);

    //! Initialize an existing transport manager
    virtual void initTransport(Transport* tr,  
			       thermo_t* thermo, int mode=0, int log_level=0);

    //! Initialize an existing transport manager for liquid phase
    /*! Similar to initTransport except uses LiquidTransportParams
     * class and calls setupLiquidTransport().
     */
    virtual void initLiquidTransport(Transport* tr,
                                     thermo_t* thermo, 
                                     int log_level=0);

  private:

    //! Static instance of the factor -> This is the only instance of this
    //! object allowed
    static TransportFactory* s_factory;
#if defined(THREAD_SAFE_CANTERA)
    static boost::mutex transport_mutex ;
#endif

    //! The constructor is private; use static method factory() to
    //! get a pointer to a factory instance
    /*!
     *      
     *   The default constructor for this class sets up 
     *   m_models[], a mapping between the string name
     *   for a transport model and the integer name.
     */
    TransportFactory();

    //! Read Transport Database
    /*!
     * Read transport property data from a file for a list of species.
     * Given the name of a file containing transport property
     * parameters and a list of species names, this method returns an
     * instance of TransportParams containing the transport data for
     * these species read from the file.
     */
    void getTransportData(const std::vector<const XML_Node*> &db,  
			  XML_Node& log, const std::vector<std::string>& names, 
			  GasTransportParams& tr);


    //! Read transport property data from a file for a list of species that comprise
    //! the phase.
    /*!
     * Given the name of a file containing transport property
     * parameters and a list of species names, this method constructs the LiquidTransport
     * Params object  containing the transport data for these species.
     *
     *  It is an error to not find a "transport" XML element within each of the species 
     *  XML elements listed in the names vector.
     *
     * @param db   Reference to a vector of XML_Node pointers containing the species XML 
     *             nodes.
     * @param log  Reference to an XML log file. (currently unused)
     * @param names Vector of names of species. On output, tr will contain transport data
     *              for each of of these names in the order determined by this vector.
     * @param tr   Reference to the LiquidTransportParams object that will contain the results.
     */
    void getLiquidSpeciesTransportData(const std::vector<const XML_Node*> &db,  
				       XML_Node& log, const std::vector<std::string>& names, 
				       LiquidTransportParams& tr);

    //! Read transport property data from a file for a list of species.
    /*!
     *
     *  Given the name of a file containing transport property
     * parameters and a list of species names, this method returns an
     * instance of TransportParams containing the transport data for
     * these species read from the file.
     *
     */
    void getLiquidInteractionsTransportData(const XML_Node &db,  
				XML_Node& log, const std::vector<std::string>& names, 
				LiquidTransportParams& tr);

    //! Generate polynomial fits to viscosity, conductivity, and
    //!  binary diffusion coefficients */
    /*! If CK_mode, then the fits are of the form 
     * \f[
     * \log(\eta(i)) = \sum_{n = 0}^3 a_n(i) (\log T)^n
     * \f]
     * and \f[
     * \log(D(i,j)) = \sum_{n = 0}^3 a_n(i,j) (\log T)^n
     * \f]
     * Otherwise the fits are of the form
     * \f[
     * \eta(i)/sqrt(k_BT) = \sum_{n = 0}^4 a_n(i) (\log T)^n
     * \f]
     * and \f[
     * D(i,j)/sqrt(k_BT)) = \sum_{n = 0}^4 a_n(i,j) (\log T)^n
     * \f]
     */
    void fitProperties(GasTransportParams& tr, std::ostream & logfile);

    //! Generate polynomial fits to collision integrals
    void fitCollisionIntegrals(std::ostream & logfile, 
			       GasTransportParams& tr);

   
    /** 
     * Prepare to build a new kinetic-theory-based transport manager
     * for low-density gases. Uses polynomial fits to Monchick & Mason
     * collision integrals.
     */
    void setupMM(std::ostream &flog,  const std::vector<const XML_Node*> &transport_database, 
		 thermo_t* thermo, int mode, int log_level, 
		 GasTransportParams& tr);

    /** 
     * Prepare to build a new transport manager for liquids assuming that 
     * viscosity transport data is provided in Arhennius form.
     */
    void setupLiquidTransport(std::ostream &flog,  
			      thermo_t* thermo, int log_level, 
			      LiquidTransportParams& tr);


    //! Second-order correction to the binary diffusion coefficients
    /*!
     * Calculate second-order corrections to binary diffusion
     * coefficient pair (dkj, djk). At first order, the binary
     * diffusion coefficients are independent of composition, and
     * d(k,j) = d(j,k). But at second order, there is a weak
     * dependence on composition, with the result that d(k,j) !=
     * d(j,k). This method computes the multiplier by which the
     * first-order binary diffusion coefficient should be multiplied
     * to produce the value correct to second order. The expressions
     * here are taken from Marerro and Mason,
     * J. Phys. Chem. Ref. Data, vol. 1, p. 3 (1972).
     *
     * @param t   Temperature (K)
     * @param tr  Transport parameters
     * @param k   index of first species
     * @param j   index of second species
     * @param xmk mole fraction of species k
     * @param xmj mole fraction of species j
     * @param fkj multiplier for d(k,j)
     * @param fjk multiplier for d(j,k) 
     *
     * @note This method is not used currently.
     */
    void getBinDiffCorrection(doublereal t, 
			      const GasTransportParams& tr, int k, int j,
			      doublereal xk, doublereal xj, 
			      doublereal& fkj, doublereal& fjk);

    //! Corrections for polar-nonpolar binary diffusion coefficients
    /*!
     * Calculate corrections to the well depth parameter and the
     * diamter for use in computing the binary diffusion coefficient
     * of polar-nonpolar pairs. For more information about this
     * correction, see Dixon-Lewis, Proc. Royal Society (1968).
     */
    void makePolarCorrections(int i, int j, 
			      const GasTransportParams& tr, doublereal& f_eps, 
			      doublereal& f_sigma);


    //! Boolean indicating whether to turn on verbose printing
    bool m_verbose;
   
    //! Pointer to the collision integrals
    MMCollisionInt* m_integrals;
    
    //! Mapping between between the string name
    //!   for a transport model and the integer name.
    std::map<std::string, int> m_models;

    //! Mapping between between the string name
    //! for a transport property and the integer name.
    std::map<std::string, TransportPropertyList> m_tranPropMap;

    //! Mapping between between the string name for a 
    //! species-specific transport property model and the integer name.
    std::map<std::string, LiquidTR_Model> m_LTRmodelMap;

    //! Mapping between between the string name for a 
    //! liquid mixture transport property model and the integer name.
    std::map<std::string, LiquidTranMixingModel> m_LTImodelMap;
  };


  /**
   *  Create a new transport manager instance.
   * @ingroup transportProps
   */
  inline Transport* newTransportMgr(std::string transportModel = "", 
				    thermo_t* thermo = 0, int loglevel=0, 
				    TransportFactory* f=0) {
    if (f == 0) {
      f = TransportFactory::factory();
    }
    Transport* ptr = f->newTransport(transportModel, thermo, loglevel);
    /*
     * Note: We delete the static s_factory instance here, instead of in
     *       appdelete() in misc.cpp, to avoid linking problems involving
     *       the need for multiple cantera and transport library statements
     *       for applications that don't have transport in them.
     */
    return ptr;
  }

  /**
   *  Create a new transport manager instance.
   * @ingroup transportProps
   */
  inline Transport* newDefaultTransportMgr(thermo_t* thermo, int loglevel=0,
                                           TransportFactory* f=0) {
    if (f == 0) {
      f = TransportFactory::factory();
    }
    Transport* ptr = f->newTransport(thermo, loglevel);
    /*
     * Note: We delete the static s_factory instance here, instead of in
     *       appdelete() in misc.cpp, to avoid linking problems involving
     *       the need for multiple cantera and transport library statements
     *       for applications that don't have transport in them.
     */
    return ptr;
  }


}
#endif
