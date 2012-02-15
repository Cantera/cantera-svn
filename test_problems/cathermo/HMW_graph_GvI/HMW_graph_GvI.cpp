/**
 *  @file HMW_graph_1.cpp
 */

#include "cantera/base/logger.h"
#include "cantera/thermo/HMWSoln.h"

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
    }
    virtual ~fileLog() {
        m_fs.close();
    }
    string m_fName;
    fstream m_fs;
};

void printUsage()
{
    cout << "usage: HMW_test " <<  endl;
    cout <<"                -> Everything is hardwired" << endl;
}

void pAtable(HMWSoln* HMW)
{
    int nsp = HMW->nSpecies();
    double acMol[100];
    double mf[100];
    double activities[100];
    double moll[100];
    for (int i = 0; i < 100; i++) {
        acMol[i] = 1.0;
        mf[i] = 0.0;
        activities[i] = 1.0;
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
    string commandFile;
    try {

        char iFile[80];
        strcpy(iFile, "HMW_NaCl.xml");
        if (argc > 1) {
            strcpy(iFile, argv[1]);
        }
        double Temp = 273.15 + 275.;

        double aTemp[7];
        aTemp[0] = 298.15;
        aTemp[1] = 273.15 + 100.;
        aTemp[2] = 273.15 + 150.;
        aTemp[3] = 273.15 + 200.;
        aTemp[4] = 273.15 + 250.;
        aTemp[5] = 273.15 + 275.;
        aTemp[6] = 273.15 + 300.;

        //fileLog *fl = new fileLog("HMW_graph_1.log");
        //setLogger(fl);

        HMWSoln* HMW = new HMWSoln(iFile, "NaCl_electrolyte");

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
        FILE* ff;
        char fname[64];

        for (int jTemp = 0; jTemp < 7; jTemp++) {
            Temp = aTemp[jTemp];
            sprintf(fname, "T%3.0f.csv", Temp);

            ff = fopen(fname, "w");
            HMW->setState_TP(Temp, 1.01325E5);
            printf("   Temperature = %g K\n", Temp);
            int i1 = HMW->speciesIndex("Na+");
            int i2 = HMW->speciesIndex("Cl-");
            int i3 = HMW->speciesIndex("H2O(L)");
            for (i = 1; i < nsp; i++) {
                moll[i] = 0.0;
            }
            HMW->setState_TPM(Temp, OneAtm, moll);
            double Itop = 10.;
            double Ibot = 0.0;
            double ISQRTtop = sqrt(Itop);
            double ISQRTbot = sqrt(Ibot);
            double ISQRT;
            double Is = 0.0;
            int its = 100;
            bool doneSp = false;
            fprintf(ff,"              Is,     sqrtIs,     meanAc,"
                    "  log10(meanAC),     acMol_Na+,"
                    "     acMol_Cl-,   ac_Water, act_Water, OsmoticCoeff\n");
            for (i = 0; i < its; i++) {
                ISQRT = ISQRTtop*((double)i)/(its - 1.0)
                        + ISQRTbot*(1.0 - (double)i/(its - 1.0));

                Is = ISQRT * ISQRT;
                if (!doneSp) {
                    if (Is > 6.146) {
                        Is = 6.146;
                        doneSp = true;
                        i = i - 1;
                    }
                }
                moll[i1] = Is;
                moll[i2] = Is;
                HMW->setMolalities(moll);
                HMW->getMolalityActivityCoefficients(acMol);
                HMW->getActivities(act);
                double oc = HMW->osmoticCoefficient();
                double meanAC = sqrt(acMol[i1] * acMol[i2]);
                fprintf(ff,"%15g, %15g, %15g, %15g, %15g, %15g, %15g, %15g, %15g\n",
                        Is, ISQRT, meanAC, log10(meanAC),
                        acMol[i1], acMol[i2], acMol[i3], act[i3], oc);
            }
            fclose(ff);
        }


        delete HMW;
        HMW = 0;
        Cantera::appdelete();

        return retn;

    } catch (CanteraError) {

        showErrors();
        return -1;
    }
}
