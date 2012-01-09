/**
 *  @file LiquidTransport.cpp
 *  Mixture-averaged transport properties for ideal gas mixtures.
 */
#include "ThermoPhase.h"
#include "LiquidTransport.h"

#include "utilities.h"
#include "LiquidTransportParams.h"
#include "TransportFactory.h"

#include "ctlapack.h"

#include <iostream>
using namespace std;

/** 
 * Mole fractions below MIN_X will be set to MIN_X when computing
 * transport properties.
 */
#define MIN_X 1.e-14


namespace Cantera {

  //////////////////// class LiquidTransport methods //////////////


  LiquidTransport::LiquidTransport(thermo_t* thermo, int ndim) :
    Transport(thermo, ndim),
    m_nsp(0),
    m_tmin(-1.0),
    m_tmax(100000.),
    m_iStateMF(-1),
    m_temp(-1.0),
    m_logt(0.0),
    m_press(-1.0),
    m_lambda(-1.0),
    m_viscmix(-1.0),
    m_visc_mix_ok(false),
    m_visc_temp_ok(false),
    m_visc_conc_ok(false),
    m_diff_mix_ok(false),
    m_diff_temp_ok(false),
    m_cond_temp_ok(false),
    m_cond_mix_ok(false),
    m_mode(-1000),
    m_debug(false)
  {
  }


  LiquidTransport::LiquidTransport(const LiquidTransport &right) :
    Transport(),
    m_nsp(0),
    m_tmin(-1.0),
    m_tmax(100000.),
    m_iStateMF(-1),
    m_temp(-1.0),
    m_logt(0.0),
    m_press(-1.0),
    m_lambda(-1.0),
    m_viscmix(-1.0),
    m_visc_mix_ok(false),
    m_visc_temp_ok(false),
    m_visc_conc_ok(false),
    m_diff_mix_ok(false),
    m_diff_temp_ok(false),
    m_cond_temp_ok(false),
    m_cond_mix_ok(false),
    m_mode(-1000),
    m_debug(false)
  {
    /*
     * Use the assignment operator to do the brunt
     * of the work for the copy construtor.
     */
    *this = right;
  }

  LiquidTransport& LiquidTransport::operator=(const LiquidTransport& right) {
    if (&right != this) {
      return *this; 
    }
    Transport::operator=(right);
    m_nsp                                 = right.m_nsp;
    m_tmin                                = right.m_tmin;
    m_tmax                                = right.m_tmax;
    m_mw                                  = right.m_mw;
    m_visc_A                              = right.m_visc_A; 
    m_visc_logA                           = right.m_visc_logA; 
    m_visc_n                              = right.m_visc_n; 
    m_visc_Tact                           = right.m_visc_Tact; 
    m_visc_Eij                            = right.m_visc_Eij; 
    m_visc_Sij                            = right.m_visc_Sij; 
    m_thermCond_A                         = right.m_thermCond_A; 
    m_thermCond_n                         = right.m_thermCond_n; 
    m_thermCond_Tact                      = right.m_thermCond_Tact; 
    m_hydrodynamic_radius                 = right.m_hydrodynamic_radius;
    m_diffcoeffs                          = right.m_diffcoeffs;
    m_Grad_X                              = right.m_Grad_X;
    m_Grad_T                              = right.m_Grad_T;
    m_Grad_V                              = right.m_Grad_V;
    m_ck_Grad_mu                          = right.m_ck_Grad_mu;
    m_bdiff                               = right.m_bdiff;
    m_viscSpecies                         = right.m_viscSpecies;
    m_logViscSpecies                      = right.m_logViscSpecies;
    m_condSpecies                         = right.m_condSpecies;
    m_iStateMF = -1;
    m_molefracs                           = right.m_molefracs;
    m_concentrations                      = right.m_concentrations;
    m_chargeSpecies                       = right.m_chargeSpecies;
    m_DiffCoeff_StefMax                   = right.m_DiffCoeff_StefMax;
    viscosityModel_                       = right.viscosityModel_;
    m_B                                   = right.m_B;
    m_A                                   = right.m_A;
    m_temp                                = right.m_temp;
    m_logt                                = right.m_logt;
    m_press                               = right.m_press;
    m_flux                                = right.m_flux;
    m_lambda                              = right.m_lambda;
    m_viscmix                             = right.m_viscmix;
    m_spwork                              = right.m_spwork;
    m_visc_mix_ok    = false;
    m_visc_temp_ok   = false;
    m_visc_conc_ok   = false;
    m_diff_mix_ok    = false;
    m_diff_temp_ok   = false;
    m_cond_temp_ok   = false;
    m_cond_mix_ok    = false;
    m_mode                                = right.m_mode;
    m_diam                                = right.m_diam;
    m_debug                               = right.m_debug;
    m_nDim                                = right.m_nDim;

    return *this; 
  }


