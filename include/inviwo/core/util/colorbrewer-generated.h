/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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
#include <fmt/base.h>
#include <fmt/ostream.h>

namespace inviwo::colorbrewer {

// clang-format off
enum class Colormap {
    Accent_1 = 0, Accent_2 = 1, Accent_3 = 2, Accent_4 = 3, Accent_5 = 4, Accent_6 = 5, Accent_7 = 6, Accent_8 = 7, 
    Blues_3 = 8, Blues_4 = 9, Blues_5 = 10, Blues_6 = 11, Blues_7 = 12, Blues_8 = 13, Blues_9 = 14, 
    BrBG_3 = 15, BrBG_4 = 16, BrBG_5 = 17, BrBG_6 = 18, BrBG_7 = 19, BrBG_8 = 20, BrBG_9 = 21, BrBG_10 = 22, BrBG_11 = 23, 
    BuGn_3 = 24, BuGn_4 = 25, BuGn_5 = 26, BuGn_6 = 27, BuGn_7 = 28, BuGn_8 = 29, BuGn_9 = 30, 
    BuPu_3 = 31, BuPu_4 = 32, BuPu_5 = 33, BuPu_6 = 34, BuPu_7 = 35, BuPu_8 = 36, BuPu_9 = 37, 
    Dark2_3 = 38, Dark2_4 = 39, Dark2_5 = 40, Dark2_6 = 41, Dark2_7 = 42, Dark2_8 = 43, 
    GnBu_3 = 44, GnBu_4 = 45, GnBu_5 = 46, GnBu_6 = 47, GnBu_7 = 48, GnBu_8 = 49, GnBu_9 = 50, 
    Greens_3 = 51, Greens_4 = 52, Greens_5 = 53, Greens_6 = 54, Greens_7 = 55, Greens_8 = 56, Greens_9 = 57, 
    Greys_3 = 58, Greys_4 = 59, Greys_5 = 60, Greys_6 = 61, Greys_7 = 62, Greys_8 = 63, Greys_9 = 64, 
    OrRd_3 = 65, OrRd_4 = 66, OrRd_5 = 67, OrRd_6 = 68, OrRd_7 = 69, OrRd_8 = 70, OrRd_9 = 71, 
    Oranges_3 = 72, Oranges_4 = 73, Oranges_5 = 74, Oranges_6 = 75, Oranges_7 = 76, Oranges_8 = 77, Oranges_9 = 78, 
    PRGn_3 = 79, PRGn_4 = 80, PRGn_5 = 81, PRGn_6 = 82, PRGn_7 = 83, PRGn_8 = 84, PRGn_9 = 85, PRGn_10 = 86, PRGn_11 = 87, 
    Paired_1 = 88, Paired_2 = 89, Paired_3 = 90, Paired_4 = 91, Paired_5 = 92, Paired_6 = 93, Paired_7 = 94, Paired_8 = 95, Paired_9 = 96, Paired_10 = 97, Paired_11 = 98, Paired_12 = 99, 
    Pastel1_3 = 100, Pastel1_4 = 101, Pastel1_5 = 102, Pastel1_6 = 103, Pastel1_7 = 104, Pastel1_8 = 105, Pastel1_9 = 106, 
    Pastel2_3 = 107, Pastel2_4 = 108, Pastel2_5 = 109, Pastel2_6 = 110, Pastel2_7 = 111, Pastel2_8 = 112, 
    PiYG_3 = 113, PiYG_4 = 114, PiYG_5 = 115, PiYG_6 = 116, PiYG_7 = 117, PiYG_8 = 118, PiYG_9 = 119, PiYG_10 = 120, PiYG_11 = 121, 
    PuBu_3 = 122, PuBu_4 = 123, PuBu_5 = 124, PuBu_6 = 125, PuBu_7 = 126, PuBu_8 = 127, PuBu_9 = 128, 
    PuBuGn_3 = 129, PuBuGn_4 = 130, PuBuGn_5 = 131, PuBuGn_6 = 132, PuBuGn_7 = 133, PuBuGn_8 = 134, PuBuGn_9 = 135, 
    PuOr_3 = 136, PuOr_4 = 137, PuOr_5 = 138, PuOr_6 = 139, PuOr_7 = 140, PuOr_8 = 141, PuOr_9 = 142, PuOr_10 = 143, PuOr_11 = 144, 
    PuRd_3 = 145, PuRd_4 = 146, PuRd_5 = 147, PuRd_6 = 148, PuRd_7 = 149, PuRd_8 = 150, PuRd_9 = 151, 
    Purples_3 = 152, Purples_4 = 153, Purples_5 = 154, Purples_6 = 155, Purples_7 = 156, Purples_8 = 157, Purples_9 = 158, 
    RdBu_3 = 159, RdBu_4 = 160, RdBu_5 = 161, RdBu_6 = 162, RdBu_7 = 163, RdBu_8 = 164, RdBu_9 = 165, RdBu_10 = 166, RdBu_11 = 167, 
    RdGy_3 = 168, RdGy_4 = 169, RdGy_5 = 170, RdGy_6 = 171, RdGy_7 = 172, RdGy_8 = 173, RdGy_9 = 174, RdGy_10 = 175, RdGy_11 = 176, 
    RdPu_3 = 177, RdPu_4 = 178, RdPu_5 = 179, RdPu_6 = 180, RdPu_7 = 181, RdPu_8 = 182, RdPu_9 = 183, 
    RdYlBu_3 = 184, RdYlBu_4 = 185, RdYlBu_5 = 186, RdYlBu_6 = 187, RdYlBu_7 = 188, RdYlBu_8 = 189, RdYlBu_9 = 190, RdYlBu_10 = 191, RdYlBu_11 = 192, 
    RdYlGn_3 = 193, RdYlGn_4 = 194, RdYlGn_5 = 195, RdYlGn_6 = 196, RdYlGn_7 = 197, RdYlGn_8 = 198, RdYlGn_9 = 199, RdYlGn_10 = 200, RdYlGn_11 = 201, 
    Reds_3 = 202, Reds_4 = 203, Reds_5 = 204, Reds_6 = 205, Reds_7 = 206, Reds_8 = 207, Reds_9 = 208, 
    Set1_1 = 209, Set1_2 = 210, Set1_3 = 211, Set1_4 = 212, Set1_5 = 213, Set1_6 = 214, Set1_7 = 215, Set1_8 = 216, Set1_9 = 217, 
    Set2_1 = 218, Set2_2 = 219, Set2_3 = 220, Set2_4 = 221, Set2_5 = 222, Set2_6 = 223, Set2_7 = 224, Set2_8 = 225, 
    Set3_3 = 226, Set3_4 = 227, Set3_5 = 228, Set3_6 = 229, Set3_7 = 230, Set3_8 = 231, Set3_9 = 232, Set3_10 = 233, Set3_11 = 234, Set3_12 = 235, 
    Spectral_3 = 236, Spectral_4 = 237, Spectral_5 = 238, Spectral_6 = 239, Spectral_7 = 240, Spectral_8 = 241, Spectral_9 = 242, Spectral_10 = 243, Spectral_11 = 244, 
    YlGn_3 = 245, YlGn_4 = 246, YlGn_5 = 247, YlGn_6 = 248, YlGn_7 = 249, YlGn_8 = 250, YlGn_9 = 251, 
    YlGnBu_3 = 252, YlGnBu_4 = 253, YlGnBu_5 = 254, YlGnBu_6 = 255, YlGnBu_7 = 256, YlGnBu_8 = 257, YlGnBu_9 = 258, 
    YlOrBr_3 = 259, YlOrBr_4 = 260, YlOrBr_5 = 261, YlOrBr_6 = 262, YlOrBr_7 = 263, YlOrBr_8 = 264, YlOrBr_9 = 265, 
    YlOrRd_3 = 266, YlOrRd_4 = 267, YlOrRd_5 = 268, YlOrRd_6 = 269, YlOrRd_7 = 270, YlOrRd_8 = 271, 
    FirstMap=Accent_1, LastMap=YlOrRd_8
};

enum class Category { Diverging, Qualitative, Sequential, NumberOfColormapCategories, Undefined };

enum class Family {
    Accent, Blues, BrBG, BuGn, BuPu, Dark2, GnBu, 
    Greens, Greys, OrRd, Oranges, PRGn, Paired, Pastel1, 
    Pastel2, PiYG, PuBu, PuBuGn, PuOr, PuRd, Purples, 
    RdBu, RdGy, RdPu, RdYlBu, RdYlGn, Reds, Set1, 
    Set2, Set3, Spectral, YlGn, YlGnBu, YlOrBr, YlOrRd, 
    NumberOfColormapFamilies, Undefined
};
// clang-format on

IVW_CORE_API std::ostream& operator<<(std::ostream& os, Colormap colormap);
IVW_CORE_API std::ostream& operator<<(std::ostream& os, Category category);
IVW_CORE_API std::ostream& operator<<(std::ostream& os, Family family);

/**
 * Returns the specified colormap. For reference see http://colorbrewer2.org/
 **/
IVW_CORE_API const std::vector<dvec4>& getColormap(Colormap colormap);

/**
 * Returns the minimum number of colors for which the requested family is available.
 **/
IVW_CORE_API glm::uint8 getMinNumberOfColorsForFamily(const Family& family);

/**
 * Returns the maximum number of colors for which the requested family is available.
 **/
IVW_CORE_API glm::uint8 getMaxNumberOfColorsForFamily(const Family& family);

/**
 * Returns all families contained in the specified category.
 **/
IVW_CORE_API std::vector<Family> getFamiliesForCategory(const Category& category);

}  // namespace inviwo::colorbrewer

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::colorbrewer::Colormap> : ostream_formatter {};
template <>
struct fmt::formatter<inviwo::colorbrewer::Category> : ostream_formatter {};
template <>
struct fmt::formatter<inviwo::colorbrewer::Family> : ostream_formatter {};
#endif
