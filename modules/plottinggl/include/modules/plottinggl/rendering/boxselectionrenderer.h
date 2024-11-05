/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>  // for IVW_MODULE_PLOTTINGGL_API

#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for ConnectivityType, Connecti...
#include <inviwo/core/datastructures/geometry/typedmesh.h>     // for TypedMesh<>::Vertex, Color...
#include <inviwo/core/util/glmvec.h>                           // for vec4, vec3, dvec2, size2_t
#include <modules/basegl/datastructures/linesettings.h>        // for LineSettings
#include <modules/basegl/rendering/linerenderer.h>             // for LineRenderer

#include <array>     // for array
#include <optional>  // for optional

namespace inviwo {

namespace plot {
class BoxSelectionProperty;

/**
 * \brief Renders a 2D rectangle in screen space.
 * Use in combination with BoxSelectionInteractionHandler.
 * @see BoxSelectionInteractionHandler
 */
class IVW_MODULE_PLOTTINGGL_API BoxSelectionRenderer {
public:
    explicit BoxSelectionRenderer(const BoxSelectionProperty& settings);
    virtual ~BoxSelectionRenderer() = default;
    /*
     * \brief Draw rectangle if dragRect exists.
     *
     * @param dragRect start/end pixel coordinates
     * @param screenDim size of render surface
     */
    void render(std::optional<std::array<dvec2, 2>> dragRect, size2_t screenDim);

protected:
    const BoxSelectionProperty& settings_;
    LineSettings lineSettings_;
    algorithm::LineRenderer lineRenderer_;

    using PositionMesh = TypedMesh<buffertraits::PositionsBuffer2D>;
    PositionMesh dragRectMesh_{DrawType::Lines,
                               ConnectivityType::Loop,
                               {
                                   {vec2{0.f, 0.f}},
                                   {vec2{1.f, 0.f}},
                                   {vec2{1.f, 1.f}},
                                   {vec2{0.f, 1.f}},
                               },
                               {0, 1, 2, 3}};
};

}  // namespace plot

}  // namespace inviwo
