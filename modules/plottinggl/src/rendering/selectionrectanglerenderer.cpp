/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/plottinggl/rendering/selectionrectanglerenderer.h>

namespace inviwo {

SelectionRectangleRenderer::SelectionRectangleRenderer(const LineSettingsInterface* lineSettings): lineRenderer_(lineSettings) {

}

void SelectionRectangleRenderer::render(std::optional<std::array<dvec2, 2>> dragRect,
                                        size2_t screenDim) {
   if (dragRect) {
        
        auto start = vec2((*dragRect)[0]);
        auto end = vec2((*dragRect)[1]);
        auto scale = vec2((*dragRect)[1] - (*dragRect)[0]);
        mat4 m(vec4(scale.x, 0.f, 0.f, 0.f), vec4(0.f, scale.y, 0.f, 0.f), vec4(0.f, 0.f, 1.f, 0.f),
               vec4(start.x, start.y, 0.f, 1.f));
        dragRectMesh_.setModelMatrix(m);
        OrthographicCamera camera_;
        camera_.setFrustum(ivec4(0, screenDim.x, 0, screenDim.y));
        lineRenderer_.render(dragRectMesh_, camera_, screenDim);
    }
}

}  // namespace inviwo
