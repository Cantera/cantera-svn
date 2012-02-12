#ifndef TPX_WATER_H
#define TPX_WATER_H

#include "Sub.h"

namespace tpx
{

class water : public Substance
{
public:
    water() {
        m_name = "water";
        m_formula = "H2O";
    }
    ~water() {}

    double MolWt();
    double Tcrit();
    double Pcrit();
    double Vcrit();
    double Tmin();
    double Tmax();
    char* name();
    char* formula();

    double Pp();
    double up();
    double sp();
    double Psat();
    double dPsatdT();

private:
    double ldens();
    double C(int i);
    double Cprime(int i);
    double I(int i);
    double H(int i);
};

}
#endif // ! WATER_H

