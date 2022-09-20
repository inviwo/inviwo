/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <modules/opengl/image/imagegl.h>

#include <inviwo/core/datastructures/image/image.h>                     // for Image
#include <inviwo/core/datastructures/image/imagerepresentation.h>       // for ImageRepresentation
#include <inviwo/core/datastructures/image/imagetypes.h>                // for LayerType, typeCo...
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/util/formats.h>                                   // for DataFormat, DataF...
#include <inviwo/core/util/glmvec.h>                                    // for ivec2, size2_t
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT
#include <inviwo/core/util/stdextensions.h>                             // for all_of
#include <inviwo/core/util/stringconversion.h>                          // for toString
#include <modules/opengl/buffer/framebufferobject.h>                    // for FrameBufferObject
#include <modules/opengl/image/layergl.h>                               // for LayerGL
#include <modules/opengl/inviwoopengl.h>                                // for GLenum, glReadBuffer
#include <modules/opengl/openglexception.h>                             // for OpenGLException
#include <modules/opengl/openglutils.h>                                 // for DepthMaskState
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/sharedopenglresources.h>                       // for SharedOpenGLResou...
#include <modules/opengl/texture/texture2d.h>                           // IWYU pragma: keep
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnit, Text...
#include <modules/opengl/texture/textureutils.h>                        // for singleDrawImagePl...

#include <array>          // for array
#include <cstddef>        // for size_t
#include <memory>         // for shared_ptr, share...
#include <string>         // for operator+, basic_...
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for move

#include <glm/ext/matrix_transform.hpp>  // for scale
#include <glm/fwd.hpp>                   // for vec3
#include <glm/gtx/transform.hpp>         // for scale
#include <glm/vec2.hpp>                  // for vec<>::(anonymous)
#include <glm/vec4.hpp>                  // for operator*
#include <tcb/span.hpp>                  // for span

namespace inviwo {

ImageGL::ImageGL()
    : ImageRepresentation{}
    , colorLayersGL_{}
    , depthLayerGL_{nullptr}
    , pickingLayerGL_{nullptr}
    , frameBufferObject_{}
    , colorAttachmentIDs_{}
    , pickingAttachmentID_{}
    , colorAndPickingAttachmentIDs_{}
    , colorLayerCopyCount_{0} {}

ImageGL::ImageGL(const ImageGL& rhs)
    : ImageRepresentation(rhs)
    , colorLayersGL_{}
    , depthLayerGL_{nullptr}
    , pickingLayerGL_{nullptr}
    , frameBufferObject_{}
    , colorAttachmentIDs_{}
    , pickingAttachmentID_{}
    , colorAndPickingAttachmentIDs_{}
    , colorLayerCopyCount_{0} {}

ImageGL::~ImageGL() = default;

ImageGL* ImageGL::clone() const { return new ImageGL(*this); }

auto ImageGL::activate(ImageType type) -> ActiveState {

    auto fbo = utilgl::ActivateFBO{frameBufferObject_};

    const util::span drawBuffers =
        typeContainsPicking(type) ? colorAndPickingAttachmentIDs_ : colorAttachmentIDs_;
    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

    const auto depth = typeContainsDepth(type);
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, depth);
    utilgl::DepthMaskState depthMask(depth);

    const auto dim = getDimensions();
    utilgl::ViewportState viewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));
    return ActiveState{std::move(fbo), std::move(depthTest), std::move(depthMask),
                       std::move(viewport)};
}

void ImageGL::activateBuffer(ImageType type) {
    frameBufferObject_.activate();

    const util::span drawBuffers =
        typeContainsPicking(type) ? colorAndPickingAttachmentIDs_ : colorAttachmentIDs_;
    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

    if (!typeContainsDepth(type)) {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    } else {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }

    const uvec2 dim = getDimensions();
    glViewport(0, 0, dim.x, dim.y);
}

void ImageGL::deactivateBuffer() { frameBufferObject_.deactivate(); }

size2_t ImageGL::getDimensions() const { return colorLayersGL_.front()->getDimensions(); }

bool ImageGL::copyRepresentationsTo(ImageRepresentation* targetRep) const {
    return copyRepresentationsTo(dynamic_cast<ImageGL*>(targetRep));
}

size_t ImageGL::priority() const { return 300; }

