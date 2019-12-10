/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/userinterfacegl/glui/renderer.h>
#include <modules/userinterfacegl/userinterfaceglmodule.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/filesystem.h>

#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <modules/opengl/buffer/bufferobjectarray.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/texture2darray.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/glformats.h>
#include <modules/opengl/shader/shaderutils.h>

#include <modules/fontrendering/util/fontutils.h>

#include <modules/cimg/cimgutils.h>

namespace inviwo {

namespace glui {

Renderer::Renderer()
    : uiShader_("renderui.vert", "renderui.frag")
    , textRenderer_(util::getDefaultFontPath() + "/OpenSans-Semibold.ttf")
    , textRendererBold_(util::getDefaultFontPath() + "/OpenSans-Bold.ttf")
    , quadRenderer_(Shader("rendertexturequad.vert", "labelui.frag"))
    , colorUI_(0.8f, 0.8f, 0.8f, 1.0f)
    , colorSecondaryUI_(0.2f, 0.2f, 0.25f, 1.0f)
    , colorBorder_(0.0f, 0.0f, 0.0f, 1.0f)
    , colorText_(0.0f, 0.0f, 0.0f, 1.0f)
    , colorHover_(0.0f, 0.0f, 0.0f, 1.0f)
    , colorDisabled_(0.4f, 0.4f, 0.4f, 1.0f) {
    textRenderer_.setFontSize(defaultFontSize_);
    textRendererBold_.setFontSize(defaultFontSize_);

    setupRectangleMesh();
}

Texture2DArray* Renderer::createUITextures(const std::string& name,
                                           const std::vector<std::string>& files,
                                           const std::string& sourcePath) {
    if (auto textures = getUITextures(name)) {
        return textures;
    }
    auto textures = createUITextureObject(files, sourcePath);
    uiTextureMap_.insert({name, textures});
    return textures.get();
}

const Shader& Renderer::getShader() const { return uiShader_; }

Shader& Renderer::getShader() { return uiShader_; }

const TextRenderer& Renderer::getTextRenderer(bool bold) const {
    if (bold) {
        return textRendererBold_;
    } else {
        return textRenderer_;
    }
}

int Renderer::getDefaultFontSize() const { return defaultFontSize_; }

TextRenderer& Renderer::getTextRenderer(bool bold) {
    if (bold)
        return textRendererBold_;
    else
        return textRenderer_;
}

const TextureQuadRenderer& Renderer::getTextureQuadRenderer() const { return quadRenderer_; }

TextureQuadRenderer& Renderer::getTextureQuadRenderer() { return quadRenderer_; }

MeshDrawerGL* Renderer::getMeshDrawer() const { return meshDrawer_.get(); }

Texture2DArray* Renderer::getUITextures(const std::string& name) const {
    auto it = uiTextureMap_.find(name);
    if (it != uiTextureMap_.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

void Renderer::setTextColor(const vec4& color) {
    if (glm::any(glm::notEqual(color, colorText_))) {
        colorText_ = color;
        auto& shader = quadRenderer_.getShader();
        shader.activate();
        shader.setUniform("uiColor", color);
    }
}

const vec4& Renderer::getTextColor() const { return colorText_; }

void Renderer::setUIColor(const vec4& color) { colorUI_ = color; }

const vec4& Renderer::getUIColor() const { return colorUI_; }

void Renderer::setSecondaryUIColor(const vec4& color) { colorSecondaryUI_ = color; }

const vec4& Renderer::getSecondaryUIColor() const { return colorSecondaryUI_; }

void Renderer::setBorderColor(const vec4& color) { colorBorder_ = color; }

const vec4& Renderer::getBorderColor() const { return colorBorder_; }

void Renderer::setHoverColor(const vec4& color) { colorHover_ = color; }

const vec4& Renderer::getHoverColor() const { return colorHover_; }

void Renderer::setDisabledColor(const vec4& color) { colorDisabled_ = color; }

const vec4& Renderer::getDisabledColor() const { return colorDisabled_; }

void Renderer::setupRectangleMesh() {
    // set up mesh for drawing a single quad from (0,0) to (1,1) with subdivisions at .45 and
    // .55
    auto verticesBuffer = util::makeBuffer<vec2>({{0.0f, 0.0f},
                                                  {0.45f, 0.0f},
                                                  {0.55f, 0.0f},
                                                  {1.0f, 0.0f},
                                                  {0.0f, 0.45f},
                                                  {0.45f, 0.45f},
                                                  {0.55f, 0.45f},
                                                  {1.0f, 0.45f},
                                                  {0.0f, 0.55f},
                                                  {0.45f, 0.55f},
                                                  {0.55f, 0.55f},
                                                  {1.0f, 0.55f},
                                                  {0.0f, 1.0f},
                                                  {0.45f, 1.0f},
                                                  {0.55f, 1.0f},
                                                  {1.0f, 1.0f}});
    auto texCoordsBuffer = util::makeBuffer<vec2>({{0.0f, 1.0f},
                                                   {0.45f, 1.0f},
                                                   {0.55f, 1.0f},
                                                   {1.0f, 1.0f},
                                                   {0.0f, 0.55f},
                                                   {0.45f, 0.55f},
                                                   {0.55f, 0.55f},
                                                   {1.0f, 0.55f},
                                                   {0.0f, 0.45f},
                                                   {0.45f, 0.45f},
                                                   {0.55f, 0.45f},
                                                   {1.0f, 0.45f},
                                                   {0.0f, 0.0f},
                                                   {0.45f, 0.0f},
                                                   {0.55f, 0.0f},
                                                   {1.0f, 0.0f}});

    rectangleMesh_ = std::make_shared<Mesh>();
    rectangleMesh_->addBuffer(BufferType::PositionAttrib, verticesBuffer);
    rectangleMesh_->addBuffer(BufferType::TexcoordAttrib, texCoordsBuffer);

    // first row
    auto indices = util::makeIndexBuffer({0, 4, 1, 5, 2, 6, 3, 7});
    rectangleMesh_->addIndices(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip),
                               indices);
    // second row
    indices = util::makeIndexBuffer({4, 8, 5, 9, 6, 10, 7, 11});
    rectangleMesh_->addIndices(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip),
                               indices);
    // third row
    indices = util::makeIndexBuffer({8, 12, 9, 13, 10, 14, 11, 15});
    rectangleMesh_->addIndices(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip),
                               indices);