  Transport *LiquidTransport::duplMyselfAsTransport() const {
    LiquidTransport* tr = new LiquidTransport(*this);
    return (dynamic_cast<Transport *>(tr));
  }

  // Initialize the object
  /*
   *  This is where we dimension everything.
   */
  bool LiquidTransport::initLiquid(LiquidTransportParams& tr) {

    // constant substance attributes
    m_thermo = tr.thermo;
    m_nsp   = m_thermo->nSpecies();
    m_tmin  = m_thermo->minTemp();
    m_tmax  = m_thermo->maxTemp();

    // make a local copy of the molecular weights
    m_mw.resize(m_nsp);
    copy(m_thermo->molecularWeights().begin(), 
	 m_thermo->molecularWeights().end(), m_mw.begin());

    // copy parameters into local storage
    m_visc_A           = tr.visc_A ; 
    m_visc_n           = tr.visc_n ;
    m_visc_Tact        = tr.visc_Tact ;

    //The following two are not yet filled in LiquidTransportParams
    m_visc_Eij         = tr.visc_Eij ; 
    m_visc_Sij         = tr.visc_Sij ; 

    //save logarithm of pre-exponential for easier computation
    m_visc_logA.resize(m_nsp);
    for ( int i = 0; i < m_nsp; i++ )
      m_visc_logA[i] = log( m_visc_A[i] );

    m_thermCond_A      = tr.thermCond_A ; 
    m_thermCond_n      = tr.thermCond_n ;
    m_thermCond_Tact   = tr.thermCond_Tact ;
    
    m_hydrodynamic_radius = tr.hydroRadius ;


    //m_diffcoeffs = tr.diffcoeffs;

    m_mode       = tr.mode_;

    m_viscSpecies.resize(m_nsp);
    m_logViscSpecies.resize(m_nsp);
    m_condSpecies.resize(m_nsp);
    m_bdiff.resize(m_nsp, m_nsp);

    m_molefracs.resize(m_nsp);
    m_spwork.resize(m_nsp);

    // resize the internal gradient variables
    m_Grad_X.resize(m_nDim * m_nsp, 0.0);
    m_Grad_T.resize(m_nDim, 0.0);
    m_Grad_V.resize(m_nDim, 0.0);
    m_ck_Grad_mu.resize(m_nDim * m_nsp, 0.0);


    // set all flags to false
    m_visc_mix_ok   = false;
    m_visc_temp_ok  = false;
    m_visc_conc_ok  = false;

    m_cond_temp_ok = false;
    m_cond_mix_ok  = false;
    m_diff_temp_ok   = false;
    m_diff_mix_ok  = false;

    return true;
  }



  /******************  viscosity ******************************/

