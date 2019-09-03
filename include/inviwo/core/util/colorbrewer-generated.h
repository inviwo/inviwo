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

/*
This complete file is auto-generated with python script
tools/codegen/colorbrewer/colorbrewer.py
*/

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <vector>
#include <ostream>

namespace inviwo {
namespace colorbrewer {

// clang-format off
enum class Colormap {
    Accent_3, Accent_4, Accent_5, Accent_6, Accent_7, Accent_8, 
    Blues_3, Blues_4, Blues_5, Blues_6, Blues_7, Blues_8, Blues_9, 
    BrBG_3, BrBG_4, BrBG_5, BrBG_6, BrBG_7, BrBG_8, BrBG_9, BrBG_10, BrBG_11, 
    BuGn_3, BuGn_4, BuGn_5, BuGn_6, BuGn_7, BuGn_8, BuGn_9, 
    BuPu_3, BuPu_4, BuPu_5, BuPu_6, BuPu_7, BuPu_8, BuPu_9, 
    Dark2_3, Dark2_4, Dark2_5, Dark2_6, Dark2_7, Dark2_8, 
    GnBu_3, GnBu_4, GnBu_5, GnBu_6, GnBu_7, GnBu_8, GnBu_9, 
    Greens_3, Greens_4, Greens_5, Greens_6, Greens_7, Greens_8, Greens_9, 
    Greys_3, Greys_4, Greys_5, Greys_6, Greys_7, Greys_8, Greys_9, 
    OrRd_3, OrRd_4, OrRd_5, OrRd_6, OrRd_7, OrRd_8, OrRd_9, 
    Oranges_3, Oranges_4, Oranges_5, Oranges_6, Oranges_7, Oranges_8, Oranges_9, 
    PRGn_3, PRGn_4, PRGn_5, PRGn_6, PRGn_7, PRGn_8, PRGn_9, PRGn_10, PRGn_11, 
    Paired_3, Paired_4, Paired_5, Paired_6, Paired_7, Paired_8, Paired_9, Paired_10, Paired_11, Paired_12, 
    Pastel1_3, Pastel1_4, Pastel1_5, Pastel1_6, Pastel1_7, Pastel1_8, Pastel1_9, 
    Pastel2_3, Pastel2_4, Pastel2_5, Pastel2_6, Pastel2_7, Pastel2_8, 
    PiYG_3, PiYG_4, PiYG_5, PiYG_6, PiYG_7, PiYG_8, PiYG_9, PiYG_10, PiYG_11, 
    PuBu_3, PuBu_4, PuBu_5, PuBu_6, PuBu_7, PuBu_8, PuBu_9, 
    PuBuGn_3, PuBuGn_4, PuBuGn_5, PuBuGn_6, PuBuGn_7, PuBuGn_8, PuBuGn_9, 
    PuOr_3, PuOr_4, PuOr_5, PuOr_6, PuOr_7, PuOr_8, PuOr_9, PuOr_10, PuOr_11, 
    PuRd_3, PuRd_4, PuRd_5, PuRd_6, PuRd_7, PuRd_8, PuRd_9, 
    Purples_3, Purples_4, Purples_5, Purples_6, Purples_7, Purples_8, Purples_9, 
    RdBu_3, RdBu_4, RdBu_5, RdBu_6, RdBu_7, RdBu_8, RdBu_9, RdBu_10, RdBu_11, 
    RdGy_3, RdGy_4, RdGy_5, RdGy_6, RdGy_7, RdGy_8, RdGy_9, RdGy_10, RdGy_11, 
    RdPu_3, RdPu_4, RdPu_5, RdPu_6, RdPu_7, RdPu_8, RdPu_9, 
    RdYlBu_3, RdYlBu_4, RdYlBu_5, RdYlBu_6, RdYlBu_7, RdYlBu_8, RdYlBu_9, RdYlBu_10, RdYlBu_11, 
    RdYlGn_3, RdYlGn_4, RdYlGn_5, RdYlGn_6, RdYlGn_7, RdYlGn_8, RdYlGn_9, RdYlGn_10, RdYlGn_11, 
    Reds_3, Reds_4, Reds_5, Reds_6, Reds_7, Reds_8, Reds_9, 
    Set1_3, Set1_4, Set1_5, Set1_6, Set1_7, Set1_8, Set1_9, 
    Set2_3, Set2_4, Set2_5, Set2_6, Set2_7, Set2_8, 
    Set3_3, Set3_4, Set3_5, Set3_6, Set3_7, Set3_8, Set3_9, Set3_10, Set3_11, Set3_12, 
    Spectral_3, Spectral_4, Spectral_5, Spectral_6, Spectral_7, Spectral_8, Spectral_9, Spectral_10, Spectral_11, 
    YlGn_3, YlGn_4, YlGn_5, YlGn_6, YlGn_7, YlGn_8, YlGn_9, 
    YlGnBu_3, YlGnBu_4, YlGnBu_5, YlGnBu_6, YlGnBu_7, YlGnBu_8, YlGnBu_9, 
    YlOrBr_3, YlOrBr_4, YlOrBr_5, YlOrBr_6, YlOrBr_7, YlOrBr_8, YlOrBr_9, 
    YlOrRd_3, YlOrRd_4, YlOrRd_5, YlOrRd_6, YlOrRd_7, YlOrRd_8, 
    FirstMap=Accent_3, LastMap=YlOrRd_8
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
template <class Elem, class Traits>
std::basic_ostream<Elem, Traits> &operator<<(std::basic_ostream<Elem, Traits> &os,
                                             Colormap colormap) {
    switch (colormap) {
        // clang-format off
        case Colormap::Accent_3: os << "Accent_3"; break;
        case Colormap::Accent_4: os << "Accent_4"; break;
        case Colormap::Accent_5: os << "Accent_5"; break;
        case Colormap::Accent_6: os << "Accent_6"; break;
        case Colormap::Accent_7: os << "Accent_7"; break;
        case Colormap::Accent_8: os << "Accent_8"; break;
        case Colormap::Blues_3: os << "Blues_3"; break;
        case Colormap::Blues_4: os << "Blues_4"; break;
        case Colormap::Blues_5: os << "Blues_5"; break;
        case Colormap::Blues_6: os << "Blues_6"; break;
        case Colormap::Blues_7: os << "Blues_7"; break;
        case Colormap::Blues_8: os << "Blues_8"; break;
        case Colormap::Blues_9: os << "Blues_9"; break;
        case Colormap::BrBG_3: os << "BrBG_3"; break;
        case Colormap::BrBG_4: os << "BrBG_4"; break;
        case Colormap::BrBG_5: os << "BrBG_5"; break;
        case Colormap::BrBG_6: os << "BrBG_6"; break;
        case Colormap::BrBG_7: os << "BrBG_7"; break;
        case Colormap::BrBG_8: os << "BrBG_8"; break;
        case Colormap::BrBG_9: os << "BrBG_9"; break;
        case Colormap::BrBG_10: os << "BrBG_10"; break;
        case Colormap::BrBG_11: os << "BrBG_11"; break;
        case Colormap::BuGn_3: os << "BuGn_3"; break;
        case Colormap::BuGn_4: os << "BuGn_4"; break;
        case Colormap::BuGn_5: os << "BuGn_5"; break;
        case Colormap::BuGn_6: os << "BuGn_6"; break;
        case Colormap::BuGn_7: os << "BuGn_7"; break;
        case Colormap::BuGn_8: os << "BuGn_8"; break;
        case Colormap::BuGn_9: os << "BuGn_9"; break;
        case Colormap::BuPu_3: os << "BuPu_3"; break;
        case Colormap::BuPu_4: os << "BuPu_4"; break;
        case Colormap::BuPu_5: os << "BuPu_5"; break;
        case Colormap::BuPu_6: os << "BuPu_6"; break;
        case Colormap::BuPu_7: os << "BuPu_7"; break;
        case Colormap::BuPu_8: os << "BuPu_8"; break;
        case Colormap::BuPu_9: os << "BuPu_9"; break;
        case Colormap::Dark2_3: os << "Dark2_3"; break;
        case Colormap::Dark2_4: os << "Dark2_4"; break;
        case Colormap::Dark2_5: os << "Dark2_5"; break;
        case Colormap::Dark2_6: os << "Dark2_6"; break;
        case Colormap::Dark2_7: os << "Dark2_7"; break;
        case Colormap::Dark2_8: os << "Dark2_8"; break;
        case Colormap::GnBu_3: os << "GnBu_3"; break;
        case Colormap::GnBu_4: os << "GnBu_4"; break;
        case Colormap::GnBu_5: os << "GnBu_5"; break;
        case Colormap::GnBu_6: os << "GnBu_6"; break;
        case Colormap::GnBu_7: os << "GnBu_7"; break;
        case Colormap::GnBu_8: os << "GnBu_8"; break;
        case Colormap::GnBu_9: os << "GnBu_9"; break;
        case Colormap::Greens_3: os << "Greens_3"; break;
        case Colormap::Greens_4: os << "Greens_4"; break;
        case Colormap::Greens_5: os << "Greens_5"; break;
        case Colormap::Greens_6: os << "Greens_6"; break;
        case Colormap::Greens_7: os << "Greens_7"; break;
        case Colormap::Greens_8: os << "Greens_8"; break;
        case Colormap::Greens_9: os << "Greens_9"; break;
        case Colormap::Greys_3: os << "Greys_3"; break;
        case Colormap::Greys_4: os << "Greys_4"; break;
        case Colormap::Greys_5: os << "Greys_5"; break;
        case Colormap::Greys_6: os << "Greys_6"; break;
        case Colormap::Greys_7: os << "Greys_7"; break;
        case Colormap::Greys_8: os << "Greys_8"; break;
        case Colormap::Greys_9: os << "Greys_9"; break;
        case Colormap::OrRd_3: os << "OrRd_3"; break;
        case Colormap::OrRd_4: os << "OrRd_4"; break;
        case Colormap::OrRd_5: os << "OrRd_5"; break;
        case Colormap::OrRd_6: os << "OrRd_6"; break;
        case Colormap::OrRd_7: os << "OrRd_7"; break;
        case Colormap::OrRd_8: os << "OrRd_8"; break;
        case Colormap::OrRd_9: os << "OrRd_9"; break;
        case Colormap::Oranges_3: os << "Oranges_3"; break;
        case Colormap::Oranges_4: os << "Oranges_4"; break;
        case Colormap::Oranges_5: os << "Oranges_5"; break;
        case Colormap::Oranges_6: os << "Oranges_6"; break;
        case Colormap::Oranges_7: os << "Oranges_7"; break;
        case Colormap::Oranges_8: os << "Oranges_8"; break;
        case Colormap::Oranges_9: os << "Oranges_9"; break;
        case Colormap::PRGn_3: os << "PRGn_3"; break;
        case Colormap::PRGn_4: os << "PRGn_4"; break;
        case Colormap::PRGn_5: os << "PRGn_5"; break;
        case Colormap::PRGn_6: os << "PRGn_6"; break;
        case Colormap::PRGn_7: os << "PRGn_7"; break;
        case Colormap::PRGn_8: os << "PRGn_8"; break;
        case Colormap::PRGn_9: os << "PRGn_9"; break;
        case Colormap::PRGn_10: os << "PRGn_10"; break;
        case Colormap::PRGn_11: os << "PRGn_11"; break;
        case Colormap::Paired_3: os << "Paired_3"; break;
        case Colormap::Paired_4: os << "Paired_4"; break;
        case Colormap::Paired_5: os << "Paired_5"; break;
        case Colormap::Paired_6: os << "Paired_6"; break;
        case Colormap::Paired_7: os << "Paired_7"; break;
        case Colormap::Paired_8: os << "Paired_8"; break;
        case Colormap::Paired_9: os << "Paired_9"; break;
        case Colormap::Paired_10: os << "Paired_10"; break;
        case Colormap::Paired_11: os << "Paired_11"; break;
        case Colormap::Paired_12: os << "Paired_12"; break;
        case Colormap::Pastel1_3: os << "Pastel1_3"; break;
        case Colormap::Pastel1_4: os << "Pastel1_4"; break;
        case Colormap::Pastel1_5: os << "Pastel1_5"; break;
        case Colormap::Pastel1_6: os << "Pastel1_6"; break;
        case Colormap::Pastel1_7: os << "Pastel1_7"; break;
        case Colormap::Pastel1_8: os << "Pastel1_8"; break;
        case Colormap::Pastel1_9: os << "Pastel1_9"; break;
        case Colormap::Pastel2_3: os << "Pastel2_3"; break;
        case Colormap::Pastel2_4: os << "Pastel2_4"; break;
        case Colormap::Pastel2_5: os << "Pastel2_5"; break;
        case Colormap::Pastel2_6: os << "Pastel2_6"; break;
        case Colormap::Pastel2_7: os << "Pastel2_7"; break;
        case Colormap::Pastel2_8: os << "Pastel2_8"; break;
        case Colormap::PiYG_3: os << "PiYG_3"; break;
        case Colormap::PiYG_4: os << "PiYG_4"; break;
        case Colormap::PiYG_5: os << "PiYG_5"; break;
        case Colormap::PiYG_6: os << "PiYG_6"; break;
        case Colormap::PiYG_7: os << "PiYG_7"; break;
        case Colormap::PiYG_8: os << "PiYG_8"; break;
        case Colormap::PiYG_9: os << "PiYG_9"; break;
        case Colormap::PiYG_10: os << "PiYG_10"; break;
        case Colormap::PiYG_11: os << "PiYG_11"; break;
        case Colormap::PuBu_3: os << "PuBu_3"; break;
        case Colormap::PuBu_4: os << "PuBu_4"; break;
        case Colormap::PuBu_5: os << "PuBu_5"; break;
        case Colormap::PuBu_6: os << "PuBu_6"; break;
        case Colormap::PuBu_7: os << "PuBu_7"; break;
        case Colormap::PuBu_8: os << "PuBu_8"; break;
        case Colormap::PuBu_9: os << "PuBu_9"; break;
        case Colormap::PuBuGn_3: os << "PuBuGn_3"; break;
        case Colormap::PuBuGn_4: os << "PuBuGn_4"; break;
        case Colormap::PuBuGn_5: os << "PuBuGn_5"; break;
        case Colormap::PuBuGn_6: os << "PuBuGn_6"; break;
        case Colormap::PuBuGn_7: os << "PuBuGn_7"; break;
        case Colormap::PuBuGn_8: os << "PuBuGn_8"; break;
        case Colormap::PuBuGn_9: os << "PuBuGn_9"; break;
        case Colormap::PuOr_3: os << "PuOr_3"; break;
        case Colormap::PuOr_4: os << "PuOr_4"; break;
        case Colormap::PuOr_5: os << "PuOr_5"; break;
        case Colormap::PuOr_6: os << "PuOr_6"; break;
        case Colormap::PuOr_7: os << "PuOr_7"; break;
        case Colormap::PuOr_8: os << "PuOr_8"; break;
        case Colormap::PuOr_9: os << "PuOr_9"; break;
        case Colormap::PuOr_10: os << "PuOr_10"; break;
        case Colormap::PuOr_11: os << "PuOr_11"; break;
        case Colormap::PuRd_3: os << "PuRd_3"; break;
        case Colormap::PuRd_4: os << "PuRd_4"; break;
        case Colormap::PuRd_5: os << "PuRd_5"; break;
        case Colormap::PuRd_6: os << "PuRd_6"; break;
        case Colormap::PuRd_7: os << "PuRd_7"; break;
        case Colormap::PuRd_8: os << "PuRd_8"; break;
        case Colormap::PuRd_9: os << "PuRd_9"; break;
        case Colormap::Purples_3: os << "Purples_3"; break;
        case Colormap::Purples_4: os << "Purples_4"; break;
        case Colormap::Purples_5: os << "Purples_5"; break;
        case Colormap::Purples_6: os << "Purples_6"; break;
        case Colormap::Purples_7: os << "Purples_7"; break;
        case Colormap::Purples_8: os << "Purples_8"; break;
        case Colormap::Purples_9: os << "Purples_9"; break;
        case Colormap::RdBu_3: os << "RdBu_3"; break;
        case Colormap::RdBu_4: os << "RdBu_4"; break;
        case Colormap::RdBu_5: os << "RdBu_5"; break;
        case Colormap::RdBu_6: os << "RdBu_6"; break;
        case Colormap::RdBu_7: os << "RdBu_7"; break;
        case Colormap::RdBu_8: os << "RdBu_8"; break;
        case Colormap::RdBu_9: os << "RdBu_9"; break;
        case Colormap::RdBu_10: os << "RdBu_10"; break;
        case Colormap::RdBu_11: os << "RdBu_11"; break;
        case Colormap::RdGy_3: os << "RdGy_3"; break;
        case Colormap::RdGy_4: os << "RdGy_4"; break;
        case Colormap::RdGy_5: os << "RdGy_5"; break;
        case Colormap::RdGy_6: os << "RdGy_6"; break;
        case Colormap::RdGy_7: os << "RdGy_7"; break;
        case Colormap::RdGy_8: os << "RdGy_8"; break;
        case Colormap::RdGy_9: os << "RdGy_9"; break;
        case Colormap::RdGy_10: os << "RdGy_10"; break;
        case Colormap::RdGy_11: os << "RdGy_11"; break;
        case Colormap::RdPu_3: os << "RdPu_3"; break;
        case Colormap::RdPu_4: os << "RdPu_4"; break;
        case Colormap::RdPu_5: os << "RdPu_5"; break;
        case Colormap::RdPu_6: os << "RdPu_6"; break;
        case Colormap::RdPu_7: os << "RdPu_7"; break;
        case Colormap::RdPu_8: os << "RdPu_8"; break;
        case Colormap::RdPu_9: os << "RdPu_9"; break;
        case Colormap::RdYlBu_3: os << "RdYlBu_3"; break;
        case Colormap::RdYlBu_4: os << "RdYlBu_4"; break;
        case Colormap::RdYlBu_5: os << "RdYlBu_5"; break;
        case Colormap::RdYlBu_6: os << "RdYlBu_6"; break;
        case Colormap::RdYlBu_7: os << "RdYlBu_7"; break;
        case Colormap::RdYlBu_8: os << "RdYlBu_8"; break;
        case Colormap::RdYlBu_9: os << "RdYlBu_9"; break;
        case Colormap::RdYlBu_10: os << "RdYlBu_10"; break;
        case Colormap::RdYlBu_11: os << "RdYlBu_11"; break;
        case Colormap::RdYlGn_3: os << "RdYlGn_3"; break;
        case Colormap::RdYlGn_4: os << "RdYlGn_4"; break;
        case Colormap::RdYlGn_5: os << "RdYlGn_5"; break;
        case Colormap::RdYlGn_6: os << "RdYlGn_6"; break;
        case Colormap::RdYlGn_7: os << "RdYlGn_7"; break;
        case Colormap::RdYlGn_8: os << "RdYlGn_8"; break;
        case Colormap::RdYlGn_9: os << "RdYlGn_9"; break;
        case Colormap::RdYlGn_10: os << "RdYlGn_10"; break;
        case Colormap::RdYlGn_11: os << "RdYlGn_11"; break;
        case Colormap::Reds_3: os << "Reds_3"; break;
        case Colormap::Reds_4: os << "Reds_4"; break;
        case Colormap::Reds_5: os << "Reds_5"; break;
        case Colormap::Reds_6: os << "Reds_6"; break;
        case Colormap::Reds_7: os << "Reds_7"; break;
        case Colormap::Reds_8: os << "Reds_8"; break;
        case Colormap::Reds_9: os << "Reds_9"; break;
        case Colormap::Set1_3: os << "Set1_3"; break;
        case Colormap::Set1_4: os << "Set1_4"; break;
        case Colormap::Set1_5: os << "Set1_5"; break;
        case Colormap::Set1_6: os << "Set1_6"; break;
        case Colormap::Set1_7: os << "Set1_7"; break;
        case Colormap::Set1_8: os << "Set1_8"; break;
        case Colormap::Set1_9: os << "Set1_9"; break;
        case Colormap::Set2_3: os << "Set2_3"; break;
        case Colormap::Set2_4: os << "Set2_4"; break;
        case Colormap::Set2_5: os << "Set2_5"; break;
        case Colormap::Set2_6: os << "Set2_6"; break;
        case Colormap::Set2_7: os << "Set2_7"; break;
        case Colormap::Set2_8: os << "Set2_8"; break;
        case Colormap::Set3_3: os << "Set3_3"; break;
        case Colormap::Set3_4: os << "Set3_4"; break;
        case Colormap::Set3_5: os << "Set3_5"; break;
        case Colormap::Set3_6: os << "Set3_6"; break;
        case Colormap::Set3_7: os << "Set3_7"; break;
        case Colormap::Set3_8: os << "Set3_8"; break;
        case Colormap::Set3_9: os << "Set3_9"; break;
        case Colormap::Set3_10: os << "Set3_10"; break;
        case Colormap::Set3_11: os << "Set3_11"; break;
        case Colormap::Set3_12: os << "Set3_12"; break;
        case Colormap::Spectral_3: os << "Spectral_3"; break;
        case Colormap::Spectral_4: os << "Spectral_4"; break;
        case Colormap::Spectral_5: os << "Spectral_5"; break;
        case Colormap::Spectral_6: os << "Spectral_6"; break;
        case Colormap::Spectral_7: os << "Spectral_7"; break;
        case Colormap::Spectral_8: os << "Spectral_8"; break;
        case Colormap::Spectral_9: os << "Spectral_9"; break;
        case Colormap::Spectral_10: os << "Spectral_10"; break;
        case Colormap::Spectral_11: os << "Spectral_11"; break;
        case Colormap::YlGn_3: os << "YlGn_3"; break;
        case Colormap::YlGn_4: os << "YlGn_4"; break;
        case Colormap::YlGn_5: os << "YlGn_5"; break;
        case Colormap::YlGn_6: os << "YlGn_6"; break;
        case Colormap::YlGn_7: os << "YlGn_7"; break;
        case Colormap::YlGn_8: os << "YlGn_8"; break;
        case Colormap::YlGn_9: os << "YlGn_9"; break;
        case Colormap::YlGnBu_3: os << "YlGnBu_3"; break;
        case Colormap::YlGnBu_4: os << "YlGnBu_4"; break;
        case Colormap::YlGnBu_5: os << "YlGnBu_5"; break;
        case Colormap::YlGnBu_6: os << "YlGnBu_6"; break;
        case Colormap::YlGnBu_7: os << "YlGnBu_7"; break;
        case Colormap::YlGnBu_8: os << "YlGnBu_8"; break;
        case Colormap::YlGnBu_9: os << "YlGnBu_9"; break;
        case Colormap::YlOrBr_3: os << "YlOrBr_3"; break;
        case Colormap::YlOrBr_4: os << "YlOrBr_4"; break;
        case Colormap::YlOrBr_5: os << "YlOrBr_5"; break;
        case Colormap::YlOrBr_6: os << "YlOrBr_6"; break;
        case Colormap::YlOrBr_7: os << "YlOrBr_7"; break;
        case Colormap::YlOrBr_8: os << "YlOrBr_8"; break;
        case Colormap::YlOrBr_9: os << "YlOrBr_9"; break;
        case Colormap::YlOrRd_3: os << "YlOrRd_3"; break;
        case Colormap::YlOrRd_4: os << "YlOrRd_4"; break;
        case Colormap::YlOrRd_5: os << "YlOrRd_5"; break;
        case Colormap::YlOrRd_6: os << "YlOrRd_6"; break;
        case Colormap::YlOrRd_7: os << "YlOrRd_7"; break;
        case Colormap::YlOrRd_8: os << "YlOrRd_8"; break;
            // clang-format on
    }
    return os;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits> &operator<<(std::basic_ostream<Elem, Traits> &os,
                                             Category category) {
    switch (category) {
        // clang-format off
        case Category::Diverging: os << "Diverging"; break;
        case Category::Qualitative: os << "Qualitative"; break;
        case Category::Sequential: os << "Sequential"; break;
        case Category::NumberOfColormapCategories: os << "NumberOfColormapCategories"; break;
        case Category::Undefined: os << "Undefined"; break;
            // clang-format on
    }
    return os;
}

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits> &operator<<(std::basic_ostream<Elem, Traits> &os, Family family) {
    switch (family) {
        // clang-format off
        case Family::Accent: os << "Accent"; break;
        case Family::Blues: os << "Blues"; break;
        case Family::BrBG: os << "BrBG"; break;
        case Family::BuGn: os << "BuGn"; break;
        case Family::BuPu: os << "BuPu"; break;
        case Family::Dark2: os << "Dark2"; break;
        case Family::GnBu: os << "GnBu"; break;
        case Family::Greens: os << "Greens"; break;
        case Family::Greys: os << "Greys"; break;
        case Family::OrRd: os << "OrRd"; break;
        case Family::Oranges: os << "Oranges"; break;
        case Family::PRGn: os << "PRGn"; break;
        case Family::Paired: os << "Paired"; break;
        case Family::Pastel1: os << "Pastel1"; break;
        case Family::Pastel2: os << "Pastel2"; break;
        case Family::PiYG: os << "PiYG"; break;
        case Family::PuBu: os << "PuBu"; break;
        case Family::PuBuGn: os << "PuBuGn"; break;
        case Family::PuOr: os << "PuOr"; break;
        case Family::PuRd: os << "PuRd"; break;
        case Family::Purples: os << "Purples"; break;
        case Family::RdBu: os << "RdBu"; break;
        case Family::RdGy: os << "RdGy"; break;
        case Family::RdPu: os << "RdPu"; break;
        case Family::RdYlBu: os << "RdYlBu"; break;
        case Family::RdYlGn: os << "RdYlGn"; break;
        case Family::Reds: os << "Reds"; break;
        case Family::Set1: os << "Set1"; break;
        case Family::Set2: os << "Set2"; break;
        case Family::Set3: os << "Set3"; break;
        case Family::Spectral: os << "Spectral"; break;
        case Family::YlGn: os << "YlGn"; break;
        case Family::YlGnBu: os << "YlGnBu"; break;
        case Family::YlOrBr: os << "YlOrBr"; break;
        case Family::YlOrRd: os << "YlOrRd"; break;
        case Family::NumberOfColormapFamilies: os << "NumberOfColormapFamilies"; break;
        case Family::Undefined: os << "Undefined"; break;
            // clang-format on
    }
    return os;
}

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
