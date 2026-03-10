/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

#include <modules/plottinggl/utils/axisrenderer.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/datastructures/camera/plotcamera.h>
#include <modules/basegl/datastructures/meshshadercache.h>
#include <modules/fontrendering/datastructures/fontdata.h>
#include <modules/fontrendering/datastructures/texatlasentry.h>
#include <modules/fontrendering/datastructures/textboundingbox.h>
#include <modules/fontrendering/textrenderer.h>
#include <modules/fontrendering/util/textureatlas.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/rendering/texturequadrenderer.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/shader/shadertype.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/plotting/utils/axisutils.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <type_traits>

#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

namespace inviwo {

namespace plot {

namespace detail {

TickMesh::TickMesh() = default;

Mesh* TickMesh::get(const std::vector<double>& positions, const dvec2& range,
                    TickData::Style style) {
    if (style == TickData::Style::None || positions.empty()) return nullptr;

    range_.check(*this, range);
    style_.check(*this, style);
    positions_.check(*this, positions);
    if (!mesh_) {
        mesh_ = generateTicksMesh(positions_.get(), range_.get(), style_.get());
    }
    return mesh_.get();
}

}  // namespace detail

AxisRendererBase::AxisRendererBase(AxisData data) : data_(std::move(data)) {}

std::shared_ptr<Shader> AxisRendererBase::shaderCache() {
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
Shader& AxisRendererBase::getShader() {
    if (!shader_) {
        shader_ = shaderCache();
    }
    return *shader_;
}

namespace {

glm::mat4 makeLineTransform(const glm::vec3& start, const glm::vec3& stop) {
    glm::vec3 d = stop - start;
    float len = glm::length(d);
    glm::vec3 dir = d / len;

    return glm::translate(glm::mat4(1.0f), start) *
           glm::mat4_cast(glm::rotation(glm::vec3(1, 0, 0), dir)) *
           glm::scale(glm::mat4(1.0f), glm::vec3(len, 1, 1));
}

glm::mat4 makeTickTransform(const glm::vec3& start, const glm::vec3& stop, const glm::vec3& dir) {
    glm::vec3 X = stop - start;
    glm::vec3 Y = dir;
    glm::vec3 Z = glm::cross(glm::normalize(X), Y);

    glm::mat4 M(1.0f);
    M[0] = glm::vec4(X, 0.0f);
    M[1] = glm::vec4(Y, 0.0f);
    M[2] = glm::vec4(Z, 0.0f);
    M[3] = glm::vec4(start, 1.0f);

    return M;
}
struct Transform : public SpatialEntity {
    Transform(const mat4& worldMatrix) : SpatialEntity(mat4{1.0f}, worldMatrix) {}
    virtual SpatialEntity* clone() const override { return new Transform(*this); }
    virtual const Axis* getAxis(size_t) const override { return nullptr; }
};

}  // namespace

void AxisRendererBase::renderAxis(Camera* camera, const vec3& start, const vec3& end,
                                  const vec3& tickDir, const size2_t& outputDims,
                                  bool antialiasing) {

    auto& lineShader = getShader();
    lineShader.activate();
    lineShader.setUniform("screenDim", vec2(outputDims));
    lineShader.setUniform("defaultColor", data_.color);
    lineShader.setUniform("defaultPickID", (axisPickingId_ == std::numeric_limits<size_t>::max()
                                                ? 0u
                                                : static_cast<unsigned int>(axisPickingId_)));
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
                        mat4 transform = mat4(1.0f)) {
        lineShader.setUniform("lineWidth", lineWidth);
        MeshDrawerGL::DrawObject drawer(meshGL, meshGL->getDefaultMeshInfo());
        utilgl::setShaderUniforms(lineShader, Transform{transform}, "geometry");
        lineShader.setUniform("antialiasing", antialiasWidth(lineWidth));
        lineShader.setUniform("roundCaps", caps);
        drawer.draw();
    };

    if (data_.width > 0.0f) {
        const auto* lineMesh = SharedOpenGLResources::getPtr()->lineMesh();
        drawMesh(lineMesh, data_.width, false, makeLineTransform(start, end));
    }
    const auto tick = glm::normalize(tickDir) * (data_.mirrored ? -1.0f : 1.0f);
    if (auto* majorMesh = majorMesh_.get(data_.majorPositions, data_.range, data_.major.style)) {
        drawMesh(majorMesh->getRepresentation<MeshGL>(), data_.major.width, true,
                 makeTickTransform(start, end, tick * data_.major.length));
    }
    if (auto* minorMesh = minorMesh_.get(data_.minorPositions, data_.range, data_.minor.style)) {
        drawMesh(minorMesh->getRepresentation<MeshGL>(), data_.minor.width, true,
                 makeTickTransform(start, end, tick * data_.minor.length));
    }
    lineShader.deactivate();
}

AxisRenderer::AxisRenderer(AxisData data)
    : AxisRendererBase(std::move(data))
    , labels_{[](std::vector<ivec2>& labelPos, util::TextureAtlas&,
                 const std::vector<double>& majorPositions, const AxisData& data, const vec3& start,
                 const vec3& end, const vec3&) {
        const auto tickmarks = plot::getLabelPositions(majorPositions, data, start, end);
        labelPos.resize(tickmarks.size());
        std::ranges::transform(tickmarks, labelPos.begin(), [&](auto&& p) { return p.second; });
    }} {}

void AxisRenderer::render(const size2_t& outputDims, const ivec2& startPos, const ivec2& endPos,
                          bool antialiasing) {
    if (!data_.visible) {
        return;
    }

    const utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    const auto axisDir = glm::normalize(vec2{endPos} - vec2{startPos});
    const auto tickDir = vec3(-axisDir.y, axisDir.x, 0.0f);
    renderAxis(nullptr, vec3{startPos, 0}, vec3{endPos, 0}, tickDir, outputDims, antialiasing);
    renderText(outputDims, startPos, endPos);
}

namespace {

// create a transformation matrix that consideres the anchor position _after_ the bbox rotation
mat4 textTransform(const TextBoundingBox& bbox, const vec2& anchor, float angleRadians) {
    const auto textExtentRotated = [&]() {
        const float c = std::abs(cos(angleRadians));
        const float s = std::abs(sin(angleRadians));
        return vec2{
            bbox.textExtent.x * c + bbox.textExtent.y * s,
            bbox.textExtent.x * s + bbox.textExtent.y * c,
        };
    }();
    const vec2 translation = textExtentRotated * 0.5f * -(anchor);
    const auto textCenter = glm::round(vec2{bbox.textExtent} * 0.5f);

    // translate to anchor pos and apply rotation
    return glm::translate(vec3(translation, 0.0f)) *
           glm::rotate(angleRadians, vec3(0.0f, 0.0f, 1.0f)) *
           glm::translate(vec3(-textCenter + vec2(bbox.glyphsOrigin), 0.f));
}

}  // namespace

void AxisRenderer::renderText(const size2_t& outputDims, const ivec2& startPos,
                              const ivec2& endPos) {
    // axis caption
    if (data_.captionSettings.enabled) {
        const auto& cs = data_.captionSettings;
        const auto& capTex = caption_.getCaption(data_.caption, cs, textRenderer_);

        const auto pos = plot::getAxisCaptionPosition(data_, startPos, endPos);
        const auto posi = glm::ivec2{glm::round(pos)};

        const auto m =
            textTransform(capTex.bbox, (data_.mirrored ? -1.0f : 1.0f) * cs.font.anchorPos,
                          glm::radians(cs.rotation));
        quadRenderer_.render(*capTex.texture, posi, outputDims, m);
    }

    // axis labels
    if (data_.labelSettings.enabled) {
        const auto& ls = data_.labelSettings;
        const auto& pos =
            labels_.getLabelPos(data_, vec3{startPos, 0}, vec3{endPos, 0}, textRenderer_, vec3{1});

        const auto& atlas = labels_.getAtlas(data_, textRenderer_);

        // translate to anchor pos and apply rotation
        std::vector<mat4> transforms;
        const auto& ri = atlas.getRenderInfo();
        std::ranges::transform(
            ri.boundingBoxes, std::back_inserter(transforms), [&](const TextBoundingBox& bb) {
                return textTransform(bb, (data_.mirrored ? -1.0f : 1.0f) * ls.font.anchorPos,
                                     glm::radians(ls.rotation));
            });

        // render axis labels
        quadRenderer_.renderToRect(*atlas.getTexture(), pos, ri.size, ri.texTransform, outputDims,
                                   transforms);
    }
}

std::pair<vec2, vec2> AxisRenderer::boundingRect(const ivec2& startPos, const ivec2& endPos) {
    auto bRect =
        tickBoundingRect(data_, data_.majorPositions, data_.minorPositions, startPos, endPos);

    if (data_.captionSettings.enabled) {
        const auto& cs = data_.captionSettings;
        const auto& captex = caption_.getCaption(data_.caption, cs, textRenderer_);
        const auto texDims(captex.texture->getDimensions());

        const auto m =
            textTransform(captex.bbox, (data_.mirrored ? -1.0f : 1.0f) * cs.font.anchorPos,
                          glm::radians(cs.rotation));

        const auto pos = plot::getAxisCaptionPosition(data_, startPos, endPos);

        const auto pos1 = pos + vec2{m * vec4{0.0f, 0.0f, 0.0f, 1.0f}};
        const auto pos2 = pos + vec2{m * vec4{texDims.x, texDims.y, 0.0f, 1.0f}};

        bRect.first = glm::min(bRect.first, pos1);
        bRect.first = glm::min(bRect.first, pos2);

        bRect.second = glm::max(bRect.second, pos1);
        bRect.second = glm::max(bRect.second, pos2);
    }

    // axis labels
    if (data_.labelSettings.enabled) {
        const auto& ls = data_.labelSettings;
        const auto& positions =
            labels_.getLabelPos(data_, vec3{startPos, 0}, vec3{endPos, 0}, textRenderer_, vec3{1});

        const auto& atlas = labels_.getAtlas(data_, textRenderer_);

        const auto anchor = (data_.mirrored ? -1.0f : 1.0f) * ls.font.anchorPos;
        const auto angle = glm::radians(ls.rotation);

        // render axis labels
        const auto& ri = atlas.getRenderInfo();

        for (auto&& item : util::zip(positions, ri.boundingBoxes)) {
            const auto& pos = item.first();
            const auto& bb = item.second();
            const auto m = textTransform(bb, anchor, angle);

            const auto pos1 = vec2{pos} + vec2{m * vec4{0.0f, 0.0f, 0.0f, 1.0f}};
            const auto pos2 = vec2{pos} + vec2{m * vec4{bb.glyphsExtent, 0.0f, 1.0f}};

            bRect.first = glm::min(bRect.first, pos1);
            bRect.first = glm::min(bRect.first, pos2);

            bRect.second = glm::max(bRect.second, pos1);
            bRect.second = glm::max(bRect.second, pos2);
        }
    }

    return bRect;
}

AxisRenderer3D::AxisRenderer3D(AxisData data)
    : AxisRendererBase(std::move(data))
    , labels_{[](std::vector<vec3>& labelPos, util::TextureAtlas&,
                 const std::vector<double>& majorPositions, const AxisData& data, const vec3& start,
                 const vec3& end, const vec3& tickDirection) {
        const auto tickmarks =
            plot::getLabelPositions3D(majorPositions, data, start, end, tickDirection);
        labelPos.resize(tickmarks.size());
        std::ranges::transform(tickmarks, labelPos.begin(), [](auto& tick) { return tick.second; });
    }} {}

void AxisRenderer3D::render(Camera* camera, const size2_t& outputDims, const vec3& startPos,
                            const vec3& endPos, const vec3& tickDirection, bool antialiasing) {
    if (!data_.visible) return;

    const utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    renderAxis(camera, startPos, endPos, tickDirection, outputDims, antialiasing);
    renderText(camera, outputDims, startPos, endPos, tickDirection);
}

void AxisRenderer3D::renderText(Camera* camera, const size2_t& outputDims, const vec3& startPos,
                                const vec3& endPos, const vec3& tickDirection) {
    // axis caption
    if (data_.captionSettings.enabled) {
        const auto& cs = data_.captionSettings;
        auto captex = caption_.getCaption(data_.caption, cs, textRenderer_);

        // render axis caption centered at the axis using the offset
        const vec2 texDims(captex.texture->getDimensions());
        const auto anchor(cs.font.anchorPos);

        const vec3 pos(plot::getAxisCaptionPosition3D(data_, startPos, endPos, tickDirection));

        const auto transform = glm::rotate(glm::radians(cs.rotation), vec3(0.0f, 0.0f, 1.0f));

        quadRenderer_.renderToRect3D(*camera, *captex.texture, pos,
                                     ivec2(captex.texture->getDimensions()), outputDims, anchor,
                                     transform);
    }

    // axis labels
    if (data_.labelSettings.enabled) {
        const auto& ls = data_.labelSettings;
        const auto& pos =
            labels_.getLabelPos(data_, startPos, endPos, textRenderer_, tickDirection);

        const auto& atlas = labels_.getAtlas(data_, textRenderer_);
        const auto& renderInfo = atlas.getRenderInfo();
        quadRenderer_.renderToRect3D(*camera, *atlas.getTexture(), pos, renderInfo.size,
                                     renderInfo.texTransform, outputDims, ls.font.anchorPos);
    }
}

static_assert(!std::is_copy_constructible_v<detail::AxisCaption>);
static_assert(!std::is_copy_assignable_v<detail::AxisCaption>);
static_assert(std::is_nothrow_move_constructible_v<detail::AxisCaption>);
static_assert(std::is_nothrow_move_assignable_v<detail::AxisCaption>);

static_assert(std::is_nothrow_move_constructible_v<detail::TickMesh>);
static_assert(std::is_nothrow_move_assignable_v<detail::TickMesh>);

static_assert(std::is_nothrow_move_constructible_v<TextRenderer>);
static_assert(std::is_nothrow_move_assignable_v<TextRenderer>);
static_assert(std::is_nothrow_move_constructible_v<TextureQuadRenderer>);
static_assert(std::is_nothrow_move_assignable_v<TextureQuadRenderer>);

static_assert(!std::is_copy_constructible_v<detail::AxisTickLabels<ivec2>>);
static_assert(!std::is_copy_assignable_v<detail::AxisTickLabels<ivec2>>);
static_assert(std::is_nothrow_move_constructible_v<detail::AxisTickLabels<ivec2>>);
static_assert(std::is_nothrow_move_assignable_v<detail::AxisTickLabels<ivec2>>);

static_assert(!std::is_copy_constructible_v<AxisRendererBase>);
static_assert(!std::is_copy_assignable_v<AxisRendererBase>);
static_assert(std::is_move_constructible_v<AxisRendererBase>);
static_assert(std::is_nothrow_move_constructible_v<AxisRendererBase>);
static_assert(std::is_nothrow_move_assignable_v<AxisRendererBase>);

static_assert(!std::is_copy_constructible_v<AxisRenderer>);
static_assert(!std::is_copy_assignable_v<AxisRenderer>);
static_assert(std::is_nothrow_move_constructible_v<AxisRenderer>);
static_assert(std::is_nothrow_move_assignable_v<AxisRenderer>);

static_assert(!std::is_copy_constructible_v<AxisRenderer3D>);
static_assert(!std::is_copy_assignable_v<AxisRenderer3D>);
static_assert(std::is_nothrow_move_constructible_v<AxisRenderer3D>);
static_assert(std::is_nothrow_move_assignable_v<AxisRenderer3D>);

}  // namespace plot

}  // namespace inviwo
