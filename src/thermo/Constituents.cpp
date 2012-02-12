/**
 *  @file Constituents.cpp
 *  Header file  Class \link Cantera::Constituents Constitutents\endlink which
 *  manages a set of elements and species (see \ref phases).
 */
//  Copyright 2001  California Institute of Technology

#include "cantera/thermo/Constituents.h"
#include "cantera/thermo/Elements.h"
using namespace std;

namespace Cantera
{

/*
 *  Constructor sets all base variable types to zero. Also, it
 *  sets the pointer to the Elements object for this object to the
 *  default value of BaseElements. If the BaseElements Elements
 *  object doesn't exist, it creates it.
 *
 * Input
 * --------
 * ptr_Elements: If the Constituents object requires a different
 *               Elements object than the default one, input
 *               address here. This argument defaults to null,
 *               in which case the default Elements Object is
 *               chosen.
 */

/*
 * DGG: I have reversed the role of ptr_Elements. In this version,
 * the default is that a new Elements object is created, so this
 * Constituents object is independent of any other object. But if
 * ptr_Elements is supplied, it will be used. This way, a class
 * implementing a multi-phase mixture is responsible for
 * maintaining the global elements list for the mixture, and no
 * static global element list is required.
 */
Constituents::Constituents(Elements* ptr_Elements) :
    m_kk(0),
    m_speciesFrozen(false) ,
    m_Elements(ptr_Elements)
{
    if (!m_Elements) {
        m_Elements = new Elements();
    }

    // Register subscription to Elements object whether or not we
    // created it here.
    m_Elements->subscribe();
}

/**
 * Destructor for class Constituents.
 *
 *  Some cleanup of of the Global_Elements_List array is
 *  effected by unsubscribing to m_Elements.
 */
Constituents::~Constituents()
{
    int ileft = m_Elements->unsubscribe();
    /*
     * Here we may delete Elements Objects or not. Right now, we
     * will delete them. We also delete the global pointer entry
     * to keep everything consistent.
     */
    if (ileft <= 0) {
        vector<Elements*>::iterator it;
        for (it  = Elements::Global_Elements_List.begin();
                it != Elements::Global_Elements_List.end(); ++it) {
            if (*it == m_Elements) {
                Elements::Global_Elements_List.erase(it);
                break;
            }
        }
        delete m_Elements;
    }
}

size_t Constituents::nElements() const
{
    return m_Elements->nElements();
}

/*
 * Return the Atomic weight of element m.
 * units = Kg / Kmol
 */
doublereal Constituents::atomicWeight(size_t m) const
{
    return m_Elements->atomicWeight(m);
}

doublereal Constituents::entropyElement298(size_t m) const
{
    return m_Elements->entropyElement298(m);
}

/*
 *  returns a reference to the vector of atomic weights pertinent
 *  to this constituents object
 *  units = kg / Kmol
 */
const vector_fp& Constituents::atomicWeights() const
{
    return m_Elements->atomicWeights();
}

/*
 * Return the atomic number of element m.
 */
int Constituents::atomicNumber(size_t m) const
{
    return m_Elements->atomicNumber(m);
}

int Constituents::elementType(int m) const
{
    return m_Elements->elementType(m);
}

/*
 * Add an element to the set.
 * @param symbol  symbol string
 * @param weight  atomic weight in kg/mol.
 *
 * If weight is not given, then a lookup is performed in the
 * element object
 *
 */
void Constituents::
addElement(const std::string& symbol, doublereal weight)
{
    m_Elements->addElement(symbol, weight);
}

void Constituents::
addElement(const XML_Node& e)
{
    m_Elements->addElement(e);
}

/*
 * Add a unique element to the set. A check on the symbol is made
 * If the symbol is already an element, then a new element is
 * not created.
 *
 * @param symbol  symbol string
 * @param weight  atomic weight in kg/mol.
 *
 * If weight is not given, then a lookup is performed in the
 * element object
 *
 * -> Passthrough to the Element lvl.
 */
void Constituents::
addUniqueElement(const std::string& symbol, doublereal weight,
                 int atomicNumber, doublereal entropy298, int elem_type)
{
    m_Elements->addUniqueElement(symbol, weight, atomicNumber, entropy298, elem_type);
}

void Constituents::
addUniqueElement(const XML_Node& e)
{
    m_Elements->addUniqueElement(e);
}

void Constituents::addElementsFromXML(const XML_Node& phase)
{
    m_Elements->addElementsFromXML(phase);
}

/*
 * -> Passthrough to the Element lvl.
 */
void Constituents::freezeElements()
{
    m_Elements->freezeElements();
}

/*
 * -> Passthrough to the Element lvl.
 */
bool Constituents::elementsFrozen()
{
    return m_Elements->elementsFrozen();
}

/*
 * Index of element named \a name. The index is an integer
 * assigned to each element in the order it was added,
 * beginning with 0 for the first element.  If \a name is not
 * the name of an element in the set, then the value -1 is
 * returned.
 *
 *
 * -> Passthrough to the Element class.
 */
size_t Constituents::elementIndex(std::string name) const
{
    return (m_Elements->elementIndex(name));
}

/*
 * Name of the element with index m.
 *
 *   This is a passthrough routine to the Element object.
 *   @param m  @{ Element index. @}
 *   \exception If m < 0 or m >= nElements(), the
 *              exception, ElementRangeError, is thrown.
 */
string Constituents::elementName(size_t m) const
{
    return (m_Elements->elementName(m));
}

/*******************************************************************
 *
 * elementNames():
 *
 * Returns a read-only reference to the vector of element names.
 * @code
 * Constituents c;
 * ...
 * const vector<string>& enames = c.elementNames();
 * int n = enames.size();
 * for (int i = 0; i < n; i++) cout << enames[i] << endl;
 * @endcode
 *
 *
 * -> Passthrough to the Element lvl.
 */
const vector<string>& Constituents::elementNames() const
{
    return m_Elements->elementNames();
}

/*
 * molecularWeight()
 *
 *  Returns the molecular weight of a species given the species index
 *
 *  units = kg / kmol.
 */
doublereal Constituents::molecularWeight(size_t k) const
{
    if (k >= nSpecies()) {
        throw SpeciesRangeError("Constituents::molecularWeight",
                                k, nSpecies());
    }
    return m_weight[k];
}

/*
 * molecularWeights()
 *
 *  Returns a const reference to the vector of molecular weights
 *  for all of the species defined in the object.
 *
 *  units = kg / kmol.
 */
const array_fp& Constituents::molecularWeights() const
{
    return m_weight;
}

/*
 * charge():
 *
 * Electrical charge of one species k molecule, divided by
 * \f$ e = 1.602 \times 10^{-19}\f$ Coulombs.
 */
doublereal Constituents::charge(size_t k) const
{
    return m_speciesCharge[k];
}

/*
 * addSpecies()
 *
 *   Add a species to a Constituents object. Note, no check is made
 *   as to whether the species has a unique name.
 *
 *  Input
 *  ---------
 *   name = string containing the name
 *   comp[]
 *   charge =
 *   weight = weight of the species. Default = 0.0.
 *            Note, the weight is a bit redundent and potentially
 *            harmful. If weight is less than or equal to zero,
 *            the weight is calculated from the element composition
 *            and it need not be supplied on the command line.
 */
void Constituents::
addSpecies(const std::string& name, const doublereal* comp,
           doublereal charge, doublereal size)
{
    m_Elements->freezeElements();
    m_speciesNames.push_back(name);
    m_speciesCharge.push_back(charge);
    m_speciesSize.push_back(size);
    size_t ne = m_Elements->nElements();
    // Create a changeable copy of the element composition. We now change the charge potentially
    vector_fp compNew(ne);
    for (size_t m = 0; m < ne; m++) {
        compNew[m] = comp[m];
    }
    double wt = 0.0;
    const vector_fp& aw = m_Elements->atomicWeights();
    if (charge != 0.0) {
        size_t eindex = m_Elements->elementIndex("E");
        if (eindex != npos) {
            doublereal ecomp = compNew[eindex];
            if (fabs(charge + ecomp) > 0.001) {
                if (ecomp != 0.0) {
                    throw CanteraError("Constituents::addSpecies",
                                       "Input charge and element E compositions differ for species " + name);
                } else {
                    // Just fix up the element E composition based on the input species charge
                    compNew[eindex] = -charge;
                }
            }
        } else {
            addUniqueElementAfterFreeze("E", 0.000545, 0, 0.0, CT_ELEM_TYPE_ELECTRONCHARGE);
            ne = m_Elements->nElements();
            eindex = m_Elements->elementIndex("E");
            compNew.resize(ne);
            compNew[ne - 1] = - charge;
            //comp[eindex] = -charge;
            // throw CanteraError("Constituents::addSpecies",
            //                 "Element List doesn't include E, yet this species has charge:" + name);
        }
    }
    for (size_t m = 0; m < ne; m++) {
        m_speciesComp.push_back(compNew[m]);
        wt += compNew[m] * aw[m];
    }
    m_weight.push_back(wt);
    m_kk++;
}

/*
 *
 * addUniqueSpecies():
 *
 *   Add a species to a Constituents object. This routine will
 *   first check to see if the species is already part of the
 *   phase. It does this via a string comparison with the
 *   existing species in the phase.
 */
void Constituents::
addUniqueSpecies(const std::string& name, const doublereal* comp,
                 doublereal charge, doublereal size)
{
    vector<string>::const_iterator it = m_speciesNames.begin();
    for (size_t k = 0; k < m_kk; k++) {
        if (*it == name) {
            /*
             * We have found a match. At this point we could do some
             * compatibility checks. However, let's just return for the
             * moment without specifying any error.
             */
            size_t m_mm = m_Elements->nElements();
            for (size_t i = 0; i < m_mm; i++) {
                if (comp[i] != m_speciesComp[m_kk * m_mm + i]) {
                    throw CanteraError("addUniqueSpecies",
                                       "Duplicate species have different "
                                       "compositions: " + *it);
                }
            }
            if (charge != m_speciesCharge[m_kk]) {
                throw CanteraError("addUniqueSpecies",
                                   "Duplicate species have different "
                                   "charges: " + *it);
            }
            if (size != m_speciesSize[m_kk]) {
                throw CanteraError("addUniqueSpecies",
                                   "Duplicate species have different "
                                   "sizes: " + *it);
            }
            return;
        }
        ++it;
    }
    addSpecies(name, comp, charge, size);
}

/*
 *
 * freezeSpecies()
 *   Set the boolean indicating that we are no longer allowing
 *   species to be added to the Constituents class object.
 */
void Constituents::freezeSpecies()
{
    m_speciesFrozen = true;
}

/*
 *
 * speciesIndex()
 *
 * Index of species named \c name. The first species added
 * will have index 0, and the last one index nSpecies() - 1.
 *
 *  Note, the [] operator shouldn't be used for map's because it
 *  creates new entries. Here, we use find() to look up entries.
 *
 *  If name isn't in the list, then a -1 is returned.
 */
size_t Constituents::speciesIndex(std::string name) const
{
    vector<string>::const_iterator it = m_speciesNames.begin();
    for (size_t k = 0; k < m_kk; k++) {
        if (*it == name) {
            /*
             * We have found a match.
             */
            return k;
        }
        ++it;
    }
    return  -1;
}

/*
 *
 * speciesName()
 *
 *      Name of the species with index k
 */
string Constituents::speciesName(size_t k) const
{
    if (k >= nSpecies())
        throw SpeciesRangeError("Constituents::speciesName",
                                k, nSpecies());
    return m_speciesNames[k];
}

/*
 *
 * speciesNames()
 *
 *    Return a const reference to the vector of species names
 */
const vector<string>& Constituents::speciesNames() const
{
    return m_speciesNames;
}

/*
 *
 * ready():
 *   True if both elements and species have been frozen
 */
bool Constituents::ready() const
{
    return (m_Elements->elementsFrozen() && m_speciesFrozen);
}

/*
 * Returns the number of atoms of element \c m in species \c k.
 */
doublereal Constituents::nAtoms(size_t k, size_t m) const
{
    const size_t m_mm = m_Elements->nElements();
    if (m >= m_mm) {
        throw ElementRangeError("Constituents::nAtoms",m,nElements());
    }
    if (k >= nSpecies()) {
        throw SpeciesRangeError("Constituents::nAtoms",k,nSpecies());
    }
    return m_speciesComp[m_mm * k + m];
}


//====================================================================================================================
/*
 *
 * getAtoms()
 *
 * Get a vector containing the atomic composition
 * of species k
 */
void Constituents::getAtoms(size_t k, double* atomArray) const
{
    const size_t m_mm = m_Elements->nElements();
    for (size_t m = 0; m < m_mm; m++) {
        atomArray[m] = (double) m_speciesComp[m_mm * k + m];
    }
}



//====================================================================================================================
int Constituents::addUniqueElementAfterFreeze(const std::string& symbol, doublereal weight, int atomicNumber,
        doublereal entropy298, int elem_type)
{
    int ii = elementIndex(symbol);
    if (ii != -1) {
        return ii;
    }
    // Check to see that the element isn't really in the list
    m_Elements->m_elementsFrozen = false;
    addUniqueElement(symbol, weight, atomicNumber, entropy298, elem_type);
    m_Elements->m_elementsFrozen = true;
    int m_mm = m_Elements->nElements();
    ii = elementIndex(symbol);
    if (ii != m_mm-1) {
        throw CanteraError("Constituents::addElementAfterFreeze()", "confused");
    }
    if (m_kk > 0) {
        vector_fp old(m_speciesComp);
        m_speciesComp.resize(m_kk*m_mm, 0.0);
        for (int k = 0; k < m_kk; k++) {
            int m_old = m_mm - 1;
            for (int m = 0; m < m_old; m++) {
                m_speciesComp[k * m_mm + m] =  old[k * (m_old) + m];
            }
            m_speciesComp[k * (m_mm) + (m_mm-1)] = 0.0;
        }
    }
    return ii;
}
//====================================================================================================================
/*
 * This copy constructor just calls the assignment operator
 * for this class.
 * The assignment operator does a deep copy.
 */
Constituents::Constituents(const Constituents& right) :
    m_kk(0),
    m_speciesFrozen(false),
    m_Elements(0)
{
    *this = right;
}

/*
 *  Assignment operator for the Constituents class.
 *  Right now we pretty much do a straight uncomplicated
 *  copy of all of the protected data.
 */
Constituents& Constituents::operator=(const Constituents& right)
{
    /*
     * Check for self assignment.
     */
    if (this == &right) {
        return *this;
    }
    /*
     * We do a straight assignment operator on all of the
     * data. The vectors are copied.
     */
    m_kk             = right.m_kk;
    m_weight         = right.m_weight;
    m_speciesFrozen  = right.m_speciesFrozen;
    if (m_Elements) {
        int nleft = m_Elements->unsubscribe();
        if (nleft <= 0) {
            vector<Elements*>::iterator it;
            for (it  = Elements::Global_Elements_List.begin();
                    it != Elements::Global_Elements_List.end(); ++it) {
                if (*it == m_Elements) {
                    Elements::Global_Elements_List.erase(it);
                    break;
                }
            }
            delete m_Elements;
        }
    }
    m_Elements       = right.m_Elements;
    if (m_Elements) {
        m_Elements->subscribe();
    }
    m_speciesNames   = right.m_speciesNames;
    m_speciesComp    = right.m_speciesComp;
    m_speciesCharge  = right.m_speciesCharge;
    m_speciesSize    = right.m_speciesSize;
    /*
     * Return the reference to the current object
     */
    return *this;
}


}
