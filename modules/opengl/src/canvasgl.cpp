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

#include <modules/opengl/canvasgl.h>

#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/sharedopenglresources.h>
#include <inviwo/core/datastructures/image/image.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/util/rendercontext.h>

namespace inviwo {

CanvasGL::CanvasGL(size2_t dimensions) : Canvas(dimensions) {}

void CanvasGL::defaultGLState() {
    if (!OpenGLCapabilities::hasSupportedOpenGLVersion()) return;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

void CanvasGL::render(std::shared_ptr<const Image> image, LayerType layerType, size_t idx) {
    image_ = image;
    layerType_ = layerType;
    pickingController_.setPickingSource(image);
    if (image_) {
        imageGL_ = image_->getRepresentation<ImageGL>();
        activeRenderLayerIdx_ = (imageGL_->getLayerGL(layerType_, idx) != nullptr) ? idx : 0;
    } else {
        imageGL_ = nullptr;
        activeRenderLayerIdx_ = 0;
    }
    update();
}

void CanvasGL::resize(size2_t size) {
    imageGL_ = nullptr;
    pickingController_.setPickingSource(nullptr);
    Canvas::resize(size);
}

void CanvasGL::update() { renderLayer(activeRenderLayerIdx_); }

void CanvasGL::renderLayer(size_t idx) {
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

        // Setup the GL state for the canvas, only need to do this once, since this
        // context will only be used to render the canvas on screen.
        // All other computation is done in the hidden contexts, which should never
        // call this function.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        square_ = utilgl::planeRect();
        squareGL_ = square_->getRepresentation<MeshGL>();
        textureShader_ = SharedOpenGLResources::getPtr()->getTextureShader();
        noiseShader_ = SharedOpenGLResources::getPtr()->getNoiseShader();
        return true;
    } else {
        return false;
    }
}

void CanvasGL::renderNoise() {
    if (!ready()) return;

    glViewport(0, 0, static_cast<GLsizei>(getCanvasDimensions().x),
               static_cast<GLsizei>(getCanvasDimensions().y));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    noiseShader_->activate();
    drawSquare();
    noiseShader_->deactivate();
    glSwapBuffers();
}

void CanvasGL::renderTexture(int unitNumber) {
    if (!ready()) return;

    glViewport(0, 0, static_cast<GLsizei>(getCanvasDimensions().x),
               static_cast<GLsizei>(getCanvasDimensions().y));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    textureShader_->activate();
    textureShader_->setUniform("tex_", unitNumber);
    drawSquare();
    textureShader_->deactivate();
    glSwapBuffers();
}

void CanvasGL::drawSquare() {
    squareGL_->enable();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    squareGL_->disable();
}

double CanvasGL::getDepthValueAtCoord(ivec2 coord) const {
    const dvec2 canvasDims{getCanvasDimensions() - size2_t(1)};
    return getDepthValueAtNormalizedCoord(dvec2(coord) / canvasDims);
}

double CanvasGL::getDepthValueAtNormalizedCoord(dvec2 normalizedScreenCoordinate) const {
    const dvec2 coords = glm::clamp(normalizedScreenCoordinate, dvec2(0.0), dvec2(1.0));

    if (imageGL_) {
        RenderContext::getPtr()->activateDefaultRenderContext();
        const dvec2 depthDims{imageGL_->getDimensions() - size2_t(1, 1)};
        const size2_t coordDepth{depthDims * coords};
        auto depth = imageGL_->readPixel(coordDepth, LayerType::Depth).x;

        // Convert to normalized device coordinates
        return 2.0 * depth - 1.0;
    }

    // RAM fallback
    if (!image_) return 1.0;
    auto depthLayer = image_->getDepthLayer();
    if (!depthLayer) return 1.0;
    auto depthLayerRAM = depthLayer->getRepresentation<LayerRAM>();
    if (!depthLayerRAM) return 1.0;

    const dvec2 depthDims{depthLayerRAM->getDimensions() - size2_t(1, 1)};
    const size2_t coordDepth{depthDims * coords};
    const double depthValue = depthLayerRAM->getAsNormalizedDouble(coordDepth);

    // Convert to normalized device coordinates
    return 2.0 * depthValue - 1.0;
}

void CanvasGL::setProcessorWidgetOwner(ProcessorWidget* widget) {
    // Clear internal state
    image_.reset();
    imageGL_ = nullptr;
    pickingController_.setPickingSource(nullptr);
    Canvas::setProcessorWidgetOwner(widget);
}

size2_t CanvasGL::getImageDimensions() const {
    return image_ ? image_->getDimensions() : size2_t(0, 0);
}

}  // namespace inviwo
