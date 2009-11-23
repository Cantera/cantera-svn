/**
 *  @file LiquidTransportParams.h
 *  Header file defining class LiquidTransportParams
 */
/*
 *  $Author$
 *  $Date$
 *  $Revision$
 *
 *
 *
 */
#ifndef CT_LIQUIDTRANSPORTPARAMS_H
#define CT_LIQUIDTRANSPORTPARAMS_H

#include <vector>

#include "ct_defs.h"
#include "TransportBase.h"
#include "TransportParams.h"
#include "LiquidTransportData.h"
#include "xml.h"
#include "XML_Writer.h"

namespace Cantera {


    //! Composition dependence type for liquid mixture transport properties
    /*!
     *  Types of temperature dependencies:
     *     0  - Mixture calculations with this property are not allowed
     *     1  - Use solvent (species 0) properties
     *     2  - Properties weighted linearly by mole fractions
     *     3  - Properties weighted linearly by mass fractions
     *     4  - Properties weighted logarithmically by mole fractions (interaction energy weighting)
     *     5  - Interactions given pairwise between each possible species (i.e. D_ij)
     *
     *    <transport model="Liquid">
     *       <viscosity>
     *          <compositionDependence model="logMoleFractions">
     *             <interaction>
     *                <speciesA> LiCl(L) </speciesA>
     *                <speciesB> KCl(L)  </speciesB>
     *                <Eij units="J/kmol"> -1.0 </Eij>
     *                <Sij units="J/kmol/K"> 1.0E-1 </Eij>
     *             </interaction>
     *          </compositionDependence>          
     *       </viscosity>
     *       <speciesDiffusivity>
     *          <compositionDependence model="pairwiseInteraction">
     *             <interaction>
     *                <speciesA> Li+ </speciesA>
     *                <speciesB> K+  </speciesB>
     *                <Dij units="m2/s"> 1.5 </Dij>
     *             </interaction>
     *             <interaction>
     *                <speciesA> K+  </speciesA>
     *                <speciesB> Cl- </speciesB>
     *                <Dij units="m2/s"> 1.0 </Dij>
     *             </interaction>
     *             <interaction>
     *                <speciesA> Li+  </speciesA>
     *                <speciesB> Cl-  </speciesB>
     *                <Dij units="m2/s"> 1.2 </Dij>
     *             </interaction>
     *          </compositionDependence>          
     *       </speciesDiffusivity>
     *       <thermalConductivity>
     *          <compositionDependence model="massFractions"/>
     *       </thermalConductivity>
     *       <hydrodynamicRadius>
     *          <compositionDependence model="none"/>
     *       </hydrodynamicRadius>
     *    </transport>     
     *
     */
  enum LiquidTranMixingModel {
    LTI_MODEL_NOTSET=-1,
    LTI_MODEL_NONE, 
    LTI_MODEL_SOLVENT, 
    LTI_MODEL_MOLEFRACS,
    LTI_MODEL_MASSFRACS,
    LTI_MODEL_LOG_MOLEFRACS,
    LTI_MODEL_PAIRWISE_INTERACTION
  };
  

  class LiquidTranInteraction {
    
  public:
    LiquidTranInteraction( const XML_Node &compModelNode = 0, 
			   TransportPropertyList tp_ind = TP_UNKNOWN, 
			   thermo_t* thermo = 0 );
    
    //! Copy constructor
    LiquidTranInteraction( const LiquidTranInteraction &right );
    
    //! Assignment operator
    LiquidTranInteraction& operator=( const LiquidTranInteraction &right );
    
    //! destructor
    virtual ~LiquidTranInteraction() { }

    //! Return the mixture transport property value.
    //! (Must be implemented in subclasses.)
    virtual doublereal getMixTransProp( ) { return 0.0; }
    
  protected:
    //! Model for species interaction effects 
    //! Takes enum LiquidTranMixingModel
    LiquidTranMixingModel m_model;
    
    //! enum indicating what property this is (i.e viscosity)
    TransportPropertyList m_property;
    
