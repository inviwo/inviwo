#ifndef NURB_NURB_H
#define NURB_NURB_H

#include "../../../../ext/tinynurbs/tinynurbs/tinynurbs.h"
#include <vector>

/**
 * Wrapper class for higher dimension nurbs using tinynurbs implementation
 */
class Nurb {

public:
    Nurb(int dim, std::vector<tinynurbs::Curve<3, float>> curves);
    Nurb(int dim, std::vector<std::vector<float>> points);
    std::vector<float> element(int rank);
    std::vector<float> evaluate(float u);
private:
    std::vector<tinynurbs::Curve<3, float>> _curves;
    int _dim;
};

#endif //NURB_NURB_H
