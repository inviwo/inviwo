/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2026 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>

#include <modules/basegl/datastructures/stipplingdata.h>
#include <inviwo/core/datastructures/transferfunction.h>

namespace inviwo {
/**
 * @brief Settings for line rendering
 */
struct IVW_MODULE_BASEGL_API LineData {
    float lineWidth = 1.f;
    float antialiasing = 0.5f;
    float miterLimit = 0.8f;
    bool roundCaps = true;
    bool pseudoLighting = false;
    bool roundDepthProfile = false;
    bool overrideColor = false;
    bool overrideAlpha = false;
    bool useMetaColor = false;
    StipplingData stippling;
    vec4 defaultColor = vec4{1.0f, 0.7f, 0.2f, 1.0f};
    vec3 overrideColorValue = vec3{0.7f, 0.7f, 0.7f};
    float overrideAlphaValue = 1.0f;
    TransferFunction metaColor{
        {{.pos = 0.0, .color = vec4{0.0f, 0.0f, 0.0f, 0.0f}}, 
         {.pos = 1.0, .color = vec4{1.0f, 1.0f, 1.0f, 1.0f}}}};

    bool operator==(const LineData&) const = default;
};

}  // namespace inviwo
