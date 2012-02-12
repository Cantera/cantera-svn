/**
 *
 *  @file HMW_graph_1.cpp
 */
#ifdef SRCDIRTREE
#include "cantera/base/ct_defs.h"
#include "cantera/base/logger.h"
#include "cantera/thermo/ThermoPhase.h"
#include "cantera/thermo/HMWSoln.h"
#include "importCTML.h"
#else
#include "cantera/Cantera.h"
#include "cantera/thermo/ThermoPhase.h"
#include "cantera/base/logger.h"
#include "cantera/thermo.h"
#include "cantera/thermo/HMWSoln.h"
#endif

#include "TemperatureTable.h"

#include <cstdio>

using namespace std;
using namespace Cantera;

class fileLog: public Logger
{
public:
    fileLog(string fName) {
        m_fName = fName;
        m_fs.open(fName.c_str());
    }

    virtual void write(const string& msg) {
        m_fs << msg;
        m_fs.flush();
    }

    virtual ~fileLog() {
        m_fs.close();
    }

    string m_fName;
    ofstream m_fs;

};

void printUsage()
{
    cout << "usage: HMW_test " <<  endl;
    cout <<"                -> Everything is hardwired" << endl;
}

void pAtable(HMWSoln* HMW)
{
    double acMol[30];
    double mf[30];
    double activities[30];
    double moll[30];
    int nsp = HMW->nSpecies();

    for (int i = 0; i < 30; i++) {
        acMol[i] = 1.0;
        mf[i] = 0.0;
        activities[i] = 0.0;
        moll[i] = 0.0;
    }

    HMW->getMolalityActivityCoefficients(acMol);
    HMW->getMoleFractions(mf);
    HMW->getActivities(activities);
    HMW->getMolalities(moll);
    string sName;
    printf("            Name      Activity  ActCoeffMolal "
           "   MoleFract      Molality\n");
    for (int k = 0; k < nsp; k++) {
        sName = HMW->speciesName(k);
        printf("%16s %13g %13g %13g %13g\n",
               sName.c_str(), activities[k], acMol[k], mf[k], moll[k]);
    }
}

