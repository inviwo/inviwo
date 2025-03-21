/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/geometrytype.h>      // for BufferType, BufferType...
#include <inviwo/core/datastructures/geometry/mesh.h>              // for Mesh
#include <inviwo/core/util/glmmat.h>                               // for mat4
#include <inviwo/core/util/glmvec.h>                               // for vec3, vec2, vec4, dvec2
#include <inviwo/core/util/zip.h>                                  // for zip, zipIterator, zipper
#include <modules/basegl/datastructures/meshshadercache.h>         // for MeshShaderCache::Requi...
#include <modules/fontrendering/datastructures/fontsettings.h>     // for FontSettings
#include <modules/fontrendering/datastructures/texatlasentry.h>    // for TexAtlasRenderInfo
#include <modules/fontrendering/datastructures/textboundingbox.h>  // for TextBoundingBox
#include <modules/fontrendering/textrenderer.h>                    // for TextTextureObject
#include <modules/fontrendering/util/textureatlas.h>               // for TextureAtlas
#include <modules/opengl/geometry/meshgl.h>                        // for MeshGL
#include <modules/opengl/inviwoopengl.h>                           // for GL_ONE, GL_ONE_MINUS_S...
#include <modules/opengl/openglutils.h>                            // for BlendModeState
#include <modules/opengl/rendering/meshdrawergl.h>                 // for MeshDrawerGL::DrawObject
#include <modules/opengl/rendering/texturequadrenderer.h>          // for TextureQuadRenderer
#include <modules/opengl/shader/shader.h>                          // for Shader
#include <modules/opengl/shader/shaderobject.h>                    // for ShaderObject
#include <modules/opengl/shader/shadertype.h>                      // for ShaderType, ShaderType...
#include <modules/opengl/shader/shaderutils.h>                     // for setShaderUniforms
#include <modules/opengl/texture/texture2d.h>                      // IWYU pragma: keep
#include <modules/plotting/datastructures/axissettings.h>          // for AxisSettings
#include <modules/plotting/datastructures/majortickdata.h>         // for MajorTickData
#include <modules/plotting/datastructures/majorticksettings.h>     // for MajorTickSettings
#include <modules/plotting/datastructures/minortickdata.h>         // for MinorTickData
#include <modules/plotting/datastructures/minorticksettings.h>     // for MinorTickSettings, ope...
#include <modules/plotting/datastructures/plottextsettings.h>      // for PlotTextSettings
#include <modules/plotting/utils/axisutils.h>                      // for generateTicksMesh, get...

#include <algorithm>    // for transform
#include <cmath>        // for round, abs
#include <cstdlib>      // for abs
#include <functional>   // for reference_wrapper, __base
#include <iterator>     // for back_insert_iterator
#include <type_traits>  // for is_nothrow_move_assign...

#include <glm/common.hpp>                // for round, max, min
#include <glm/ext/matrix_transform.hpp>  // for rotate, translate
#include <glm/fwd.hpp>                   // for ivec2
#include <glm/geometric.hpp>             // for normalize
#include <glm/gtc/constants.hpp>         // for half_pi
#include <glm/gtx/transform.hpp>         // for rotate, translate
#include <glm/mat4x4.hpp>                // for operator*, mat
#include <glm/trigonometric.hpp>         // for radians

