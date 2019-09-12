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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <vector>
#include <ostream>

#include <inviwo/core/util/colorbrewer-generated.h>

namespace inviwo {
namespace colorbrewer {

class IVW_CORE_API ColorBrewerException : public Exception {
public:
    ColorBrewerException(const std::string &message, ExceptionContext context)
        : Exception(message, context) {}
    virtual ~ColorBrewerException() throw() {}
};

class IVW_CORE_API UnsupportedNumberOfColorsException : public Exception {
public:
    UnsupportedNumberOfColorsException(const std::string &message, ExceptionContext context)
        : Exception(message, context) {}
    virtual ~UnsupportedNumberOfColorsException() throw() {}
};

/**
 * Returns the colormap specified by its family and number of colors containted in the colormap. For
 * reference see http://colorbrewer2.org/. If the colormap is not available for the given number of
 * colors, a ColorBrewerException is thrown.
 **/
IVW_CORE_API const std::vector<dvec4> &getColormap(const Family &family, glm::uint8 numberOfColors);

/**
 * Returns all colormaps of a family. For example, if family Blues is requested, 6 cololormaps will
 * be returned since the family contains 6 levels of detail (Blues_3 - Blues_9).
 **/
IVW_CORE_API std::vector<std::vector<dvec4>> getColormaps(const Family &family);

/**
 * Returns all colormaps of a category. Returns a map with one entry per family storing all
 * colormaps for that family.
 **/
IVW_CORE_API std::map<Family, std::vector<std::vector<dvec4>>> getColormaps(
    const Category &category);

/**
 * Returns all colormaps of a category with given number of colors. If a colormap is not available
 * for the given number of colors, it is omitted. If none of the colormaps are available for the
 * whole category and the given number of colors, a ColorBrewerException is thrown.
 **/
IVW_CORE_API std::map<Family, std::vector<dvec4>> getColormaps(const Category &category,
                                                               glm::uint8 numberOfColors);

/**
 * Returns a transfer function for the given parameters.
 *
 * @param category according to ColorBrewer2
 * @param family color scheme name
 * @param discrete will make each color constant instead of linearly varying inbetween colors.
 * @param divergenceMidpoint in [0 1]. Only used when category is Diverging
 **/
IVW_CORE_API TransferFunction getTransferFunction(const Category &category, const Family &family,
                                                  glm::uint8 nColors, bool discrete,
                                                  double divergenceMidPoint);

}  // namespace colorbrewer
}  // namespace inviwo