  /*
   * The viscosity is computed using the Wilke mixture rule.
   * \f[
   * \mu = \sum_k \frac{\mu_k X_k}{\sum_j \Phi_{k,j} X_j}.
   * \f]
   * Here \f$ \mu_k \f$ is the viscosity of pure species \e k,
   * and 
   * \f[
   * \Phi_{k,j} = \frac{\left[1 
   * + \sqrt{\left(\frac{\mu_k}{\mu_j}\sqrt{\frac{M_j}{M_k}}\right)}\right]^2}
   * {\sqrt{8}\sqrt{1 + M_k/M_j}}
   * \f] 
   * @see updateViscosity_T();
   */ 
  doublereal LiquidTransport::viscosity() {
        
    update_temp();
    update_conc();

    if (m_visc_mix_ok) return m_viscmix;
  
    // update m_viscSpecies[] if necessary
    if (!m_visc_temp_ok) {
      updateViscosity_temp();
    }

    if (!m_visc_conc_ok) {
      updateViscosities_conc();
    }

    /* We still need to implement interaction parameters */
    /* This constant viscosity model has no input */
    if (viscosityModel_ == LVISC_CONSTANT) {

      err("constant viscosity not implemented for LiquidTransport.");
      //return m_viscmix;

    } else if (viscosityModel_ == LVISC_AVG_ENERGIES) {

      m_viscmix = exp( dot_product(m_logViscSpecies, m_molefracs) );

    } else if (viscosityModel_ == LVISC_INTERACTION) {

      // log_visc_mix = sum_i (X_i log_visc_i) + sum_i sum_j X_i X_j G_ij
      double interaction = dot_product(m_logViscSpecies, m_molefracs);
      for ( int i = 0; i < m_nsp; i++ ) 
	for ( int j = 0; j < i; j++ ) 
	  interaction += m_molefracs[i] * m_molefracs[j] 
	    * ( m_visc_Sij(i,j) + m_visc_Eij(i,j) / m_temp );
      m_viscmix = exp( interaction );

    }
    
    return m_viscmix;
  }

  void LiquidTransport::getSpeciesViscosities(doublereal* visc) { 
    update_temp();
    if (!m_visc_temp_ok) {
      updateViscosity_temp();
    }
    copy(m_viscSpecies.begin(), m_viscSpecies.end(), visc); 
  }


  /******************* binary diffusion coefficients **************/


  void LiquidTransport::getBinaryDiffCoeffs(int ld, doublereal* d) {
    int i,j;

    update_temp();

    // if necessary, evaluate the binary diffusion coefficents
    // from the polynomial fits
    if (!m_diff_temp_ok) updateDiff_temp();
    doublereal pres = m_thermo->pressure();

    doublereal rp = 1.0/pres;
    for (i = 0; i < m_nsp; i++) 
      for (j = 0; j < m_nsp; j++) {
	d[ld*j + i] = rp * m_bdiff(i,j);
      }
  }
 //================================================================================================
  //  Get the electrical Mobilities (m^2/V/s).
  /*
   *   This function returns the mobilities. In some formulations
   *   this is equal to the normal mobility multiplied by faraday's constant.
   *
   *   Frequently, but not always, the mobility is calculated from the
   *   diffusion coefficient using the Einstein relation
   *
   *     \f[
   *          \mu^e_k = \frac{F D_k}{R T}
   *     \f]
   *
   * @param mobil_e  Returns the mobilities of
   *               the species in array \c mobil_e. The array must be
   *               dimensioned at least as large as the number of species.
   */
  void LiquidTransport::getMobilities(doublereal* const mobil) {
    int k;
    getMixDiffCoeffs(DATA_PTR(m_spwork));
    doublereal c1 = ElectronCharge / (Boltzmann * m_temp);
    for (k = 0; k < m_nsp; k++) {
      mobil[k] = c1 * m_spwork[k];
    }
  } 

