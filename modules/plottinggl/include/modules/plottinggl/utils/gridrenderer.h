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

#include <modules/plottinggl/plottingglmoduledefine.h>

#include <modules/opengl/shader/shader.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/plotting/datastructures/griddata.h>
#include <modules/plottinggl/utils/axisrenderer.h>

namespace inviwo {

class Camera;
class Mesh;

namespace plot {

namespace detail {

struct IVW_MODULE_PLOTTINGGL_API GridMesh {
    GridMesh();
    Mesh* get(const std::vector<double>& horizontalPositions,
              const std::vector<double>& verticalPositions, PlotAxis plotAxis, dvec2 majorRange,
              dvec2 minorRange);

private:
    std::unique_ptr<Mesh> mesh_;

    using MP = MemPtr<GridMesh, std::unique_ptr<Mesh>, &GridMesh::mesh_>;

    Guard<dvec2, MP> majorRange_;
    Guard<dvec2, MP> minorRange_;
    Guard<PlotAxis, MP> plotAxis_;
    Guard<std::vector<double>, MP> horizontal_;
    Guard<std::vector<double>, MP> vertical_;
};

}  // namespace detail

class IVW_MODULE_PLOTTINGGL_API GridRenderer {
public:
    explicit GridRenderer(GridData data = {});
    GridRenderer(const GridRenderer& rhs) = delete;
    GridRenderer(GridRenderer&& rhs) = default;
    GridRenderer& operator=(const GridRenderer& rhs) = delete;
    GridRenderer& operator=(GridRenderer&& rhs) = default;
    ~GridRenderer() = default;

    GridData& getGridData() { return data_; }
    const GridData& getGridData() const { return data_; }

    /**
     * Render the grid into the plot area of the current framebuffer spanned from pixel position
     * @p origin to @p right and @p origin to @p up.
     * @param outputDims   Dimensions of the currently bound output framebuffer
     * @param origin       Lower left corner of the plot area in 2D screen coordinates
     *                     [0, outputDims)
     * @param right        End point of the major axis in 2D screen coordinates [0, outputDims)
     * @param up           End point of the minor axis in 2D screen coordinates [0, outputDims)
     * @param antialiasing If true, lines will be rendered using an exponential alpha fall-off at
     *                     the edges and alpha blending
     */
    void render(const size2_t& outputDims, const ivec2& origin, const ivec2& right, const ivec2& up,
                bool antialiasing = true);

private:
    void renderGrid(Camera* camera, const vec3& origin, const vec3& right, const vec3& up,
                    const size2_t& outputDims, bool antialiasing);

    GridData data_;

    detail::GridMesh gridMesh_;

    Shader& getShader();

    static std::shared_ptr<Shader> shaderCache();
    std::shared_ptr<Shader> shader_;
};

}  // namespace plot

}  // namespace inviwo
