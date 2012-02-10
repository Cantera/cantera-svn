/**
 *  @file MixTransport.h
 *    Headers for the MixTransport object, which models transport properties
 *    in ideal gas solutions using a mixture averaged approximation
 *    (see \ref tranprops and \link Cantera::MixTransport MixTransport \endlink) .
 */

// Copyright 2001  California Institute of Technology

#ifndef CT_MIXTRAN_H
#define CT_MIXTRAN_H

// STL includes
#include <vector>
#include <string>
#include <map>
#include <numeric>
#include <algorithm>

// Cantera includes
#include "TransportBase.h"
#include "DenseMatrix.h"

namespace Cantera
{


class GasTransportParams;


//! Class MixTransport implements mixture-averaged transport properties for ideal gas mixtures.
/*!
 *    The model is based on that described by Kee, Coltrin, and Glarborg, "Theoretical and
 *    Practical Aspects of Chemically Reacting Flow Modeling."
 *
 *
 * The viscosity is computed using the Wilke mixture rule (kg /m /s)
 *
 *    \f[
 *        \mu = \sum_k \frac{\mu_k X_k}{\sum_j \Phi_{k,j} X_j}.
 *    \f]
 *
 *     Here \f$ \mu_k \f$ is the viscosity of pure species \e k, and
 *
 *    \f[
 *        \Phi_{k,j} = \frac{\left[1
 *                     + \sqrt{\left(\frac{\mu_k}{\mu_j}\sqrt{\frac{M_j}{M_k}}\right)}\right]^2}
 *                     {\sqrt{8}\sqrt{1 + M_k/M_j}}
 *    \f]
 *
 *
 * The thermal conductivity is computed from the following mixture rule:
 *   \f[
 *          \lambda = 0.5 \left( \sum_k X_k \lambda_k  + \frac{1}{\sum_k X_k/\lambda_k} \right)
 *   \f]
 *
 *  It's used to compute the flux of energy due to a thermal gradient
 *
 *   \f[
 *          j_T =  - \lambda  \nabla T
 *   \f]
 *
 *  The flux of energy has units of energy (kg m2 /s2) per second per area.
 *
 *  The units of lambda are W / m K which is equivalent to kg m / s^3 K.
 *
 *
 */
class MixTransport : public Transport
{

protected:

    //! Default constructor.
    /*!
     *
     */
    MixTransport();

public:

    //!Copy Constructor for the %MixTransport object.
    /*!
     * @param right  %LiquidTransport to be copied
     */
    MixTransport(const MixTransport& right);

    //! Assignment operator
    /*!
     *  This is NOT a virtual function.
     *
     * @param right    Reference to %LiquidTransport object to be copied
     *                 into the current one.
     */
    MixTransport& operator=(const  MixTransport& right);

    //! Duplication routine for objects which inherit from
    //! %Transport
    /*!
     *  This virtual routine can be used to duplicate %Transport objects
     *  inherited from %Transport even if the application only has
     *  a pointer to %Transport to work with.
     *
     *  These routines are basically wrappers around the derived copy
     *  constructor.
     */
    virtual Transport* duplMyselfAsTransport() const;


    //! Destructor
    virtual ~MixTransport();

    //! Return the model id for transport
    /*!
     * @return cMixtureAverage
     */
    virtual int model() const {
        return cMixtureAveraged;
    }

    //! Viscosity of the mixture  (kg /m /s)
    /*!
     * The viscosity is computed using the Wilke mixture rule (kg /m /s)
     *
     *    \f[
     *        \mu = \sum_k \frac{\mu_k X_k}{\sum_j \Phi_{k,j} X_j}.
     *    \f]
     *
     *     Here \f$ \mu_k \f$ is the viscosity of pure species \e k, and
     *
     *    \f[
     *        \Phi_{k,j} = \frac{\left[1
     *                     + \sqrt{\left(\frac{\mu_k}{\mu_j}\sqrt{\frac{M_j}{M_k}}\right)}\right]^2}
     *                     {\sqrt{8}\sqrt{1 + M_k/M_j}}
     *    \f]
     *
     *  @return   Returns the viscosity of the mixture  ( units =  Pa s  = kg /m /s)
     *
     * @see updateViscosity_T();
     */
    virtual doublereal viscosity();

    //! returns the vector of species viscosities
    /*!
     *   @param visc  Vector of species viscosities
     */
    virtual void getSpeciesViscosities(doublereal* visc) {
        update_T();
        updateViscosity_T();
        copy(m_visc.begin(), m_visc.end(), visc);
    }

