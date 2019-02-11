/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/datatraits.h>

namespace inviwo {

uvec3 util::getDataFormatColor(NumericType t, size_t comp, size_t size) {
    uvec3 color;
    switch (t) {
        case NumericType::Float:
            color.r = 30;
            break;
        case NumericType::SignedInteger:
            color.r = 60;
            break;
        case NumericType::UnsignedInteger:
            color.r = 90;
            break;
        default:
            color.r = 0;
            break;
    }

    switch (comp) {
        case 1:
            color.g = 30;
            break;
        case 2:
            color.g = 60;
            break;
        case 3:
            color.g = 90;
            break;
        case 4:
            color.g = 120;
            break;
        default:
            color.g = 0;
            break;
    }
    switch (size) {
        case 1:
            color.b = 30;
            break;
        case 2:
            color.b = 60;
            break;
        case 3:
            color.b = 90;
            break;
        case 4:
            color.b = 120;
            break;
        default:
            color.b = 0;
            break;
    }
    return color;
}

std::string util::appendIfNotEmpty(const std::string& a, const std::string& b) {
    if (a.empty() || b.empty()) {
        return a;
    } else {
        return a + b;
    }
}

}  // namespace inviwo
