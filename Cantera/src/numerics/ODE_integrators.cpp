#include "ct_defs.h"
#include "Integrator.h"


#ifdef HAS_SUNDIALS
#include "CVodesIntegrator.h"
#else
#include "CVodeInt.h"
#endif

namespace Cantera {

    Integrator* newIntegrator(std::string itype) {
        if (itype == "CVODE") {
#ifdef HAS_SUNDIALS
            return new CVodesIntegrator();
#else
            return new CVodeInt();
#endif
        }
        else {
            throw CanteraError("newIntegrator",
                "unknown ODE integrator: "+itype);
        }
        return 0;
    }

    void deleteIntegrator(Integrator *cv) {
        delete cv;
    }
}
