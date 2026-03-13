/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/plotting/plottingmoduledefine.h>

#include <inviwo/core/properties/optionproperty.h>

#include <inviwo/core/datastructures/unitsystem.h>

#include <glm/gtx/component_wise.hpp>

#include <fmt/base.h>

#include <cstdint>
#include <optional>
#include <string>
#include <array>

namespace inviwo::plot {

enum class CaptionType : std::uint8_t { String, Data, Custom };
enum class LabelScale : std::uint8_t { None, Tens, Thousands };

constexpr std::optional<int> labelScaleStep(LabelScale scale) {
    switch (scale) {
        case LabelScale::None:
            return std::nullopt;
        case LabelScale::Tens:
            return 1;
        case LabelScale::Thousands:
            return 3;
    }
    return std::nullopt;
}

constexpr auto scaleUnit(LabelScale labelScale, Unit unit, int exp) {
    switch (labelScale) {
        case LabelScale::None:
            return unit;
        case LabelScale::Tens:
            [[fallthrough]];
        case LabelScale::Thousands:
            [[fallthrough]];
        default:
            return Unit{std::pow(10, exp), unit};
    }
}

constexpr std::pair<dvec2, int> scaleRange(const dvec2& r, const std::optional<int>& autoScale) {
    if (autoScale) {
        const auto s = static_cast<double>(*autoScale);
        const auto exp = std::floor(std::log10(glm::compMax(glm::abs(r))));
        const auto majorExp = (exp > 0 ? std::floor(exp / s) : std::round(exp / s)) * s;
        const auto factor = std::pow(10.0, -majorExp);
        return {r * factor, static_cast<int>(majorExp)};
    } else {
        return {r, 0};
    }
}

IVW_MODULE_PLOTTING_API OptionPropertyState<CaptionType> captionTypeState();

IVW_MODULE_PLOTTING_API OptionPropertyState<LabelScale> labelScaleState();

IVW_MODULE_PLOTTING_API std::string formatAxisCaption(const Axis& axis, CaptionType captionType,
                                                      LabelScale labelScale,
                                                      std::string_view customFormat, int exponent,
                                                      std::string_view origCaption);

}  // namespace inviwo::plot
