/**
 *  @file PDSS_SSVol.h
 *    Declarations for the class PDSS_SSVol (pressure dependent standard state)
 *    which handles calculations for a single species with an expression for the standard state molar volume in a phase
 *    given by an enumerated data type
 *    (see class \ref pdssthermo and \link Cantera::PDSS_SSVol PDSS_SSVol\endlink).
 */
/*
 * Copywrite (2009) Sandia Corporation. Under the terms of
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */
/*
 *  $Id: PDSS_SSVol.h,v 1.6 2008/10/13 21:01:48 hkmoffa Exp $
 */

#ifndef CT_PDSS_SSVOL_H
#define CT_PDSS_SSVOL_H

#include "PDSS.h"

namespace Cantera {
  class XML_Node;
  class VPStandardStateTP;

  //! Class for pressure dependent standard states that uses a standard state volume 
  //! model of some sort.
  /*!
   *
   *
   *
   * @ingroup pdssthermo
   */
  class PDSS_SSVol : public PDSS {

  public:
  
    /**
     * @name  Constructors
     * @{
     */

    //! Constructor
    /*!
     *  @param tp        Pointer to the ThermoPhase object pertaining to the phase
     *  @param spindex   Species index of the species in the phase
     */
    PDSS_SSVol(VPStandardStateTP *tp, int spindex);


    //! Constructor that initializes the object by examining the input file
    //! of the ThermoPhase object
    /*!
     *  This function calls the constructPDSSFile member function.
     * 
     *  @param tp        Pointer to the ThermoPhase object pertaining to the phase
     *  @param spindex   Species index of the species in the phase
     *  @param inputFile String name of the input file
     *  @param id        String name of the phase in the input file. The default
     *                   is the empty string, in which case the first phase in the
     *                   file is used.
     */
    PDSS_SSVol(VPStandardStateTP *tp, int spindex,
		 std::string inputFile, std::string id = "");

    //! Constructor that initializes the object by examining the input file
    //! of the ThermoPhase object
    /*!
     *  This function calls the constructPDSSXML member function.
     * 
     *  @param vptp_ptr    Pointer to the ThermoPhase object pertaining to the phase
     *  @param spindex     Species index of the species in the phase
     *  @param speciesNode Reference to the species XML tree.
     *  @param phaseRef    Reference to the XML tree containing the phase information.
     *  @param spInstalled Boolean indicating whether the species is installed yet
     *                     or not.
     */
    PDSS_SSVol(VPStandardStateTP *vptp_ptr, int spindex, const XML_Node& speciesNode, 
		  const XML_Node& phaseRef, bool spInstalled);

    //! Copy Constructur
    /*!
     * @param b Object to be copied
     */
    PDSS_SSVol(const PDSS_SSVol &b);

    //! Assignment operator
    /*!
     * @param b Object to be copeid
     */
    PDSS_SSVol& operator=(const PDSS_SSVol&b);

    //! Destructor
    virtual ~PDSS_SSVol();

    //! Duplicator
    virtual PDSS *duplMyselfAsPDSS() const;
        
    /**
     * @}
     * @name  Utilities  
     * @{
     */

    /**
     * @} 
     * @name  Molar Thermodynamic Properties of the Species Standard State
     *        in the Solution
     * @{
     */
  
    //! Return the molar enthalpy in units of J kmol-1
    /*!
     * Returns the species standard state enthalpy in J kmol-1 at the
     * current temperature and pressure.
     *
     * @return returns the species standard state enthalpy in  J kmol-1
     */
    virtual doublereal enthalpy_mole() const;

    //! Return the standard state molar enthalpy divided by RT
    /*!
     * Returns the species standard state enthalpy divided by RT at the
     * current temperature and pressure.
     *
     * @return returns the species standard state enthalpy in unitless form
     */
    virtual doublereal enthalpy_RT() const;

    //! Return the molar internal Energy in units of J kmol-1
    /*!
     * Returns the species standard state internal Energy in J kmol-1 at the
     * current temperature and pressure.
     *
     * @return returns the species standard state internal Energy in  J kmol-1
     */
    virtual doublereal intEnergy_mole() const;

