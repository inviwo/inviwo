/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/plotting/utils/axisutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>

#include <array>
#include <algorithm>
#include <numeric>

namespace inviwo {

namespace plot {

AxisRendererBase::AxisRendererBase(const AxisProperty& property)
    : property_(property)
    , lineShader_("linerenderer.vert", "linerenderer.geom", "linerenderer.frag", false) {
    lineShader_.getGeometryShaderObject()->addShaderDefine("ENABLE_ADJACENCY", "0");
    lineShader_.build();
}

AxisRendererBase::AxisRendererBase(const AxisRendererBase& rhs)
    : property_(rhs.property_), lineShader_(rhs.lineShader_) {}

void AxisRendererBase::renderAxis(Camera* camera, const size2_t& outputDims, bool antialiasing) {
    if (!axisMesh_ && !majorTicksMesh_ && !minorTicksMesh_) return;

    lineShader_.activate();
    lineShader_.setUniform("screenDim", vec2(outputDims));
    if (camera) {
        utilgl::setShaderUniforms(lineShader_, *camera, "camera");
    }

    // returns thickness of antialiased edge based on the global antialiasing flag
    // and whether the line is 1px wide
    auto antialiasWidth = [&](float lineWidth) {
        if (!antialiasing || (std::abs(lineWidth - 1.0f) < 0.01f)) {
            return 0.0f;
        } else {
            return 0.5f;
        }
    };

    auto drawMesh = [&](const MeshGL* meshgl, float lineWidth, bool caps) {
        lineShader_.setUniform("lineWidth", lineWidth);
        auto mesh = meshgl->getOwner();
        MeshDrawerGL::DrawObject drawer(meshgl, mesh->getDefaultMeshInfo());
        utilgl::setShaderUniforms(lineShader_, *mesh, "geometry");
        lineShader_.setUniform("antialiasing", antialiasWidth(lineWidth));
        lineShader_.setUniform("roundCaps", caps);
        drawer.draw();
    };

    mat4 m(1.0f);
    if (!camera) {
        // compute matrix to transform meshes from screen coords to clip space
        m = mat4(vec4(2.0f / outputDims.x, 0.0f, 0.0f, 0.0f),
                 vec4(0.0f, 2.0f / outputDims.y, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f),
                 vec4(-1.0f, -1.0f, 0.0f, 1.0f));
    }

    // draw axis
    if (axisMesh_) {
        axisMesh_->setWorldMatrix(m);
        drawMesh(axisMesh_->getRepresentation<MeshGL>(), property_.width_.get(), false);
    }

    // draw major ticks
    if (majorTicksMesh_) {
        majorTicksMesh_->setWorldMatrix(m);
        drawMesh(majorTicksMesh_->getRepresentation<MeshGL>(), 
                 property_.ticks_.majorTicks_.tickWidth_.get(), true);
    }

    // draw minor ticks
    if (minorTicksMesh_) {
        minorTicksMesh_->setWorldMatrix(m);
        drawMesh(minorTicksMesh_->getRepresentation<MeshGL>(), 
                 property_.ticks_.minorTicks_.tickWidth_.get(), true);
    }

    lineShader_.deactivate();
}

void AxisRendererBase::invalidateInternalState(bool positionChange) {
    // figure out which parts have been invalidated due to property modifications
    bool invalidateAxis = false;
    bool invalidateCaption = false;
    bool invalidateMajorTicks = false;
    bool invalidateMinorTicks = false;
    bool invalidateLabelAtlas = false;
    bool invalidateLabelPos = false;

    auto isModified = [](const std::vector<const Property*>& props) {
        return util::any_of(props, [](const auto& p) { return p->isModified(); });
    };

    // start and end position of axis
    if (positionChange) {
        // invalidate axis, major tick mesh, minor tick mesh, and label positions
        invalidateAxis = true;
        invalidateMajorTicks = true;
        invalidateMinorTicks = true;
        invalidateLabelPos = true;
    }
    // check axis range
    if (property_.range_.isModified()) {
        invalidateMajorTicks = true;
        invalidateMinorTicks = true;
        invalidateLabelPos = true;
        invalidateLabelAtlas = true;
    }

    if (property_.color_.isModified()) {
        invalidateAxis = true;
    }

    // check caption
    if (isModified({&property_.caption_.title_, &property_.caption_.color_,
                    &property_.caption_.font_.fontFace_, &property_.caption_.font_.fontSize_})) {
        // invalidate caption
        invalidateCaption = true;
    }
    // labels
    if (isModified({&property_.labels_.title_, &property_.labels_.color_,
                    &property_.labels_.font_.fontFace_, &property_.labels_.font_.fontSize_})) {
        // invalidate labels
        invalidateLabelAtlas = true;
    }
    if (isModified({&property_.labels_.offset_, &property_.labels_.font_.anchorPos_})) {
        invalidateLabelPos = true;
    }
    // major ticks
    if (isModified({&property_.range_, &property_.ticks_.majorTicks_.style_,
                    &property_.ticks_.majorTicks_.rangeBasedTicks_,
                    &property_.ticks_.majorTicks_.color_, &property_.ticks_.majorTicks_.tickLength_,
                    &property_.ticks_.majorTicks_.tickDelta_})) {
        // invalidate labels and tick meshes
        invalidateMajorTicks = true;
        invalidateMinorTicks = true;
        invalidateLabelAtlas = true;
    }
    // minor ticks
    if (isModified({&property_.ticks_.minorTicks_.style_, &property_.ticks_.minorTicks_.color_,
                    &property_.ticks_.minorTicks_.tickLength_,
                    &property_.ticks_.minorTicks_.tickFrequency_,
                    &property_.ticks_.minorTicks_.fillAxis_})) {
        // invalidate minor tick mesh
        invalidateMinorTicks = true;
    }

    if (invalidateAxis) {
        axisMesh_ = nullptr;
    }
    if (invalidateCaption) {
        axisCaptionTex_ = nullptr;
    }
    if (invalidateMajorTicks) {
        majorTicksMesh_ = nullptr;
    }
    if (invalidateMinorTicks) {
        minorTicksMesh_ = nullptr;
    }
    if (invalidateLabelPos) {
        invalidateLabelPositions();
    }
    if (invalidateLabelAtlas) {
        labelTexAtlas_.clear();
    }
}

std::shared_ptr<Mesh> AxisRendererBase::getMesh() const {
    if (!axisMesh_) return std::make_shared<Mesh>();

    auto mesh = std::shared_ptr<Mesh>(axisMesh_->clone());

    if (majorTicksMesh_ && (majorTicksMesh_->getNumberOfBuffers() > 0)) {
        mesh->append(*majorTicksMesh_.get());
    }
    if (minorTicksMesh_ && (minorTicksMesh_->getNumberOfBuffers() > 0)) {
        mesh->append(*minorTicksMesh_.get());
    }

    return mesh;
}

std::shared_ptr<Texture2D> AxisRendererBase::getLabelAtlasTexture() const {
    return labelTexAtlas_.getTexture();
}

void AxisRendererBase::updateCaptionTexture() {
    // set up text renderer
    textRenderer_.setFont(property_.caption_.font_.fontFace_.get());
    textRenderer_.setFontSize(property_.caption_.font_.fontSize_.getSelectedValue());

    axisCaptionTex_ = util::createTextTexture(textRenderer_, property_.caption_.title_.get(),
                                              property_.caption_.font_.fontSize_.getSelectedValue(),
                                              property_.caption_.color_.get());
}

void AxisRendererBase::updateLabelAtlas() {
    textRenderer_.setFont(property_.labels_.font_.fontFace_.get());
    textRenderer_.setFontSize(property_.labels_.font_.fontSize_.getSelectedValue());

    const auto tickmarks = plot::getMajorTickPositions(property_);
    if (tickmarks.empty()) {
        // create a dummy texture
        labelTexAtlas_.initTexture(size2_t(1u));
        return;
    }

    // fill map with all labels
    std::array<char, 100> buf;
    const char* format = property_.labels_.title_.get().c_str();

    const vec4 color(property_.labels_.color_.get());

    std::vector<TexAtlasEntry> atlasEntries;
    atlasEntries.reserve(tickmarks.size());
    for (auto& tick : tickmarks) {
        // convert current tick value into string
        snprintf(buf.data(), buf.size(), format, tick);
        const ivec2 size(textRenderer_.computeTextSize(buf.data()));

        atlasEntries.push_back({buf.data(), ivec2(0), size, color});
    }

    labelTexAtlas_.fillAtlas(textRenderer_, atlasEntries);
}

// ---------------------------------------

AxisRenderer::AxisRenderer(const AxisProperty& property)
    : AxisRendererBase(property), prevStartPos_(0u), prevEndPos_(0u) {}

AxisRenderer::AxisRenderer(const AxisRenderer& rhs) : AxisRendererBase(rhs) {}

void AxisRenderer::render(const size2_t& outputDims, const size2_t& startPos, const size2_t& endPos,
                          bool antialiasing) {
    if (!property_.visible_.get()) {
        return;
    }

    bool posChange = ((startPos != prevStartPos_) || (endPos != prevEndPos_));
    if (posChange) {
        prevStartPos_ = startPos;
        prevEndPos_ = endPos;
    }

    invalidateInternalState(posChange);

    updateMeshes(startPos, endPos);

    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    renderAxis(nullptr, outputDims, antialiasing);
    renderText(outputDims, startPos, endPos);
}

void AxisRenderer::renderText(const size2_t& outputDims, const size2_t& startPos,
                              const size2_t& endPos) {
    // axis caption
    if (property_.caption_.isChecked()) {
        if (!axisCaptionTex_) {
            updateCaptionTexture();
        }

        // render axis caption centered at the axis using the offset
        const vec2 texDims(axisCaptionTex_->getDimensions());
        const auto anchor(property_.caption_.font_.anchorPos_.get());

        mat4 m;
        vec2 offset;
        if (property_.orientation_.get() == AxisProperty::Orientation::Vertical) {
            // rotate labels for vertical axis by 90 degree ccw.
            m = glm::rotate(glm::half_pi<float>(), vec3(0.0f, 0.0f, 1.0f));

            // need to invert anchor.x due to rotation
            offset = vec2(-texDims.y, texDims.x) * 0.5f * (vec2(-anchor.x, anchor.y) + vec2(1.0f));
        } else {
            offset = texDims * 0.5f * (anchor + vec2(1.0f));
        }

        const ivec2 posi(plot::getAxisCaptionPosition(property_, startPos, endPos) - offset);
        quadRenderer_.render(axisCaptionTex_, posi, outputDims, m);
    }

    // axis labels
    if (property_.labels_.isChecked()) {
        if (!labelTexAtlas_.valid()) {
            updateLabelAtlas();
            labelPos_.clear();
        }

        if (labelPos_.empty()) {
            updateLabelPositions(startPos, endPos);
        }

        // render axis labels
        const auto& renderInfo = labelTexAtlas_.getRenderInfo();
        quadRenderer_.renderToRect(labelTexAtlas_.getTexture(), labelPos_, renderInfo.size,
                                   renderInfo.texTransform, outputDims);
    }
}

void AxisRenderer::updateMeshes(const size2_t& startPos, const size2_t& endPos) {
    if (!axisMesh_) {
        axisMesh_ = plot::generateAxisMesh(property_, startPos, endPos);
    }
    if (!majorTicksMesh_) {
        majorTicksMesh_ = plot::generateMajorTicksMesh(property_, startPos, endPos);
    }
    if (!minorTicksMesh_) {
        minorTicksMesh_ = plot::generateMinorTicksMesh(property_, startPos, endPos);
    }
}

void AxisRenderer::invalidateLabelPositions() { labelPos_.clear(); }

void AxisRenderer::updateLabelPositions(const size2_t& startPos, const size2_t& endPos) {
    const auto& tickmarks = plot::getLabelPositions(property_, startPos, endPos);
    labelPos_.resize(tickmarks.size());

    const vec2 anchorPos(property_.labels_.font_.anchorPos_.get());

    auto v = util::zip(labelTexAtlas_.getEntries(), tickmarks);
    std::transform(v.begin(), v.end(), labelPos_.begin(), [&](auto&& p) {
        const vec2 size(get<0>(p).texExtent);
        const vec2 offset = 0.5f * size * (anchorPos + vec2(1.0f, 1.0f));

        return ivec2(get<1>(p).second - offset);
    });
}

// ---------------------------------------

AxisRenderer3D::AxisRenderer3D(const AxisProperty& property)
    : AxisRendererBase(property), prevStartPos_(0.0f), prevEndPos_(0.0f) {}

AxisRenderer3D::AxisRenderer3D(const AxisRenderer3D& rhs) : AxisRendererBase(rhs) {}

void AxisRenderer3D::render(Camera* camera, const size2_t& outputDims, const vec3& startPos,
                            const vec3& endPos, const vec3& tickDirection, bool antialiasing) {
    if (!property_.visible_.get()) {
        return;
    }

    bool posChange = ((startPos != prevStartPos_) || (endPos != prevEndPos_) ||
                      (tickDirection != prevTickDirection_));
    if (posChange) {
        prevStartPos_ = startPos;
        prevEndPos_ = endPos;
        prevTickDirection_ = tickDirection;
    }

    invalidateInternalState(posChange);

    updateMeshes(startPos, endPos, tickDirection);

    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    renderAxis(camera, outputDims, antialiasing);
    renderText(camera, outputDims, startPos, endPos, tickDirection);
}

void AxisRenderer3D::renderText(Camera* camera, const size2_t& outputDims, const vec3& startPos,
                                const vec3& endPos, const vec3& tickDirection) {
    // axis caption
    if (property_.caption_.isChecked()) {
        if (!axisCaptionTex_) {
            updateCaptionTexture();
        }

        // render axis caption centered at the axis using the offset
        const vec2 texDims(axisCaptionTex_->getDimensions());
        const auto anchor(property_.caption_.font_.anchorPos_.get());

        const vec3 pos(
            plot::getAxisCaptionPosition3D(property_, startPos, endPos, tickDirection));

        quadRenderer_.renderToRect3D(*camera, axisCaptionTex_, pos,
                                     ivec2(axisCaptionTex_->getDimensions()), outputDims, anchor);
    }

    // axis labels
    if (property_.labels_.isChecked()) {
        if (!labelTexAtlas_.valid()) {
            updateLabelAtlas();
            labelPos_.clear();
        }

        if (labelPos_.empty()) {
            updateLabelPositions(startPos, endPos, tickDirection);
        }

        // render axis labels
        const vec2 anchorPos(property_.labels_.font_.anchorPos_.get());
        const auto& renderInfo = labelTexAtlas_.getRenderInfo();
        quadRenderer_.renderToRect3D(*camera, labelTexAtlas_.getTexture(), labelPos_, renderInfo.size,
                                     renderInfo.texTransform, outputDims, anchorPos);
    }
}

void AxisRenderer3D::updateMeshes(const vec3& startPos, const vec3& endPos,
                                  const vec3& tickDirection) {
    if (!axisMesh_) {
        axisMesh_ = plot::generateAxisMesh3D(property_, startPos, endPos);
    }
    if (!majorTicksMesh_) {
        majorTicksMesh_ =
            plot::generateMajorTicksMesh3D(property_, startPos, endPos, tickDirection);
    }
    if (!minorTicksMesh_) {
        minorTicksMesh_ =
            plot::generateMinorTicksMesh3D(property_, startPos, endPos, tickDirection);
    }
}

void AxisRenderer3D::invalidateLabelPositions() { labelPos_.clear(); }

void AxisRenderer3D::updateLabelPositions(const vec3& startPos, const vec3& endPos,
                                          const vec3& tickDirection) {
    const auto& tickmarks =
        plot::getLabelPositions3D(property_, startPos, endPos, tickDirection);
    labelPos_.resize(tickmarks.size());

    std::transform(tickmarks.begin(), tickmarks.end(), labelPos_.begin(),
                   [](auto& tick) { return tick.second; });
}

}  // namespace plot

}  // namespace inviwo