  //================================================================================================
  //! Get the fluid mobilities (s kmol/kg).
  /*!
   *   This function returns the fluid mobilities. Usually, you have
   *   to multiply Faraday's constant into the resulting expression
   *   to general a species flux expression.
   *
   *   Frequently, but not always, the mobility is calculated from the
   *   diffusion coefficient using the Einstein relation
   *
   *     \f[ 
   *          \mu^f_k = \frac{D_k}{R T}
   *     \f]
   *
   *
   * @param mobil_f  Returns the mobilities of
   *               the species in array \c mobil. The array must be
   *               dimensioned at least as large as the number of species.
   */
  void  LiquidTransport::getFluidMobilities(doublereal* const mobil_f) {
    getMixDiffCoeffs(DATA_PTR(m_spwork));
    doublereal c1 = 1.0 / (GasConstant * m_temp);
    for (int k = 0; k < m_nsp; k++) {
      mobil_f[k] = c1 * m_spwork[k];
    }
  } 
  //================================================================================================
  void LiquidTransport::set_Grad_V(const doublereal* const grad_V) {
    for (int a = 0; a < m_nDim; a++) {
      m_Grad_V[a] = grad_V[a];
    }
  }
  //================================================================================================
  void LiquidTransport::set_Grad_T(const doublereal* const grad_T) {
    for (int a = 0; a < m_nDim; a++) {
      m_Grad_T[a] = grad_T[a];
    }
  }
  //================================================================================================
  void LiquidTransport::set_Grad_X(const doublereal* const grad_X) {
    int itop = m_nDim * m_nsp;
    for (int i = 0; i < itop; i++) {
      m_Grad_X[i] = grad_X[i];
    }
    update_Grad_lnAC();
  }
  //================================================================================================
  /****************** thermal conductivity **********************/

  /*
   * The thermal conductivity is computed from the following mixture rule:
   * \[
   * \lambda = 0.5 \left( \sum_k X_k \lambda_k 
   * + \frac{1}{\sum_k X_k/\lambda_k}\right)
   * \]
   */
  doublereal LiquidTransport::thermalConductivity() {
   
    update_temp();
    update_conc();

    if (!m_cond_temp_ok) {
      updateCond_temp();
    } 
    if (!m_cond_mix_ok) {
      doublereal sum1 = 0.0, sum2 = 0.0;
      for (int k = 0; k < m_nsp; k++) {
	sum1 += m_molefracs[k] * m_condSpecies[k];
	sum2 += m_molefracs[k] / m_condSpecies[k];
      }
      m_lambda = 0.5*(sum1 + 1.0/sum2);
      m_cond_mix_ok = true;
    }

    return m_lambda;
  }


  /****************** thermal diffusion coefficients ************/

  /**
   * Thermal diffusion is not considered in this mixture-averaged
   * model. To include thermal diffusion, use transport manager
   * MultiTransport instead. This methods fills out array dt with
   * zeros.
   */
  void LiquidTransport::getThermalDiffCoeffs(doublereal* const dt) {
    for (int k = 0; k < m_nsp; k++) {
      dt[k] = 0.0;
    }
  }

  /**
   * @param ndim The number of spatial dimensions (1, 2, or 3).
   * @param grad_T The temperature gradient (ignored in this model).
   * @param ldx  Leading dimension of the grad_X array.
   * The diffusive mass flux of species \e k is computed from
   *
   * \f[
   *      \vec{j}_k = -n M_k D_k \nabla X_k.
   * \f]
   */
  void LiquidTransport::getSpeciesFluxes(int ndim, 
					 const doublereal* grad_T, 
					 int ldx, const doublereal* grad_X, 
					 int ldf, doublereal* fluxes) {
    set_Grad_T(grad_T);
    set_Grad_X(grad_X);
    getSpeciesFluxesExt(ldf, fluxes);
  }

  /**
   * @param ndim The number of spatial dimensions (1, 2, or 3).
   * @param grad_T The temperature gradient (ignored in this model).
   * @param ldx  Leading dimension of the grad_X array.
   * The diffusive mass flux of species \e k is computed from
   *
   * \f[
   *      \vec{j}_k = -n M_k D_k \nabla X_k.
   * \f]
   */
  void LiquidTransport::getSpeciesFluxesExt(int ldf, doublereal* fluxes) {
    int n, k;

    update_temp();
    update_conc();


    getMixDiffCoeffs(DATA_PTR(m_spwork));


    const array_fp& mw = m_thermo->molecularWeights();
    const doublereal* y  = m_thermo->massFractions();
    doublereal rhon = m_thermo->molarDensity();
    // Unroll wrt ndim
    vector_fp sum(m_nDim,0.0);
    for (n = 0; n < m_nDim; n++) {
      for (k = 0; k < m_nsp; k++) {
	fluxes[n*ldf + k] = -rhon * mw[k] * m_spwork[k] * m_Grad_X[n*m_nsp + k];
	sum[n] += fluxes[n*ldf + k];
      }
    }
    // add correction flux to enforce sum to zero
    for (n = 0; n < m_nDim; n++) {
      for (k = 0; k < m_nsp; k++) {
	fluxes[n*ldf + k] -= y[k]*sum[n];
      }
    }
  }

