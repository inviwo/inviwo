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
#pragma once

#include <modules/opactopt/opactoptmoduledefine.h>

#include <modules/opengl/texture/texture1d.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/util/glmvec.h>
#include <string_view>
#include <vector>
#include <numbers>

namespace inviwo {

namespace approximations {

// In how may ways can I select k items from n items without repetition and without order
constexpr size_t choose(size_t n, size_t k) {
    if (k == 0 || k == n) return 1;
    if (k < 0 || k > n) return 0;

    // Use symmetry
    if (k > n - k) {
        k = n - k;
    }

    size_t result = 1;
    for (size_t i = 0; i < k; ++i) {
        result = result * (n - i) / (i + 1);
    }

    return result;
}

enum class Type : std::uint8_t { Fourier, Legendre, Piecewise, PowerMoments, TrigonometricMoments };

/**
 * @brief Describes the approximation properties
 */
struct IVW_MODULE_OPACTOPT_API ApproximationProperties {
    std::string_view identifier;
    std::string_view name;
    std::string_view shaderDefineName;
    std::string_view shaderFile;
    int minCoefficients;
    int maxCoefficients;
    int increment;
    Type type;
};

constexpr std::array<const ApproximationProperties, 5> approximations{
    {{"fourier", "Fourier", "FOURIER", "opactopt/approximation/fourier.glsl", 1, 31, 1,
      Type::Fourier},
     {"legendre", "Legendre", "LEGENDRE", "opactopt/approximation/legendre.glsl", 1, 31, 1,
      Type::Legendre},
     {"piecewise", "Piecewise", "PIECEWISE", "opactopt/approximation/piecewise.glsl", 1, 30, 1,
      Type::Piecewise},
     {"powermoments", "Power moments", "POWER_MOMENTS", "opactopt/approximation/powermoments.glsl",
      5, 9, 2, Type::PowerMoments},
     {"trigmoments", "Trigonometric moments", "TRIG_MOMENTS",
      "opactopt/approximation/trigmoments.glsl", 5, 9, 2, Type::TrigonometricMoments}}};

constexpr const ApproximationProperties& get(Type type) {
    return approximations.at(std::to_underlying(type));
}

IVW_MODULE_OPACTOPT_API std::vector<OptionPropertyOption<Type>>
generateApproximationStringOptions();

IVW_MODULE_OPACTOPT_API std::vector<float> generateLegendreCoefficients();

struct IVW_MODULE_OPACTOPT_API MomentSettingsGL {
    glm::vec4 wrappingZoneParameters;
    float wrappingZoneAngle;
    float overestimation;
    std::array<float, 2> pad;
};

IVW_MODULE_OPACTOPT_API MomentSettingsGL generateMomentSettings(
    float wrappingZoneAngle = 0.1f * std::numbers::pi_v<float>, float overestimation = 0.25f);

}  // namespace approximations

}  // namespace inviwo
