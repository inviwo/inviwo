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

#include <modules/plotting/utils/labelscaling.h>

#include <inviwo/core/util/logcentral.h>

namespace inviwo::plot {

namespace {

std::string formatExponent(int exp) {
    constexpr std::array<std::string_view, 10> powers = {"\u2070", "\u00B9", "\u00B2", "\u00B3",
                                                         "\u2074", "\u2075", "\u2076", "\u2077",
                                                         "\u2078", "\u2079"};
    constexpr std::string_view minus = "\u207B";

    std::string res{};
    if (exp == 0) return res;
    res += "10";

    if (exp < 0) {
        res += minus;
        exp = -exp;
    }
    while (exp != 0) {
        res += powers[exp % 10];
        exp /= 10;
    }
    res += ' ';
    return res;
}
}  // namespace

OptionPropertyState<CaptionType> captionTypeState() {
    return {.options = {{"string", "Caption String", CaptionType::String},
                        {"data", "Caption from Data", CaptionType::Data},
                        {"custom", "Custom Format (example '{n}{u: [}')", CaptionType::Custom}},
            .selectedIndex = 0};
}

OptionPropertyState<LabelScale> labelScaleState() {
    return {.options = {{"none", "None", LabelScale::None},
                        {"tens", "Tens", LabelScale::Tens},
                        {"thousands", "Thousands", LabelScale::Thousands}},
            .selectedIndex = 0};
}

std::string formatAxisCaption(const Axis& axis, CaptionType captionType, LabelScale labelScale,
                              std::string_view customFormat, int exponent,
                              std::string_view originalCaption) {

    switch (captionType) {
        case CaptionType::Data:
            return fmt::format("{}{: [}", axis.name, scaleUnit(labelScale, axis.unit, exponent));
        case CaptionType::Custom:
            try {
                return fmt::format(
                    fmt::runtime(customFormat), fmt::arg("n", axis.name), fmt::arg("u", axis.unit),
                    fmt::arg("su", scaleUnit(labelScale, axis.unit, exponent)),
                    fmt::arg("s", formatExponent(exponent)), fmt::arg("e", exponent));
            } catch (const fmt::format_error& e) {
                log::error("Invalid custom caption format: {}: {}", customFormat, e.what());
                return std::string{originalCaption};
            }

        case CaptionType::String:
            [[fallthrough]];
        default:
            return std::string{originalCaption};
    }
}

}  // namespace inviwo::plot
