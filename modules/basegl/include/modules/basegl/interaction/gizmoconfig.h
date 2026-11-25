/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/coordinatetransformer.h>

#include <optional>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace inviwo {

struct IVW_MODULE_BASEGL_API GizmoConfig {
    int widgetSize = defaultWidgetSize;
    float lineWidth = defaultLineWidth;
    float pickingLineWidth = defaultPickingLineWidth;
    bool trackballVisible = defaultTrackballVisible;

    glm::vec3 xAxisColor = defaultXAxisColor;
    glm::vec3 yAxisColor = defaultYAxisColor;
    glm::vec3 zAxisColor = defaultZAxisColor;
    glm::vec4 neutralColor = defaultNeutralColor;
    float alpha = defaultAlpha;

    static constexpr int defaultWidgetSize = 150;
    static constexpr float defaultLineWidth = 4.0f;
    static constexpr float defaultPickingLineWidth = 6.0f;
    static constexpr bool defaultTrackballVisible = true;
    static constexpr glm::vec3 defaultXAxisColor{1.0f, 0.09f, 0.12f};
    static constexpr glm::vec3 defaultYAxisColor{0.23f, 1.0f, 0.329};
    static constexpr glm::vec3 defaultZAxisColor{0.173f, 0.561f, 1.0f};
    static constexpr glm::vec4 defaultNeutralColor{0.38f, 0.39f, 0.427f, 0.500f};
    static constexpr float defaultAlpha = 0.7f;

    glm::vec3 getAxisColor(size_t i) const;
};

}  // namespace inviwo