  /**
   * Mixture-averaged diffusion coefficients [m^2/s]. 
   *
   * For the single species case or the pure fluid case
   * the routine returns the self-diffusion coefficient.
   * This is need to avoid a Nan result in the formula
   * below.
   */
  void LiquidTransport::getMixDiffCoeffs(doublereal* const d) {

    update_temp();
    update_conc();

    // update the binary diffusion coefficients if necessary
    if (!m_diff_temp_ok) {
      updateDiff_temp();
    }
 
    int k, j;
    doublereal mmw = m_thermo->meanMolecularWeight();
    doublereal sumxw_tran = 0.0;
    doublereal sum2;
 
    if (m_nsp == 1) {
      d[0] = m_bdiff(0,0);
    } else {
      for (k = 0; k < m_nsp; k++) {
	sumxw_tran += m_molefracs_tran[k] * m_mw[k];
      }
      for (k = 0; k < m_nsp; k++) {
	sum2 = 0.0;
	for (j = 0; j < m_nsp; j++) {
	  if (j != k) {
	    sum2 += m_molefracs_tran[j] / m_bdiff(j,k);
	  }
	}
	// Because we use m_molefracs_tran, sum2 must be positive definate
	// if (sum2 <= 0.0) {
	//  d[k] = m_bdiff(k,k);
	//  } else {
	d[k] = (sumxw_tran - m_molefracs_tran[k] * m_mw[k])/(mmw * sum2);
	//  }
      }
    }
  }

                 
  // Handles the effects of changes in the Temperature, internally
  // within the object.
  /*
   *  This is called whenever a transport property is
   *  requested.  
   *  The first task is to check whether the temperature has changed
   *  since the last call to update_temp().
   *  If it hasn't then an immediate return is carried out.
   *
   *     @internal
   */ 
  void LiquidTransport::update_temp()
  {
    // First make a decision about whether we need to recalculate
    doublereal t = m_thermo->temperature();
    if (t == m_temp) return;

    // Next do a reality check on temperature value
    if (t < 0.0) {
      throw CanteraError("LiquidTransport::update_temp()",
			 "negative temperature "+fp2str(t));
    }

    // Compute various direct functions of temperature
    m_temp = t;
    m_logt = log(m_temp);
    m_kbt = Boltzmann * m_temp;

    // temperature has changed so temp flags are flipped
    m_visc_temp_ok  = false;
    m_diff_temp_ok  = false;

    // temperature has changed, so polynomial temperature 
    // interpolations will need to be reevaluated.
    // This means that many concentration 
    m_visc_conc_ok  = false;
    m_cond_temp_ok  = false;

    // Mixture stuff needs to be evaluated 
    m_visc_mix_ok = false;
    m_diff_mix_ok = false;
    //  m_cond_mix_ok = false; (don't need it because a lower lvl flag is set    

  }                 


