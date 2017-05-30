/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

/**
This complete file is auto-generated with python script
tools/codegen/colorbrewer/colorbrewer.py
**/

#include <inviwo/core/util/colorbrewer.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {
namespace colorbrewer {

const std::vector<dvec4> &getColormap(Colormap colormap) {
    switch (colormap) {
##PLACEHOLDER##
    }
    throw Exception("invalid colorbrewer colormap");
}

const std::vector<dvec4> &getColormap(const Family &family, glm::uint8 numberOfColors) {
    if (getMinNumberOfColorsForFamily(family) > numberOfColors ||
        getMaxNumberOfColorsForFamily(family) < numberOfColors) {
        throw ColorBrewerException();
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
        try {
            v.emplace(family, getColormap(family, numberOfColors));
        }
        catch (ColorBrewerException&) {
        }
    }

    if (v.empty()) {
        throw ColorBrewerException();
    }

    return v;
}

glm::uint8 getMinNumberOfColorsForFamily(const Family &family) { return 3; }

glm::uint8 getMaxNumberOfColorsForFamily(const Family &family) {
##GETMAXIMPL##
    return 0;
}

std::vector<Family> getFamiliesForCategory(const Category &category) {
    std::vector<Family> v;
    switch (category) {
##GETFAMILIESIMPL##
    }

    return v;
}

}  // namespace colorbrewer

}  // namespace inviwo
