/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
 
#ifndef IWW_COLORBREWER_H
#define IWW_COLORBREWER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <vector>

namespace inviwo{
namespace colorbrewer {

    class ColorBrewerException : public Exception {
    public:
        ColorBrewerException(const std::string &message) : Exception(message, ExceptionContext()) {}
        virtual ~ColorBrewerException() throw() {}
    };

IVW_CORE_API std::vector<dvec4> Accent(int dataClasses);
IVW_CORE_API std::vector<dvec4> Blues(int dataClasses);
IVW_CORE_API std::vector<dvec4> BrBG(int dataClasses);
IVW_CORE_API std::vector<dvec4> BuGn(int dataClasses);
IVW_CORE_API std::vector<dvec4> BuPu(int dataClasses);
IVW_CORE_API std::vector<dvec4> Dark2(int dataClasses);
IVW_CORE_API std::vector<dvec4> GnBu(int dataClasses);
IVW_CORE_API std::vector<dvec4> Greens(int dataClasses);
IVW_CORE_API std::vector<dvec4> Greys(int dataClasses);
IVW_CORE_API std::vector<dvec4> OrRd(int dataClasses);
IVW_CORE_API std::vector<dvec4> Oranges(int dataClasses);
IVW_CORE_API std::vector<dvec4> PRGn(int dataClasses);
IVW_CORE_API std::vector<dvec4> Paired(int dataClasses);
IVW_CORE_API std::vector<dvec4> Pastel1(int dataClasses);
IVW_CORE_API std::vector<dvec4> Pastel2(int dataClasses);
IVW_CORE_API std::vector<dvec4> PiYG(int dataClasses);
IVW_CORE_API std::vector<dvec4> PuBu(int dataClasses);
IVW_CORE_API std::vector<dvec4> PuBuGn(int dataClasses);
IVW_CORE_API std::vector<dvec4> PuOr(int dataClasses);
IVW_CORE_API std::vector<dvec4> PuRd(int dataClasses);
IVW_CORE_API std::vector<dvec4> Purples(int dataClasses);
IVW_CORE_API std::vector<dvec4> RdBu(int dataClasses);
IVW_CORE_API std::vector<dvec4> RdGy(int dataClasses);
IVW_CORE_API std::vector<dvec4> RdPu(int dataClasses);
IVW_CORE_API std::vector<dvec4> RdYlBu(int dataClasses);
IVW_CORE_API std::vector<dvec4> RdYlGn(int dataClasses);
IVW_CORE_API std::vector<dvec4> Reds(int dataClasses);
IVW_CORE_API std::vector<dvec4> Set1(int dataClasses);
IVW_CORE_API std::vector<dvec4> Set2(int dataClasses);
IVW_CORE_API std::vector<dvec4> Set3(int dataClasses);
IVW_CORE_API std::vector<dvec4> Spectral(int dataClasses);
IVW_CORE_API std::vector<dvec4> YlGn(int dataClasses);
IVW_CORE_API std::vector<dvec4> YlGnBu(int dataClasses);
IVW_CORE_API std::vector<dvec4> YlOrBr(int dataClasses);
IVW_CORE_API std::vector<dvec4> YlOrRd(int dataClasses);

} // namespace
}
#endif // COLORBREWER_H



