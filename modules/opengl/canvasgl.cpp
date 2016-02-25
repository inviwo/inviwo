/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "canvasgl.h"

#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglcapabilities.h>
#include <inviwo/core/datastructures/image/image.h>
#include <modules/opengl/geometry/meshgl.h>

namespace inviwo {

CanvasGL::CanvasGL(uvec2 dimensions)
    : Canvas(dimensions)
    , imageGL_(nullptr)
    , image_()
    , layerType_(LayerType::Color)
    , textureShader_(nullptr)
    , noiseShader_(nullptr)
    , channels_(0)
    , previousRenderedLayerIdx_(0) {}

void CanvasGL::defaultGLState() {
    if (!OpenGLCapabilities::hasSupportedOpenGLVersion()) return;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    LGL_ERROR;
}

void CanvasGL::render(std::shared_ptr<const Image> image, LayerType layerType, size_t idx) {   
    image_ = image;
    layerType_ = layerType;
    pickingContainer_.setPickingSource(image);
    if (image_) {
        imageGL_ = image_->getRepresentation<ImageGL>();
        
        if (imageGL_ && imageGL_->getLayerGL(layerType_, idx)) {
            checkChannels(imageGL_->getLayerGL(layerType_, idx)->getDataFormat()->getComponents());
        } else {
            checkChannels(image_->getDataFormat()->getComponents());
        }

        // Faster async download of textures sampled on interaction
        if (imageGL_->getDepthLayerGL()) imageGL_->getDepthLayerGL()->getTexture()->downloadToPBO();
        if (pickingContainer_.pickingEnabled() && imageGL_->getPickingLayerGL()) {
            imageGL_->getPickingLayerGL()->getTexture()->downloadToPBO();
        }

    } else {
        imageGL_ = nullptr;
    }
    update();
}

void CanvasGL::resize(uvec2 size) {
    imageGL_ = nullptr;
    pickingContainer_.setPickingSource(nullptr);
    Canvas::resize(size);
}

void CanvasGL::update() { renderLayer(previousRenderedLayerIdx_); }

void CanvasGL::renderLayer(size_t idx) {
    previousRenderedLayerIdx_ = idx;
    if (imageGL_) {
        if (auto layerGL = imageGL_->getLayerGL(layerType_, idx)) {
            TextureUnit textureUnit;
            layerGL->bindTexture(textureUnit.getEnum());
            renderTexture(textureUnit.getUnitNumber());
            layerGL->unbindTexture();
            return;
        }
    }
    renderNoise();
}

bool CanvasGL::ready() {
    if (ready_) {
        return true;
    } else if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        ready_ = true;
        LGL_ERROR;

        defaultGLState();
        square_ = utilgl::planeRect();
        squareGL_ = square_->getRepresentation<MeshGL>();
        textureShader();
        noiseShader();

        LGL_ERROR;
        return true;
    } else {
        return false;
    }
}

Shader* CanvasGL::textureShader() {
    if (!textureShader_) {
        textureShader_ = util::make_unique<Shader>("img_texturequad.vert", "img_texturequad.frag");
    }
    return textureShader_.get();
}
Shader* CanvasGL::noiseShader() {
    if (!noiseShader_) {
        noiseShader_ = util::make_unique<Shader>("img_texturequad.vert", "img_noise.frag");
    }
    return noiseShader_.get();
}

void CanvasGL::renderNoise() {
    if (!ready()) return;
    LGL_ERROR;
    
    glViewport(0, 0, getScreenDimensions().x, getScreenDimensions().y);
    LGL_ERROR;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    noiseShader_->activate();
    drawSquare();
    noiseShader_->deactivate();
    glSwapBuffers();
 
    LGL_ERROR;
}

void CanvasGL::renderTexture(int unitNumber) {
    if (!ready()) return;
    LGL_ERROR;

    glViewport(0, 0, getScreenDimensions().x, getScreenDimensions().y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    textureShader_->activate();
    textureShader_->setUniform("tex_", unitNumber);
    drawSquare();
    textureShader_->deactivate();
    glDisable(GL_BLEND);
    glSwapBuffers();
    LGL_ERROR;
}

void CanvasGL::drawSquare() {
    squareGL_->enable();
    glDepthFunc(GL_ALWAYS);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    squareGL_->disable();
    glDepthFunc(GL_LESS);
}

void CanvasGL::checkChannels(std::size_t channels) {
    if (channels_ == channels) return;

    if (channels == 1) {
        textureShader()->getFragmentShaderObject()->addShaderDefine("SINGLE_CHANNEL");
    } else {
        textureShader()->getFragmentShaderObject()->removeShaderDefine("SINGLE_CHANNEL");
    }
    
    channels_ = channels;
    textureShader_->getFragmentShaderObject()->build();
    textureShader_->link();
}

const LayerRAM* CanvasGL::getDepthLayerRAM() const {
    if (image_) {
        if (auto depthLayer = image_->getDepthLayer()) {
            return depthLayer->getRepresentation<LayerRAM>();
        }
    }
    return nullptr;
}

double CanvasGL::getDepthValueAtCoord(ivec2 coord, const LayerRAM* depthLayerRAM) const {
    if (!depthLayerRAM) depthLayerRAM = getDepthLayerRAM();

    if (depthLayerRAM) {
        const dvec2 screenDims(getScreenDimensions());
        const auto depthDims = depthLayerRAM->getDimensions();
        coord = glm::max(coord, ivec2(0));

        const dvec2 depthScreenRatio{dvec2(depthDims) / screenDims};
        size2_t coordDepth{depthScreenRatio * dvec2(coord)};
        coordDepth = glm::min(coordDepth, depthDims - size2_t(1, 1));

        const double depthValue = depthLayerRAM->getValueAsSingleDouble(coordDepth);

        // Convert to normalized device coordinates
        return 2.0 * depthValue - 1.0;
    } else {
        return 1.0;
    }
}

void CanvasGL::setProcessorWidgetOwner(ProcessorWidget* widget) {
    // Clear internal state
    image_.reset();
    imageGL_ = nullptr;
    pickingContainer_.setPickingSource(nullptr);
    Canvas::setProcessorWidgetOwner(widget);
}

}  // namespace