  // Handles the effects of changes in the mixture concentration
  /*
   *   This is called for every interface call to check whether
   *   the concentrations have changed. Concentrations change
   *   whenever the pressure or the mole fraction has changed.
   *   If it has changed, the recalculations should be done.
   *
   *   Note this should be a lightweight function since it's
   *   part of all of the interfaces.
   *
   *   @internal
   */ 
  void LiquidTransport::update_conc() {
    // If the pressure has changed then the concentrations 
    // have changed.
    doublereal pres = m_thermo->pressure();
    bool qReturn = true;
    if (pres != m_press) {
      qReturn = false;
      m_press = pres;
    } 
    int iStateNew = m_thermo->stateMFNumber();
    if (iStateNew != m_iStateMF) {
      qReturn = false;
      m_thermo->getMoleFractions(DATA_PTR(m_molefracs));
      m_thermo->getConcentrations(DATA_PTR(m_concentrations));
      concTot_ = 0.0;
      concTot_tran_ = 0.0;
      for (int k = 0; k < m_nsp; k++) {
	m_molefracs[k] = fmaxx(0.0, m_molefracs[k]);
	m_molefracs_tran[k] = fmaxx(MIN_X, m_molefracs[k]);
	concTot_tran_ += m_molefracs_tran[k];
	concTot_ += m_concentrations[k];
      }
      dens_ = m_thermo->density();
      meanMolecularWeight_ =  m_thermo->meanMolecularWeight();
      concTot_tran_ *= concTot_;
    }
    if (qReturn) {
      return;
    }

    // signal that concentration-dependent quantities will need to
    // be recomputed before use, and update the local mole
    // fractions.
    m_visc_conc_ok = false;
  
    // Mixture stuff needs to be evaluated
    m_visc_mix_ok = false;
    m_diff_mix_ok = false;
    m_cond_mix_ok = false;
  }


  // We formulate the directional derivative
  /*
   *     We only calculate the change in ac due to composition.
   *  The pressure and the temperature are taken care of in
   *  other parts of the expression.
   *
   */
  void LiquidTransport::update_Grad_lnAC() {
    int k;
    

    for (int a = 0; a < m_nDim; a++) {
      // We form the directional derivative
      double * ma_Grad_X = &m_Grad_X[a*m_nsp];
      double sum = 0.0;
      for (k = 0; k < m_nsp; k++) {
        sum += ma_Grad_X[k] * ma_Grad_X[k];
      }
      if (sum == 0.0) {
	for (k = 0; k < m_nsp; k++) {
	  m_Grad_lnAC[m_nsp * a + k] = 0.0;
	}
	continue;
      }
      double mag = 1.0E-7 / sum;
    
	for (k = 0; k < m_nsp; k++) {
	  Xdelta_[k] = m_molefracs[k] + mag * ma_Grad_X[k];
	  if (Xdelta_[k] > 1.0) {
	    Xdelta_[k] = 1.0;
	  }
	  if (Xdelta_[k] < 0.0) {
	    Xdelta_[k] = 0.0;
	  }
	}
      m_thermo->setMoleFractions(DATA_PTR(Xdelta_));
      m_thermo->getActivityCoefficients(DATA_PTR(lnActCoeffMolarDelta_));
      for (k = 0; k < m_nsp; k++) {
	lnActCoeffMolarDelta_[k] = log(lnActCoeffMolarDelta_[k]);
      }

      for (k = 0; k < m_nsp; k++) {
	m_Grad_lnAC[m_nsp * a + k] =
	  sum * (lnActCoeffMolarDelta_[k] - log(actCoeffMolar_[k])) / mag;
      }
    }
    m_thermo->setMoleFractions(DATA_PTR(m_molefracs));

  }

  /*************************************************************************
   *
   *    methods to update temperature-dependent properties
   *
   *************************************************************************/

  /**
   * Update the temperature-dependent parts of the mixture-averaged 
   * thermal conductivity. 
   */
  void LiquidTransport::updateCond_temp() {


    /*
    if (m_mode == CK_Mode) {
      for (k = 0; k < m_nsp; k++) {
	m_condSpecies[k] = exp(m_condcoeffs[k]);
      }
    } else {
      for (k = 0; k < m_nsp; k++) {
	m_condSpecies[k] = m_sqrt_t * m_condcoeffs[k];
      }
    }
    m_cond_temp_ok = true;
    m_cond_mix_ok = false;
    */
  }


  /**
   * Update the binary diffusion coefficients. These are evaluated
   * from the polynomial fits at unit pressure (1 Pa).
   */
  void LiquidTransport::updateDiff_temp() {

    // evaluate binary diffusion coefficients at unit pressure

    /*
    if (m_mode == CK_Mode) {
      for (i = 0; i < m_nsp; i++) {
	for (j = i; j < m_nsp; j++) {
	  m_bdiff(i,j) = exp(m_diffcoeffs[ic]);
	  m_bdiff(j,i) = m_bdiff(i,j);
	  ic++;
	}
      }
    }       
    else {
      for (i = 0; i < m_nsp; i++) {
	for (j = i; j < m_nsp; j++) {
	  m_bdiff(i,j) = m_temp * m_sqrt_t*m_diffcoeffs[ic];
	  m_bdiff(j,i) = m_bdiff(i,j);
	  ic++;
	}
      }
    }

    m_diff_temp_ok = true;
    m_diff_mix_ok = false;
    */
  }


