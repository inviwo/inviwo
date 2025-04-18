/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh
#include <inviwo/core/datastructures/image/image.h>                     // IWYU pragma: keep
#include <inviwo/core/datastructures/image/imagetypes.h>                // for LayerType, LayerT...
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/interaction/pickingcontroller.h>                  // for PickingController
#include <inviwo/core/util/canvas.h>                                    // for Canvas
#include <inviwo/core/util/glmvec.h>                                    // for size2_t, dvec2
#include <modules/opengl/debugmessages.h>                               // for handleOpenGLDebug...
#include <modules/opengl/geometry/meshgl.h>                             // for MeshGL
#include <modules/opengl/image/layergl.h>                               // for LayerGL
#include <modules/opengl/inviwoopengl.h>                                // for glEnable, glClear
#include <modules/opengl/openglcapabilities.h>                          // for OpenGLCapabilities
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/sharedopenglresources.h>                       // for SharedOpenGLResou...
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnit
#include <modules/opengl/texture/textureutils.h>                        // for planeRect

#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set

#include <glm/common.hpp>  // for clamp
#include <glm/vec2.hpp>    // for operator-, vec<>:...
#include <glm/vec4.hpp>    // for vec<>::(anonymous)

namespace inviwo {

CanvasGL::CanvasGL() : Canvas() {}

void CanvasGL::defaultGLState() {
    if (!OpenGLCapabilities::hasSupportedOpenGLVersion()) return;

    LGL_ERROR;
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    LGL_ERROR;
}

void CanvasGL::render(std::shared_ptr<const Image> image, LayerType layerType, size_t idx) {
    image_ = image;
    layerType_ = layerType;
    pickingController_.setPickingSource(image);
    layerIdx_ = idx;
    update();
}

void CanvasGL::update() { renderLayer(); }

void CanvasGL::setupDebug() { utilgl::handleOpenGLDebugMode(activeContext()); }

void CanvasGL::renderLayer() {
    if (auto image = image_.lock()) {
        if (auto layer = image->getLayer(layerType_, layerIdx_)) {

            if (auto layerGL = layer->getRepresentation<LayerGL>()) {
                TextureUnit textureUnit;
                layerGL->bindTexture(textureUnit.getEnum());
                renderTexture(textureUnit.getUnitNumber());
                layerGL->unbindTexture();
                return;
            }
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

    if (auto image = image_.lock()) {
        const dvec2 depthDims{image->getDimensions() - size2_t(1, 1)};
        const size2_t coordDepth{depthDims * coords};
        const auto depth = image->readPixel(coordDepth, LayerType::Depth).x;
        // Convert to normalized device coordinates
        return 2.0 * depth - 1.0;
    }
    return 1.0;
}

size2_t CanvasGL::getImageDimensions() const {
    if (auto image = image_.lock()) {
        return image->getDimensions();
    } else {
        return {0, 0};
    }
}

}  // namespace inviwo