bool ImageGL::copyRepresentationsTo(ImageGL* target) const {
    if (!target) return false;

    const ImageGL* source = this;

    // Set shader to copy all color layers
    if (!shader_ || colorLayerCopyCount_ != colorLayersGL_.size()) {
        shader_ = SharedOpenGLResources::getPtr()->getImageCopyShader(colorLayersGL_.size());
        colorLayerCopyCount_ = colorLayersGL_.size();
    }

    TextureUnit colorUnit, depthUnit, pickingUnit;
    source->getColorLayerGL()->bindTexture(colorUnit.getEnum());
    if (source->getDepthLayerGL()) {
        source->getDepthLayerGL()->bindTexture(depthUnit.getEnum());
    }
    if (source->getPickingLayerGL()) {
        source->getPickingLayerGL()->bindTexture(pickingUnit.getEnum());
    }
    TextureUnitContainer additionalColorUnits;
    for (size_t i = 1; i < colorLayersGL_.size(); ++i) {
        TextureUnit unit;
        source->getColorLayerGL(i)->bindTexture(unit.getEnum());
        additionalColorUnits.push_back(std::move(unit));
    }

    // ensure that target has at least same number of color layers
    if (target->colorLayersGL_.size() < source->colorLayersGL_.size()) {
        if (auto targetImage = target->getOwner()) {
            std::size_t delta = source->colorLayersGL_.size() - target->colorLayersGL_.size();
            for (std::size_t i = 0; i < delta; ++i) {
                targetImage->addColorLayer(
                    std::make_shared<Layer>(target->colorLayersGL_[0]->getDimensions(),
                                            source->colorLayersGL_[i]->getDataFormat(),
                                            source->colorLayersGL_[i]->getLayerType(),
                                            source->colorLayersGL_[i]->getSwizzleMask(),
                                            source->colorLayersGL_[i]->getInterpolation(),
                                            source->colorLayersGL_[i]->getWrapping()));
            }
            target->update(true);
        }
    }

    // Render to FBO, with correct scaling
    auto state = target->activate(ImageType::AllLayers);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto sourceDim = static_cast<vec2>(source->getDimensions());
    const auto targetDim = static_cast<vec2>(target->getDimensions());
    const auto sourceRatio = sourceDim.x / sourceDim.y;
    const auto targetRatio = targetDim.x / targetDim.y;
    const auto scale = targetRatio < sourceRatio
                           ? glm::scale(glm::vec3(1.0f, targetRatio / sourceRatio, 1.0f))
                           : glm::scale(glm::vec3(sourceRatio / targetRatio, 1.0f, 1.0f));

    GLint prog;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

    shader_->activate();
    shader_->setUniform("color_", colorUnit.getUnitNumber());
    if (source->getDepthLayerGL()) {
        shader_->setUniform("depth_", depthUnit.getUnitNumber());
    }
    if (source->getPickingLayerGL()) {
        shader_->setUniform("picking_", pickingUnit.getUnitNumber());
    }
    for (size_t i = 0; i < additionalColorUnits.size(); ++i) {
        shader_->setUniform("color" + toString<size_t>(i + 1),
                            additionalColorUnits[i].getUnitNumber());
    }
    shader_->setUniform("dataToClip", scale);
    utilgl::singleDrawImagePlaneRect();
    shader_->deactivate();

    glUseProgram(prog);
    return true;
}

