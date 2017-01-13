/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_TEXTUREQUADRENDERER_H
#define IVW_TEXTUREQUADRENDERER_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/shader/shader.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/imagetypes.h>

namespace inviwo {
class Image;
class Layer;
class Texture2D;

/**
 * \class TextureQuadRenderer
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_OPENGL_API TextureQuadRenderer { 
public:
    TextureQuadRenderer();
    virtual ~TextureQuadRenderer();

    /**
     * \brief renders an image at position pos onto the current canvas. The image
     * dimensions determine the covered area in pixel. The anchor point of the
     * image is in the lower left corner. By default, the first color layer is rendered.
     *
     * @param image   input image which is to be rendered onto the current render target
     * @param pos     position of lower left corner in screen space coordinates
     * @param canvasSize   dimensions of the current render target
     * @param layerType  defines which layer of the input image will be rendered, i.e. Color, Depth,
     *                   or Picking
     */
    void render(std::shared_ptr<Image> image, ivec2 pos, size2_t canvasSize,
                LayerType layerType = LayerType::Color);

    /**
    * \brief renders a color layer of an image at position pos onto the current canvas. The layer
    * dimensions determine the covered area in pixel. The anchor point of the image is in the lower
    * left corner. By default, the first color layer is rendered.
    *
    * @param image   input image which is to be rendered onto the current render target
    * @param pos     position of lower left corner in screen space coordinates
    * @param canvasSize   dimensions of the current render target
    * @param colorLayerIndex defines which color layer of the input image will be rendered.
    */
    void render(std::shared_ptr<Image> image, ivec2 pos, size2_t canvasSize,
                std::size_t colorLayerIndex);

    /**
    * \brief renders a layer at position pos onto the current canvas. The layer
    * dimensions determine the covered area in pixel. The anchor point of the
    * image is in the lower left corner. 
    *
    * @param layer   layer which is to be rendered onto the current render target
    * @param pos     position of lower left corner in screen space coordinates
    * @param canvasSize   dimensions of the current render target
    */
    void render(std::shared_ptr<Layer> layer, ivec2 pos, size2_t canvasSize);

    /**
    * \brief renders a texture at position pos onto the current canvas. The texture
    * dimensions determine the covered area in pixel. The anchor point of the
    * texture is in the lower left corner.
    *
    * @param texture   texture which is to be rendered onto the current render target
    * @param pos     position of lower left corner in screen space coordinates
    * @param canvasSize   dimensions of the current render target
    */
    void render(std::shared_ptr<Texture2D> texture, ivec2 pos, size2_t canvasSize);

private:
    Shader shader_;
};

} // namespace inviwo

#endif // IVW_TEXTUREQUADRENDERER_H
