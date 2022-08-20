/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <modules/plottinggl/rendering/boxselectionrenderer.h>
#include <inviwo/core/datastructures/camera/orthographiccamera.h>

namespace inviwo {

namespace plot {

BoxSelectionRenderer::BoxSelectionRenderer(const BoxSelectionProperty& settings)
    : settings_(settings), lineRenderer_(&lineSettings_) {
    lineSettings_.stippling.mode = StipplingSettingsInterface::Mode::ScreenSpace;
    lineSettings_.stippling.length = 5.f;
}

void BoxSelectionRenderer::render(std::optional<std::array<dvec2, 2>> dragRect, size2_t screenDim) {
    if (dragRect && settings_.getMode() != BoxSelectionSettingsInterface::Mode::None) {
        lineSettings_.lineWidth = settings_.getLineWidth();

        // Hack until we can use a uniform to change line color
        if (color_ != settings_.getLineColor()) {
            color_ = settings_.getLineColor();
            dragRectMesh_ = ColoredMesh(DrawType::Lines, ConnectivityType::Strip,
                                        {{vec3{0.f, 0.f, 0.f}, settings_.lineColor_},
                                         {vec3{1.f, 0.f, 0.f}, settings_.lineColor_},
                                         {vec3{1.f, 1.f, 0.f}, settings_.lineColor_},
                                         {vec3{0.f, 1.f, 0.f}, settings_.lineColor_}},
                                        {0, 1, 2, 3, 0});
        }

        const auto start = vec2((*dragRect)[0]);
        const auto scale = vec2((*dragRect)[1] - (*dragRect)[0]);
        const mat4 m(vec4(scale.x, 0.f, 0.f, 0.f), vec4(0.f, scale.y, 0.f, 0.f),
                     vec4(0.f, 0.f, 1.f, 0.f), vec4(start.x, start.y, 0.f, 1.f));

        dragRectMesh_.setModelMatrix(m);
        const vec2 origin{vec2{screenDim} * 0.5f};
        OrthographicCamera camera_{
            vec3{origin, 2.0f},
            vec3{origin, 0.0f},
            vec3{0.0f, 1.0f, 0.0f},
            0.01f,
            10000.0f,
            static_cast<float>(screenDim.x),
            static_cast<float>(screenDim.x) / static_cast<float>(screenDim.y)};
        lineRenderer_.render(dragRectMesh_, camera_, screenDim, &lineSettings_);
    }
}

}  // namespace plot

}  // namespace inviwo
