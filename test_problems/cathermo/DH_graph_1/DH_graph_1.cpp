/**
 *  @file DH_graph_1
 */

#include "fileLog.h"
#include "cantera/thermo/DebyeHuckel.h"

#include <cstdio>

using namespace std;
using namespace Cantera;

void printUsage()
{
    cout << "usage: DH_test " <<  endl;
    cout <<"                -> Everything is hardwired" << endl;
}


int main(int argc, char** argv)
{

    int retn = 0;
    int i;
    string fName = "DH_graph_1.log";
    fileLog* fl = new fileLog(fName);
    try {

        char iFile[80];
        strcpy(iFile, "DH_NaCl.xml");
        if (argc > 1) {
            strcpy(iFile, argv[1]);
        }
        setLogger(fl);

        DebyeHuckel* DH = new DebyeHuckel(iFile, "NaCl_electrolyte");

        int nsp = DH->nSpecies();
        double acMol[100];
        double mf[100];
        double moll[100];
        DH->getMoleFractions(mf);
        string sName;

        DH->setState_TP(298.15, 1.01325E5);

        int i1 = DH->speciesIndex("Na+");
        int i2 = DH->speciesIndex("Cl-");
        int i3 = DH->speciesIndex("H2O(L)");
        for (i = 1; i < nsp; i++) {
            moll[i] = 0.0;
        }
        DH->setMolalities(moll);
        double Itop = 10.;
        double Ibot = 0.0;
        double ISQRTtop = sqrt(Itop);
        double ISQRTbot = sqrt(Ibot);
        double ISQRT;
        double Is = 0.0;
        int its = 100;
        printf("              Is,     sqrtIs,     meanAc,"
               "  log10(meanAC),     acMol_Na+,"
               ",     acMol_Cl-,   ac_Water\n");
        for (i = 0; i < its; i++) {
            ISQRT = ISQRTtop*((double)i)/(its - 1.0)
                    + ISQRTbot*(1.0 - (double)i/(its - 1.0));
            Is = ISQRT * ISQRT;
            moll[i1] = Is;
            moll[i2] = Is;
            DH->setMolalities(moll);
            DH->getMolalityActivityCoefficients(acMol);
            double meanAC = sqrt(acMol[i1] * acMol[i2]);
            printf("%15g, %15g, %15g, %15g, %15g, %15g, %15g\n",
                   Is, ISQRT, meanAC, log10(meanAC),
                   acMol[i1], acMol[i2], acMol[i3]);
        }


        delete DH;
        DH = 0;
        /*
         * This delete the file logger amongst other things.
         */
        Cantera::appdelete();

        return retn;

    } catch (CanteraError) {
        showErrors();
        if (fl) {
            delete fl;
        }
        return -1;
    }
}
