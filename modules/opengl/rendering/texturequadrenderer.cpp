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

#include <modules/opengl/rendering/texturequadrenderer.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

TextureQuadRenderer::TextureQuadRenderer()
    : shader_("rendertexturequad.vert", "rendertexturequad.frag") {}

TextureQuadRenderer::~TextureQuadRenderer() = default;

void TextureQuadRenderer::render(std::shared_ptr<Image> image, ivec2 pos, size2_t canvasSize,
                                 LayerType layerType) {
    if (image) {
        if (auto layer = image->getLayer(layerType)) {
            render(layer->getRepresentation<LayerGL>()->getTexture(), pos, canvasSize);
        }
    }
}

void TextureQuadRenderer::render(std::shared_ptr<Image> image, ivec2 pos, size2_t canvasSize,
                                 std::size_t colorLayerIndex) {
    if (image) {
        if (auto layer = image->getLayer(LayerType::Color, colorLayerIndex)) {
            render(layer->getRepresentation<LayerGL>()->getTexture(), pos, canvasSize);
        }
    }
}

void TextureQuadRenderer::render(std::shared_ptr<Layer> layer, ivec2 pos, size2_t canvasSize) {
    if (layer) {
        render(layer->getRepresentation<LayerGL>()->getTexture(), pos, canvasSize);
    }
}

void TextureQuadRenderer::render(std::shared_ptr<Texture2D> texture, ivec2 pos,
                                 size2_t canvasSize) {
    // scaling factor from screen coords to normalized dev coords
    vec2 scaling(vec2(2.0f) / vec2(canvasSize));
    vec2 position(vec2(pos) * scaling);

    TextureUnit texUnit;
    texUnit.activate();
    texture->bind();
    shader_.activate();
    auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
    utilgl::Enable<MeshGL> enable(rect);
    utilgl::DepthFuncState depth(GL_ALWAYS);

    shader_.setUniform("tex", texUnit);
    shader_.setUniform("geometry_.dataToWorld",
                       glm::translate(vec3(-1.0f + position.x, -1.0f + position.y, 0.0f)) *
                           glm::scale(vec3(scaling * vec2(texture->getDimensions()), 1.f)));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glScissor(0, 0, 0, 1);

    shader_.deactivate();
}

}  // namespace inviwo