    //! pointer to thermo object to get current temperature
    thermo_t* m_thermo;

    //! Matrix of interactions (no temperature dependence, dimensionless)
    DenseMatrix  Aij; 

    //! Matrix of interactions (in energy units, 1/RT temperature dependence)
    DenseMatrix  Eij; 

    //! Matrix of interactions (in entropy units, divided by R)
    DenseMatrix  Sij;         
    
    //! Matrix of interactions 
    DenseMatrix  Dij;         
    
  };

  /**
   * Holds transport model parameters relevant to transport in 
   * liquids for which activated jump processes limit transport
   * (giving Arrhenius type transport properties). 
   * Used by TransportFactory.
   */
  class LiquidTransportParams : public TransportParams {
    
  public:
    
    LiquidTransportParams() {}
    ~LiquidTransportParams() {}
    
    //! Species transport parameters
    std::vector<Cantera::LiquidTransportData> LTData;
    
    //! Model for species interaction effects for viscosity
    //! Takes enum LiquidTranMixingModel
    LiquidTranMixingModel model_viscosity;
    
    
    //! Energies of molecular interaction associated with viscosity.
    /** 
     * These multiply the mixture viscosity by
     *  \f[ \exp( \sum_{i} \sum_{j} X_i X_j ( S_{i,j} + E_{i,j} / T ) ) \f].
     *
     * The overall formula for the logarithm of the mixture viscosity is 
     *
     * \f[ \ln \eta_{mix} = \sum_i X_i \ln \eta_i 
     *  + \sum_i \sum_j X_i X_j ( S_{i,j} + E_{i,j} / T ) \f].
     */
    DenseMatrix  visc_Eij; 
    
    //! Entropies of molecular interaction associated with viscosity.
    DenseMatrix  visc_Sij; 
    
    //! Model for species interaction effects for thermal conductivity
    //! Takes enum LiquidTranMixingModel
    LiquidTranMixingModel model_thermalCond;
    
    //! Interaction associated with linear weighting of 
    //! thermal conductivity.
    /**
     * This is used for either LTI_MODEL_MASSFRACS 
     * or LTI_MODEL_MOLEFRACS.  
     * The overall formula for the mixture viscosity is 
     *
     * \f[ \eta_{mix} = \sum_i X_i \eta_i 
     *  + \sum_i \sum_j X_i X_j A_{i,j} \f].
     */
    DenseMatrix  thermalCond_Aij; 
    
    //! Model for species interaction effects for mass diffusivity
    //! Takes enum LiquidTranMixingModel
    LiquidTranMixingModel model_speciesDiffusivity;
    
    //! Interaction associated with linear weighting of 
    //! thermal conductivity.
    /**
     * This is used for either LTI_MODEL_PAIRWISE_INTERACTION.
     * These provide species interaction coefficients associated with 
     * the Stefan-Maxwell formulation.
     */
    DenseMatrix  diff_Dij; 
    
    //! Model for species interaction effects for hydrodynamic radius
    //! Takes enum LiquidTranMixingModel
    LiquidTranMixingModel model_hydroradius;
    
    //! Interaction associated with hydrodynamic radius.
    /**
     * Not yet implemented 
     */
    DenseMatrix  radius_Aij; 
  };

  
  class LTI_Solvent;
  
  class LTI_MoleFracs : public LiquidTranInteraction {

  public:
    LTI_MoleFracs( const XML_Node &compModelNode = 0, 
		   TransportPropertyList tp_ind = TP_UNKNOWN, 
		   thermo_t* thermo = 0 ) ; 
    
    //! Copy constructor
    LTI_MoleFracs( const LTI_MoleFracs &right );
    
    //! Assignment operator
    LTI_MoleFracs& operator=( const LTI_MoleFracs &right );
    
    virtual ~LTI_MoleFracs( ) { } 
    
    //! Return the mixture transport property value.
    doublereal getMixTransProp( );
  protected:    
    
  };



}

#endif
