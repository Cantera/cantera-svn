/**
 *  @file PecosTransport.cpp
 *  Mixture-averaged transport properties.
 *  
 */

/* $Author$
 * $Revision$
 * $Date$
 */

#include "cantera/thermo/ThermoPhase.h"
#include "cantera/transport/PecosTransport.h"

#include "cantera/base/utilities.h"
#include "cantera/transport/TransportParams.h"
#include "cantera/transport/TransportFactory.h"
#include "cantera/base/stringUtils.h"

#include "cantera/thermo/IdealGasPhase.h"

#include <iostream>
using namespace std;

/** 
 * Mole fractions below MIN_X will be set to MIN_X when computing
 * transport properties.
 */
#define MIN_X 1.e-20

namespace Cantera {

  //////////////////// class PecosTransport methods //////////////

  PecosTransport::PecosTransport() :
    m_nsp(0),
    m_tmin(-1.0),
    m_tmax(100000.),
    m_temp(-1.0),
    m_logt(0.0)
  {


  }

  bool PecosTransport::initGas( GasTransportParams& tr ) {

    // constant substance attributes
    m_thermo = tr.thermo;
    m_nsp   = m_thermo->nSpecies();
    m_tmin  = m_thermo->minTemp();
    m_tmax  = m_thermo->maxTemp();

    // make a local copy of the molecular weights
    m_mw.resize(m_nsp);
    copy(m_thermo->molecularWeights().begin(), 
	 m_thermo->molecularWeights().end(), m_mw.begin());

    // copy polynomials and parameters into local storage
    m_poly       = tr.poly;
    m_visccoeffs = tr.visccoeffs;
    m_condcoeffs = tr.condcoeffs;
    m_diffcoeffs = tr.diffcoeffs;

    m_zrot       = tr.zrot;
    m_crot       = tr.crot;
    m_epsilon    = tr.epsilon;
    m_mode       = tr.mode_;
    m_diam       = tr.diam;
    m_eps        = tr.eps;
    m_alpha      = tr.alpha;
    m_dipoleDiag.resize(m_nsp);
    for (int i = 0; i < m_nsp; i++) {
      m_dipoleDiag[i] = tr.dipole(i,i);
    }

    m_phi.resize(m_nsp, m_nsp, 0.0);
    m_wratjk.resize(m_nsp, m_nsp, 0.0);
    m_wratkj1.resize(m_nsp, m_nsp, 0.0);
    int j, k;
    for (j = 0; j < m_nsp; j++) 
      for (k = j; k < m_nsp; k++) {
	m_wratjk(j,k) = sqrt(m_mw[j]/m_mw[k]);
	m_wratjk(k,j) = sqrt(m_wratjk(j,k));
	m_wratkj1(j,k) = sqrt(1.0 + m_mw[k]/m_mw[j]);
      }
    
    m_polytempvec.resize(5);
    m_visc.resize(m_nsp);
    m_sqvisc.resize(m_nsp);
    m_cond.resize(m_nsp);
    m_bdiff.resize(m_nsp, m_nsp);

    m_molefracs.resize(m_nsp);
    m_spwork.resize(m_nsp);

    // set flags all false
    m_viscmix_ok = false;
    m_viscwt_ok = false;
    m_spvisc_ok = false;
    m_spcond_ok = false;
    m_condmix_ok = false;
    m_spcond_ok = false;
    m_diffmix_ok = false;
    m_abc_ok = false;

    // read blottner fit parameters (A,B,C)
    read_blottner_transport_table();
 
    // set specific heats
    cv_rot.resize(m_nsp);
      cp_R.resize(m_nsp);
    cv_int.resize(m_nsp);

    for (k = 0; k < m_nsp; k++) {
      cv_rot[k] = tr.crot[k];
      cp_R[k] = ((IdealGasPhase*)tr.thermo)->cp_R_ref()[k];
      cv_int[k] = cp_R[k] - 2.5 - cv_rot[k];
    }
    return true;
  }


  /*********************************************************
   *
   *                Public methods
   *
   *********************************************************/


  /******************  viscosity ******************************/

