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
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/buffer/bufferobjectarray.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/openglcapabilities.h>
#include <inviwo/core/datastructures/image/image.h>


namespace inviwo {

CanvasGL::CanvasGL(uvec2 dimensions)
    : Canvas(dimensions)
    , imageGL_(nullptr)
    , image_()
    , layerType_(LayerType::Color)
    , shader_(nullptr)
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

void CanvasGL::activate() {}

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
        renderLayer(idx);

        // Faster async download of textures sampled on interaction
        if (imageGL_->getDepthLayerGL()) imageGL_->getDepthLayerGL()->getTexture()->downloadToPBO();
        if (pickingContainer_.pickingEnabled() && imageGL_->getPickingLayerGL()) {
            imageGL_->getPickingLayerGL()->getTexture()->downloadToPBO();
        }
        
    } else {
        imageGL_ = nullptr;
        renderNoise();
    }
}

void CanvasGL::resize(uvec2 size) {
    imageGL_ = nullptr;
    pickingContainer_.setPickingSource(nullptr);
    Canvas::resize(size);
}

void CanvasGL::update() { renderLayer(previousRenderedLayerIdx_); }

void CanvasGL::attachImagePlanRect(BufferObjectArray* arrayObject) {
    if (arrayObject) {
        arrayObject->bind();
        arrayObject->attachBufferObject(
            square()->getBufferGL(0)->getBufferObject().get(),
            static_cast<GLuint>(BufferType::PositionAttrib));
        arrayObject->attachBufferObject(
            square()->getBufferGL(1)->getBufferObject().get(),
            static_cast<GLuint>(BufferType::TexcoordAttrib));
        arrayObject->unbind();
    }
}

void CanvasGL::singleDrawImagePlaneRect() { glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); }

void CanvasGL::multiDrawImagePlaneRect(int instances) {
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instances);
}

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


const MeshGL* CanvasGL::square() {
    LGL_ERROR;
    static std::unique_ptr<Mesh> mesh{ []() {
        auto verticesBuffer =
            util::makeBuffer<vec2>({{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}, {1.0f, 1.0f}});
        auto texCoordsBuffer =
            util::makeBuffer<vec2>({{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}});
        auto indices_ = util::makeIndexBuffer({0, 1, 2, 3});

        auto mesh = util::make_unique<Mesh>();
        mesh->addBuffer(BufferType::PositionAttrib, verticesBuffer);
        mesh->addBuffer(BufferType::TexcoordAttrib, texCoordsBuffer);
        mesh->addIndicies(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip), indices_);

        return mesh;
    }() };
    LGL_ERROR;
    const static MeshGL* square{mesh->getRepresentation<MeshGL>()};
    LGL_ERROR;
    return square;
}

bool CanvasGL::ready() {
    if (ready_) {
        return true;
    } else {
        switch (glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
            case GL_FRAMEBUFFER_COMPLETE: {
                ready_ = true;
                LGL_ERROR;
                defaultGLState();
                LGL_ERROR;
                shader_ = util::make_unique<Shader>("img_texturequad.vert", "img_texturequad.frag");
                LGL_ERROR;
                noiseShader_ = util::make_unique<Shader>("img_texturequad.vert", "img_noise.frag");
                LGL_ERROR;

                return true;
                }
            default:
                return false;
        }
    }
}

void CanvasGL::renderNoise() {
    if (!ready()) return;
    LGL_ERROR;
    activate();
    glViewport(0, 0, getScreenDimensions().x, getScreenDimensions().y);
    LGL_ERROR;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    noiseShader_->activate();
    drawRect();
    noiseShader_->deactivate();
    glSwapBuffers();
    activateDefaultRenderContext();
    LGL_ERROR;
}

void CanvasGL::renderTexture(int unitNumber) {
    if (!ready()) return;
    LGL_ERROR;
    activate();
    glViewport(0, 0, getScreenDimensions().x, getScreenDimensions().y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader_->activate();
    shader_->setUniform("tex_", unitNumber);
    drawRect();
    shader_->deactivate();
    glDisable(GL_BLEND);
    glSwapBuffers();
    activateDefaultRenderContext();
    LGL_ERROR;
}

void CanvasGL::drawRect() {
    LGL_ERROR;
    BufferObjectArray rectArray;
    rectArray.bind();
    rectArray.attachBufferObject(square()->getBufferGL(0)->getBufferObject().get(),
                                   static_cast<GLuint>(BufferType::PositionAttrib));
    rectArray.attachBufferObject(square()->getBufferGL(1)->getBufferObject().get(),
                                   static_cast<GLuint>(BufferType::TexcoordAttrib));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    rectArray.unbind();
    LGL_ERROR;
}

void CanvasGL::checkChannels(std::size_t channels) {
    if (channels_ == channels) return;
    
    switch(channels) {
        case 1: {
            shader_->getFragmentShaderObject()->addShaderDefine("SINGLE_CHANNEL");
            break;
        }
        case 2: {
            shader_->getFragmentShaderObject()->removeShaderDefine("SINGLE_CHANNEL");
            break;
        }
        case 3: {
            shader_->getFragmentShaderObject()->removeShaderDefine("SINGLE_CHANNEL");
            break;
        }
        case 4: {
            shader_->getFragmentShaderObject()->removeShaderDefine("SINGLE_CHANNEL");
            break;
        }
    }
    channels_ = channels;
    shader_->getFragmentShaderObject()->build();
    shader_->link();
}

const LayerRAM* CanvasGL::getDepthLayerRAM() const {
    if (image_) {
        const Layer* depthLayer = image_->getDepthLayer();
        if (depthLayer) {
            return depthLayer->getRepresentation<LayerRAM>();
        } else
            return nullptr;
    } else
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
        coordDepth = glm::min(coordDepth, depthDims - size2_t(1,1));

        const double depthValue = depthLayerRAM->getValueAsSingleDouble(coordDepth);

        // Convert to normalized device coordinates
        return 2.0 * depthValue - 1.0;
    } else {
        return 1.0;
    }
}

void CanvasGL::enableDrawImagePlaneRect() { square()->enable(); }

void CanvasGL::disableDrawImagePlaneRect() { square()->disable(); }

void CanvasGL::setProcessorWidgetOwner(ProcessorWidget* widget) {
    // Clear internal state
    image_.reset();
    imageGL_ = nullptr;
    pickingContainer_.setPickingSource(nullptr);
    Canvas::setProcessorWidgetOwner(widget);
}

}  // namespace
