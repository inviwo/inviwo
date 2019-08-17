/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <inviwo/core/util/zip.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

TextureQuadRenderer::TextureQuadRenderer() : shader_(getDefaultShader()) {}

TextureQuadRenderer::TextureQuadRenderer(const Shader &shader)
    : shader_(std::make_shared<Shader>(shader)) {}

TextureQuadRenderer::TextureQuadRenderer(Shader &&shader)
    : shader_(std::make_shared<Shader>(std::move(shader))) {}

TextureQuadRenderer::~TextureQuadRenderer() = default;

Shader &TextureQuadRenderer::getShader() { return *shader_; }

const Shader &TextureQuadRenderer::getShader() const { return *shader_; }

std::shared_ptr<Shader> TextureQuadRenderer::getDefaultShader() {
    static std::weak_ptr<Shader> shader_;

    if (auto shader = shader_.lock()) {
        return shader;
    } else {
        shader = std::make_shared<Shader>("rendertexturequad.vert", "rendertexturequad.frag");
        shader_ = shader;
        return shader;
    }
}

void TextureQuadRenderer::render(const Image &image, const ivec2 &pos, const size2_t &canvasSize,
                                 LayerType layerType, const mat4 &transformation,
                                 const mat4 &texTransform) {
    if (auto layer = image.getLayer(layerType)) {
        render(layer->getRepresentation<LayerGL>()->getTexture(), pos, canvasSize, transformation,
               texTransform);
    }
}

void TextureQuadRenderer::render(const Image &image, const std::vector<ivec2> &pos,
                                 const std::vector<mat4> &texTransform, const size2_t &canvasSize,
                                 LayerType layerType, const mat4 &transformation) {
    if (auto layer = image.getLayer(layerType)) {
        render(layer->getRepresentation<LayerGL>()->getTexture(), pos, texTransform, canvasSize,
               transformation);
    }
}

void TextureQuadRenderer::render(const std::shared_ptr<Image> &image, const ivec2 &pos,
                                 const size2_t &canvasSize, LayerType layerType,
                                 const mat4 &transformation, const mat4 &texTransform) {
    if (image) {
        if (auto layer = image->getLayer(layerType)) {
            render(layer->getRepresentation<LayerGL>()->getTexture(), pos, canvasSize,
                   transformation, texTransform);
        }
    }
}

void TextureQuadRenderer::render(const std::shared_ptr<Image> &image, const std::vector<ivec2> &pos,
                                 const std::vector<mat4> &texTransform, const size2_t &canvasSize,
                                 LayerType layerType, const mat4 &transformation) {
    if (image) {
        if (auto layer = image->getLayer(layerType)) {
            render(layer->getRepresentation<LayerGL>()->getTexture(), pos, texTransform, canvasSize,
                   transformation);
        }
    }
}

void TextureQuadRenderer::render(const Image &image, const ivec2 &pos, const size2_t &canvasSize,
                                 std::size_t colorLayerIndex, const mat4 &transformation,
                                 const mat4 &texTransform) {
    if (auto layer = image.getLayer(LayerType::Color, colorLayerIndex)) {
        render(layer->getRepresentation<LayerGL>()->getTexture(), pos, canvasSize, transformation,
               texTransform);
    }
}

void TextureQuadRenderer::render(const Image &image, const std::vector<ivec2> &pos,
                                 const std::vector<mat4> &texTransform, const size2_t &canvasSize,
                                 std::size_t colorLayerIndex, const mat4 &transformation) {
    if (auto layer = image.getLayer(LayerType::Color, colorLayerIndex)) {
        render(layer->getRepresentation<LayerGL>()->getTexture(), pos, texTransform, canvasSize,
               transformation);
    }
}

void TextureQuadRenderer::render(const std::shared_ptr<Image> &image, const ivec2 &pos,
                                 const size2_t &canvasSize, std::size_t colorLayerIndex,
                                 const mat4 &transformation, const mat4 &texTransform) {
    if (image) {
        if (auto layer = image->getLayer(LayerType::Color, colorLayerIndex)) {
            render(layer->getRepresentation<LayerGL>()->getTexture(), pos, canvasSize,
                   transformation, texTransform);
        }
    }
}

