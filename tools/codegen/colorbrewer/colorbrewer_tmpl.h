/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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

/*
This complete file is auto-generated with python script
tools/codegen/colorbrewer/colorbrewer.py
*/

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glmvec.h>

#include <vector>
#include <ostream>
#include <fmt/core.h>
#include <fmt/ostream.h>

namespace inviwo {
namespace colorbrewer {

// clang-format off
##PLACEHOLDER##
// clang-format on

IVW_CORE_API std::ostream& operator<<(std::ostream& os, Colormap colormap);
IVW_CORE_API std::ostream& operator<<(std::ostream& os, Category category);
IVW_CORE_API std::ostream& operator<<(std::ostream& os, Family family);

/**
 * Returns the specified colormap. For reference see http://colorbrewer2.org/
 **/
IVW_CORE_API const std::vector<dvec4> &getColormap(Colormap colormap);

/**
 * Returns the minimum number of colors for which the requested family is available.
 **/
IVW_CORE_API glm::uint8 getMinNumberOfColorsForFamily(const Family &family);

/**
 * Returns the maximum number of colors for which the requested family is available.
 **/
IVW_CORE_API glm::uint8 getMaxNumberOfColorsForFamily(const Family &family);

/**
 * Returns all families contained in the specified category.
 **/
IVW_CORE_API std::vector<Family> getFamiliesForCategory(const Category &category);

}  // namespace colorbrewer
}  // namespace inviwo

template <> struct fmt::formatter<inviwo::colorbrewer::Colormap> : ostream_formatter {};
template <> struct fmt::formatter<inviwo::colorbrewer::Category> : ostream_formatter {};
template <> struct fmt::formatter<inviwo::colorbrewer::Family> : ostream_formatter {};
