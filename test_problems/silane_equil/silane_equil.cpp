/*
 *  Copyright 2002 California Institute of Technology
 */

#ifdef SRCDIRTREE
#include "ct_defs.h"
#include "ThermoPhase.h"
#include "IdealGasMix.h"
#include "equil.h"
#else
#include "Cantera.h"
#include "IdealGasMix.h"
#include "equilibrium.h"
#endif

using namespace std;
using namespace Cantera;

int main(int argc, char **argv) {
#ifdef _MSC_VER
    _set_output_format(_TWO_DIGIT_EXPONENT);
#endif
  try {
    IdealGasMix g("silane.xml", "silane");
    g.setState_TPX(1500.0, 100.0, "SIH4:0.01, H2:0.99");
    //g.setState_TPX(1500.0, 1.0132E5, "SIH4:0.01, H2:0.99");
    equilibrate(g, "TP");
    cout << g;
    return 0;
  }
  catch (CanteraError) {
    showErrors(cerr);
    cerr << "program terminating." << endl;
    return -1;
  }
}
