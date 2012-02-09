/**
 *  @file Wall.h
 *  Header file for class Wall.
 */

/*  $Author$
 *  $Date$
 *  $Revision$
 */

// Copyright 2001-2004  California Institute of Technology


#ifndef CT_WALL_H
#define CT_WALL_H

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include "ct_defs.h"
#include "ctexceptions.h"
#include "Func1.h"

namespace Cantera {
    class Kinetics;
    class Func1;
    class SurfPhase;
}

namespace CanteraZeroD {

    // forward references
    class ReactorBase;

    class Wall {

    public:

        /// Constructor
        Wall();

        /// Destructor.  Since Wall instances do not allocate memory,
        /// the destructor does nothing.
        virtual ~Wall() {}

        
        /// Rate of volume change (kg/s). Positive value increases
        /// volume of reactor on left, and decreases volume on right.
        virtual doublereal vdot(doublereal t);

        /// Heat flow rate through the wall (W). Positive values
        /// denote a flux from left to right.
        virtual doublereal Q(doublereal t);

        /// Area in m^2.
        doublereal area() { return m_area; }

        /// Set the area [m^2].
        void setArea(doublereal a) { m_area = a; }
 
        void setThermalResistance(doublereal Rth) { m_rrth = 1.0/Rth; }

        /// Set the overall heat transfer coefficient [W/m^2/K].
        void setHeatTransferCoeff(doublereal U) { m_rrth = U; }

        /// Set the emissivity.
        void setEmissivity(doublereal epsilon) {
            if (epsilon > 1.0 || epsilon < 0.0) 
                throw Cantera::CanteraError("Wall::setEmissivity", 
                    "emissivity must be between 0.0 and 1.0");
            m_emiss = epsilon; }

        /** Set the piston velocity to a specified function. */
        void setVelocity(Cantera::Func1* f=0) {if (f) m_vf = f;}

        /** 
         * Set the expansion rate coefficient.
         */
        void setExpansionRateCoeff(doublereal k) {m_k = k;}

        
        /// Specify the heat flux function \f$ q_0(t) \f$.
        void setHeatFlux(Cantera::Func1* q) { m_qf = q;}

        /// Install the wall between two reactors or reservoirs
        bool install(ReactorBase& leftReactor, ReactorBase& rightReactor);

        /// True if the wall is correctly configured and ready to use.
        virtual bool ready() { return (m_left != 0 && m_right != 0); }

        //        int type() { return 0; }


        /// Return a reference to the reactor or reservoir to the left
        /// of the wall.
        ReactorBase& left() const { return *m_left; }

        /// Return a reference to the reactor or reservoir to the
        /// right of the wall.
        const ReactorBase& right() { return *m_right; }

        // /// Set wall parameters.
        //virtual void setParameters(int n, doublereal* coeffs) {
        //    m_coeffs.resize(n);
        //    copy(coeffs, coeffs + n, m_coeffs.begin());
        //}

        // Specify the heterogeneous reaction mechanisms for each side
        // of the wall. 
        void setKinetics(Cantera::Kinetics* leftMechanism, 
            Cantera::Kinetics* rightMechanism);

        /// Return a pointer to the surface phase object for the left
        /// or right wall surface. 
        Cantera::SurfPhase* surface(int leftright) {
            return m_surf[leftright];
        }

        Cantera::Kinetics* kinetics(int leftright) {
            return m_chem[leftright];
        }

        /// Set the surface coverages on the left or right surface to
        /// the values in array 'cov'.
        void setCoverages(int leftright, const doublereal* cov);

        /// Write the coverages of the left or right surface into
        /// array cov. 
        void getCoverages(int leftright, doublereal* cov);

        /// Set the coverages in the surface phase object to the
        /// values for this wall surface.
        void syncCoverages(int leftright);


        int nSensParams(int lr) const { 
            if (lr == 0) 
                return m_pleft.size();
            else
                return m_pright.size();
        }
        void addSensitivityReaction(int leftright, int rxn);
        std::string sensitivityParamID(int leftright, int p) { 
            if (leftright == 0)
                return m_pname_left[p]; 
            else 
                return m_pname_right[p];
        }
        void setSensitivityParameters(int lr, double* params);
        void resetSensitivityParameters(int lr);
        
        //        int componentIndex(string nm) const;

    protected:

        //vector_fp m_coeffs;

        ReactorBase* m_left;
        ReactorBase* m_right;
        Cantera::Kinetics * m_chem[2];
        Cantera::SurfPhase* m_surf[2];
        int m_nsp[2];
        doublereal m_area, m_k, m_rrth;
        doublereal m_emiss;
        Cantera::Func1 *m_vf;
        Cantera::Func1 *m_qf;
        Cantera::vector_fp m_leftcov, m_rightcov;

        Cantera::vector_int m_pleft, m_pright;
        Cantera::vector_fp m_leftmult_save, m_rightmult_save;
        std::vector<std::string> m_pname_left, m_pname_right;

    private:

    };
        
}

#endif
