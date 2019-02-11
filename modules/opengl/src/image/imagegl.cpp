/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/core/util/formats.h>
#include <inviwo/core/datastructures/image/image.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/canvasgl.h>
#include <inviwo/core/datastructures/image/image.h>
#include <modules/opengl/sharedopenglresources.h>

namespace inviwo {

ImageGL::ImageGL() : ImageRepresentation(), frameBufferObject_(), colorLayerCopyCount_(0) {}

ImageGL::ImageGL(const ImageGL& rhs)
    : ImageRepresentation(rhs), frameBufferObject_(), colorLayerCopyCount_(0) {}

ImageGL::~ImageGL() {
    LGL_ERROR;
    frameBufferObject_.deactivate();
    LGL_ERROR;
}

ImageGL* ImageGL::clone() const { return new ImageGL(*this); }

void ImageGL::reAttachAllLayers(ImageType type) {
    frameBufferObject_.activate();
    frameBufferObject_.detachAllTextures();
    pickingAttachmentID_ = 0;

    for (auto layer : colorLayersGL_) {
        layer->getTexture()->bind();
        frameBufferObject_.attachColorTexture(layer->getTexture().get());
    }

    if (depthLayerGL_ && typeContainsDepth(type)) {
        depthLayerGL_->getTexture()->bind();
        frameBufferObject_.attachTexture(depthLayerGL_->getTexture().get(),
                                         static_cast<GLenum>(GL_DEPTH_ATTACHMENT));
    }

    if (pickingLayerGL_ && typeContainsPicking(type)) {
        pickingLayerGL_->getTexture()->bind();
        pickingAttachmentID_ =
            frameBufferObject_.attachColorTexture(pickingLayerGL_->getTexture().get(), 0, true, 1);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    frameBufferObject_.checkStatus();
    frameBufferObject_.deactivate();
}

void ImageGL::activateBuffer(ImageType type) {
    frameBufferObject_.activate();

    std::vector<GLenum> drawBuffers{frameBufferObject_.getDrawBuffers()};
    if (!drawBuffers.empty()) {
        GLsizei numBuffersToDrawTo = static_cast<GLsizei>(drawBuffers.size());

        // remove second render target (location = 1) when picking is disabled
        if (!typeContainsPicking(type) && (numBuffersToDrawTo > 1) &&
            (drawBuffers[1] == GL_COLOR_ATTACHMENT7)) {
            drawBuffers.erase(drawBuffers.begin() + 1);
            --numBuffersToDrawTo;
        }

        glDrawBuffers(numBuffersToDrawTo, drawBuffers.data());
        LGL_ERROR;
    }

    glGetBooleanv(GL_DEPTH_TEST, &prevDepthMask_);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &prevDepthTest_);

    if (!typeContainsDepth(type)) {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    } else {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }

    prevViewport_.get();

    uvec2 dim = getDimensions();
    glViewport(0, 0, dim.x, dim.y);
}

void ImageGL::deactivateBuffer() {
    // restore previous state
    frameBufferObject_.deactivate();  // this will activate the previous frame buffer.

    glDepthMask(prevDepthMask_);

    if (prevDepthTest_ == GL_TRUE) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    prevViewport_.set();
}

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
    if (target->colorLayersGL_.size() < this->colorLayersGL_.size()) {
        if (auto targetImage = static_cast<Image*>(target->getOwner())) {
            // TODO: what image format should be used for new layers?
            // reuse first color layer for now
            std::size_t delta = this->colorLayersGL_.size() - target->colorLayersGL_.size();
            for (std::size_t i = 0; i < delta; ++i) {
                targetImage->addColorLayer(
                    std::shared_ptr<Layer>(targetImage->getColorLayer(0)->clone()));
            }
            target->update(true);
        }
    }

    // Render to FBO, with correct scaling
    target->activateBuffer(ImageType::AllLayers);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float ratioSource = (float)source->getDimensions().x / (float)source->getDimensions().y;
    float ratioTarget = (float)target->getDimensions().x / (float)target->getDimensions().y;
    glm::mat4 scale;

    if (ratioTarget < ratioSource) {
        scale = glm::scale(glm::vec3(1.0f, ratioTarget / ratioSource, 1.0f));
    } else {
        scale = glm::scale(glm::vec3(ratioSource / ratioTarget, 1.0f, 1.0f));
    }

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
    target->renderImagePlaneRect();
    shader_->deactivate();
    target->deactivateBuffer();
    LGL_ERROR;

    glUseProgram(prog);

    return true;
}