  /**
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
  doublereal PecosTransport::viscosity() {
        
    update_T();
    update_C();

    if (m_viscmix_ok) return m_viscmix;

    doublereal vismix = 0.0;
    int k;
    // update m_visc and m_phi if necessary
    if (!m_viscwt_ok) updateViscosity_T();

    multiply(m_phi, DATA_PTR(m_molefracs), DATA_PTR(m_spwork));

    for (k = 0; k < m_nsp; k++) {
      vismix += m_molefracs[k] * m_visc[k]/m_spwork[k]; //denom;
    }
    m_viscmix = vismix;
    return vismix;
  }

  /******************* binary diffusion coefficients **************/
  /*
   *
   *  Using Ramshaw's self-consistent Effective Binary Diffusion 
   *  (1990, J. Non-Equilib. Thermo)
   *  Adding more doxygen would be good here
   */

  void PecosTransport::getBinaryDiffCoeffs(const int ld, doublereal* const d) {
    int i,j;

    update_T();

    // if necessary, evaluate the binary diffusion coefficents
    if (!m_bindiff_ok) updateDiff_T();

    doublereal rp = 1.0/pressure_ig();
    for (i = 0; i < m_nsp; i++) 
      for (j = 0; j < m_nsp; j++) {
	d[ld*j + i] = rp * m_bdiff(i,j);
      }
  }


  void PecosTransport::getMobilities(doublereal* const mobil) {
    int k;
    getMixDiffCoeffs(DATA_PTR(m_spwork));
    doublereal c1 = ElectronCharge / (Boltzmann * m_temp);
    for (k = 0; k < m_nsp; k++) {
      mobil[k] = c1 * m_spwork[k] * m_thermo->charge(k);
    }
  } 
        

  /****************** thermal conductivity **********************/

  /**
   * The thermal conductivity is computed using the Wilke mixture rule.
   * \f[
   * \k = \sum_s \frac{k_s X_s}{\sum_j \Phi_{s,j} X_j}.
   * \f]
   * Here \f$ \k_s \f$ is the conductivity of pure species \e s,
   * and 
   * \f[
   * \Phi_{s,j} = \frac{\left[1 
   * + \sqrt{\left(\frac{\mu_k}{\mu_j}\sqrt{\frac{M_j}{M_s}}\right)}\right]^2}
   * {\sqrt{8}\sqrt{1 + M_s/M_j}}
   * \f] 
   * @see updateCond_T();
   */ 
  doublereal PecosTransport::thermalConductivity() {
    int k;
    doublereal lambda = 0.0;

    update_T();
    update_C();

    // update m_cond and m_phi if necessary
    if (!m_spcond_ok)  updateCond_T(); 
    if (!m_condmix_ok) {

      multiply(m_phi, DATA_PTR(m_molefracs), DATA_PTR(m_spwork));
      
      for (k = 0; k < m_nsp; k++) {
	lambda += m_molefracs[k] * m_cond[k]/m_spwork[k]; //denom;
      }

    }
    m_lambda = lambda;
    return m_lambda;

  }


  /****************** thermal diffusion coefficients ************/

  /**
   * Thermal diffusion is not considered in this pecos 
   * model. To include thermal diffusion, use transport manager
   * MultiTransport instead. This methods fills out array dt with
   * zeros.
   */
  void PecosTransport::getThermalDiffCoeffs(doublereal* const dt) {
    int k;
    for (k = 0; k < m_nsp; k++) {
      dt[k] = 0.0;
    }
  }

