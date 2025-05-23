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

#pragma once

#include <modules/opengl/openglmoduledefine.h>  // for IVW_MODULE_OPENGL_API

#include <inviwo/core/datastructures/geometry/mesh.h>     // for Mesh
#include <inviwo/core/datastructures/image/imagetypes.h>  // for LayerType, LayerType::Color
#include <inviwo/core/util/canvas.h>                      // for Canvas
#include <inviwo/core/util/glmvec.h>                      // for size2_t, dvec2, ivec2

#include <cstddef>  // for size_t
#include <memory>   // for shared_ptr, unique_ptr, weak_ptr

namespace inviwo {

class Image;
class MeshGL;
class Shader;

class IVW_MODULE_OPENGL_API CanvasGL : public Canvas {
public:
    CanvasGL();
    virtual ~CanvasGL() = default;

    static void defaultGLState();

    virtual void render(std::shared_ptr<const Image> image, LayerType layerType = LayerType::Color,
                        size_t idx = 0) override;

    virtual void glSwapBuffers() = 0;
    virtual void update() override;

    virtual size2_t getCanvasDimensions() const = 0;
    virtual size2_t getImageDimensions() const;
    /**
     * \brief Retrieve depth value in normalized device coordinates at screen coordinate.
     *
     * Depth is defined in [-1 1], where -1 is the near plane and 1 is the far plane.
     * Will be 1 if no depth value is available.
     *
     * @param canvasCoordinate Canvas coordinates [0 dim-1]^2
     * @return NDC depth in [-1 1], 1 if no depth value exist.
     */
    double getDepthValueAtCoord(ivec2 canvasCoordinate) const;
    double getDepthValueAtNormalizedCoord(dvec2 normalizedScreenCoordinate) const;

protected:
    void setupDebug();

    void renderLayer();
    void renderNoise();

    void drawSquare();

    void renderTexture(int);

    std::weak_ptr<const Image> image_;
    std::unique_ptr<Mesh> square_;
    /**
     * Each canvas must have its own MeshGL
     * since the vertex array object in MeshGL cannot be shared.
     */
    const MeshGL* squareGL_ = nullptr;  ///< Non-owning reference.

    /**
     * Sometime on OSX in renderNoise when on the first time using
     * a canvas we get a INVALID_FRAMEBUFFER_OPERATION error
     * to avoid this we have this ready flag to check that the
     * frame buffer is complete.
     */
    bool ready();
    bool ready_ = false;

    LayerType layerType_ = LayerType::Color;
    size_t layerIdx_ = 0;

    Shader* textureShader_ = nullptr;  ///< non-owning reference
    Shader* noiseShader_ = nullptr;    ///< non-owning reference
};

}  // namespace inviwo
