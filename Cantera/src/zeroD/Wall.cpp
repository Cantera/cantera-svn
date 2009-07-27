
#include "Wall.h"
#include "ReactorBase.h"
#include "Func1.h"
#include "InterfaceKinetics.h"
#include "SurfPhase.h"

using Cantera::Func1;
using Cantera::Kinetics;

using namespace std;

namespace CanteraZeroD {

        
    Wall::Wall() : m_left(0), m_right(0),  
                   m_area(0.0), m_k(0.0), m_rrth(0.0), m_emiss(0.0), 
                   m_vf(0), m_qf(0) {
        for (int n = 0; n < 2; n++) {
            m_chem[n] = 0;
            m_surf[n] = 0;
            m_nsp[n] = 0;
        }
    }

    bool Wall::install(ReactorBase& rleft, ReactorBase& rright) {
        // check if wall is already installed
        if (m_left || m_right) return false;
        m_left =  &rleft;
        m_right = &rright;
        m_left->addWall(*this, 0);
        m_right->addWall(*this, 1);
        return true; 
    }

    /** Specify the kinetics managers for the surface mechanisms on
     * the left side and right side of the wall. Enter 0 if there is
     * no reaction mechanism.
     */
    void Wall::setKinetics(Kinetics* left, Kinetics* right) {
        m_chem[0] = left; 
        m_chem[1] = right;
        int ileft = 0, iright = 0;
        if (left) {
            ileft = left->surfacePhaseIndex();
            if (ileft >= 0) {
                m_surf[0] = (SurfPhase*)&left->thermo(ileft);
                m_nsp[0] = m_surf[0]->nSpecies();
                m_leftcov.resize(m_nsp[0]);
                m_surf[0]->getCoverages(DATA_PTR(m_leftcov));
            }
        }
        if (right) {
            iright = right->surfacePhaseIndex();
            if (iright >= 0) {
                m_surf[1] = (SurfPhase*)&right->thermo(iright);
                m_nsp[1] = m_surf[1]->nSpecies();
                m_rightcov.resize(m_nsp[1]);
                m_surf[1]->getCoverages(DATA_PTR(m_rightcov));
            }
        }
        if (ileft < 0 || iright < 0) {
            throw CanteraError("Wall::setKinetics",
                "specified surface kinetics manager does not "
                "represent a surface reaction mechanism.");
        }
    }

    /**
     * The volume rate of change is given by 
     * \f[ \dot V = K A (P_{left} - P_{right}) + F(t) \f]
     * where \f$ F(t) \f$ is a specified function of time.
     *
     * This method is used by class Reactor to compute the
     * rate of volume change of the reactor.
     */
    doublereal Wall::vdot(doublereal t) {
        double rate1 = m_k * m_area * 
                       (m_left->pressure() - m_right->pressure()); 
        if (m_vf) rate1 += m_area * m_vf->eval(t);
        return rate1;
    }

    /**
     * The heat flux is given by 
     * \f[ Q = h A (T_{left} - T_{right}) + A G(t) \f]
     * where h is the heat transfer coefficient, and 
     * \f$ G(t) \f$ is a specified function of time.
     */
    doublereal Wall::Q(doublereal t) {
        double q1 = (m_area * m_rrth) *
                    (m_left->temperature() - m_right->temperature());
        if (m_emiss > 0.0) {
            double tl = m_left->temperature();
            double tr = m_right->temperature();
            q1 += m_area * StefanBoltz * (tl*tl*tl*tl - tr*tr*tr*tr);
        }
        if (m_qf) q1 += m_area * m_qf->eval(t);
        return q1;
    }

    void Wall::setCoverages(int leftright, const doublereal* cov) {
        if (leftright == 0)
            copy(cov, cov + m_nsp[0], m_leftcov.begin());
        else
            copy(cov, cov + m_nsp[1], m_rightcov.begin());
    }

    void Wall::getCoverages(int leftright, doublereal* cov) {
        if (leftright == 0)
            copy(m_leftcov.begin(), m_leftcov.end(), cov);
        else
            copy(m_rightcov.begin(), m_rightcov.end(), cov);
    }

    void Wall::syncCoverages(int leftright) {
        if (leftright == 0)
            m_surf[0]->setCoverages(DATA_PTR(m_leftcov));
        else
            m_surf[1]->setCoverages(DATA_PTR(m_rightcov));
    }

    void Wall::addSensitivityReaction(int leftright, int rxn) {
        if (rxn < 0 || rxn >= m_chem[leftright]->nReactions()) 
            throw CanteraError("Wall::addSensitivityReaction",
                "Reaction number out of range ("+int2str(rxn)+")");
        if (leftright == 0) {
            m_pleft.push_back(rxn);
            m_leftmult_save.push_back(1.0);
            m_pname_left.push_back(m_chem[0]->reactionString(rxn));
        }
        else {
            m_pright.push_back(rxn);
            m_rightmult_save.push_back(1.0);
            m_pname_right.push_back(m_chem[1]->reactionString(rxn));
        }
    }

    void Wall::setSensitivityParameters(int lr, double* params) {
        // process sensitivity parameters
        int n, npar;
        if (lr == 0) {
            npar = m_pleft.size();
            for (n = 0; n < npar; n++) {
                m_leftmult_save[n] = m_chem[0]->multiplier(m_pleft[n]);
                m_chem[0]->setMultiplier(m_pleft[n], 
                    m_leftmult_save[n]*params[n]);
            }
        }
        else {
            npar = m_pright.size();
            for (n = 0; n < npar; n++) {
                m_rightmult_save[n] = m_chem[1]->multiplier(m_pright[n]);
                m_chem[1]->setMultiplier(m_pright[n], 
                    m_rightmult_save[n]*params[n]);
            }            
        }
    }

    void Wall::resetSensitivityParameters(int lr) {
        int n, npar;
        if (lr == 0) {
            npar = m_pleft.size();
            for (n = 0; n < npar; n++) {
                m_chem[0]->setMultiplier(m_pleft[n], m_leftmult_save[n]);
            }
        }
        else {
            npar = m_pright.size();
            for (n = 0; n < npar; n++) {
                m_chem[1]->setMultiplier(m_pright[n], 
                    m_rightmult_save[n]);
            }            
        }
    }
}