    //! Return the molar entropy in units of J kmol-1 K-1
    /*!
     * Returns the species standard state entropy in J kmol-1 K-1 at the
     * current temperature and pressure.
     *
     * @return returns the species standard state entropy in J kmol-1 K-1
     */
    virtual doublereal entropy_mole() const;

    //! Return the standard state entropy divided by RT
    /*!
     * Returns the species standard state entropy divided by RT at the
     * current temperature and pressure.
     *
     * @return returns the species standard state entropy divided by RT
     */
    virtual doublereal entropy_R() const;

    //! Return the molar gibbs free energy in units of J kmol-1
    /*!
     * Returns the species standard state gibbs free energy in J kmol-1 at the
     * current temperature and pressure.
     *
     * @return returns the species standard state gibbs free energy in  J kmol-1
     */
    virtual doublereal gibbs_mole() const;

    //! Return the molar gibbs free energy divided by RT
    /*!
     * Returns the species standard state gibbs free energy divided by RT at the
     * current temperature and pressure.
     *
     * @return returns the species standard state gibbs free energy divided by RT
     */
    virtual doublereal gibbs_RT() const;

    //! Return the molar const pressure heat capacity in units of J kmol-1 K-1
    /*!
     * Returns the species standard state Cp in J kmol-1 K-1 at the
     * current temperature and pressure.
     *
     * @return returns the species standard state Cp in J kmol-1 K-1
     */
    virtual doublereal cp_mole() const;

    //! Return the molar const pressure heat capacity divided by RT
    /*!
     * Returns the species standard state Cp divided by RT at the
     * current temperature and pressure.
     *
     * @return returns the species standard state Cp divided by RT
     */
    virtual doublereal cp_R() const;

    //! Return the molar const volume heat capacity in units of J kmol-1 K-1
    /*!
     * Returns the species standard state Cv in J kmol-1 K-1 at the
     * current temperature and pressure.
     *
     * @return returns the species standard state Cv in J kmol-1 K-1
     */
    virtual doublereal cv_mole() const;

    //! Return the molar volume at standard state
    /*!
     * Returns the species standard state molar volume at the
     * current temperature and pressure
     *
     * @return returns the standard state molar volume divided by R
     *             units are m**3 kmol-1.
     */
    virtual doublereal molarVolume() const;

   //! Return the standard state density at standard state
    /*!
     * Returns the species standard state density at the
     * current temperature and pressure
     *
     * @return returns the standard state density
     *             units are kg m-3
     */
    virtual doublereal density() const;

    /**
     * @} 
     * @name Properties of the Reference State of the Species
     *       in the Solution 
     * @{
     */

    //! Return the molar gibbs free energy divided by RT at reference pressure
    /*!
     * Returns the species reference state gibbs free energy divided by RT at the
     * current temperature.
     *
     * @return returns the reference state gibbs free energy divided by RT
     */
    virtual doublereal gibbs_RT_ref() const;

    //! Return the molar enthalpy divided by RT at reference pressure
    /*!
     * Returns the species reference state enthalpy divided by RT at the
     * current temperature.
     *
     * @return returns the reference state enthalpy divided by RT
     */
    virtual doublereal enthalpy_RT_ref() const;

    //! Return the molar entropy divided by R at reference pressure
    /*!
     * Returns the species reference state entropy divided by R at the
     * current temperature.
     *
     * @return returns the reference state entropy divided by R
     */
    virtual doublereal entropy_R_ref() const;

    //! Return the molar heat capacity divided by R at reference pressure
    /*!
     * Returns the species reference state heat capacity divided by R at the
     * current temperature.
     *
     * @return returns the reference state heat capacity divided by R
     */
    virtual doublereal cp_R_ref() const;

    //! Return the molar volume at reference pressure
    /*!
     * Returns the species reference state molar volume at the
     * current temperature.
     *
     * @return returns the reference state molar volume divided by R
     *             units are m**3 kmol-1.
     */
    virtual doublereal molarVolume_ref() const;

  private:
  
    //! Does the internal calculation of the volume
    /*!
     *
     */
    void calcMolarVolume() const;

    /**
     * @}
     *  @name Mechanical Equation of State Properties 
     * @{
     */

    //! Sets the pressure in the object
    /*!
     * Currently, this sets the pressure in the PDSS object.
     * It is indeterminant what happens to the owning VPStandardStateTP
     * object and to the VPSSMgr object.
     *
     * @param   pres   Pressure to be set (Pascal)
     */
    virtual void setPressure(doublereal pres);

