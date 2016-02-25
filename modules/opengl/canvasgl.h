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

#ifndef IVW_CANVASGL_H
#define IVW_CANVASGL_H

#include <inviwo/core/util/canvas.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/datastructures/geometry/mesh.h>

namespace inviwo {

class ImageGL;
class LayerRAM;
class MeshGL;
class BufferObjectArray;
class ProcessorWidget;

class IVW_MODULE_OPENGL_API CanvasGL : public Canvas {
public:
    CanvasGL(uvec2 dimensions);
    virtual ~CanvasGL() = default;

    static void defaultGLState();

    virtual void render(std::shared_ptr<const Image> image, LayerType layerType = LayerType::Color,
                        size_t idx = 0) override;
    virtual void resize(uvec2 size) override;
    virtual void glSwapBuffers() = 0;
    virtual void update() override;

    virtual void setProcessorWidgetOwner(ProcessorWidget*) override;

     /**
     * \brief Get depth layer RAM representation. Will return nullptr if depth layer does not exist.
     * @return Depth layer RAM representation if existing, nullptr otherwise.
     */
    const LayerRAM* getDepthLayerRAM() const;
    /**
     * \brief Retrieve depth value in normalized device coordinates at screen coordinate.
     *
     * Depth is defined in [-1 1], where -1 is the near plane and 1 is the far plane.
     * Will be 1 if no depth value is available.
     *
     * @param screenCoordinate Screen coordinates [0 dim-1]^2
     * @return NDC depth in [-1 1], 1 if no depth value exist.
     */
    double getDepthValueAtCoord(ivec2 screenCoordinate,
                                const LayerRAM* depthLayerRAM = nullptr) const;

protected:
    void renderLayer(size_t idx = 0);
    void renderNoise();

    void drawSquare();

    void renderTexture(int);

    void checkChannels(std::size_t);

    std::shared_ptr<const Image> image_;
    const ImageGL* imageGL_;
    std::unique_ptr<Mesh> square_;
    /**
     * Each canvas must have its own MeshGL
     * since QT uses a context per canvas
     * and the vertex array in MeshGL cannot be shared.
     */
    const MeshGL* squareGL_ = nullptr; ///< Non-owning reference.
private:
    /**
     * Sometime on OSX in renderNoise when on the first time using
     * a canvas we get a INVALID_FRAMEBUFFER_OPERATION error
     * to avoid this we have this ready flag to check that the 
     * frame buffer is complete.
     */
    bool ready();
    bool ready_ = false;

    Shader* textureShader();
    Shader* noiseShader();

    LayerType layerType_;
    std::unique_ptr<Shader> textureShader_;
    std::unique_ptr<Shader> noiseShader_;


    size_t channels_;
    size_t previousRenderedLayerIdx_;
};

}  // namespace

#endif  // IVW_CANVASGL_H