namespace inviwo {

namespace plot {

namespace detail {

AxisMeshes::AxisMeshes() = default;

Mesh* AxisMeshes::getAxis(const AxisSettings& settings, const vec3& start, const vec3& end,
                          size_t pickId) {
    startPos_.check(*this, start);
    endPos_.check(*this, end);
    color_.check(*this, settings.getColor());
    pickId_.check(*this, pickId);

    if (!axisMesh_) {
        axisMesh_ = plot::generateAxisMesh3D(startPos_.get(), endPos_.get(), color_, pickId_);
    }
    return axisMesh_.get();
}

Mesh* AxisMeshes::getMajor(const AxisSettings& settings, const vec3& start, const vec3& end,
                           const vec3& tickDirection) {
    startPos_.check(*this, start);
    endPos_.check(*this, end);
    range_.check(*this, settings.getRange());
    flip_.check(*this, settings.getMirrored());
    major_.check(*this, settings.getMajorTicks());
    tickDirection_.check(*this, tickDirection);
    scalingFactor_.check(*this, settings.getScalingFactor());
    if (!majorMesh_) {
        const auto tickPositions = getMajorTickPositions(major_, range_);
        majorMesh_ =
            generateTicksMesh(tickPositions, range_, startPos_.get(), endPos_.get(), tickDirection_,
                              major_.get().getTickLength() * scalingFactor_.get(),
                              major_.get().getStyle(), major_.get().getColor(), flip_);
    }
    return majorMesh_.get();
}

Mesh* AxisMeshes::getMinor(const AxisSettings& settings, const vec3& start, const vec3& end,
                           const vec3& tickDirection) {
    startPos_.check(*this, start);
    endPos_.check(*this, end);
    range_.check(*this, settings.getRange());
    flip_.check(*this, settings.getMirrored());
    major_.check(*this, settings.getMajorTicks());
    minor_.check(*this, settings.getMinorTicks());
    tickDirection_.check(*this, tickDirection);
    scalingFactor_.check(*this, settings.getScalingFactor());
    if (!minorMesh_) {
        const auto tickPositions = getMinorTickPositions(minor_, major_, range_);
        minorMesh_ =
            generateTicksMesh(tickPositions, range_, startPos_.get(), endPos_.get(), tickDirection,
                              minor_.get().getTickLength() * scalingFactor_.get(),
                              minor_.get().getStyle(), minor_.get().getColor(), flip_);
    }
    return minorMesh_.get();
}

}  // namespace detail

std::vector<std::pair<ShaderType, std::string>> AxisRendererBase::shaderItems_ = {
    {ShaderType::Vertex, "linerenderer.vert"},
    {ShaderType::Geometry, "linerenderer.geom"},
    {ShaderType::Fragment, "linerenderer.frag"}};

std::vector<MeshShaderCache::Requirement> AxisRendererBase::shaderRequirements_ = {
    {BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
    {BufferType::ColorAttrib, MeshShaderCache::Mandatory, "vec4"},
    {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"}};

AxisRendererBase::AxisRendererBase(const AxisSettings& settings)
    : settings_(settings), shaders_{getShaders()} {}

std::shared_ptr<MeshShaderCache> AxisRendererBase::getShaders() {
    static std::weak_ptr<MeshShaderCache> cache_;

    if (auto cache = cache_.lock()) {
        return cache;
    } else {
        cache = std::make_shared<MeshShaderCache>(
            AxisRendererBase::shaderItems_, AxisRendererBase::shaderRequirements_,
            [&](Shader& shader) -> void {
                shader.getGeometryShaderObject()->addShaderDefine("ENABLE_ADJACENCY", "0");
                shader.build();
            });
        cache_ = cache;
        return cache;
    }
}
void AxisRendererBase::renderAxis(Camera* camera, const vec3& start, const vec3& end,
                                  const vec3& tickdir, const size2_t& outputDims,
                                  bool antialiasing) {
    auto axisMesh = meshes_.getAxis(settings_, start, end, axisPickingId_);
    if (!axisMesh) return;

    auto& lineShader = shaders_->getShader(*axisMesh);
    lineShader.activate();
    lineShader.setUniform("screenDim", vec2(outputDims));
    if (camera) {
        utilgl::setShaderUniforms(lineShader, *camera, "camera");
    } else {
        const auto m = mat4(1.0f);
        lineShader.setUniform("camera.worldToView", m);
        lineShader.setUniform("camera.viewToWorld", m);
        lineShader.setUniform("camera.worldToClip", m);
        lineShader.setUniform("camera.viewToClip", m);
        lineShader.setUniform("camera.clipToView", m);
        lineShader.setUniform("camera.clipToWorld", m);
        lineShader.setUniform("camera.position", vec3(0.0f));
        lineShader.setUniform("camera.nearPlane", 0.0f);
        lineShader.setUniform("camera.farPlane", 1.0f);
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

    // compute matrix to transform meshes from screen coords to clip space
    const auto m = !camera ? mat4(vec4(2.0f / outputDims.x, 0.0f, 0.0f, 0.0f),
                                  vec4(0.0f, 2.0f / outputDims.y, 0.0f, 0.0f),
                                  vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(-1.0f, -1.0f, 0.0f, 1.0f))
                           : mat4{1};

    auto drawMesh = [&](Mesh* mesh, float lineWidth, bool caps) {
        if (!mesh) return;
        mesh->setWorldMatrix(m);
        auto meshgl = mesh->getRepresentation<MeshGL>();
        lineShader.setUniform("lineWidth", lineWidth);
        MeshDrawerGL::DrawObject drawer(meshgl, mesh->getDefaultMeshInfo());
        utilgl::setShaderUniforms(lineShader, *mesh, "geometry");
        lineShader.setUniform("antialiasing", antialiasWidth(lineWidth));
        lineShader.setUniform("roundCaps", caps);
        drawer.draw();
    };

    drawMesh(axisMesh, settings_.get().getWidth(), false);
    auto majorMesh = meshes_.getMajor(settings_, start, end, tickdir);
    drawMesh(majorMesh, settings_.get().getMajorTicks().getTickWidth(), true);
    auto minorMesh = meshes_.getMinor(settings_, start, end, tickdir);
    drawMesh(minorMesh, settings_.get().getMinorTicks().getTickWidth(), true);

    lineShader.deactivate();
}

AxisRenderer::AxisRenderer(const AxisSettings& settings)
    : AxisRendererBase(settings)
    , labels_{[](Labels::LabelPos& labelPos, util::TextureAtlas&, const AxisSettings& settings,
                 const vec3& start, const vec3& end, const vec3&) {
        const auto tickmarks = plot::getLabelPositions(settings, start, end);
        labelPos.resize(tickmarks.size());
        std::transform(tickmarks.begin(), tickmarks.end(), labelPos.begin(),
                       [&](auto&& p) { return p.second; });
    }} {}

void AxisRenderer::render(const size2_t& outputDims, const ivec2& startPos, const ivec2& endPos,
                          bool antialiasing) {
    if (!settings_.get().getAxisVisible()) {
        return;
    }

    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    const auto axisDir = glm::normalize(vec2{endPos} - vec2{startPos});
    const auto tickDir = vec3(-axisDir.y, axisDir.x, 0.0f);
    renderAxis(nullptr, vec3{startPos, 0}, vec3{endPos, 0}, tickDir, outputDims, antialiasing);
    renderText(outputDims, startPos, endPos);
}

namespace detail {

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

}  // namespace detail

void AxisRenderer::renderText(const size2_t& outputDims, const ivec2& startPos,
                              const ivec2& endPos) {
    // axis caption
    if (const auto& captionSettings = settings_.get().getCaptionSettings()) {
        auto& captex =
            caption_.getCaption(settings_.get().getCaption(), captionSettings, textRenderer_);

        const auto pos = plot::getAxisCaptionPosition(settings_, startPos, endPos);
        const auto posi = glm::ivec2{glm::round(pos)};

        const auto m = detail::textTransform(captex.bbox, captionSettings.getFont().getAnchorPos(),
                                             glm::radians(captionSettings.getRotation()));
        quadRenderer_.render(*captex.texture, posi, outputDims, m);
    }

    // axis labels
    if (const auto& labels = settings_.get().getLabelSettings()) {
        auto& pos = labels_.getLabelPos(settings_, vec3{startPos, 0}, vec3{endPos, 0},
                                        textRenderer_, vec3{1});

        auto& atlas =
            labels_.getAtlas(settings_, vec3{startPos, 0}, vec3{endPos, 0}, textRenderer_);

        const auto anchor = labels.getFont().getAnchorPos();
        const auto angle = glm::radians(labels.getRotation());

        // translate to anchor pos and apply rotation
        std::vector<mat4> transforms;
        const auto& ri = atlas.getRenderInfo();
        std::transform(
            ri.boundingBoxes.begin(), ri.boundingBoxes.end(), std::back_inserter(transforms),
            [&](const TextBoundingBox& bb) { return detail::textTransform(bb, anchor, angle); });

        // render axis labels
        quadRenderer_.renderToRect(*atlas.getTexture(), pos, ri.size, ri.texTransform, outputDims,
                                   transforms);
    }
}

std::pair<vec2, vec2> AxisRenderer::boundingRect(const ivec2& startPos, const ivec2& endPos) {

    auto bRect = tickBoundingRect(settings_, startPos, endPos);

    if (const auto& captionSettings = settings_.get().getCaptionSettings()) {
        auto& captex =
            caption_.getCaption(settings_.get().getCaption(), captionSettings, textRenderer_);
        const auto texDims(captex.texture->getDimensions());

        const auto m = detail::textTransform(captex.bbox, captionSettings.getFont().getAnchorPos(),
                                             glm::radians(captionSettings.getRotation()));

        const auto pos = plot::getAxisCaptionPosition(settings_, startPos, endPos);

        const auto pos1 = pos + vec2{m * vec4{0.0f, 0.0f, 0.0f, 1.0f}};
        const auto pos2 = pos + vec2{m * vec4{texDims.x, texDims.y, 0.0f, 1.0f}};

        bRect.first = glm::min(bRect.first, pos1);
        bRect.first = glm::min(bRect.first, pos2);

        bRect.second = glm::max(bRect.second, pos1);
        bRect.second = glm::max(bRect.second, pos2);
    }

    // axis labels
    if (const auto& labels = settings_.get().getLabelSettings()) {
        auto& positions = labels_.getLabelPos(settings_, vec3{startPos, 0}, vec3{endPos, 0},
                                              textRenderer_, vec3{1});

        auto& atlas =
            labels_.getAtlas(settings_, vec3{startPos, 0}, vec3{endPos, 0}, textRenderer_);

        const auto anchor = labels.getFont().getAnchorPos();
        const auto angle = glm::radians(labels.getRotation());

        // render axis labels
        const auto& ri = atlas.getRenderInfo();

        for (auto&& item : util::zip(positions, ri.boundingBoxes)) {
            const auto& pos = item.first();
            const auto& bb = item.second();
            const auto m = detail::textTransform(bb, anchor, angle);

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

AxisRenderer3D::AxisRenderer3D(const AxisSettings& settings)
    : AxisRendererBase(settings)
    , labels_{[](Labels::LabelPos& labelPos, util::TextureAtlas&, const AxisSettings& settings,
                 const vec3& start, const vec3& end, const vec3& tickDirection) {
        const auto tickmarks = plot::getLabelPositions3D(settings, start, end, tickDirection);
        labelPos.resize(tickmarks.size());

        std::transform(tickmarks.begin(), tickmarks.end(), labelPos.begin(),
                       [](auto& tick) { return tick.second; });
    }} {}

void AxisRenderer3D::render(Camera* camera, const size2_t& outputDims, const vec3& startPos,
                            const vec3& endPos, const vec3& tickDirection, bool antialiasing) {
    if (!settings_.get().getAxisVisible()) return;

    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    renderAxis(camera, startPos, endPos, tickDirection, outputDims, antialiasing);
    renderText(camera, outputDims, startPos, endPos, tickDirection);
}

void AxisRenderer3D::renderText(Camera* camera, const size2_t& outputDims, const vec3& startPos,
                                const vec3& endPos, const vec3& tickDirection) {
    // axis caption
    if (const auto& captionSettings = settings_.get().getCaptionSettings()) {
        auto captex =
            caption_.getCaption(settings_.get().getCaption(), captionSettings, textRenderer_);

        // render axis caption centered at the axis using the offset
        const vec2 texDims(captex.texture->getDimensions());
        const auto anchor(captionSettings.getFont().getAnchorPos());

        const vec3 pos(plot::getAxisCaptionPosition3D(settings_, startPos, endPos, tickDirection));

        const auto angle = glm::radians(captionSettings.getRotation());
        const auto transform = glm::rotate(angle, vec3(0.0f, 0.0f, 1.0f));

        quadRenderer_.renderToRect3D(*camera, *captex.texture, pos,
                                     ivec2(captex.texture->getDimensions()), outputDims, anchor,
                                     transform);
    }

    // axis labels
    if (const auto& labels = settings_.get().getLabelSettings()) {
        const auto& pos =
            labels_.getLabelPos(settings_, startPos, endPos, textRenderer_, tickDirection);

        auto& atlas = labels_.getAtlas(settings_, startPos, endPos, textRenderer_);

        // render axis labels
        const vec2 anchorPos(labels.getFont().getAnchorPos());
        const auto& renderInfo = atlas.getRenderInfo();
        quadRenderer_.renderToRect3D(*camera, *atlas.getTexture(), pos, renderInfo.size,
                                     renderInfo.texTransform, outputDims, anchorPos);
    }
}

static_assert(!std::is_copy_constructible_v<detail::AxisCaption>);
static_assert(!std::is_copy_assignable_v<detail::AxisCaption>);
static_assert(std::is_nothrow_move_constructible_v<detail::AxisCaption>);
static_assert(std::is_nothrow_move_assignable_v<detail::AxisCaption>);

static_assert(std::is_nothrow_move_constructible_v<detail::AxisMeshes>);
static_assert(std::is_nothrow_move_assignable_v<detail::AxisMeshes>);

static_assert(std::is_nothrow_move_constructible_v<TextRenderer>);
static_assert(std::is_nothrow_move_assignable_v<TextRenderer>);
static_assert(std::is_nothrow_move_constructible_v<TextureQuadRenderer>);
static_assert(std::is_nothrow_move_assignable_v<TextureQuadRenderer>);

static_assert(!std::is_copy_constructible_v<detail::AxisLabels<ivec2>>);
static_assert(!std::is_copy_assignable_v<detail::AxisLabels<ivec2>>);
static_assert(std::is_nothrow_move_constructible_v<detail::AxisLabels<ivec2>>);
static_assert(std::is_nothrow_move_assignable_v<detail::AxisLabels<ivec2>>);

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