    //! Return the thermal diffusion coefficients
    /*!
     * For this approximation, these are all zero.
     *
     *  Eqns. (12.168) shows how they are used in an expression for the species flux.
     *
     * @param dt  Vector of thermal diffusion coefficients. Units = kg/m/s
     */
    virtual void getThermalDiffCoeffs(doublereal* const dt);

    //! Returns the mixture thermal conductivity (W/m /K)
    /*!
     * The thermal conductivity is computed from the following mixture rule:
     *   \f[
     *          \lambda = 0.5 \left( \sum_k X_k \lambda_k  + \frac{1}{\sum_k X_k/\lambda_k} \right)
     *   \f]
     *
     *  It's used to compute the flux of energy due to a thermal gradient
     *
     *   \f[
     *          j_T =  - \lambda  \nabla T
     *   \f]
     *
     *  The flux of energy has units of energy (kg m2 /s2) per second per area.
     *
     *  The units of lambda are W / m K which is equivalent to kg m / s^3 K.
     *
     * @return Returns the mixture thermal conductivity, with units of W/m/K
     */
    virtual doublereal thermalConductivity();

    //! Returns the matrix of binary diffusion coefficients.
    /*!
     *
     *        d[ld*j + i] = rp * m_bdiff(i,j);
     *
     *  units of m**2 / s
     *
     * @param ld   offset of rows in the storage
     * @param d    output vector of diffusion coefficients
     */
    virtual void getBinaryDiffCoeffs(const size_t ld, doublereal* const d);

    //! Returns the Mixture-averaged diffusion coefficients [m^2/s].
    /*!
     * Returns the mixture averaged diffusion coefficients for a gas, appropriate for calculating the
     * mass averged diffusive flux with respect to the mass averaged velocity using gradients of the
     * mole fraction.
     * Note, for the single species case or the pure fluid case the routine returns the self-diffusion coefficient.
     * This is need to avoid a Nan result in the formula below.
     *
     *  This is Eqn. 12.180 from "Chemicaly Reacting Flow"
     *
     *   \f[
     *       D_{km}' = \frac{\left( \bar{M} - X_k M_k \right)}{ \bar{\qquad M \qquad } }  {\left( \sum_{j \ne k} \frac{X_j}{D_{kj}} \right) }^{-1}
     *   \f]
     *
     *
     *
     *  @param d  Output Vector of mixture diffusion coefficients, \f$  D_{km}'  \f$  ,  for each species (m^2/s).
     *            length m_nsp
     */
    virtual void getMixDiffCoeffs(doublereal* const d);

    //! Get the Electrical mobilities (m^2/V/s).
    /*!
     *   This function returns the mobilities. In some formulations
     *   this is equal to the normal mobility multiplied by Faraday's constant.
     *
     *   Here, the mobility is calculated from the diffusion coefficient using the Einstein relation
     *
     *     \f[
     *          \mu^e_k = \frac{F D_k}{R T}
     *     \f]
     *
     * @param mobil  Returns the mobilities of the species in array \c mobil. The array must be
     *               dimensioned at least as large as the number of species.
     */
    virtual void getMobilities(doublereal* const mobil);

    //! Update the internal parameters whenever the temperature has changed
    /*!
     *  @internal
     *      This is called whenever a transport property is requested if the temperature has changed
     *      since the last call to update_T().
     */
    virtual void update_T();

    //! Update the internal parameters whenever the concentrations have changed
    /*!
     *  @internal
     *      This is called whenever a transport property is requested if the concentrations have changed
     *      since the last call to update_C().
     */
    virtual void update_C();

    //! Get the species diffusive mass fluxes wrt to the mass averaged velocity,
    //! given the gradients in mole fraction and temperature
    /*!
     *  Units for the returned fluxes are kg m-2 s-1.
     *
     *
     * The diffusive mass flux of species \e k is computed from
     * \f[
     *          \vec{j}_k = -n M_k D_k \nabla X_k.
     * \f]
     *
     *  @param ndim      Number of dimensions in the flux expressions
     *  @param grad_T    Gradient of the temperature
     *                    (length = ndim)
     *  @param ldx        Leading dimension of the grad_X array
     *                   (usually equal to m_nsp but not always)
     *  @param grad_X     Gradients of the mole fraction
     *                    Flat vector with the m_nsp in the inner loop.
     *                    length = ldx * ndim
     *  @param ldf        Leading dimension of the fluxes array
     *                   (usually equal to m_nsp but not always)
     *  @param fluxes     Output of the diffusive mass fluxes
     *                    Flat vector with the m_nsp in the inner loop.
     *                       length = ldx * ndim
     */
    virtual void getSpeciesFluxes(size_t ndim,  const doublereal* grad_T,
                                  int ldx, const doublereal* grad_X,
                                  int ldf, doublereal* fluxes);

