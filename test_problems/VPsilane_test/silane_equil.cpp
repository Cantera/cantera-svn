/*
 *  Copyright 2002 California Institute of Technology
 */

#include "cantera/IdealGasMix.h"
#include "cantera/equilibrium.h"
#include "cantera/thermo/IdealSolnGasVPSS.h"
#include "cantera/thermo/ThermoFactory.h"
#include "cantera/base/PrintCtrl.h"

using namespace std;
using namespace Cantera;

int main(int argc, char** argv)
{
#ifdef _MSC_VER
    _set_output_format(_TWO_DIGIT_EXPONENT);
#endif
    try {
        PrintCtrl::GlobalCrop = PrintCtrl::GCT_CROP;
        Cantera::IdealSolnGasVPSS gg("silane.xml", "silane");
        ThermoPhase* g = &gg;
        //ThermoPhase *g = newPhase("silane.xml", "silane");
        g->setState_TPX(1500.0, 100.0, "SIH4:0.01, H2:0.99");
        //g.setState_TPX(1500.0, 1.0132E5, "SIH4:0.01, H2:0.99");
        Cantera::ChemEquil_print_lvl = 40;
        equilibrate(*g, "TP");
        std::string r = Cantera::report(*g, true);
        cout << r;
        cout << endl;
        return 0;
    } catch (CanteraError) {
        showErrors(cerr);
        cerr << "program terminating." << endl;
        return -1;
    }
}