void TextureQuadRenderer::render(const std::shared_ptr<Image> &image, const std::vector<ivec2> &pos,
                                 const std::vector<mat4> &texTransform, const size2_t &canvasSize,
                                 std::size_t colorLayerIndex, const mat4 &transformation) {
    if (image) {
        if (auto layer = image->getLayer(LayerType::Color, colorLayerIndex)) {
            render(layer->getRepresentation<LayerGL>()->getTexture(), pos, texTransform, canvasSize,
                   transformation);
        }
    }
}

void TextureQuadRenderer::render(const Layer &layer, const ivec2 &pos, const size2_t &canvasSize,
                                 const mat4 &transformation, const mat4 &texTransform) {
    render(layer.getRepresentation<LayerGL>()->getTexture(), pos, canvasSize, transformation,
           texTransform);
}

void TextureQuadRenderer::render(const Layer &layer, const std::vector<ivec2> &pos,
                                 const std::vector<mat4> &texTransform, const size2_t &canvasSize,
                                 const mat4 &transformation) {
    render(layer.getRepresentation<LayerGL>()->getTexture(), pos, texTransform, canvasSize,
           transformation);
}

void TextureQuadRenderer::render(const std::shared_ptr<Layer> &layer, const ivec2 &pos,
                                 const size2_t &canvasSize, const mat4 &transformation,
                                 const mat4 &texTransform) {
    if (layer) {
        render(layer->getRepresentation<LayerGL>()->getTexture(), pos, canvasSize, transformation,
               texTransform);
    }
}

void TextureQuadRenderer::render(const std::shared_ptr<Layer> &layer, const std::vector<ivec2> &pos,
                                 const std::vector<mat4> &texTransform, const size2_t &canvasSize,
                                 const mat4 &transformation) {
    if (layer) {
        render(layer->getRepresentation<LayerGL>()->getTexture(), pos, texTransform, canvasSize,
               transformation);
    }
}

void TextureQuadRenderer::render(const Texture2D &texture, const ivec2 &pos,
                                 const size2_t &canvasSize, const mat4 &transformation,
                                 const mat4 &texTransform) {
    renderToRect(texture, pos, ivec2(texture.getDimensions()), canvasSize, transformation,
                 texTransform);
}

void TextureQuadRenderer::render(const Texture2D &texture, const std::vector<ivec2> &pos,
                                 const std::vector<mat4> &texTransform, const size2_t &canvasSize,
                                 const mat4 &transformation) {
    renderToRect(texture, pos, std::vector<ivec2>(pos.size(), ivec2(texture.getDimensions())),
                 texTransform, canvasSize, transformation);
}

void TextureQuadRenderer::render(const std::shared_ptr<Texture2D> &texture, const ivec2 &pos,
                                 const size2_t &canvasSize, const mat4 &transformation,
                                 const mat4 &texTransform) {

    renderToRect(texture, pos, ivec2(texture->getDimensions()), canvasSize, transformation,
                 texTransform);
}

void TextureQuadRenderer::render(const std::shared_ptr<Texture2D> &texture,
                                 const std::vector<ivec2> &pos,
                                 const std::vector<mat4> &texTransform, const size2_t &canvasSize,
                                 const mat4 &transformation) {

    renderToRect(texture, pos, std::vector<ivec2>(pos.size(), ivec2(texture->getDimensions())),
                 texTransform, canvasSize, transformation);
}

void TextureQuadRenderer::renderToRect(const Image &image, const ivec2 &pos, const ivec2 &extent,
                                       const size2_t &canvasSize, LayerType layerType,
                                       const mat4 &transformation, const mat4 &texTransform) {
    if (auto layer = image.getLayer(layerType)) {
        renderToRect(layer->getRepresentation<LayerGL>()->getTexture(), pos, extent, canvasSize,
                     transformation, texTransform);
    }
}