int main(int argc, char** argv)
{

    int retn = 0;
    int i;
    int extraCols = 1;

    try {
        //Cantera::ThermoPhase *tp = 0;
        char iFile[80];
        strcpy(iFile, "HMW_NaCl.xml");
        if (argc > 1) {
            strcpy(iFile, argv[1]);
        }

        //fileLog *fl = new fileLog("HMW_graph_1.log");
        //setLogger(fl);

        HMWSoln* HMW = new HMWSoln(iFile, "NaCl_electrolyte");


        /*
         * Load in and initialize the
         */
        string nacl_s = "NaCl_Solid.xml";
        string id = "NaCl(S)";
        Cantera::ThermoPhase* solid = Cantera::newPhase(nacl_s, id);

        int nsp = HMW->nSpecies();
        double acMol[100];
        double act[100];
        double mf[100];
        double moll[100];
        for (i = 0; i < 100; i++) {
            acMol[i] = 1.0;
            act[i] = 1.0;
            mf[i] = 0.0;
            moll[i] = 0.0;
        }

        HMW->getMoleFractions(mf);
        string sName;

        TemperatureTable TTable(29, true, 293.15, 10., 0, 0);

        HMW->setState_TP(298.15, 1.01325E5);

        int i1 = HMW->speciesIndex("Na+");
        int i2 = HMW->speciesIndex("Cl-");
        //int i3 = HMW->speciesIndex("H2O(L)");
        for (i = 1; i < nsp; i++) {
            moll[i] = 0.0;
        }
        HMW->setMolalities(moll);

        double ISQRT;
        double Is = 0.0;

        /*
         * Set the Pressure
         */
        double pres = OneAtm;

        /*
         * Fix the molality using the setState_TPM() function.
         */
        Is = 6.146;
        ISQRT = sqrt(Is);
        moll[i1] = Is;
        moll[i2] = Is;
        HMW->setState_TPM(298.15, pres, moll);
        double Xmol[30];
        HMW->getMoleFractions(Xmol);

        printf("Fixed Concentration of the System:\n");

        printf(" Species         Mole_Fraction      Molality\n");
        printf("   Na+            %g            %g\n", Xmol[i1], moll[i1]);
        printf("   Cl-            %g            %g\n", Xmol[i2], moll[i2]);
        printf("   H2O(L)         %g            \n", Xmol[0]);
        printf("\n");
        /*
         * ThermoUnknowns
         */
        double mu0_RT[20], mu[20];
        double mu0_NaCl, mu0_Naplus, mu0_Clminus, Delta_G0;
        double mu_NaCl, mu_Naplus, mu_Clminus, Delta_G;
        double molarGibbs0, molarGibbs;

        /*
         * Create a Table of NaCl Enthalpy Properties as a Function
         * of the Temperature
         */

        printf("  Table at fixed molality(Delta_G refers to rxn, NaCl(s) -> Na+ + Cl-)\n");
        printf("               -> pressure follows the saturation pressure above one atmosphere)\n");
        printf("               ->   This calculation is meant to test Gibbs_ex -> TODO\n");
        printf("\n");
        printf("              (note Aphi = A_Debye/3.0)\n");
        printf("\n");
        printf("\n");

        printf("           T,      Pres,     Aphi,      Delta_G0,"
               "       Delta_G,"
               "   molarGibbs0,    molarGibbs,      Gibbs_ex,"
               "   meanAC_moll,  OsmCoeff-1");
        if (extraCols) {
            printf(", Gibbs_ex_Formula,    IdealMixing");
        }
        printf("\n");
        printf("      Kelvin,       bars, sqrt(kg/gmol), kJ/gmol,"
               "       kJ/gmol,"
               "    kJ/kgWater,    kJ/kgWater,    kJ/kgWater,"
               "             ,             ");
        if (extraCols) {
            printf(",      kJ/kgWater,    kJ/kgWater ");
        }
        printf("\n");
        for (i = 0; i < TTable.NPoints; i++) {
            double T = TTable.T[i];
            double RT = GasConstant * T;

            double psat = HMW->satPressure(T);
            pres = OneAtm;
            if (psat > pres) {
                pres = psat;
            }

            HMW->setState_TPM(T, pres, moll);
            solid->setState_TP(T, pres);
            /*
            * Get the Standard State DeltaH
            */
            solid->getGibbs_RT(mu0_RT);
            mu0_NaCl = mu0_RT[0] * RT * 1.0E-6;

            HMW->getGibbs_RT(mu0_RT);
            //double mu0_water = mu0_RT[0] * RT * 1.0E-6;
            mu0_Naplus = mu0_RT[i1] * RT * 1.0E-6;
            mu0_Clminus = mu0_RT[i2] * RT * 1.0E-6;
            Delta_G0 = (mu0_Naplus + mu0_Clminus) - mu0_NaCl;

            HMW->getMolalityActivityCoefficients(acMol);
            HMW->getActivities(act);

            double meanAC = sqrt(acMol[i1] * acMol[i2]);
            solid->getChemPotentials(mu);

            mu_NaCl = mu[0] * 1.0E-6;

            HMW->getChemPotentials(mu);
            for (int k = 0; k < nsp; k++) {
                mu[k] *= 1.0E-6;
            }
            mu_Naplus  = mu[i1];
            mu_Clminus = mu[i2];
            Delta_G = (mu_Naplus + mu_Clminus) - mu_NaCl;


            molarGibbs = HMW->gibbs_mole() * 1.0E-6;
            /*
            * Now the molarGibbs value is based on a mole of
            * solution. This is useless for comparison purposes.
            * Change to kg Water
            */
            double molecWater = HMW->molecularWeight(0);
            double Mo = molecWater / 1000.;
            double Gibbs_kgWater = molarGibbs / (Xmol[0] * Mo);


            double Aphi = HMW->A_Debye_TP() / 3.0;

            for (int k = 0; k < nsp; k++) {
                mu0_RT[k] *= RT * 1.0E-6;
            }

            molarGibbs0 = 0.0;
            for (int k = 0; k < nsp; k++) {
                molarGibbs0 += Xmol[k] * mu0_RT[k];
            }
            double Gibbs0_kgWater = molarGibbs0 / (Xmol[0] * Mo);

            double osm1 = HMW->osmoticCoefficient();
            osm1 = osm1 - 1.0;
            /*
            * Need the gas constant in kJ/gmolK
            */
            double rgas = 8.314472 * 1.0E-3;
            double IdealMixing = moll[i1] * 2.0 * rgas * T * (log(moll[i1]) - 1.0);


            double G_ex_kgWater = Gibbs_kgWater - Gibbs0_kgWater - IdealMixing;

            /*
            * Calcualte excess gibbs free energy from another formula
            */
            double G_ex_formula = 2 * Is * rgas * T * (- osm1 + log(meanAC));
            /*
                   if (fabs (T-298.15) < 1.0) {
            	 printf("mu0_Naplus  = %g\n", mu0_Naplus);
            	 printf("mu0_Clminus = %g\n", mu0_Clminus);
            	 printf("mu0_NaCl(s) = %g,   mu_NaCl(s) = %g\n",mu0_NaCl, mu_NaCl);
                   }
            */
            double pbar = pres * 1.0E-5;

            //if (extraCols && T == 323.15) {
            //	 for (int k = 0; k < nsp; k++) {
            //	   printf("mus_kJ/gmol - %s -  %14.8g %14.8g %g\n",
            //	  HMW->speciesName(k).c_str(), mu0_RT[k], mu[k], Xmol[k]);
            // }
            //}

            printf("%10g, %10g, %12g, %12g, %12g, %12g, %12g, %12g, %14.9g, %14.9g",
                   T, pbar, Aphi, Delta_G0, Delta_G, Gibbs0_kgWater, Gibbs_kgWater, G_ex_kgWater,
                   meanAC, osm1);
            if (extraCols) {
                printf(", %12g, %12g", G_ex_formula, IdealMixing);
            }
            printf("\n");
        }


        delete HMW;
        HMW = 0;
        delete solid;
        solid = 0;
        Cantera::appdelete();

        return retn;

    } catch (CanteraError) {

        showErrors();
        Cantera::appdelete();
        return -1;
    }
}