  /**
   * @param ndim The number of spatial dimensions (1, 2, or 3).
   * @param grad_T The temperature gradient (ignored in this model).
   * @param ldx  Leading dimension of the grad_X array.
   * The diffusive mass flux of species \e k is computed from
   * \f[
   * \vec{j}_k = -n M_k D_k \nabla X_k + \frac{\rho_k}{\rho} \sum_r n M_r D_r \nabla X_r
   * \f]
   *
   * This is neglective pressure, forced and thermal diffusion. 
   *
   */
  void PecosTransport::getSpeciesFluxes(int ndim, 
				      const doublereal* grad_T, int ldx, const doublereal* grad_X, 
				      int ldf, doublereal* fluxes) {
    int n, k;

    update_T();
    update_C();

    getMixDiffCoeffs(DATA_PTR(m_spwork));

    const vector_fp& mw = m_thermo->molecularWeights();
    const doublereal* y  = m_thermo->massFractions();
    doublereal rhon = m_thermo->molarDensity();

    vector_fp sum(ndim,0.0);

    doublereal correction=0.0;
    // grab 2nd (summation) term -- still need to multiply by mass fraction (\rho_s / \rho)
    for (k = 0; k < m_nsp; k++) 
      {
	correction += rhon * mw[k] * m_spwork[k] * grad_X[n*ldx + k];
      }

    for (n = 0; n < ndim; n++) {
      for (k = 0; k < m_nsp; k++) {
	fluxes[n*ldf + k] = -rhon * mw[k] * m_spwork[k] * grad_X[n*ldx + k] + y[k]*correction;
	sum[n] += fluxes[n*ldf + k];
      }
    }
    // add correction flux to enforce sum to zero
    for (n = 0; n < ndim; n++) {
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
  void PecosTransport::getMixDiffCoeffs(doublereal* const d) {

    update_T();
    update_C();

    // update the binary diffusion coefficients if necessary
    if (!m_bindiff_ok) updateDiff_T();

    int k, j;
    doublereal mmw = m_thermo->meanMolecularWeight();
    doublereal sumxw = 0.0, sum2;
    doublereal p = pressure_ig();
    if (m_nsp == 1) {
      d[0] = m_bdiff(0,0) / p;
    } else {
      for (k = 0; k < m_nsp; k++) sumxw += m_molefracs[k] * m_mw[k];
      for (k = 0; k < m_nsp; k++) {
	sum2 = 0.0;
	for (j = 0; j < m_nsp; j++) {
	  if (j != k) {
	    sum2 += m_molefracs[j] / m_bdiff(j,k);
	  }
	}
	if (sum2 <= 0.0) {
	  d[k] = m_bdiff(k,k) / p;
	} else {
	  d[k] = (sumxw - m_molefracs[k] * m_mw[k])/(p * mmw * sum2);
	}
      }
    }
  }

  void PecosTransport::getMixDiffCoeffsMole(doublereal* const d)
  {
    update_T();
    update_C();

    // update the binary diffusion coefficients if necessary
    if (!m_bindiff_ok) {
      updateDiff_T();
    }

    doublereal p = m_thermo->pressure();
    if (m_nsp == 1) {
      d[0] = m_bdiff(0,0) / p;
    } else {
      for (size_t k = 0; k < m_nsp; k++) {
	double sum2 = 0.0;
	for (size_t j = 0; j < m_nsp; j++) {
	  if (j != k) {
	    sum2 += m_molefracs[j] / m_bdiff(j,k);
	  }
	}
	if (sum2 <= 0.0) {
	  d[k] = m_bdiff(k,k) / p;
	} else {
	  d[k] = (1 - m_molefracs[k]) / (p * sum2);
	}
      }
    }
  }

  void PecosTransport::getMixDiffCoeffsMass(doublereal* const d)
  {
    update_T();
    update_C();

    // update the binary diffusion coefficients if necessary
    if (!m_bindiff_ok) {
      updateDiff_T();
    }

    doublereal mmw = m_thermo->meanMolecularWeight();
    doublereal p = m_thermo->pressure();

    if (m_nsp == 1) {
      d[0] = m_bdiff(0,0) / p;
    } else {
      for (size_t k=0; k<m_nsp; k++) {
	double sum1 = 0.0;
	double sum2 = 0.0;
	for (size_t i=0; i<m_nsp; i++) {
	  if (i==k) {
	    continue;
	  }
	  sum1 += m_molefracs[i] / m_bdiff(k,i);
	  sum2 += m_molefracs[i] * m_mw[i] / m_bdiff(k,i);
	}
	sum1 *= p;
	sum2 *= p * m_molefracs[k] / (mmw - m_mw[k]*m_molefracs[k]);
	d[k] = 1.0 / (sum1 +  sum2);
      }
    }
  }
                 
  /**
   *  @internal This is called whenever a transport property is
   *  requested from ThermoSubstance if the temperature has changed
   *  since the last call to update_T.
   */ 
  void PecosTransport::update_T() 
  {
    doublereal t = m_thermo->temperature();
    if (t == m_temp) return;
    if (t <= 0.0) {
      throw CanteraError("PecosTransport::update_T",
			 "negative temperature "+fp2str(t));
    }
    m_temp = t;
    m_logt = log(m_temp);
    m_kbt = Boltzmann * m_temp;
    m_sqrt_t = sqrt(m_temp);
    m_t14 = sqrt(m_sqrt_t);
    m_t32 = m_temp * m_sqrt_t;
    m_sqrt_kbt = sqrt(Boltzmann*m_temp);

    // compute powers of log(T)
    m_polytempvec[0] = 1.0;
    m_polytempvec[1] = m_logt;
    m_polytempvec[2] = m_logt*m_logt;
    m_polytempvec[3] = m_logt*m_logt*m_logt;
    m_polytempvec[4] = m_logt*m_logt*m_logt*m_logt;

    // temperature has changed, so polynomial fits will need to be
    // redone.
    m_viscmix_ok = false;
    m_spvisc_ok = false;
    m_viscwt_ok = false;
    m_spcond_ok = false;
    m_diffmix_ok = false;
    m_bindiff_ok = false;
    m_abc_ok  = false;
    m_condmix_ok = false;                 
  }                 

  /**
   *  @internal This is called the first time any transport property
   *  is requested from Mixture after the concentrations
   *  have changed.
   */ 
  void PecosTransport::update_C()  
  {
    // signal that concentration-dependent quantities will need to
    // be recomputed before use, and update the local mole
    // fractions.

    m_viscmix_ok = false;
    m_diffmix_ok = false;
    m_condmix_ok = false;

    m_thermo->getMoleFractions(DATA_PTR(m_molefracs));

    // add an offset to avoid a pure species condition
    int k;
    for (k = 0; k < m_nsp; k++) {
      m_molefracs[k] = std::max(MIN_X, m_molefracs[k]);
    }
  }

  /*************************************************************************
   *
   *    methods to update temperature-dependent properties
   *
   *************************************************************************/

  /**
   *
   * Update the temperature-dependent parts of the mixture-averaged 
   * thermal conductivity. 
   * 
   * Calculated as,
   * \f[
   *    k= \mu_s (5/2 * C_{v,s}^{trans} + C_{v,s}^{rot} + C_{v,s}^{vib} 
   * \f]   
   *
   *
   */
  void PecosTransport::updateCond_T() {

    int k;
    doublereal fivehalves = 5/2;
    for (k = 0; k < m_nsp; k++) {
      // need to add cv_elec in the future
      m_cond[k] = m_visc[k] * ( fivehalves * cv_int[k]  + cv_rot[k] + m_thermo->cv_vib(k,m_temp) ); 
    }
    m_spcond_ok = true;
    m_condmix_ok = false;
  }


  /**
   * Update the binary diffusion coefficients. These are evaluated
   * from the polynomial fits at unit pressure (1 Pa).
   */
  void PecosTransport::updateDiff_T() {

    // evaluate binary diffusion coefficients at unit pressure
    int i,j;
    int ic = 0;
    if (m_mode == CK_Mode) {
      for (i = 0; i < m_nsp; i++) {
	for (j = i; j < m_nsp; j++) {
	  m_bdiff(i,j) = exp(dot4(m_polytempvec, m_diffcoeffs[ic]));
	  m_bdiff(j,i) = m_bdiff(i,j);
	  ic++;
	}
      }
    }       
    else {
      for (i = 0; i < m_nsp; i++) {
	for (j = i; j < m_nsp; j++) {
	  m_bdiff(i,j) = m_temp * m_sqrt_t*dot5(m_polytempvec, 
						m_diffcoeffs[ic]);
	  m_bdiff(j,i) = m_bdiff(i,j);
	  ic++;
	}
      }
    }

    m_bindiff_ok = true;
    m_diffmix_ok = false;
  }


  /**
   *
   * Update the pure-species viscosities. (Pa-s) = (kg/m/sec)
   * 
   * Using Blottner fit for viscosity. Defines kinematic viscosity
   * of the form     
   * \f[     
   *   \mu_s\left(T\right) = 0.10 \exp\left(A_s\left(\log T\right)^2 + B_s\log T + C_s\right) 
   * \f] 
   * where \f$ A_s \f$, \f$ B_s \f$, and \f$ C_s \f$ are constants.   
   *
   */
  void PecosTransport::updateSpeciesViscosities() {

    // blottner
    // return 0.10*std::exp(_a*(logT*logT) + _b*logT + _c);

    int k;
    // iterate over species, update pure-species viscosity
    for (k = 0; k < m_nsp; k++) {
      m_visc[k] = 0.10*std::exp(a[k]*(m_logt*m_logt) + b[k]*m_logt + c[k]);
      m_sqvisc[k] = sqrt(m_visc[k]);
    }

    // time to update mixing
    m_spvisc_ok = true;
  }

  /*
   * read_blottner_transport_table()
   *  loads up A B and C for blottner fits
   *  hardcoded for air, will need to generalize later
   */ 

  void PecosTransport::read_blottner_transport_table()
  {
    // istringstream blot
    //   ("Air 2.68142000000e-02  3.17783800000e-01 -1.13155513000e+01\n"
    //    "CPAir   2.68142000000e-02  3.17783800000e-01 -1.13155513000e+01\n"
    //    "N       1.15572000000e-02  6.03167900000e-01 -1.24327495000e+01\n"
    //    "N2      2.68142000000e-02  3.17783800000e-01 -1.13155513000e+01\n"
    //    "CPN2    2.68142000000e-02  3.17783800000e-01 -1.13155513000e+01\n"
    //    "NO      4.36378000000e-02 -3.35511000000e-02 -9.57674300000e+00\n"
    //    "O       2.03144000000e-02  4.29440400000e-01 -1.16031403000e+01\n"
    //    "O2      4.49290000000e-02 -8.26158000000e-02 -9.20194750000e+00\n"
    //    "C       -8.3285e-3         0.7703240         -12.7378000\n"
    //    "C2      -8.4311e-3         0.7876060         -13.0268000\n"
    //    "C3      -8.4312e-3         0.7876090         -12.8240000\n"
    //    "C2H     -2.4241e-2         1.0946550         -14.5835500\n"
    //    "CN      -8.3811e-3         0.7860330         -12.9406000\n"
    //    "CO      -0.019527394       1.013295          -13.97873\n"
    //    "CO2     -0.019527387       1.047818          -14.32212\n"
    //    "HCN     -2.4241e-2         1.0946550         -14.5835500\n"
    //    "H       -8.3912e-3         0.7743270         -13.6653000\n"
    //    "H2      -8.3346e-3         0.7815380         -13.5351000\n"
    //    "e       0.00000000000e+00  0.00000000000e+00 -1.16031403000e+01\n");
    
    //
    // from: AIAA-1997-2474 and Sandia Report SC-RR-70-754
    // 
    //   # Air  -- Identical to N2 fit
    //   # N    -- Sandia Report SC-RR-70-754
    //   # N2   -- Sandia Report SC-RR-70-754
    //   # CPN2 -- Identical to N2 fit 
    //   # NO   -- Sandia Report SC-RR-70-754
    //   # O    -- Sandia Report SC-RR-70-754
    //   # O2   -- Sandia Report SC-RR-70-754
    //   # C    -- AIAA-1997-2474
    //   # C2   -- AIAA-1997-2474
    //   # C3   -- AIAA-1997-2474
    //   # C2H  -- wild-ass guess: identical to HCN fit
    //   # CN   -- AIAA-1997-2474
    //   # CO   -- AIAA-1997-2474
    //   # CO2  -- AIAA-1997-2474
    //   # HCN  -- AIAA-1997-2474
    //   # H    -- AIAA-1997-2474
    //   # H2   -- AIAA-1997-2474
    //   # e    -- Sandia Report SC-RR-70-754

    istringstream blot
      ("Air 2.68142000000e-02 3.17783800000e-01 -1.13155513000e+01\n"
       "CPAir 2.68142000000e-02 3.17783800000e-01 -1.13155513000e+01\n"
       "N 1.15572000000e-02 6.03167900000e-01 -1.24327495000e+01\n"
       "N2 2.68142000000e-02 3.17783800000e-01 -1.13155513000e+01\n"
       "CPN2 2.68142000000e-02 3.17783800000e-01 -1.13155513000e+01\n"
       "NO 4.36378000000e-02 -3.35511000000e-02 -9.57674300000e+00\n"
       "O 2.03144000000e-02 4.29440400000e-01 -1.16031403000e+01\n"
       "O2 4.49290000000e-02 -8.26158000000e-02 -9.20194750000e+00\n"
       "C -8.3285e-3 0.7703240 -12.7378000\n"
       "C2 -8.4311e-3 0.7876060 -13.0268000\n"
       "C3 -8.4312e-3 0.7876090 -12.8240000\n"
       "C2H -2.4241e-2 1.0946550 -14.5835500\n"
       "CN -8.3811e-3 0.7860330 -12.9406000\n"
       "CO -0.019527394 1.013295 -13.97873\n"
       "CO2 -0.019527387 1.047818 -14.32212\n"
       "HCN -2.4241e-2 1.0946550 -14.5835500\n"
       "H -8.3912e-3 0.7743270 -13.6653000\n"
       "H2 -8.3346e-3 0.7815380 -13.5351000\n"
       "e 0.00000000000e+00 0.00000000000e+00 -1.16031403000e+01\n");

    string line;
    string name;
    string ss1,ss2,ss3,ss4,sss; 
    int k;
    int i = 0;

    while (std::getline(blot, line)) 
      {

	istringstream ss(line);      
	std::getline(ss, ss1, ' ');
	std::getline(ss, ss2, ' ');
	std::getline(ss, ss3, ' ');       
	std::getline(ss, ss4, ' ');       
	name = ss1;

	// now put coefficients in correct species
	for (k = 0; k < m_nsp; k++) 
	  {
	    string sss = m_thermo->speciesName(k);

	    // this is the right species index
	    if(sss.compare(ss1) == 0)
	      {
	    	a[k] = atof(ss2.c_str());
	    	b[k] = atof(ss3.c_str());
	    	c[k] = atof(ss4.c_str());
    
	    	// index
	    	i++;
	      }
	    else // default to air
	      {
		
	    	a[k] = 0.026;
	    	b[k] = 0.3;
	    	c[k] = -11.3;
    	      }
	    
	  } // done with for loop
      }


	// for (k = 0; k < m_nsp; k++) 
	//   {
	//     string sss = m_thermo->speciesName(k);
	//     cout << sss  << endl;
	//     cout << a[k] << endl;
	//     cout << b[k] << endl;
	//     cout << c[k] << endl;	    
	//   }

    // simple sanity check
    // if(i != m_nsp-1)
    //   {
    // 	std::cout << "error\n" << i << std::endl;
    //   }

  }

  /**
   *
   * Update the temperature-dependent viscosity terms.
   * Updates the array of pure species viscosities, and the 
   * weighting functions in the viscosity mixture rule.
   * The flag m_visc_ok is set to true.
   * 
   */
  void PecosTransport::updateViscosity_T() {
    doublereal vratiokj, wratiojk, factor1;

    if (!m_spvisc_ok) updateSpeciesViscosities();

    // see Eq. (9-5.15) of Reid, Prausnitz, and Poling
    int j, k;
    for (j = 0; j < m_nsp; j++) {
      for (k = j; k < m_nsp; k++) {
	vratiokj = m_visc[k]/m_visc[j];
	wratiojk = m_mw[j]/m_mw[k];

	// Note that m_wratjk(k,j) holds the square root of
	// m_wratjk(j,k)!
	factor1 = 1.0 + (m_sqvisc[k]/m_sqvisc[j]) * m_wratjk(k,j);
	m_phi(k,j) = factor1*factor1 /
	  (SqrtEight * m_wratkj1(j,k)); 
	m_phi(j,k) = m_phi(k,j)/(vratiokj * wratiojk);
      }
    }
    m_viscwt_ok = true;
  }

}