void TextureQuadRenderer::renderToRect(const Image &image, const std::vector<ivec2> &pos,
                                       const std::vector<ivec2> &extent,
                                       const std::vector<mat4> &texTransform,
                                       const size2_t &canvasSize, LayerType layerType,
                                       const mat4 &transformation) {
    if (auto layer = image.getLayer(layerType)) {
        renderToRect(layer->getRepresentation<LayerGL>()->getTexture(), pos, extent, texTransform,
                     canvasSize, transformation);
    }
}

void TextureQuadRenderer::renderToRect(const std::shared_ptr<Image> &image, const ivec2 &pos,
                                       const ivec2 &extent, const size2_t &canvasSize,
                                       LayerType layerType, const mat4 &transformation,
                                       const mat4 &texTransform) {
    if (image) {
        if (auto layer = image->getLayer(layerType)) {
            renderToRect(layer->getRepresentation<LayerGL>()->getTexture(), pos, extent, canvasSize,
                         transformation, texTransform);
        }
    }
}

void TextureQuadRenderer::renderToRect(const std::shared_ptr<Image> &image,
                                       const std::vector<ivec2> &pos,
                                       const std::vector<ivec2> &extent,
                                       const std::vector<mat4> &texTransform,
                                       const size2_t &canvasSize, LayerType layerType,
                                       const mat4 &transformation) {
    if (image) {
        if (auto layer = image->getLayer(layerType)) {
            renderToRect(layer->getRepresentation<LayerGL>()->getTexture(), pos, extent,
                         texTransform, canvasSize, transformation);
        }
    }
}

void TextureQuadRenderer::renderToRect(const Image &image, const ivec2 &pos, const ivec2 &extent,
                                       const size2_t &canvasSize, std::size_t colorLayerIndex,
                                       const mat4 &transformation, const mat4 &texTransform) {
    if (auto layer = image.getLayer(LayerType::Color, colorLayerIndex)) {
        renderToRect(layer->getRepresentation<LayerGL>()->getTexture(), pos, extent, canvasSize,
                     transformation, texTransform);
    }
}

void TextureQuadRenderer::renderToRect(const Image &image, const std::vector<ivec2> &pos,
                                       const std::vector<ivec2> &extent,
                                       const std::vector<mat4> &texTransform,
                                       const size2_t &canvasSize, std::size_t colorLayerIndex,
                                       const mat4 &transformation) {
    if (auto layer = image.getLayer(LayerType::Color, colorLayerIndex)) {
        renderToRect(layer->getRepresentation<LayerGL>()->getTexture(), pos, extent, texTransform,
                     canvasSize, transformation);
    }
}

void TextureQuadRenderer::renderToRect(const std::shared_ptr<Image> &image, const ivec2 &pos,
                                       const ivec2 &extent, const size2_t &canvasSize,
                                       std::size_t colorLayerIndex, const mat4 &transformation,
                                       const mat4 &texTransform) {
    if (image) {
        if (auto layer = image->getLayer(LayerType::Color, colorLayerIndex)) {
            renderToRect(layer->getRepresentation<LayerGL>()->getTexture(), pos, extent, canvasSize,
                         transformation, texTransform);
        }
    }
}

void TextureQuadRenderer::renderToRect(const std::shared_ptr<Image> &image,
                                       const std::vector<ivec2> &pos,
                                       const std::vector<ivec2> &extent,
                                       const std::vector<mat4> &texTransform,
                                       const size2_t &canvasSize, std::size_t colorLayerIndex,
                                       const mat4 &transformation) {
    if (image) {
        if (auto layer = image->getLayer(LayerType::Color, colorLayerIndex)) {
            renderToRect(layer->getRepresentation<LayerGL>()->getTexture(), pos, extent,
                         texTransform, canvasSize, transformation);
        }
    }
}

void TextureQuadRenderer::renderToRect(const Layer &layer, const ivec2 &pos, const ivec2 &extent,
                                       const size2_t &canvasSize, const mat4 &transformation,
                                       const mat4 &texTransform) {
    renderToRect(layer.getRepresentation<LayerGL>()->getTexture(), pos, extent, canvasSize,
                 transformation, texTransform);
}