bool ImageGL::updateFrom(const ImageGL* source) {
    ImageGL* target = this;

    // Primarily Copy by FBO blitting, all from source FBO to target FBO
    const auto sourceDim = static_cast<ivec2>(source->getDimensions());
    const auto targetDim = static_cast<ivec2>(target->getDimensions());

    const auto sourceFBO = source->getFBO();
    auto targetFBO = target->getFBO();
    const auto& sourceBuffers = sourceFBO->attachedColorTextureIds();
    const auto& targetBuffers = targetFBO->attachedColorTextureIds();
    sourceFBO->setReadBlit();
    targetFBO->setDrawBlit();

    GLbitfield mask = GL_COLOR_BUFFER_BIT;
    if (sourceFBO->hasDepthAttachment() && targetFBO->hasDepthAttachment())
        mask |= GL_DEPTH_BUFFER_BIT;
    if (sourceFBO->hasStencilAttachment() && targetFBO->hasStencilAttachment())
        mask |= GL_STENCIL_BUFFER_BIT;

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, sourceDim.x, sourceDim.y, 0, 0, targetDim.x, targetDim.y, mask,
                      GL_NEAREST);

    bool pickingCopied = false;
    for (GLuint i = 1; i < sourceFBO->maxColorAttachments(); i++) {
        if (sourceBuffers[i] != 0 && targetBuffers[i] != 0) {
            glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
            glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
            glBlitFramebuffer(0, 0, sourceDim.x, sourceDim.y, 0, 0, targetDim.x, targetDim.y,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);

            if (pickingAttachmentID_ &&
                GL_COLOR_ATTACHMENT0 + i == static_cast<GLuint>(*pickingAttachmentID_)) {
                pickingCopied = true;
            }
        }
    }

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    sourceFBO->setReadBlit(false);
    targetFBO->setDrawBlit(false);

    // Secondary copy using PBO
    // Depth texture
    if ((mask & GL_DEPTH_BUFFER_BIT) == 0) {
        auto sDepth = source->getDepthLayerGL()->getTexture();
        auto tDepth = target->getDepthLayerGL()->getTexture();

        if (sDepth && tDepth) tDepth->loadFromPBO(sDepth.get());
    }

    // Picking texture
    if (!pickingCopied && pickingAttachmentID_ != GLuint{0}) {
        auto sPicking = source->getPickingLayerGL()->getTexture();
        auto tPicking = target->getPickingLayerGL()->getTexture();

        if (sPicking && tPicking) tPicking->loadFromPBO(sPicking.get());
    }

    LGL_ERROR;
    return true;
}

FrameBufferObject* ImageGL::getFBO() { return &frameBufferObject_; }

const FrameBufferObject* ImageGL::getFBO() const { return &frameBufferObject_; }

LayerGL* ImageGL::getLayerGL(LayerType type, size_t idx) {
    switch (type) {
        case LayerType::Color:
            return getColorLayerGL(idx);
        case LayerType::Depth:
            return getDepthLayerGL();
        case LayerType::Picking:
            return getPickingLayerGL();
    }
    return nullptr;
}

const LayerGL* ImageGL::getLayerGL(LayerType type, size_t idx) const {
    switch (type) {
        case LayerType::Color:
            return getColorLayerGL(idx);
        case LayerType::Depth:
            return getDepthLayerGL();
        case LayerType::Picking:
            return getPickingLayerGL();
    }
    return nullptr;
}

LayerGL* ImageGL::getColorLayerGL(size_t idx) { return colorLayersGL_.at(idx); }

LayerGL* ImageGL::getDepthLayerGL() { return depthLayerGL_; }

LayerGL* ImageGL::getPickingLayerGL() { return pickingLayerGL_; }

const LayerGL* ImageGL::getColorLayerGL(size_t idx) const { return colorLayersGL_.at(idx); }

const LayerGL* ImageGL::getDepthLayerGL() const { return depthLayerGL_; }

const LayerGL* ImageGL::getPickingLayerGL() const { return pickingLayerGL_; }

size_t ImageGL::getNumberOfColorLayers() const { return colorLayersGL_.size(); }

void ImageGL::update(bool editable) {
    colorLayersGL_.clear();
    depthLayerGL_ = nullptr;
    pickingLayerGL_ = nullptr;

    bool needReattach = false;

    auto owner = getOwner();
    if (editable) {
        for (size_t i = 0; i < owner->getNumberOfColorLayers(); ++i) {
            colorLayersGL_.push_back(owner->getColorLayer(i)->getEditableRepresentation<LayerGL>());
            needReattach |= frameBufferObject_.attachedColorTextureIds()[i] !=
                            colorLayersGL_.back()->getTexture()->getID();
        }
        if (auto depthLayer = owner->getDepthLayer()) {
            depthLayerGL_ = depthLayer->getEditableRepresentation<LayerGL>();
            needReattach |=
                frameBufferObject_.attachedDepthTextureId() != depthLayerGL_->getTexture()->getID();
        }
        if (auto pickingLayer = owner->getPickingLayer()) {
            pickingLayerGL_ = pickingLayer->getEditableRepresentation<LayerGL>();
            needReattach |= frameBufferObject_.attachedColorTextureIds().back() !=
                            pickingLayerGL_->getTexture()->getID();
        }
    } else {
        for (size_t i = 0; i < owner->getNumberOfColorLayers(); ++i) {
            colorLayersGL_.push_back(
                const_cast<LayerGL*>(owner->getColorLayer(i)->getRepresentation<LayerGL>()));
            needReattach |= frameBufferObject_.attachedColorTextureIds()[i] !=
                            colorLayersGL_.back()->getTexture()->getID();
        }
        if (auto depthLayer = owner->getDepthLayer()) {
            depthLayerGL_ = const_cast<LayerGL*>(depthLayer->getRepresentation<LayerGL>());
            needReattach |=
                frameBufferObject_.attachedDepthTextureId() != depthLayerGL_->getTexture()->getID();
        }
        if (auto pickingLayer = owner->getPickingLayer()) {
            pickingLayerGL_ = const_cast<LayerGL*>(pickingLayer->getRepresentation<LayerGL>());
            needReattach |= frameBufferObject_.attachedColorTextureIds().back() !=
                            pickingLayerGL_->getTexture()->getID();
        }
    }

    // Attach all targets
    if (needReattach) reAttachAllLayers();
}