    //! Initialize the transport object
    /*!
     * Here we change all of the internal dimensions to be sufficient.
     * We get the object ready to do property evaluations.
     *
     * @param tr  Transport parameters for all of the species
     *            in the phase.
     */
    virtual bool initGas(GasTransportParams& tr);

    friend class TransportFactory;


    //! Return a structure containing all of the pertinent parameters about a species that was
    //! used to construct the Transport properties in this object.
    /*!
     * @param kspec Species number to obtain the properties from.
     *
     * @return GasTransportData  returned structure.
     */
    struct GasTransportData getGasTransportData(int kspec) const;



private:

    //! Calculate the pressure from the ideal gas law
    doublereal pressure_ig() const {
        return (m_thermo->molarDensity() * GasConstant *
                m_thermo->temperature());
    }

    //! Update the temperature-dependent viscosity terms.
    /*!
     * Updates the array of pure species viscosities, and the weighting functions in the viscosity mixture rule.
     * The flag m_visc_ok is set to true.
     *
     * The formula for the weighting function is from Poling and Prausnitz.
     * See Eq. (9-5.14) of  Poling, Prausnitz, and O'Connell. The equation for the weighting function
     * \f$ \phi_{ij} \f$ is reproduced below.
     *
     *  \f[
     *        \phi_{ij} = \frac{ \left[ 1 + \left( \mu_i / \mu_j \right)^{1/2} \left( M_j / M_i \right)^{1/4} \right]^2 }
     *                    {\left[ 8 \left( 1 + M_i / M_j \right) \right]^{1/2}}
     *  \f]
     */
    void updateViscosity_T();

    //! Update the temperature dependent parts of the species thermal conductivities
    /*!
     * These are evaluated from the polynomial fits of the temperature and are assumed to be
     * independent of pressure
     */
    void updateCond_T();

    //! Update the species viscosities
    /*!
     * These are evaluated from the polynomial fits of the temperature and are assumed to be
     * independent of pressure
     */
    void updateSpeciesViscosities();

    //! Update the binary diffusion coefficients
    /*!
     * These are evaluated from the polynomial fits of the temperature at the unit pressure of 1 Pa.
     */
    void updateDiff_T();


    // --------- Member Data -------------
private:

    //! Number of species in the phase
    size_t m_nsp;

    //! Minimum value of the temperature that this transport parameterization is valid
    doublereal m_tmin;

    //! Maximum value of the temperature that this transport parameterization is valid
    doublereal m_tmax;

    //! Local copy of the species molecular weights.
    vector_fp  m_mw;

    //! Polynomial fits to the viscosity of each species
    /*!
     *  m_visccoeffs[k] is vector of polynomial coefficients for species k
     *  that fits the viscosity as a function of temperature
     */
    std::vector<vector_fp>            m_visccoeffs;

    //! Polynomial fits to the thermal conductivity of each species
    /*!
     *  m_condcoeffs[k] is vector of polynomial coefficients for species k
     *  that fits the thermal conductivity
     */
    std::vector<vector_fp>            m_condcoeffs;

    //! Polynomial fits to the binary diffusivity of each species
    /*!
     *  m_diffcoeff[ic] is vector of polynomial coefficients for species  i species  j
     *  that fits the binary diffusion coefficient. The relationship between i
     *  j and ic is determined from the following algorithm:
     *
     *     int ic = 0;
     *     for (i = 0; i < m_nsp; i++) {
     *        for (j = i; j < m_nsp; j++) {
     *          ic++;
     *        }
     *     }
     *
     */
    std::vector<vector_fp>            m_diffcoeffs;

    //! Powers of the ln temperature
    /*!
     *  up to fourth order
     */
    vector_fp                    m_polytempvec;

    //! Matrix of binary diffusion coefficients at the reference pressure and the current temperature
    /*!
     *   Size is nsp x nsp
     */
    DenseMatrix m_bdiff;

    //! vector of species viscosities (kg /m /s)
    /*!
     *  These are used in wilke's rule to calculate the viscosity of the solution
     *  length = m_kk
     */
    vector_fp m_visc;

    //! vector of square root of species viscosities sqrt(kg /m /s)
    /*!
     *  These are used in wilke's rule to calculate the viscosity of the solution
     *  length = m_kk
     */
    vector_fp m_sqvisc;

    //! vector of species thermal conductivities (W/m /K)
    /*!
     *  These are used in wilke's rule to calculate the viscosity of the solution
     *  units = W /m /K = kg m /s^3 /K.
     *  length = m_kk
     */
    vector_fp m_cond;