void TextureQuadRenderer::renderToRect(const Layer &layer, const std::vector<ivec2> &pos,
                                       const std::vector<ivec2> &extent,
                                       const std::vector<mat4> &texTransform,
                                       const size2_t &canvasSize, const mat4 &transformation) {
    renderToRect(layer.getRepresentation<LayerGL>()->getTexture(), pos, extent, texTransform,
                 canvasSize, transformation);
}

void TextureQuadRenderer::renderToRect(const std::shared_ptr<Layer> &layer, const ivec2 &pos,
                                       const ivec2 &extent, const size2_t &canvasSize,
                                       const mat4 &transformation, const mat4 &texTransform) {
    if (layer) {
        renderToRect(layer->getRepresentation<LayerGL>()->getTexture(), pos, extent, canvasSize,
                     transformation, texTransform);
    }
}

void TextureQuadRenderer::renderToRect(const std::shared_ptr<Layer> &layer,
                                       const std::vector<ivec2> &pos,
                                       const std::vector<ivec2> &extent,
                                       const std::vector<mat4> &texTransform,
                                       const size2_t &canvasSize, const mat4 &transformation) {
    if (layer) {
        renderToRect(layer->getRepresentation<LayerGL>()->getTexture(), pos, extent, texTransform,
                     canvasSize, transformation);
    }
}

void TextureQuadRenderer::renderToRect(const Texture2D &texture, const ivec2 &pos,
                                       const ivec2 &extent, const size2_t &canvasSize,
                                       const mat4 &transformation, const mat4 &texTransform) {
    renderToRect(texture, {pos}, {extent}, {texTransform}, canvasSize, transformation);
}

void TextureQuadRenderer::renderToRect(const std::shared_ptr<Texture2D> &texture,
                                       const std::vector<ivec2> &pos,
                                       const std::vector<ivec2> &extent,
                                       const std::vector<mat4> &texTransform,
                                       const size2_t &canvasSize, const mat4 &transformation) {
    if (texture) {
        renderToRect(*texture, pos, extent, texTransform, canvasSize, transformation);
    }
}

void TextureQuadRenderer::renderToRect(const std::shared_ptr<Texture2D> &texture, const ivec2 &pos,
                                       const ivec2 &extent, const size2_t &canvasSize,
                                       const mat4 &transformation, const mat4 &texTransform) {
    if (texture) {
        renderToRect(texture, {pos}, {extent}, {texTransform}, canvasSize, transformation);
    }
}