void ImageGL::reAttachAllLayers() {
    utilgl::ActivateFBO fbo{frameBufferObject_};
    frameBufferObject_.detachAllTextures();
    colorAttachmentIDs_.clear();
    pickingAttachmentID_ = std::nullopt;

    for (auto layer : colorLayersGL_) {
        layer->getTexture()->bind();
        const auto id = frameBufferObject_.attachColorTexture(layer->getTexture().get());
        colorAttachmentIDs_.push_back(id);
    }

    if (depthLayerGL_) {
        depthLayerGL_->getTexture()->bind();
        frameBufferObject_.attachTexture(depthLayerGL_->getTexture().get(),
                                         static_cast<GLenum>(GL_DEPTH_ATTACHMENT));
    }

    colorAndPickingAttachmentIDs_.assign(colorAttachmentIDs_.begin(), colorAttachmentIDs_.end());
    if (pickingLayerGL_) {
        pickingLayerGL_->getTexture()->bind();
        pickingAttachmentID_ = frameBufferObject_.attachColorTexture(
            pickingLayerGL_->getTexture().get(), frameBufferObject_.maxColorAttachments() - 1);
        colorAndPickingAttachmentIDs_.insert(colorAndPickingAttachmentIDs_.begin() + 1,
                                             *pickingAttachmentID_);
    }

    if (auto status = frameBufferObject_.status(); status != GL_FRAMEBUFFER_COMPLETE) {
        throw OpenGLException(IVW_CONTEXT, "Framebuffer ({}) incomplete: {}",
                              frameBufferObject_.getID(),
                              utilgl::framebufferStatusToString(status));
    }
}

std::type_index ImageGL::getTypeIndex() const { return std::type_index(typeid(ImageGL)); }

bool ImageGL::isValid() const {
    return depthLayerGL_ && pickingLayerGL_ && depthLayerGL_->isValid() &&
           pickingLayerGL_->isValid() &&
           util::all_of(colorLayersGL_, [](const auto& l) { return l->isValid(); });
}

dvec4 ImageGL::readPixel(size2_t pos, LayerType layer, size_t index) const {
    frameBufferObject_.setReadBlit(true);

    const auto layergl = getLayerGL(layer, index);
    const auto tex = layergl->getTexture();

    switch (layer) {
        case LayerType::Depth:
            break;
        case LayerType::Picking:
            glReadBuffer(*pickingAttachmentID_);
            break;
        case LayerType::Color:
        default:
            glReadBuffer(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index));
            break;
    }

    // Make a buffer that can hold the largest possible pixel type
    std::array<char, DataFormat<dvec4>::typesize> buffer;
    GLvoid* ptr = static_cast<GLvoid*>(buffer.data());
    const auto x = static_cast<GLint>(pos.x);
    const auto y = static_cast<GLint>(pos.y);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glReadPixels(x, y, 1, 1, tex->getFormat(), tex->getDataType(), ptr);

    // restore
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    frameBufferObject_.setReadBlit(false);

    return layergl->getDataFormat()->valueToVec4Double(ptr);
}

GLenum ImageGL::getPickingAttachmentID() const {
    return pickingAttachmentID_ ? *pickingAttachmentID_ : 0;
}

}  // namespace inviwo
