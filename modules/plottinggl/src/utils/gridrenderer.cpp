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

#include <modules/plottinggl/utils/gridrenderer.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/camera/plotcamera.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/sharedopenglresources.h>

#include <cmath>
#include <algorithm>
#include <ranges>

namespace inviwo::plot {

namespace {

std::unique_ptr<Mesh> generateGridMesh(const std::vector<double>& horizontalPositions,
                                       const std::vector<double>& verticalPositions,
                                       PlotAxis plotAxis, dvec2 majorRange, dvec2 minorRange) {
    const size_t numLines = [&]() {
        switch (plotAxis) {
            case PlotAxis::Major:
                return horizontalPositions.size();
            case PlotAxis::Minor:
                return verticalPositions.size();
            case PlotAxis::Both:
                return horizontalPositions.size() + verticalPositions.size();
            case PlotAxis::None:
            default:
                return 0uz;
        }
    }();

    if (numLines == 0) {
        return nullptr;
    }

    std::vector<vec2> vertices;
    vertices.reserve(numLines * 2);

    auto createLines = [&vertices](const std::vector<double>& positions, vec2 axisDir, vec2 lineDir,
                                   dvec2 range) {
        const auto axisLength = range.y - range.x;
        const vec2 scaling{axisDir / axisLength};

        for (auto&& [position, i] : std::views::zip(positions, std::views::iota(0uz))) {
            const vec2 p{scaling * static_cast<float>(position - range.x)};
            vertices.emplace_back(p);
            vertices.emplace_back(p + lineDir);
        }
    };
    if (plotAxis == PlotAxis::Major || plotAxis == PlotAxis::Both) {
        createLines(horizontalPositions, vec2{1.0f, 0.0f}, vec2{0.0f, 1.0f}, majorRange);
    }
    if (plotAxis == PlotAxis::Minor || plotAxis == PlotAxis::Both) {
        createLines(verticalPositions, vec2{0.0f, 1.0f}, vec2{1.0f, 0.0f}, minorRange);
    }

    auto mesh = std::make_unique<Mesh>(DrawType::Lines, ConnectivityType::None);
    mesh->addBuffer(BufferType::PositionAttrib, util::makeBuffer(std::move(vertices)));

    mesh->addIndices(
        Mesh::MeshInfo{.dt = DrawType::Lines, .ct = ConnectivityType::None},
        inviwo::util::makeIndexBuffer(
            std::views::iota(std::uint32_t{0}, static_cast<std::uint32_t>(numLines * 2)) |
            std::ranges::to<std::vector>()));

    return mesh;
}

mat4 makeLineTransform(const vec3& start, const vec3& stop) {
    const vec3 d = stop - start;
    const float len = glm::length(d);
    const vec3 dir = d / len;

    return glm::translate(mat4(1.0f), start) * glm::mat4_cast(rotation(vec3(1, 0, 0), dir)) *
           glm::scale(mat4(1.0f), vec3(len, 1, 1));
}

mat4 makeGridTransform(vec3 origin, vec3 right, vec3 up) {
    const auto shearX =
        std::tan(std::acos(glm::dot(glm::normalize(right - origin), vec3{1.0f, 0.0f, 0.0f})));
    const auto shearY =
        std::tan(std::acos(glm::dot(glm::normalize(up - origin), vec3{0.0f, 1.0f, 0.0f})));

    const mat4 shear{vec4{1.0f + shearX * shearY, shearY, 0.0f, 0.0f},
                     vec4{shearX, 1.0f, 0.0f, 0.0f}, vec4{0.0f, 0.0f, 1.0f, 0.0f},
                     vec4{0.0f, 0.0f, 0.0f, 1.0f}};

    return glm::translate(mat4{1.0f}, origin) * shear *
           glm::scale(mat4{1.0f},
                      vec3{glm::length(right - origin), glm::length(up - origin), 1.0f});
}

struct Transform : public SpatialEntity {
    explicit Transform(const mat4& worldMatrix) : SpatialEntity(mat4{1.0f}, worldMatrix) {}
    virtual SpatialEntity* clone() const override { return new Transform(*this); }
    virtual const Axis* getAxis(size_t) const override { return nullptr; }
};

}  // namespace

namespace detail {

GridMesh::GridMesh() = default;

Mesh* GridMesh::get(const std::vector<double>& horizontalPositions,
                    const std::vector<double>& verticalPositions, PlotAxis plotAxis,
                    dvec2 majorRange, dvec2 minorRange) {
    if (horizontalPositions.empty() && verticalPositions.empty()) return nullptr;

    majorRange_.check(*this, majorRange);
    minorRange_.check(*this, minorRange);
    plotAxis_.check(*this, plotAxis);
    horizontal_.check(*this, horizontalPositions);
    vertical_.check(*this, verticalPositions);
    if (!mesh_) {
        mesh_ = generateGridMesh(horizontal_.get(), vertical_.get(), plotAxis, majorRange_.get(),
                                 minorRange_.get());
    }
    return mesh_.get();
}

}  // namespace detail

GridRenderer::GridRenderer(GridData data) : data_(std::move(data)) {}

void GridRenderer::render(const size2_t& outputDims, const ivec2& origin, const ivec2& right,
                          const ivec2& up, bool antialiasing) {
    if (!data_.visible) {
        return;
    }

    const utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    renderGrid(nullptr, vec3{origin, 0.0f}, vec3{right, 0.0f}, vec3{up, 0.0f}, outputDims,
               antialiasing);
}

void GridRenderer::renderGrid(Camera* camera, const vec3& origin, const vec3& right, const vec3& up,
                              const size2_t& outputDims, bool antialiasing) {
    auto& lineShader = getShader();
    lineShader.activate();
    lineShader.setUniform("screenDim", vec2(outputDims));
    lineShader.setUniform("defaultColor", data_.color);
    lineShader.setUniform("defaultPickID", 0u);

    if (camera) {
        utilgl::setShaderUniforms(lineShader, *camera, "camera");
    } else {
        const auto dim = vec2{outputDims};
        const PlotCamera cam{vec3{dim / 2.0f, 1.0f},
                             vec3{dim / 2.0f, 0.0f},
                             vec3(0.0f, 1.0f, 0.0f),
                             0.001f,
                             100.0f,
                             dim.x / dim.y,
                             dim};
        utilgl::setShaderUniforms(lineShader, cam, "camera");
    }

    // returns thickness of antialiased edge based on the global antialiasing flag
    // and whether the line is 1px wide
    const auto antialiasWidth = [&](float lineWidth) {
        if (!antialiasing || (std::abs(lineWidth - 1.0f) < 0.01f)) {
            return 0.0f;
        } else {
            return 0.5f;
        }
    };

    auto drawMesh = [&](const MeshGL* meshGL, float lineWidth, bool caps,
                        const mat4& transform = mat4{1.0f}) {
        lineShader.setUniform("lineWidth", lineWidth);
        MeshDrawerGL::DrawObject drawer(meshGL, meshGL->getDefaultMeshInfo());
        utilgl::setShaderUniforms(lineShader, Transform{transform}, "geometry");
        lineShader.setUniform("antialiasing", antialiasWidth(lineWidth));
        lineShader.setUniform("roundCaps", caps);
        drawer.draw();
    };

    if (data_.width > 0.0f) {
        if (const auto* gridMesh = gridMesh_.get(data_.horizontalPositions, data_.verticalPositions,
                                                 data_.axis, data_.majorRange, data_.minorRange)) {
            drawMesh(gridMesh->getRepresentation<MeshGL>(), data_.width, true,
                     makeGridTransform(origin, right, up));
        }
    }

    lineShader.deactivate();
}

std::shared_ptr<Shader> GridRenderer::shaderCache() {
    static std::weak_ptr<Shader> cache_;

    if (auto cache = cache_.lock()) {
        return cache;
    } else {
        cache = std::make_shared<Shader>("linerenderer.vert", "linerenderer.geom",
                                         "linerenderer.frag", Shader::Build::No);

        cache->getGeometryShaderObject()->addShaderDefine("ENABLE_ADJACENCY", "0");
        cache->getVertexShaderObject()->clearInDeclarations();
        cache->getVertexShaderObject()->addInDeclaration(
            "in_Position", static_cast<int>(BufferType::PositionAttrib), "vec3");
        cache->build();
        cache_ = cache;
        return cache;
    }
}

Shader& GridRenderer::getShader() {
    if (!shader_) {
        shader_ = shaderCache();
    }
    return *shader_;
}

}  // namespace inviwo::plot
