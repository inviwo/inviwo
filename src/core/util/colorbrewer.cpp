/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwo/core/util/colorbrewer.h>
#include <inviwo/core/util/exception.h>

#include <fmt/format.h>

namespace inviwo {
namespace colorbrewer {

const std::vector<dvec4> &getColormap(const Family &family, glm::uint8 numberOfColors) {
    const auto minColors = getMinNumberOfColorsForFamily(family);
    const auto maxColors = getMaxNumberOfColorsForFamily(family);
    if (numberOfColors < minColors || numberOfColors > maxColors) {
        throw UnsupportedNumberOfColorsException(
            fmt::format("The colormap '{}' support between {} and {} colors. Requested {} colors",
                        family, minColors, maxColors, numberOfColors),
            IVW_CONTEXT_CUSTOM("inviwo::colorbrewer::getColormap"));
    }

    // Calculate offset into the std::vector<dvec4> enum class
    auto familyIndex = static_cast<int>(family);
    int accumulated = 0;
    for (int i = 0; i < familyIndex; i++) {
        auto a = getMinNumberOfColorsForFamily(static_cast<Family>(i));
        auto b = getMaxNumberOfColorsForFamily(static_cast<Family>(i));
        accumulated += (b - a) + 1;
    }

    accumulated += numberOfColors - getMinNumberOfColorsForFamily(family);

    auto c = static_cast<Colormap>(accumulated);

    return getColormap(c);
}

std::vector<std::vector<dvec4>> getColormaps(const Family &family) {
    // Calculate offset into the std::vector<dvec4> enum class
    auto familyIndex = static_cast<int>(family);
    int accumulated = 0;
    for (int i = 0; i < familyIndex; i++) {
        auto a = getMinNumberOfColorsForFamily(static_cast<Family>(i));
        auto b = getMaxNumberOfColorsForFamily(static_cast<Family>(i));
        accumulated += (b - a) + 1;
    }

    auto numColormapsInThisFamily =
        getMaxNumberOfColorsForFamily(static_cast<Family>(familyIndex)) -
        getMinNumberOfColorsForFamily(static_cast<Family>(familyIndex));

    std::vector<Colormap> v;
    for (int i = 0; i < numColormapsInThisFamily; i++)
        v.emplace_back(static_cast<Colormap>(accumulated + i));

    std::vector<std::vector<dvec4>> ret;
    for (const auto &c : v) {
        ret.emplace_back(getColormap(c));
    }

    return ret;
}

std::map<Family, std::vector<std::vector<dvec4>>> getColormaps(const Category &category) {
    std::map<Family, std::vector<std::vector<dvec4>>> v;

    for (const auto &family : getFamiliesForCategory(category))
        v.emplace(family, getColormaps(family));

    return v;
}

std::map<Family, std::vector<dvec4>> getColormaps(const Category &category,
                                                  glm::uint8 numberOfColors) {
    std::map<Family, std::vector<dvec4>> v;

    for (const auto &family : getFamiliesForCategory(category)) {
        // We catch the exceptions here because otherwise, the method would just throw an
        // exception if one of the requested colormaps is not available, even if the others were.
        // This way, if 3 out of 4 requested colormaps exist, they are returned.
        try {
            v.emplace(family, getColormap(family, numberOfColors));
        } catch (UnsupportedNumberOfColorsException &e) {
            LogWarnCustom("colorbrewer", "Family " << family << " omitted, reason: \n"
                                                   << e.getMessage(););
        }
    }

    if (v.empty()) {
        throw ColorBrewerException("Requested colormap is not available.",
                                   IVW_CONTEXT_CUSTOM("inviwo::colorbrewer::getColormaps"));
    }

    return v;
}

TransferFunction getTransferFunction(const Category &category, const Family &family,
                                     glm::uint8 nColors, bool discrete, double midPoint) {
    TransferFunction tf;
    auto colors = colorbrewer::getColormap(family, nColors);

    if (category == colorbrewer::Category::Diverging) {
        if (discrete) {
            auto dt = midPoint / (0.5 * (colors.size()));
            double start = 0, end = std::max(dt - std::numeric_limits<double>::epsilon(), 0.);
            for (auto i = 0u; i < colors.size() / 2; i++) {
                tf.add(start, vec4(colors[i]));
                tf.add(end, vec4(colors[i]));
                start += dt;
                end += dt;
            }
            tf.add(start, vec4(colors[colors.size() / 2]));
            if (midPoint < 1.0) {
                dt = (1.0 - midPoint) / (0.5 * (colors.size()));
                tf.add(start + dt - std::numeric_limits<double>::epsilon(),
                       vec4(colors[colors.size() / 2]));
                start = start + dt;
                end = start + dt - std::numeric_limits<double>::epsilon();
                for (auto i = colors.size() / 2 + 1; i < colors.size(); i++) {
                    // Avoid numerical issues with min
                    tf.add(std::min(start, 1.0), vec4(colors[i]));
                    tf.add(std::min(end, 1.0), vec4(colors[i]));
                    start += dt;
                    end += dt;
                }
            }
        } else {
            auto dt = midPoint / (0.5 * (colors.size() - 1.0));
            for (auto i = 0u; i < colors.size() / 2; i++) {
                tf.add(i * dt, vec4(colors[i]));
            }
            tf.add(midPoint, vec4(colors[colors.size() / 2]));
            if (midPoint < 1.0) {
                dt = (1.0 - midPoint) / (0.5 * (colors.size() - 1.0));
                auto t = midPoint + dt;
                for (auto i = colors.size() / 2 + 1; i < colors.size(); i++) {
                    // Avoid numerical issues with min
                    tf.add(std::min(t, 1.0), vec4(colors[i]));
                    t += dt;
                }
            }
        }

    } else {
        if (discrete) {
            double dt = 1.0 / (colors.size());
            double start = 0, end = dt - std::numeric_limits<double>::epsilon();
            for (const auto &c : colors) {
                tf.add(start, vec4(c));
                tf.add(end, vec4(c));
                start += dt;
                end += dt;
            }
        } else {
            auto dt = 1.0 / (colors.size() - 1.0);
            size_t idx = 0;
            for (const auto &c : colors) {
                tf.add(idx++ * dt, vec4(c));
            }
        }
    }
    return tf;
}

}  // namespace colorbrewer

}  // namespace inviwo
