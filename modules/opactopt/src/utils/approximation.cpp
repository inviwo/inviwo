/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/
#include <modules/opactopt/utils/approximation.h>

namespace inviwo {
namespace Approximations {

double choose(double n, double k) {
    if (k == 0.0) return 1.0;
    return (n * choose(n - 1, k - 1)) / k;
}

std::vector<OptionPropertyStringOption> generateApproximationStringOptions() {
    size_t n = approximations.size();
    std::vector<OptionPropertyStringOption> options;
    options.reserve(n);

    for (const auto& [key, val] : approximations) {
        OptionPropertyStringOption option(key, val.name, key);
        options.push_back(option);
    }
    return options;
}

std::vector<float> generateLegendreCoefficients() {
    size_t maxDegree = Approximations::approximations.at("legendre").maxCoefficients - 1;
    std::vector<float> coeffs;
    coeffs.reserve((maxDegree + 1) * (maxDegree + 2) / 2);

    for (size_t n = 0; n <= maxDegree; n++) {
        for (size_t k = 0; k <= n; k++) {
            double res = choose(static_cast<double>(n), static_cast<double>(k)) * choose(static_cast<double>(n + k), static_cast<double>(k));
            res *= (int)(n + k) % 2 == 0 ? 1.0f : -1.0f;
            coeffs.push_back(static_cast<float>(res));
        }
    }

    return coeffs;
}

MomentSettings::MomentSettings()
    : momentSettingsBuffer_{2 * sizeof(float) + sizeof(glm::vec4),
                            GLFormats::getGLFormat(GL_FLOAT, 1), GL_STATIC_READ,
                            GL_SHADER_STORAGE_BUFFER} {
    glm::vec4 wzp;
    computeWrappingZoneParameters(wzp, wrapping_zone_angle);

    momentSettingsBuffer_.bind();
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4), &wzp[0]);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4), sizeof(float),
                    &wrapping_zone_angle);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) + sizeof(float), sizeof(float),
                    &overestimation);
    momentSettingsBuffer_.bindBase(13);
}

// Taken from LineVis: https://github.com/chrismile/LineVis
// --------------------------------------------------------

/**
 * This utility function turns an angle from 0 to 2*pi into a parameter that grows monotonically as
 * function of the input. It is designed to be efficiently computable from a point on the unit
 * circle and must match the function in TrigonometricMomentMath.glsl.
 * @param pOutMaxParameter Set to the maximal possible output value.
 */
float MomentSettings::circleToParameter(float angle, float* pOutMaxParameter /* = nullptr */) {
    float x = std::cos(angle);
    float y = std::sin(angle);
    float result = std::abs(y) - std::abs(x);
    result = (x < 0.0f) ? (2.0f - result) : result;
    result = (y < 0.0f) ? (6.0f - result) : result;
    result += (angle >= 2.0f * M_PI) ? 8.0f : 0.0f;
    if (pOutMaxParameter) {
        (*pOutMaxParameter) = 7.0f;
    }
    return result;
}

/**
 * Given an angle in radians providing the size of the wrapping zone, this function computes all
 * constants required by the shader.
 */
void MomentSettings::computeWrappingZoneParameters(glm::vec4& p_out_wrapping_zone_parameters,
                                                   float new_wrapping_zone_angle) {
    p_out_wrapping_zone_parameters[0] = new_wrapping_zone_angle;
    p_out_wrapping_zone_parameters[1] = static_cast<float>(M_PI) - 0.5f * new_wrapping_zone_angle;
    if (new_wrapping_zone_angle <= 0.0f) {
        p_out_wrapping_zone_parameters[2] = 0.0f;
        p_out_wrapping_zone_parameters[3] = 0.0f;
    } else {
        float zone_end_parameter;
        float zone_begin_parameter =
            circleToParameter(2.0f * static_cast<float>(M_PI) - new_wrapping_zone_angle, &zone_end_parameter);
        p_out_wrapping_zone_parameters[2] = 1.0f / (zone_end_parameter - zone_begin_parameter);
        p_out_wrapping_zone_parameters[3] =
            1.0f - zone_end_parameter * p_out_wrapping_zone_parameters[2];
    }
}

}  // namespace Approximations
}  // namespace inviwo
