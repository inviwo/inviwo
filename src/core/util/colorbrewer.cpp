/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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
#include <fmt/ostream.h>
#include <cmath>

namespace inviwo {
namespace colorbrewer {

std::vector<colorbrewer::Family> getFamilies() {
    using Index = std::underlying_type_t<Family>;
    std::vector<Family> res;
    for (Index i = 0; i < static_cast<Index>(Family::NumberOfColormapFamilies); ++i) {
        res.push_back(static_cast<Family>(i));
    }
    return res;
}

const std::vector<dvec4>& getColormap(const Family& family, glm::uint8 numberOfColors) {
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

std::vector<std::vector<dvec4>> getColormaps(const Family& family) {
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
    for (const auto& c : v) {
        ret.emplace_back(getColormap(c));
    }

    return ret;
}

std::map<Family, std::vector<std::vector<dvec4>>> getColormaps(const Category& category) {
    std::map<Family, std::vector<std::vector<dvec4>>> v;

    for (const auto& family : getFamiliesForCategory(category))
        v.emplace(family, getColormaps(family));

    return v;
}

std::map<Family, std::vector<dvec4>> getColormaps(const Category& category,
                                                  glm::uint8 numberOfColors) {
    std::map<Family, std::vector<dvec4>> v;

    for (const auto& family : getFamiliesForCategory(category)) {
        // We catch the exceptions here because otherwise, the method would just throw an
        // exception if one of the requested colormaps is not available, even if the others were.
        // This way, if 3 out of 4 requested colormaps exist, they are returned.
        try {
            v.emplace(family, getColormap(family, numberOfColors));
        } catch (UnsupportedNumberOfColorsException& e) {
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

TransferFunction getTransferFunction(const Category& category, const Family& family,
                                     const size_t nColors, const bool discrete,
                                     const double midPoint, const double start, const double stop) {
    TransferFunction tf;

    const size_t minColors = getMinNumberOfColorsForFamily(family);
    const size_t maxColors = getMaxNumberOfColorsForFamily(family);

    const auto& colormap = colorbrewer::getColormap(
        family, static_cast<glm::uint8>(std::clamp(nColors, minColors, maxColors)));

    auto color = [&](size_t i) { return vec4(colormap[i % colormap.size()]); };

    auto addPoint = [&](double pos, size_t i) { tf.add(std::clamp(pos, 0.0, 1.0), color(i)); };
    auto addPointAlmost = [&](double pos, size_t i) {
        addPoint(pos - 100.0 * std::numeric_limits<double>::epsilon(), i);
    };

    if (category == colorbrewer::Category::Diverging) {
        if (discrete) {
            double midPointOffset = 0.0;
            if (midPoint > start) {
                const auto dt = (midPoint - start) / (0.5 * nColors);
                midPointOffset = 0.5 * dt * (nColors % 2);
                for (size_t i = 0; i < nColors / 2; i++) {
                    addPoint(start + i * dt, i);
                    addPointAlmost(start + (i + 1) * dt, i);
                }
            }
            addPoint(midPoint - midPointOffset, nColors / 2);
            if (midPoint < stop) {
                const auto dt = (stop - midPoint) / (0.5 * nColors);
                midPointOffset = 0.5 * dt * (nColors % 2);
                addPointAlmost(midPoint + dt - midPointOffset, nColors / 2);
                for (auto i = nColors - 1; i > nColors / 2; i--) {
                    addPointAlmost(stop - (nColors - 1 - i) * dt, i);
                    addPoint(stop - (nColors - i) * dt, i);
                }
            }
        } else {
            if (midPoint > start) {
                const auto dt = (midPoint - start) / (0.5 * (nColors - 1.0));
                for (size_t i = 0; i < nColors / 2; i++) {
                    addPoint(start + i * dt, i);
                }
            }
            addPoint(midPoint, nColors / 2);
            if (midPoint < stop) {
                const auto dt = (stop - midPoint) / (0.5 * (nColors - 1.0));
                for (size_t i = nColors / 2 + 1; i < nColors; i++) {
                    addPoint(midPoint + (i - nColors / 2) * dt, i);
                }
            }
        }

    } else {
        if (discrete) {
            const auto dt = (stop - start) / nColors;
            for (size_t i = 0; i < nColors; i++) {
                addPoint(start + i * dt, i);
                addPointAlmost(start + (i + 1) * dt, i);
            }
        } else {
            const auto dt = (stop - start) / (nColors - 1.0);
            for (size_t i = 0; i < nColors; i++) {
                addPoint(start + i * dt, i);
            }
        }
    }
    return tf;
}

}  // namespace colorbrewer

}  // namespace inviwo
