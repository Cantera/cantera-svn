/**
 *
 *  @file HMW_graph_1.cpp
 */

#include "cantera/thermo.h"
#include "cantera/thermo/StoichSubstanceSSTP.h"
#include "cantera/base/logger.h"

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
    cout << "usage: stoichSubSSTP " <<  endl;
    cout <<"                -> Everything is hardwired" << endl;
}



int main(int argc, char** argv)
{

    int retn = 0;
    int i;

    try {
        //Cantera::ThermoPhase *tp = 0;
        char iFile[80], file_ID[80];
        strcpy(iFile, "NaCl_Solid.xml");
        if (argc > 1) {
            strcpy(iFile, argv[1]);
        }

        //fileLog *fl = new fileLog("HMW_graph_1.log");
        //setLogger(fl);
        sprintf(file_ID,"%s#NaCl(S)", iFile);
        XML_Node* xm = get_XML_NameID("phase", file_ID, 0);
        StoichSubstanceSSTP* solid = new StoichSubstanceSSTP(*xm);


        /*
         * Load in and initialize the
         */
        //string nacl_s = "NaCl_Solid.xml";
        //string id = "NaCl(S)";
        //Cantera::ThermoPhase *solid = Cantera::newPhase(nacl_s, id);


        int nsp = solid->nSpecies();
        if (nsp != 1) {
            throw CanteraError("","Should just be one species");
        }
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
        string sName;

        TemperatureTable TTable(8, true, 300, 100., 0, 0);

        /*
         * Set the Pressure
         */
        double pres = OneAtm;
        double T = 298.15;
        solid->setState_TP(T, pres);

        /*
         * ThermoUnknowns
         */
        double mu0_RT[20], mu[20], cp_r[20];
        double enth_RT[20];
        double entrop_RT[20], intE_RT[20];
        double mu_NaCl, enth_NaCl, entrop_NaCl;
        double mu0_NaCl, molarGibbs, intE_NaCl, cp_NaCl;
        /*
         * Create a Table of NaCl  Properties as a Function
         * of the Temperature
         */

        double RT = GasConstant * T;
        solid->getEnthalpy_RT(enth_RT);
        double enth_NaCl_298 = enth_RT[0] * RT * 1.0E-6;

        printf(" Data from http://webbook.nist.gov\n");
        printf("\n");


        printf("           T,    Pres,    molarGibbs0,    Enthalpy,      Entropy,         Cp  ,"
               "  -(G-H298)/T,     H-H298 ");
        printf("\n");

        printf("      Kelvin,    bars,       kJ/gmol,      kJ/gmol,      J/gmolK,     J/gmolK ,"
               "      J/gmolK,     J/gmol");
        printf("\n");

        for (i = 0; i < TTable.NPoints; i++) {
            T = TTable.T[i];

            // GasConstant is in J/kmol
            RT = GasConstant * T;

            pres = OneAtm;


            solid->setState_TP(T, pres);
            /*
            * Get the Standard State DeltaH
            */
            solid->getGibbs_RT(mu0_RT);
            mu0_NaCl = mu0_RT[0] * RT * 1.0E-6;

            solid->getEnthalpy_RT(enth_RT);
            enth_NaCl = enth_RT[0] * RT * 1.0E-6;


            solid->getChemPotentials(mu);
            mu_NaCl = mu[0] * 1.0E-6;

            solid->getEntropy_R(entrop_RT);
            entrop_NaCl = entrop_RT[0] * GasConstant * 1.0E-3;

            molarGibbs = solid->gibbs_mole() * 1.0E-6;

            solid->getIntEnergy_RT(intE_RT);
            intE_NaCl = intE_RT[0] * RT * 1.0E-6;

            solid->getCp_R(cp_r);
            cp_NaCl = cp_r[0] * GasConstant * 1.0E-3;

            /*
            * Need the gas constant in kJ/gmolK
            */
            //       double rgas = 8.314472 * 1.0E-3;

            double pbar = pres * 1.0E-5;

            printf("%10g, %10g, %12g, %12g, %12g, %12g, %12g, %12g",
                   T, pbar, mu_NaCl, enth_NaCl, entrop_NaCl, cp_NaCl, -1.0E3*(mu_NaCl-enth_NaCl_298)/T, enth_NaCl-enth_NaCl_298);
            printf("\n");
        }



        delete solid;
        solid = 0;
        Cantera::appdelete();

        return retn;

    } catch (CanteraError) {

        showErrors();
        Cantera::appdelete();
        return -1;
    }
    return 0;
}
