/**
 *  @file FalloffMgr.h
 */

// Copyright 2001  California Institute of Technology

#ifndef CT_FALLOFFMGR_H
#define CT_FALLOFFMGR_H

#include "reaction_defs.h"
#include "FalloffFactory.h"

namespace Cantera
{

/**
 *  A falloff manager that implements any set of falloff functions.
 *  @ingroup falloffGroup
 */
class FalloffMgr
{
public:

    //! Constructor.
    FalloffMgr(/*FalloffFactory* f = 0*/) :
        m_n(0), m_n0(0), m_worksize(0) {
        //if (f == 0)
        m_factory = FalloffFactory::factory();   // RFB:TODO This raw pointer should be encapsulated
        // because accessing a 'Singleton Factory'
        //else m_factory = f;
    }

    /**
     * Destructor. Deletes all installed falloff function
     * calculators.
     */
    virtual ~FalloffMgr() {
        int i;
        for (i = 0; i < m_n; i++) {
            delete m_falloff[i];
        }
        //if (m_factory) {
        //FalloffFactory::deleteFalloffFactory();
        //m_factory = 0;
        //}
    }

    /**
     * Install a new falloff function calculator.  @param rxn
     * Index of the falloff reaction. This will be used to determine
     * which array entry is modified in method pr_to_falloff.
     *
     * @param type of falloff function to install.
     * @param c vector of coefficients for the falloff function.
     */
    void install(size_t rxn, int type,
                 const vector_fp& c) {
        if (type != SIMPLE_FALLOFF) {
            m_rxn.push_back(rxn);
            Falloff* f = m_factory->newFalloff(type,c);
            m_offset.push_back(m_worksize);
            m_worksize += f->workSize();
            m_falloff.push_back(f);
            m_n++;
        } else {
            m_rxn0.push_back(rxn);
            m_n0++;
        }
    }

    /**
     * Size of the work array required to store intermediate results.
     */
    size_t workSize() {
        return m_worksize;
    }

    /**
     * Update the cached temperature-dependent intermediate
     * results for all installed falloff functions.
     * @param t Temperature [K].
     * @param work Work array. Must be dimensioned at least workSize().
     */
    void updateTemp(doublereal t, doublereal* work) {
        int i;
        for (i = 0; i < m_n; i++) {
            m_falloff[i]->updateTemp(t,
                                     work + m_offset[i]);
        }
    }

    /**
     * Given a vector of reduced pressures for each falloff reaction,
     * replace each entry by the value of the falloff function.
     */
    void pr_to_falloff(doublereal* values, const doublereal* work) {
        doublereal pr;
        int i;
        for (i = 0; i < m_n0; i++) {
            values[m_rxn0[i]] /= (1.0 + values[m_rxn0[i]]);
        }
        for (i = 0; i < m_n; i++) {
            pr = values[m_rxn[i]];
            values[m_rxn[i]] *=
                m_falloff[i]->F(pr, work + m_offset[i]) /(1.0 + pr);
        }
    }

protected:
    std::vector<size_t> m_rxn, m_rxn0;
    std::vector<Falloff*> m_falloff;
    FalloffFactory* m_factory;
    vector_int m_loc;
    int m_n, m_n0;
    std::vector<vector_fp::difference_type> m_offset;
    size_t m_worksize;
};
}

#endif

