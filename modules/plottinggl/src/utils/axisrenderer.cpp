/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/camera.h>

#include <modules/plotting/properties/categoricalaxisproperty.h>
#include <modules/plotting/utils/axisutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/shader/shadertype.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>

#include <array>
#include <algorithm>
#include <numeric>
#include <functional>

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
    flip_.check(*this, settings.getFlipped());
    major_.check(*this, settings.getMajorTicks());
    tickDirection_.check(*this, tickDirection);
    if (!majorMesh_) {
        const auto tickPositions = getMajorTickPositions(major_, range_);
        majorMesh_ = generateTicksMesh(tickPositions, range_, startPos_.get(), endPos_.get(),
                                       tickDirection_, major_.get().getTickLength(),
                                       major_.get().getStyle(), major_.get().getColor(), flip_);
    }
    return majorMesh_.get();
}

Mesh* AxisMeshes::getMinor(const AxisSettings& settings, const vec3& start, const vec3& end,
                           const vec3& tickDirection) {
    startPos_.check(*this, start);
    endPos_.check(*this, end);
    range_.check(*this, settings.getRange());
    flip_.check(*this, settings.getFlipped());
    major_.check(*this, settings.getMajorTicks());
    minor_.check(*this, settings.getMinorTicks());
    tickDirection_.check(*this, tickDirection);
    if (!minorMesh_) {
        const auto tickPositions = getMinorTickPositions(minor_, major_, range_);
        minorMesh_ = generateTicksMesh(tickPositions, range_, startPos_.get(), endPos_.get(),
                                       tickDirection, minor_.get().getTickLength(),
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

AxisRendererBase::AxisRendererBase(const AxisRendererBase& rhs)
    : settings_{rhs.settings_}, shaders_{getShaders()} {}

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

    drawMesh(axisMesh, settings_.getWidth(), false);
    auto majorMesh = meshes_.getMajor(settings_, start, end, tickdir);
    drawMesh(majorMesh, settings_.getMajorTicks().getTickWidth(), true);
    auto minorMesh = meshes_.getMinor(settings_, start, end, tickdir);
    drawMesh(minorMesh, settings_.getMinorTicks().getTickWidth(), true);

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

void AxisRenderer::render(const size2_t& outputDims, const size2_t& startPos, const size2_t& endPos,
                          bool antialiasing) {
    if (!settings_.getAxisVisible()) {
        return;
    }

    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    const auto axisDir = glm::normalize(vec2{endPos} - vec2{startPos});
    const auto tickDir = vec3(-axisDir.y, axisDir.x, 0.0f);
    renderAxis(nullptr, vec3{startPos, 0}, vec3{endPos, 0}, tickDir, outputDims, antialiasing);
    renderText(outputDims, startPos, endPos);
}

void AxisRenderer::renderText(const size2_t& outputDims, const size2_t& startPos,
                              const size2_t& endPos) {
    // axis caption
    if (const auto& captionSettings = settings_.getCaptionSettings()) {
        auto& captex = caption_.getCaption(settings_.getCaption(), captionSettings, textRenderer_);

        // render axis caption centered at the axis using the offset
        const auto anchor = captionSettings.getFont().getAnchorPos();

        // The anchor position in the texture;
        const auto anchorPos = vec2{captex.bbox.textExtent} * (anchor + vec2{1.0f}) * 0.5f -
                               vec2{captex.bbox.glyphsOrigin};

        const auto angle = glm::radians(captionSettings.getRotation()) +
                           (settings_.isVertical() ? glm::half_pi<float>() : 0.0f);

        // translate to anchor pos and apply rotation
        const auto transform =
            glm::rotate(angle, vec3(0.0f, 0.0f, 1.0f)) * glm::translate(vec3(-anchorPos, 0.f));

        const auto pos = plot::getAxisCaptionPosition(settings_, startPos, endPos);

        const auto posi = glm::ivec2{glm::round(pos)};
        quadRenderer_.render(*captex.texture, posi, outputDims, transform);
    }

    // axis labels
    if (const auto& labels = settings_.getLabelSettings()) {
        auto& pos = labels_.getLabelPos(settings_, vec3{startPos, 0}, vec3{endPos, 0},
                                        textRenderer_, vec3{1});

        auto& atlas =
            labels_.getAtlas(settings_, vec3{startPos, 0}, vec3{endPos, 0}, textRenderer_);

        const auto anchor = labels.getFont().getAnchorPos();
        const auto angle = glm::radians(labels.getRotation());

        // translate to anchor pos and apply rotation
        std::vector<mat4> transforms;
        const auto& ri = atlas.getRenderInfo();
        std::transform(ri.boundingBoxes.begin(), ri.boundingBoxes.end(),
                       std::back_inserter(transforms), [&](const TextBoundingBox& bb) {
                           const auto anchorPos =
                               vec2{bb.textExtent} * (anchor + vec2{1.0f}) * 0.5f -
                               vec2{bb.glyphsOrigin};
                           return glm::rotate(angle, vec3(0.0f, 0.0f, 1.0f)) *
                                  glm::translate(vec3(-anchorPos, 0.f));
                       });

        // render axis labels
        quadRenderer_.renderToRect(*atlas.getTexture(), pos, ri.size, ri.texTransform, outputDims,
                                   transforms);
    }
}

std::pair<vec2, vec2> AxisRenderer::boundingRect(const size2_t& startPos, const size2_t& endPos) {

    auto bRect = tickBoundingRect(settings_, startPos, endPos);

    if (const auto& captionSettings = settings_.getCaptionSettings()) {

        auto& captex = caption_.getCaption(settings_.getCaption(), captionSettings, textRenderer_);
        const auto texDims(captex.texture->getDimensions());

        const auto anchor = captionSettings.getFont().getAnchorPos();

        // The anchor position in the texture;
        const auto anchorPos = vec2{captex.bbox.textExtent} * (anchor + vec2{1.0f}) * 0.5f -
                               vec2{captex.bbox.glyphsOrigin};

        const auto angle = glm::radians(captionSettings.getRotation()) +
                           (settings_.isVertical() ? glm::half_pi<float>() : 0.0f);

        // translate to anchor pos and apply rotation
        const auto transform =
            glm::rotate(angle, vec3(0.0f, 0.0f, 1.0f)) * glm::translate(vec3(-anchorPos, 0.f));

        const auto pos = plot::getAxisCaptionPosition(settings_, startPos, endPos);

        const auto pos1 = pos + vec2{transform * vec4{0.0f, 0.0f, 0.0f, 1.0f}};
        const auto pos2 = pos + vec2{transform * vec4{texDims.x, texDims.y, 0.0f, 1.0f}};

        bRect.first = glm::min(bRect.first, pos1);
        bRect.first = glm::min(bRect.first, pos2);

        bRect.second = glm::max(bRect.second, pos1);
        bRect.second = glm::max(bRect.second, pos2);
    }

    // axis labels
    if (const auto& labels = settings_.getLabelSettings()) {
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
            const auto anchorPos =
                vec2{bb.textExtent} * (anchor + vec2{1.0f}) * 0.5f - vec2{bb.glyphsOrigin};
            const auto transform =
                glm::rotate(angle, vec3(0.0f, 0.0f, 1.0f)) * glm::translate(vec3(-anchorPos, 0.f));
            const auto pos1 = vec2{pos} + vec2{transform * vec4{0.0f, 0.0f, 0.0f, 1.0f}};
            const auto pos2 = vec2{pos} + vec2{transform * vec4{bb.glyphsExtent, 0.0f, 1.0f}};

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
    if (!settings_.getAxisVisible()) return;

    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    renderAxis(camera, startPos, endPos, tickDirection, outputDims, antialiasing);
    renderText(camera, outputDims, startPos, endPos, tickDirection);
}

void AxisRenderer3D::renderText(Camera* camera, const size2_t& outputDims, const vec3& startPos,
                                const vec3& endPos, const vec3& tickDirection) {
    // axis caption
    if (const auto& captionSettings = settings_.getCaptionSettings()) {
        auto captex = caption_.getCaption(settings_.getCaption(), captionSettings, textRenderer_);

        // render axis caption centered at the axis using the offset
        const vec2 texDims(captex.texture->getDimensions());
        const auto anchor(captionSettings.getFont().getAnchorPos());

        const vec3 pos(plot::getAxisCaptionPosition3D(settings_, startPos, endPos, tickDirection));

        quadRenderer_.renderToRect3D(*camera, *captex.texture, pos,
                                     ivec2(captex.texture->getDimensions()), outputDims, anchor);
    }

    // axis labels
    if (const auto& labels = settings_.getLabelSettings()) {
        const auto& pos =
            labels_.getLabelPos(settings_, startPos, endPos, textRenderer_, tickDirection);

        auto atlas = labels_.getAtlas(settings_, startPos, endPos, textRenderer_);

        // render axis labels
        const vec2 anchorPos(labels.getFont().getAnchorPos());
        const auto& renderInfo = atlas.getRenderInfo();
        quadRenderer_.renderToRect3D(*camera, *atlas.getTexture(), pos, renderInfo.size,
                                     renderInfo.texTransform, outputDims, anchorPos);
    }
}

}  // namespace plot

}  // namespace inviwo