bool ImageGL::updateFrom(const ImageGL* source) {
    ImageGL* target = this;

    // Primarily Copy by FBO blitting, all from source FBO to target FBO
    const FrameBufferObject* srcFBO = source->getFBO();
    FrameBufferObject* tgtFBO = target->getFBO();
    const Texture2D* sTex = source->getColorLayerGL()->getTexture().get();
    Texture2D* tTex = target->getColorLayerGL()->getTexture().get();

    const std::vector<bool>& srcBuffers = srcFBO->getDrawBuffersInUse();
    const std::vector<bool>& targetBuffers = tgtFBO->getDrawBuffersInUse();
    srcFBO->setRead_Blit();
    tgtFBO->setDraw_Blit();
    GLbitfield mask = GL_COLOR_BUFFER_BIT;

    if (srcFBO->hasDepthAttachment() && tgtFBO->hasDepthAttachment()) mask |= GL_DEPTH_BUFFER_BIT;

    if (srcFBO->hasStencilAttachment() && tgtFBO->hasStencilAttachment())
        mask |= GL_STENCIL_BUFFER_BIT;

    glBlitFramebufferEXT(0, 0, static_cast<GLint>(sTex->getWidth()),
                         static_cast<GLint>(sTex->getHeight()), 0, 0,
                         static_cast<GLint>(tTex->getWidth()),
                         static_cast<GLint>(tTex->getHeight()), mask, GL_NEAREST);
    bool pickingCopied = false;

    for (int i = 1; i < srcFBO->getMaxColorAttachments(); i++) {
        if (srcBuffers[i] && targetBuffers[i]) {
            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
            glBlitFramebufferEXT(
                0, 0, static_cast<GLint>(sTex->getWidth()), static_cast<GLint>(sTex->getHeight()),
                0, 0, static_cast<GLint>(tTex->getWidth()), static_cast<GLint>(tTex->getHeight()),
                GL_COLOR_BUFFER_BIT, GL_NEAREST);

            if (GL_COLOR_ATTACHMENT0_EXT + i == static_cast<int>(pickingAttachmentID_))
                pickingCopied = true;
        }
    }

    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    srcFBO->setRead_Blit(false);
    tgtFBO->setDraw_Blit(false);

    LGL_ERROR;

    // Secondary copy using PBO

    // Depth texture
    if ((mask & GL_DEPTH_BUFFER_BIT) == 0) {
        sTex = source->getDepthLayerGL()->getTexture().get();
        tTex = target->getDepthLayerGL()->getTexture().get();

        if (sTex && tTex) tTex->loadFromPBO(sTex);
    }

    LGL_ERROR;

    // Picking texture
    if (!pickingCopied && pickingAttachmentID_ != 0) {
        sTex = source->getPickingLayerGL()->getTexture().get();
        tTex = target->getPickingLayerGL()->getTexture().get();

        if (sTex && tTex) tTex->loadFromPBO(sTex);
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

void ImageGL::updateExistingLayers() const {
    auto owner = static_cast<const Image*>(this->getOwner());

    for (size_t i = 0; i < owner->getNumberOfColorLayers(); ++i) {
        owner->getColorLayer(i)->getRepresentation<LayerGL>();
    }

    const Layer* depthLayer = owner->getDepthLayer();

    if (depthLayer) depthLayer->getRepresentation<LayerGL>();

    const Layer* pickingLayer = owner->getPickingLayer();

    if (pickingLayer) pickingLayer->getRepresentation<LayerGL>();
}

void ImageGL::update(bool editable) {
    colorLayersGL_.clear();
    depthLayerGL_ = nullptr;
    pickingLayerGL_ = nullptr;

    if (editable) {
        auto owner = static_cast<Image*>(this->getOwner());

        for (size_t i = 0; i < owner->getNumberOfColorLayers(); ++i) {
            colorLayersGL_.push_back(owner->getColorLayer(i)->getEditableRepresentation<LayerGL>());
        }

        Layer* depthLayer = owner->getDepthLayer();
        if (depthLayer) {
            depthLayerGL_ = depthLayer->getEditableRepresentation<LayerGL>();
        }

        Layer* pickingLayer = owner->getPickingLayer();
        if (pickingLayer) {
            pickingLayerGL_ = pickingLayer->getEditableRepresentation<LayerGL>();
        }

    } else {
        auto owner = static_cast<const Image*>(this->getOwner());

        for (size_t i = 0; i < owner->getNumberOfColorLayers(); ++i) {
            colorLayersGL_.push_back(
                const_cast<LayerGL*>(owner->getColorLayer(i)->getRepresentation<LayerGL>()));
        }

        const Layer* depthLayer = owner->getDepthLayer();
        if (depthLayer) {
            depthLayerGL_ = const_cast<LayerGL*>(depthLayer->getRepresentation<LayerGL>());
        }

        const Layer* pickingLayer = owner->getPickingLayer();
        if (pickingLayer) {
            pickingLayerGL_ = const_cast<LayerGL*>(pickingLayer->getRepresentation<LayerGL>());
        }
    }

    // Attach all targets
    reAttachAllLayers(ImageType::AllLayers);
}

void ImageGL::renderImagePlaneRect() const { utilgl::singleDrawImagePlaneRect(); }

std::type_index ImageGL::getTypeIndex() const { return std::type_index(typeid(ImageGL)); }

bool ImageGL::isValid() const {
    return depthLayerGL_->isValid() && pickingLayerGL_->isValid() &&
           util::all_of(colorLayersGL_, [](const auto& l) { return l->isValid(); });
}

dvec4 ImageGL::readPixel(size2_t pos, LayerType layer, size_t index) const {
    frameBufferObject_.setRead_Blit(true);

    const auto layergl = getLayerGL(layer, index);
    const auto tex = layergl->getTexture();

    switch (layer) {
        case LayerType::Depth:
            break;
        case LayerType::Picking:
            glReadBuffer(pickingAttachmentID_);
            break;
        case LayerType::Color:
        default:
            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + static_cast<GLenum>(index));
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
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    frameBufferObject_.setRead_Blit(false);

    return layergl->getDataFormat()->valueToVec4Double(ptr);
}

GLenum ImageGL::getPickingAttachmentID() const { return pickingAttachmentID_; }

}  // namespace inviwo