void TextureQuadRenderer::renderToRect(const Texture2D &texture, const std::vector<ivec2> &pos,
                                       const std::vector<ivec2> &extent,
                                       const std::vector<mat4> &texTransform,
                                       const size2_t &canvasSize, const mat4 &transformation) {
    utilgl::DepthFuncState depth(GL_ALWAYS);

    TextureUnit texUnit;
    texUnit.activate();
    texture.bind();

    shader_->activate();
    shader_->setUniform("tex", texUnit);

    auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
    utilgl::Enable<MeshGL> enable(rect);

    // scaling factor from screen coords to normalized dev coords
    const vec2 scaling(vec2(2.0f) / vec2(canvasSize));

    for (auto &&elem : util::zip(pos, extent, texTransform)) {
        const auto p = vec2{elem.first()} * scaling;
        const auto ext = vec2{elem.second()};

        const auto dataToWorld = glm::translate(vec3{-1.0f + p.x, -1.0f + p.y, -1.0f}) *
                                 glm::scale(vec3{scaling, 1.f}) * transformation *
                                 glm::scale(vec3{ext, 1.f});

        shader_->setUniform("geometry_.dataToWorld", dataToWorld);
        shader_->setUniform("texCoordTransform", elem.third());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    shader_->deactivate();
}
void TextureQuadRenderer::renderToRect(const Texture2D &texture,
                                       const std::vector<ivec2> &positions,
                                       const std::vector<ivec2> &extent,
                                       const std::vector<mat4> &texTransform,
                                       const size2_t &canvasSize,
                                       const std::vector<mat4> &transformations) {
    utilgl::DepthFuncState depth(GL_ALWAYS);

    TextureUnit texUnit;
    texUnit.activate();
    texture.bind();

    shader_->activate();
    shader_->setUniform("tex", texUnit);

    auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
    utilgl::Enable<MeshGL> enable(rect);

    // scaling factor from screen coords to normalized dev coords
    const vec2 scaling(vec2(2.0f) / vec2(canvasSize));
    const auto toNDC = glm::translate(vec3(-1.0f, -1.0f, -1.0f)) * glm::scale(vec3{scaling, 1.f});

    for (auto &&elem : util::zip(positions, extent, texTransform, transformations)) {
        const auto ext = vec2{elem.second()};
        const auto pos = vec3{elem.first().x, elem.first().y, 0.0f};

        const auto dataToWorld =
            toNDC * glm::translate(pos) * get<3>(elem) * glm::scale(vec3{ext, 1.f});

        shader_->setUniform("geometry_.dataToWorld", dataToWorld);
        shader_->setUniform("texCoordTransform", elem.third());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    shader_->deactivate();
}

void TextureQuadRenderer::renderToRect3D(const Camera &camera, const Texture2D &texture,
                                         const vec3 &pos, const ivec2 &extent,
                                         const size2_t &canvasSize, const vec2 &anchor,
                                         const mat4 &transformation, const mat4 &texTransform) {
    renderToRect3D(camera, texture, {pos}, {extent}, {texTransform}, canvasSize, anchor,
                   transformation);
}

void TextureQuadRenderer::renderToRect3D(
    const Camera &camera, const std::shared_ptr<Texture2D> &texture, const std::vector<vec3> &pos,
    const std::vector<ivec2> &extent, const std::vector<mat4> &texTransform,
    const size2_t &canvasSize, const vec2 &anchor, const mat4 &transformation) {
    if (texture) {
        renderToRect3D(camera, *texture, pos, extent, texTransform, canvasSize, anchor,
                       transformation);
    }
}

void TextureQuadRenderer::renderToRect3D(const Camera &camera,
                                         const std::shared_ptr<Texture2D> &texture, const vec3 &pos,
                                         const ivec2 &extent, const size2_t &canvasSize,
                                         const vec2 &anchor, const mat4 &transformation,
                                         const mat4 &texTransform) {
    if (texture) {
        renderToRect3D(camera, texture, {pos}, {extent}, {texTransform}, canvasSize, anchor,
                       transformation);
    }
}

void TextureQuadRenderer::renderToRect3D(const Camera &camera, const Texture2D &texture,
                                         const std::vector<vec3> &pos,
                                         const std::vector<ivec2> &extent,
                                         const std::vector<mat4> &texTransform,
                                         const size2_t &canvasSize, const vec2 &anchor,
                                         const mat4 &transformation) {

    utilgl::DepthFuncState depth(GL_LESS);

    TextureUnit texUnit;
    texUnit.activate();
    texture.bind();

    shader_->activate();
    shader_->setUniform("tex", texUnit);

    auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
    utilgl::Enable<MeshGL> enable(rect);

    // scaling factor from screen coords to normalized dev coords
    const vec2 scaling(vec2(2.0f) / vec2(canvasSize));

    const mat4 viewprojMatrix(camera.getProjectionMatrix() * camera.getViewMatrix());

    for (auto &&elem : util::zip(pos, extent, texTransform)) {
        // transform position from world space into normalized dev coords
        vec4 p = viewprojMatrix * vec4{elem.first(), 1.0f};
        p /= p.w;

        const auto ext = vec2{elem.second()};
        // consider anchor position
        const auto offset = 0.5f * ext * scaling * (anchor + vec2{1.0f, 1.0f});

        // ensure that the lower left position is pixel aligned
        const vec3 origin{
            glm::round(vec2{p.x - offset.x, p.y - offset.y} * vec2{canvasSize} * 0.5f) * scaling,
            p.z};

        const mat4 dataToWorld = glm::translate(origin) * glm::scale(vec3{scaling, 1.f}) *
                                 transformation * glm::scale(vec3{ext, 1.f});

        shader_->setUniform("geometry_.dataToWorld", dataToWorld);
        shader_->setUniform("texCoordTransform", get<2>(elem));

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    shader_->deactivate();
}

}  // namespace inviwo
