/**
 *  @file ReactorFactory.cpp
 */
// Copyright 2006  California Institute of Technology

#include "cantera/zeroD/ReactorFactory.h"

#include "cantera/zeroD/Reservoir.h"
#include "cantera/zeroD/Reactor.h"
#include "cantera/zeroD/FlowReactor.h"
#include "cantera/zeroD/ConstPressureReactor.h"

using namespace std;
namespace Cantera
{

ReactorFactory* ReactorFactory::s_factory = 0;
mutex_t ReactorFactory::reactor_mutex;

static int ntypes = 4;
static string _types[] = {"Reservoir", "Reactor", "ConstPressureReactor",
                          "FlowReactor"
                         };

// these constants are defined in ReactorBase.h
static int _itypes[]   = {ReservoirType, ReactorType, ConstPressureReactorType,
                          FlowReactorType
                         };

/**
 * This method returns a new instance of a subclass of ThermoPhase
 */
ReactorBase* ReactorFactory::newReactor(string reactorType)
{

    int ir=-1;

    for (int n = 0; n < ntypes; n++) {
        if (reactorType == _types[n]) {
            ir = _itypes[n];
        }
    }

    return newReactor(ir);
}


ReactorBase* ReactorFactory::newReactor(int ir)
{
    switch (ir) {
    case ReservoirType:
        return new Reservoir();
    case ReactorType:
        return new Reactor();
    case FlowReactorType:
        return new FlowReactor();
    case ConstPressureReactorType:
        return new ConstPressureReactor();
    default:
        throw Cantera::CanteraError("ReactorFactory::newReactor",
                                    "unknown reactor type!");
    }
    return 0;
}

}

