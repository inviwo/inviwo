#pragma once

#include "numeric.h"

#include <glm/glm.hpp>
#include <limits>

/**
 * @brief use of machine epsilon to compare floating-point values for equality
 * http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
 */
bool almost_equal(float a, float b, int ulp)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return glm::abs(a - b) <= std::numeric_limits<float>::epsilon() * std::abs(a + b) * ulp
    // unless the result is subnormal
        || glm::abs(a - b) < std::numeric_limits<float>::min();
}
