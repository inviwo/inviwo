/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2026 Inviwo Foundation
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

namespace inviwo::approximations {

// In how may ways can I select k items from n items without repetition and without order
constexpr size_t choose(size_t n, size_t k) {
    if (k == 0 || k == n) return 1;
    if (k > n) return 0;

    k = std::min(k, n - k);  // Use symmetry

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
struct IVW_MODULE_OPACTOPT_API Properties {
    std::string_view identifier;
    std::string_view name;
    std::string_view define;
    std::string_view file;
    int min;
    int max;
    int inc;
    Type type;
};

constexpr std::array<const Properties, 5> approximations{
    {{.identifier = "fourier",
      .name = "Fourier",
      .define = "FOURIER",
      .file = "opactopt/approximation/fourier.glsl",
      .min = 1,
      .max = 31,
      .inc = 1,
      .type = Type::Fourier},
     {.identifier = "legendre",
      .name = "Legendre",
      .define = "LEGENDRE",
      .file = "opactopt/approximation/legendre.glsl",
      .min = 1,
      .max = 31,
      .inc = 1,
      .type = Type::Legendre},
     {.identifier = "piecewise",
      .name = "Piecewise",
      .define = "PIECEWISE",
      .file = "opactopt/approximation/piecewise.glsl",
      .min = 1,
      .max = 30,
      .inc = 1,
      .type = Type::Piecewise},
     {.identifier = "powermoments",
      .name = "Power moments",
      .define = "POWER_MOMENTS",
      .file = "opactopt/approximation/powermoments.glsl",
      .min = 5,
      .max = 9,
      .inc = 2,
      .type = Type::PowerMoments},
     {.identifier = "trigmoments",
      .name = "Trigonometric moments",
      .define = "TRIG_MOMENTS",
      .file = "opactopt/approximation/trigmoments.glsl",
      .min = 5,
      .max = 9,
      .inc = 2,
      .type = Type::TrigonometricMoments}}};

constexpr const Properties& get(Type type) { return approximations.at(std::to_underlying(type)); }

IVW_MODULE_OPACTOPT_API std::vector<OptionPropertyOption<Type>>
generateApproximationStringOptions();

IVW_MODULE_OPACTOPT_API std::vector<float> generateLegendreCoefficients();

struct IVW_MODULE_OPACTOPT_API MomentSettingsGL {
    glm::vec4 wrappingZoneParameters{};
    float wrappingZoneAngle{};
    float overestimation{};
    std::array<float, 2> pad{};
};

IVW_MODULE_OPACTOPT_API MomentSettingsGL generateMomentSettings(
    float wrappingZoneAngle = 0.1f * std::numbers::pi_v<float>, float overestimation = 0.25f);

}  // namespace inviwo::approximations
