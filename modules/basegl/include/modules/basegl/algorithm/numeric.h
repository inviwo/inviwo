#pragma once

/**
 * @brief use of machine epsilon to compare floating-point values for equality
 * http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
 */
bool almost_equal(float a, float b, int ulp = 2);