    //! Vector of species molefractions
    /*!
     *  These are processed so that all mole fractions are >= MIN_X
     *  Length = m_kk
     */
    vector_fp m_molefracs;

    //! m_phi is a Viscosity Weighting Function
    /*!
     *  size = m_nsp * n_nsp
     */
    DenseMatrix m_phi;

    //! Holds square roots or molecular weight ratios
    /*!
     *   m_wratjk(j,k)  = sqrt(mw[j]/mw[k])        j < k
     *   m_wratjk(k,j)  = sqrt(sqrt(mw[j]/mw[k]))  j < k
     */
    DenseMatrix m_wratjk;

    //! Holds square roots of molecular weight ratios
    /*!
     *   m_wratjk1(j,k)  = sqrt(1.0 + mw[k]/mw[j])        j < k
     */
    DenseMatrix m_wratkj1;





    //! Current value of the temperature at which the properties in this object are calculated (Kelvin)
    doublereal m_temp;

    //! Current value of the log of the temperature
    doublereal m_logt;

    //! Current value of Boltzman's constant times the temperature (Joules)
    doublereal m_kbt;

    //! Current value of temperature to 1/4 power
    doublereal m_t14;

    //! Current value of temperature to the 3/2 power
    doublereal m_t32;

    //! current value of  Boltzman's constant times the temperature (Joules) to 1/2 power
    doublereal m_sqrt_kbt;

    //! current value of temperature to 1/2 power
    doublereal m_sqrt_t;

    //! Internal storage for the calculated mixture thermal conductivity
    /*!
     *  Units = W /m /K
     */
    doublereal m_lambda;

    //! Internal storage for the viscosity of the mixture  (kg /m /s)
    doublereal m_viscmix;

    //! work space length = m_kk
    vector_fp  m_spwork;

    //! Update boolean for mixture rule for the mixture viscosity
    bool m_viscmix_ok;

    //! Update boolean for the weighting factors for the mixture viscosity
    bool m_viscwt_ok;

    //! Update boolean for the species viscosities
    bool m_spvisc_ok;

    //! Update boolean for the binary diffusivities at unit pressure
    bool m_bindiff_ok;

    //! Update boolean for the species thermal conductivities
    bool m_spcond_ok;

    //! Update boolean for the mixture rule for the mixture thermal conductivity
    bool m_condmix_ok;

    //! Type of the polynomial fits to temperature
    /*!
     * CK_Mode means chemkin mode. Currently CA_Mode is used which are different types
     * of fits to temperature.
     */
    int m_mode;

    //! Lennard-Jones well-depth of the species in the current phase
    /*!
     *  Not used in this routine -> just a passthrough
     *
     * length is the number of species in the phase
     * Units are Joules (Note this is not Joules/kmol) (note, no kmol -> this is a per molecule amount)
     */
    vector_fp m_eps;

    //! hard-sphere diameter for (i,j) collision
    /*!
     *  Not used in this routine -> just a passthrough
     *
     *  diam(i,j) = 0.5*(tr.sigma[i] + tr.sigma[j]);
     *  Units are m (note, no kmol -> this is a per molecule amount)
     *
     *  Length nsp * nsp. This is a symmetric matrix.
     */
    DenseMatrix m_diam;

    //! The effective dipole moment for (i,j) collisions
    /*!
     *  tr.dipoleMoment has units of Debye's. A Debye is 10-18 cm3/2 erg1/2
     *
     *  Not used in this routine -> just a passthrough
     *
     *    tr.dipole(i,i) = 1.e-25 * SqrtTen * trdat.dipoleMoment;
     *    tr.dipole(i,j) = sqrt(tr.dipole(i,i)*tr.dipole(j,j));
     *  Units are  in Debye  (note, no kmol -> this is a per molecule amount)
     *
     *  Length nsp. We store only the diagonal component here.
     */
    vector_fp m_dipoleDiag;

    //! Polarizability of each species in the phase
    /*!
     *  Not used in this routine -> just a passthrough
     *
     *  Length = nsp
     *  Units = m^3
     */
    vector_fp m_alpha;

    //! Dimensionless rotational heat capacity of the species in the current phase
    /*!
     *  Not used in this routine -> just a passthrough
     *
     *  These values are 0, 1 and 1.5 for single-molecule, linear, and nonlinear species respectively
     *  length is the number of species in the pahse
     *  units are dimensionless  (Cr / R)
     */
    vector_fp m_crot;

    //! Rotational relaxation number for the species in the current phase
    /*!
     *  Not used in this routine -> just a passthrough
     *
     * length is the number of species in the phase
     * units are dimensionless
     */
    vector_fp m_zrot;


    //! Debug flag - turns on more printing
    bool m_debug;
};
}
#endif
