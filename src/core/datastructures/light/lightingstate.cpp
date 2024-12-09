/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/light/lightingstate.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/glmfmt.h>

#include <fmt/format.h>
#include <ostream>

namespace inviwo {

std::string_view enumToStr(ShadingMode sm) {
    switch (sm) {
        case ShadingMode::None:
            return "None";
        case ShadingMode::Ambient:
            return "Ambient";
        case ShadingMode::Diffuse:
            return "Diffuse";
        case ShadingMode::Specular:
            return "Specular";
        case ShadingMode::BlinnPhong:
            return "Blinn-Phong";
        case ShadingMode::Phong:
            return "Phong";
        case ShadingMode::BlinnPhongFront:
            return "Blinn-Phong frontside only";
        case ShadingMode::BlinnPhongBack:
            return "Blinn-Phong backside only";
        case ShadingMode::PhongFront:
            return "Phong frontside only";
        case ShadingMode::PhongBack:
            return "Phong backside only";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid ShadingMode enum value '{}'",
                    static_cast<int>(sm));
}

std::ostream& operator<<(std::ostream& ss, ShadingMode sm) { return ss << enumToStr(sm); }

}  // namespace inviwo
