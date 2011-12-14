
/** 
 * @file TortuosityBruggeman.h
 *  Class to compute the increase in diffusive path length in porous media
 *  assuming the Bruggeman exponent relation
 */

/*
 * Copywrite (2005) Sandia Corporation. Under the terms of
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */

/* 
 * $Revision: 572 $
 * $Date: 2010-08-13 20:21:57 -0600 (Fri, 13 Aug 2010) $
 */
#ifndef CT_TORTUOSITYBRUGGEMAN_H
#define CT_TORTUOSITYBRUGGEMAN_H


namespace Cantera {

  //! Specific Class to handle tortuosity corrections for diffusive transport
  //! in porous media using the Bruggeman exponent
  /*!
   * Class to compute the increase in diffusive path length associated with 
   * tortuous path diffusion through, for example, porous media.
   * This base class implementation relates tortuosity to volume fraction
   * through a power-law relationship that goes back to Bruggemann.  The 
   * exponent is referred to as the Bruggemann exponent.
   * 
   * Note that the total diffusional flux is generally written as 
   * 
   * \f[ 
   *   \frac{ \phi C_T D_i \nabla X_i }{ \tau^2 } 
   * \f]
   * 
   * where \f$ \phi \f$ is the volume fraction of the transported phase,
   * \f$ \tau \f$ is referred to as the tortuosity.  (Other variables are 
   * \f$ C_T \f$, the total concentration, \f$ D_i \f$, the diffusion 
   * coefficient, and \f$ X_i \f$, the mole fraction with Fickian 
   * transport assumed.)
   *
   * The tortuosity comes into play in conjunction the the 
   */
  class TortuosityBruggeman {
    
  public: 
    //! Default constructor uses Bruggemann exponent of 1.5 
    TortuosityBruggeman(double setPower = 1.5 ) : expBrug_(setPower) {
    }
    
    //! The tortuosity factor models the effective increase in the
    //! diffusive transport length.
    /** 
     * This method returns \f$ 1/\tau^2 \f$ in the description of the 
     * flux \f$ \phi C_T D_i \nabla X_i / \tau^2 \f$.  
     */ 
    virtual double toruosityFactor( double porosity ) { 
      return pow( porosity, expBrug_ - 1.0 );
    }

    //! The McMillan number is the ratio of the flux-like 
    //! variable to the value it would have without porous flow.
    /** 
     * The McMillan number combines the effect of toruosity 
     * and volume fraction of the transported phase.  The net flux
     * observed is then the product of the McMillan number and the 
     * non-porous transport rate.  For a conductivity in a non-porous 
     * media, \f$ \kappa_0 \f$, the conductivity in the porous media
     * would be \f$ \kappa = (\rm McMillan) \kappa_0 \f$.
     */
    virtual double McMillan( double porosity ) { 
      return pow( porosity, expBrug_  );
    }
    
  protected:
    //! Bruggemann exponent: power to which the tortuosity depends on the volume fraction 
    double expBrug_ ;
    
  };
    


  /** This class implements transport coefficient corrections 
   * appropriate for porous media where percollation theory applies.
   * It is derived from the Tortuosity class.
   */
    class TortuosityPercolation : public  Tortuosity  {
      
    public: 
      //! Default constructor uses Bruggemann exponent of 1.5 
      TortuosityPercolation( double percolationThreshold = 0.4, double conductivityExponent = 2.0 ) : percolationThreshold_(percolationThreshold), conductivityExponent_(conductivityExponent)  {
      }

      //! The tortuosity factor models the effective increase in the
      //! diffusive transport length.
      /** 
       * This method returns \f$ 1/\tau^2 \f$ in the description of the 
       * flux \f$ \phi C_T D_i \nabla X_i / \tau^2 \f$.  
       */ 
      double toruosityFactor( double porosity ) { 
	return McMillan( porosity ) / porosity;
      }
      
      //! The McMillan number is the ratio of the flux-like 
      //! variable to the value it would have without porous flow.
      /** 
       * The McMillan number combines the effect of toruosity 
       * and volume fraction of the transported phase.  The net flux
       * observed is then the product of the McMillan number and the 
       * non-porous transport rate.  For a conductivity in a non-porous 
       * media, \f$ \kappa_0 \f$, the conductivity in the porous media
       * would be \f$ \kappa = (\rm McMillan) \kappa_0 \f$.
       */
      double McMillan( double porosity ) { 
	return pow( ( porosity - percolationThreshold_ )
		    / ( 1.0 - percolationThreshold_ ), 
		    conductivityExponent_  );
      }
      
    protected:
      //! Critical volume fraction / site density for percolation
      double percolationThreshold_;
      //! Conductivity exponent
      /**
       * The McMillan number (ratio of effective conductivity 
       * to non-porous conductivity) is 
       * \f[ \kappa/\kappa_0 = ( \phi - \phi_c )^\mu \f]
       * where \f$ \mu \f$ is the conductivity exponent (typical
       * values range from 1.6 to 2.0) and \f$ \phi_c \f$
       * is the percolation threshold.
       */
      double conductivityExponent_;
    };



  /** This class implements transport coefficient corrections 
   * appropriate for porous media with a dispersed phase.
   * This model goes back to Maxwell.  The formula for the 
   * conductivity is expressed in terms of the volume fraction
   * of the continuous phase, \f$ \phi \f$, and the relative 
   * conductivities of the dispersed and continuous phases, 
   * \f$ r = \kappa_d / \kappa_0 \f$.  For dilute particle
   * suspensions the effective conductivity is 
   * \f[ 
   *    \kappa / \kappa_0 = 1 + 3 ( 1 - \phi ) ( r - 1 ) / ( r + 2 ) 
   *                        + O(\phi^2)
   * \f]  
   * The class is derived from the Tortuosity class.
   */
    class TortuosityMaxwell : public Tortuosity {
      
    public: 
      //! Default constructor uses Bruggemann exponent of 1.5 
      TortuosityMaxwell( double relativeConductivites = 0.0 ) : relativeConductivites_(relativeConductivites)  {
      }

      //! The tortuosity factor models the effective increase in the
      //! diffusive transport length.
      /** 
       * This method returns \f$ 1/\tau^2 \f$ in the description of the 
       * flux \f$ \phi C_T D_i \nabla X_i / \tau^2 \f$.  
       */ 
      double toruosityFactor( double porosity ) { 
	return McMillan( porosity ) / porosity;
      }
      
      //! The McMillan number is the ratio of the flux-like 
      //! variable to the value it would have without porous flow.
      /** 
       * The McMillan number combines the effect of toruosity 
       * and volume fraction of the transported phase.  The net flux
       * observed is then the product of the McMillan number and the 
       * non-porous transport rate.  For a conductivity in a non-porous 
       * media, \f$ \kappa_0 \f$, the conductivity in the porous media
       * would be \f$ \kappa = (\rm McMillan) \kappa_0 \f$.
       */
      double McMillan( double porosity ) { 
	return 1 + 3 * ( 1.0 - porosity ) * ( relativeConductivites_ - 1.0 ) / ( relativeConductivites_ + 2 );
      }
      
    protected:
      //! Relative  conductivities of the dispersed and continuous phases, 
      //! \code{relativeConductivites_}\f$  = \kappa_d / \kappa_0 \f$.
      double relativeConductivites_;

    };

}