    meshDrawer_ = std::make_shared<MeshDrawerGL>(rectangleMesh_.get());
}

std::shared_ptr<Texture2DArray> Renderer::createUITextureObject(
    const std::vector<std::string>& textureFiles, const std::string& sourcePath) const {
    // read in textures
    std::vector<std::shared_ptr<Layer>> textureLayers;
    auto factory = InviwoApplication::getPtr()->getDataReaderFactory();
    if (auto reader = factory->getReaderForTypeAndExtension<Layer>("png")) {
        for (auto filename : textureFiles) {
            std::shared_ptr<Layer> layer;
            // try to load texture data from current file
            try {
                layer = reader->readData(sourcePath + "/" + filename);
            } catch (DataReaderException const& e) {
                util::log(e.getContext(),
                          "Could not load texture data: " + filename + ", " + e.getMessage(),
                          LogLevel::Error);
            }
            // add current layer, might be a nullptr if the image data could not be loaded
            textureLayers.push_back(layer);
        }
    } else {
        throw Exception("Could not find a data reader for texture data (png).", IVW_CONTEXT);
    }

    // find common texture size, use tex format of first available resource
    const DataFormatBase* dataformat = nullptr;
    size2_t texDim(std::numeric_limits<std::size_t>::max());
    for (auto texLayer : textureLayers) {
        if (texLayer.get() != nullptr) {
            if (!dataformat) {
                dataformat = texLayer->getDataFormat();
            } else if (dataformat->getId() != texLayer->getDataFormat()->getId()) {
                LogWarn("Different image formats used for UI textures.");
            }
            texDim = glm::min(texDim, texLayer->getDimensions());
        }
    }

    // upload the individual textures, rescale where necessary
    auto texture = std::make_shared<Texture2DArray>(size3_t(texDim, textureLayers.size()), GL_RGBA,
                                                    GL_RGBA8, GL_UNSIGNED_BYTE, GL_LINEAR);
    texture->initialize(nullptr);

    TextureUnit texUnit;
    texUnit.activate();
    texture->bind();
    int zIndex = 0;
    for (auto texLayer : textureLayers) {
        if (texLayer.get() != nullptr) {
            const void* data = nullptr;
            auto layerRAM = texLayer->getRepresentation<LayerRAM>();
            std::shared_ptr<LayerRAM> resizedTex;
            if (texLayer->getDimensions() != texDim) {
                // need to resize layer
                resizedTex = createLayerRAM(texDim, LayerType::Color, texLayer->getDataFormat());
                cimgutil::rescaleLayerRamToLayerRam(layerRAM, resizedTex.get());
                data = resizedTex->getData();
            } else {
                data = layerRAM->getData();
            }

            auto glformat = GLFormats::get(layerRAM->getDataFormat()->getId());
            // upload data into array texture
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, zIndex, static_cast<GLsizei>(texDim.x),
                            static_cast<GLsizei>(texDim.y), 1, glformat.format, glformat.type,
                            data);
        }
        ++zIndex;
    }
    return texture;
}

}  // namespace glui

}  // namespace inviwo