    //! Set the internal temperature
    /*!
     * @param temp Temperature (Kelvin)
     */
    virtual void setTemperature(doublereal temp);

    //! Set the internal temperature and pressure
    /*!
     * @param  temp     Temperature (Kelvin)
     * @param  pres     pressure (Pascals)
     */
    virtual void setState_TP(doublereal temp, doublereal pres);


    //! Set the internal temperature and density
    /*!
     * @param  temp     Temperature (Kelvin)
     * @param  rho      Density (kg m-3)
     */
    virtual void setState_TR(doublereal temp, doublereal rho);

    /**
     * @}
     *  @name  Miscellaneous properties of the standard state
     * @{
     */

    /// critical temperature 
    virtual doublereal critTemperature() const;
 
    /// critical pressure
    virtual doublereal critPressure() const;
        
    /// critical density
    virtual doublereal critDensity() const;
  
    /// saturation pressure
    /*!
     *  @param t  Temperature (kelvin)
     */
    virtual doublereal satPressure(doublereal t);
    
    /**
     * @}
     *  @name  Initialization of the Object
     * @{
     */
    
    //! Initialization routine for all of the shallow pointers
    /*!
     *  This is a cascading call, where each level should call the
     *  the parent level.
     *
     *  The initThermo() routines get called before the initThermoXML() routines
     *  from the constructPDSSXML() routine.
     *
     *
     *  Calls initPtrs();
     */
    virtual void initThermo();

    //! Initialization of a PDSS object using an
    //! input XML file.
    /*!
     *
     * This routine is a precursor to constructPDSSXML(XML_Node*)
     * routine, which does most of the work.
     *
     * @param vptp_ptr    Pointer to the Variable pressure %ThermoPhase object
     *                    This object must have already been malloced.
     *
     * @param spindex     Species index within the phase
     *
     * @param inputFile   XML file containing the description of the
     *                    phase
     *
     * @param id          Optional parameter identifying the name of the
     *                    phase. If none is given, the first XML
     *                    phase element will be used.
     */
    void constructPDSSFile(VPStandardStateTP *vptp_ptr, int spindex, 
			   std::string inputFile, std::string id);

    //!  Initialization of a PDSS object using an xml tree
    /*!
     * This routine is a driver for the initialization of the
     * object.
     * 
     *   basic logic:
     *       initThermo()                 (cascade)
     *       getStuff from species Part of XML file
     *       initThermoXML(phaseNode)      (cascade)
     * 
     * @param vptp_ptr   Pointer to the Variable pressure %ThermoPhase object
     *                   This object must have already been malloced.
     *
     * @param spindex    Species index within the phase
     *
     * @param speciesNode XML Node containing the species information
     *
     * @param phaseNode  Reference to the phase Information for the phase
     *                   that owns this species.
     *
     * @param spInstalled  Boolean indicating whether the species is 
     *                     already installed.
     */
    void constructPDSSXML(VPStandardStateTP *vptp_ptr, int spindex, 
			  const XML_Node& speciesNode,
			  const XML_Node& phaseNode, bool spInstalled);

    //! Initialization routine for the PDSS object based on the phaseNode
    /*!
     *  This is a cascading call, where each level should call the
     *  the parent level.
     *
     * @param phaseNode  Reference to the phase Information for the phase
     *                   that owns this species.
     *
     * @param id         Optional parameter identifying the name of the
     *                   phase. If none is given, the first XML
     *                   phase element will be used.
     */
    virtual void initThermoXML(const XML_Node& phaseNode, std::string& id);

    //@}

  private:

    //! Enumerated data type describing the type of volume model
    //! used to calculate the standard state volume of the species
    SSVolume_Model_enumType  volumeModel_;

    //! Value of the constant molar volume for the species
    /*!
     *    m3 / kmol
     */
    doublereal m_constMolarVolume;

    //! coefficients for the temperature representation
    vector_fp TCoeff_;

    //! Derivative of the volume wrt temperature
    mutable doublereal dVdT_;

    //! 2nd derivative of the volume wrt temperature
    mutable doublereal d2VdT2_;

  };

}

#endif