  /**
   * Update the pure-species viscosities.
   */
  void LiquidTransport::updateViscosities_conc() {
    m_visc_conc_ok = true;
  }


  /**
   * Update the temperature-dependent viscosity terms.
   * Updates the array of pure species viscosities, and the 
   * weighting functions in the viscosity mixture rule.
   * The flag m_visc_ok is set to true.
   */
  void LiquidTransport::updateViscosity_temp() {
    int k;

    for (k = 0; k < m_nsp; k++) {
      m_logViscSpecies[k] = m_visc_logA[k] + m_visc_n[k] * m_logt 
					    + m_visc_Tact[k] / m_temp ;
      m_viscSpecies[k] = exp( m_logViscSpecies[k] );
    }
    //for (k = 0; k < m_nsp; k++) {
      //m_viscSpecies[k] = m_visc_A[k] * exp( m_visc_n[k] * m_logt 
      //				    + m_visc_Tact[k] / m_temp );
    //}
    m_visc_temp_ok = true;
    m_visc_mix_ok = false;
  }


  /*
   *
   *    Solve for the diffusional velocities in the Stefan-Maxwell equations
   *
   */
  void LiquidTransport::stefan_maxwell_solve() {
    int i, j, a;
    doublereal tmp;
    int VIM = m_nDim;
    m_B.resize(m_nsp, VIM);
    //! grab a local copy of the molecular weights
    const vector_fp& M =  m_thermo->molecularWeights();
    
 
    /*
     * Update the concentrations in the mixture.
     */
    update_conc();

    double T = m_thermo->temperature();

 
    m_thermo->getStandardVolumes(DATA_PTR(volume_specPM_));
    m_thermo->getActivityCoefficients(DATA_PTR(actCoeffMolar_));

    /* 
     *  Calculate the electrochemical potential gradient. This is the
     *  driving force for relative diffusional transport.
     *
     *  Here we calculate
     *
     *          c_i * (grad (mu_i) + S_i grad T - M_i / dens * grad P
     *
     *   This is  Eqn. 13-1 p. 318 Newman. The original equation is from
     *   Hershfeld, Curtis, and Bird.
     *
     *   S_i is the partial molar entropy of species i. This term will cancel
     *   out a lot of the grad T terms in grad (mu_i), therefore simplifying
     *   the expression.
     *
     *  Ok I think there may be many ways to do this. One way is to do it via basis
     *  functions, at the nodes, as a function of the variables in the problem.
     *
     *  For calculation of molality based thermo systems, we current get
     *  the molar based values. This may change.
     *
     *  Note, we have broken the symmetry of the matrix here, due to 
     *  consideratins involving species concentrations going to zero.
     *
     */
    for (i = 0; i < m_nsp; i++) {
      double xi_denom = m_molefracs_tran[i];
      for (a = 0; a < VIM; a++) {
	m_ck_Grad_mu[a*m_nsp + i] =
	  m_chargeSpecies[i] * concTot_ * Faraday * m_Grad_V[a]
	  + concTot_ * (volume_specPM_[i] - M[i]/dens_) * m_Grad_P[a]
	  + concTot_ * GasConstant * T * m_Grad_lnAC[a*m_nsp+i] / actCoeffMolar_[i]
	  + concTot_ * GasConstant * T * m_Grad_X[a*m_nsp+i] / xi_denom;
      }
    }

    if (m_thermo->activityConvention() == cAC_CONVENTION_MOLALITY) {
      int iSolvent = 0;
      double mwSolvent = m_thermo->molecularWeight(iSolvent);
      double mnaught = mwSolvent/ 1000.;
      double lnmnaught = log(mnaught);
      for (i = 1; i < m_nsp; i++) {
	for (a = 0; a < VIM; a++) {
	  m_ck_Grad_mu[a*m_nsp + i] -=
	    m_concentrations[i] * GasConstant * m_Grad_T[a] * lnmnaught;
	}
      }
    }

    /*
     * Just for Note, m_A(i,j) refers to the ith row and jth column.
     * They are still fortran ordered, so that i varies fastest.
     */
    switch (VIM) {
    case 1:  /* 1-D approximation */
      m_B(0,0) = 0.0;
      for (j = 0; j < m_nsp; j++) {
	m_A(0,j) = M[j] * m_concentrations[j];
      }
      for (i = 1; i < m_nsp; i++){
	m_B(i,0) = m_ck_Grad_mu[i] / (GasConstant * T);
	m_A(i,i) = 0.0;
	for (j = 0; j < m_nsp; j++){
	  if (j != i) {
	    tmp = m_concentrations[j] / m_DiffCoeff_StefMax(i,j);
	    m_A(i,i) +=   tmp;
	    m_A(i,j)  = - tmp;
	  }
	}
      }

      //! invert and solve the system  Ax = b. Answer is in m_B
      solve(m_A, m_B);
  	
      break;
    case 2:  /* 2-D approximation */
      m_B(0,0) = 0.0;
      m_B(0,1) = 0.0;
      for (j = 0; j < m_nsp; j++) {
	m_A(0,j) = M[j] * m_concentrations[j];
      }
      for (i = 1; i < m_nsp; i++){
	m_B(i,0) =  m_ck_Grad_mu[i]         / (GasConstant * T);
	m_B(i,1) =  m_ck_Grad_mu[m_nsp + i] / (GasConstant * T);
	m_A(i,i) = 0.0;
	for (j = 0; j < m_nsp; j++) {
	  if (j != i) {
	    tmp =  m_concentrations[j] / m_DiffCoeff_StefMax(i,j);
	    m_A(i,i) +=   tmp;
	    m_A(i,j)  = - tmp;
	  }
	}
      }

      //! invert and solve the system  Ax = b. Answer is in m_B
      solve(m_A, m_B);
	 
 	
      break;

    case 3:  /* 3-D approximation */
      m_B(0,0) = 0.0;
      m_B(0,1) = 0.0;
      m_B(0,2) = 0.0;
      for (j = 0; j < m_nsp; j++) {
	m_A(0,j) = M[j] * m_concentrations[j];
      }
      for (i = 1; i < m_nsp; i++){
	m_B(i,0) = m_ck_Grad_mu[i]           / (GasConstant * T);
	m_B(i,1) = m_ck_Grad_mu[m_nsp + i]   / (GasConstant * T);
	m_B(i,2) = m_ck_Grad_mu[2*m_nsp + i] / (GasConstant * T);
	m_A(i,i) = 0.0;
	for (j = 0; j < m_nsp; j++) {
	  if (j != i) {
	    tmp =  m_concentrations[j] / m_DiffCoeff_StefMax(i,j);
	    m_A(i,i) +=   tmp;
	    m_A(i,j)  = - tmp;
	  }
	}
      }

      //! invert and solve the system  Ax = b. Answer is in m_B
      solve(m_A, m_B);

      break;
    default:
      printf("uninmplemetnd\n");
      throw CanteraError("routine", "not done");
      break;
    }

    for (a = 0; a < VIM; a++) {
      for (j = 0; j < m_nsp; j++) {
	m_flux(j,a) =  M[j] * m_concentrations[j] * m_B(j,a);
      }
    }
  }


  /**
   * Throw an exception if this method is invoked. 
   * This probably indicates something is not yet implemented.
   */
    doublereal LiquidTransport::err(std::string msg) const {
      throw CanteraError("Liquid Transport Class",
			 "\n\n\n**** Method "+ msg +" not implemented in model "
			 + int2str(model()) + " ****\n"
			 "(Did you forget to specify a transport model?)\n\n\n");
      
      return 0.0;
    }


}
