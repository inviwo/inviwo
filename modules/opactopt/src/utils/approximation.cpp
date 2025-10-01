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

#include <numbers>

namespace inviwo {
namespace approximations {

std::vector<OptionPropertyOption<Type>> generateApproximationStringOptions() {
    constexpr auto n = approximations.size();
    std::vector<OptionPropertyOption<Type>> options;
    options.reserve(n);

    for (const auto& val : approximations) {
        options.emplace_back(val.identifier, val.name, val.type);
    }
    return options;
}

std::vector<float> generateLegendreCoefficients() {
    constexpr size_t maxDegree =
        approximations::approximations[std::to_underlying(Type::Legendre)].maxCoefficients - 1;
    std::vector<float> coeffs;
    coeffs.reserve((maxDegree + 1) * (maxDegree + 2) / 2);

    for (size_t n = 0; n <= maxDegree; n++) {
        for (size_t k = 0; k <= n; k++) {
            auto res = static_cast<float>(choose(n, k) * choose(n + k, k));
            res *= (n + k) % 2 == 0 ? 1.0f : -1.0f;
            coeffs.push_back(res);
        }
    }

    return coeffs;
}

// Taken from LineVis: https://github.com/chrismile/LineVis
// --------------------------------------------------------

/**
 * This utility function turns an angle from 0 to 2*pi into a parameter that grows monotonically as
 * function of the input. It is designed to be efficiently computable from a point on the unit
 * circle and must match the function in TrigonometricMomentMath.glsl.
 * @param pOutMaxParameter Set to the maximal possible output value.
 */
std::pair<float, float> circleToParameter(float angle) {
    const float x = std::cos(angle);
    const float y = std::sin(angle);
    float result = std::abs(y) - std::abs(x);
    result = (x < 0.0f) ? (2.0f - result) : result;
    result = (y < 0.0f) ? (6.0f - result) : result;
    result += (angle >= 2.0f * std::numbers::pi_v<float>) ? 8.0f : 0.0f;
    return {result, 7.0f};
}

/**
 * Given an angle in radians providing the size of the wrapping zone, this function computes all
 * constants required by the shader.
 */
glm::vec4 computeWrappingZoneParameters(float wrappingZoneAngle) {
    glm::vec4 wrappingZoneParameters;
    wrappingZoneParameters[0] = wrappingZoneAngle;
    wrappingZoneParameters[1] = std::numbers::pi_v<float> - 0.5f * wrappingZoneAngle;
    if (wrappingZoneAngle <= 0.0f) {
        wrappingZoneParameters[2] = 0.0f;
        wrappingZoneParameters[3] = 0.0f;
    } else {
        const auto [zoneBeginParameter, zoneEndParameter] =
            circleToParameter(2.0f * std::numbers::pi_v<float> - wrappingZoneAngle);
        wrappingZoneParameters[2] = 1.0f / (zoneEndParameter - zoneBeginParameter);
        wrappingZoneParameters[3] = 1.0f - zoneEndParameter * wrappingZoneParameters[2];
    }
    return wrappingZoneParameters;
}

MomentSettingsGL generateMomentSettings(const float wrappingZoneAngle, const float overestimation) {
    return {.wrappingZoneParameters = computeWrappingZoneParameters(wrappingZoneAngle),
            .wrappingZoneAngle = wrappingZoneAngle,
            .overestimation = overestimation};
}

}  // namespace approximations
}  // namespace inviwo
