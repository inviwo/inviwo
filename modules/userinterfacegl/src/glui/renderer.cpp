/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>                       // for InviwoApplication
#include <inviwo/core/datastructures/buffer/buffer.h>                   // for makeIndexBuffer
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>       // for BufferRAMPrecision
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for ConnectivityType
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::MeshInfo
#include <inviwo/core/datastructures/image/layer.h>                     // IWYU pragma: keep
#include <inviwo/core/datastructures/image/layerram.h>                  // for LayerRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/io/datareader.h>                                  // for DataReaderType
#include <inviwo/core/io/datareaderfactory.h>                           // for DataReaderFactory
#include <inviwo/core/util/exception.h>                                 // for Exception
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                    // for vec2, vec4, size2_t
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT
#include <inviwo/core/util/stringconversion.h>                          // for joinString
#include <inviwo/core/util/zip.h>                                       // for enumerate, zipIte...
#include <modules/fontrendering/textrenderer.h>                         // for TextRenderer
#include <modules/fontrendering/util/fontutils.h>                       // for getFont, FontType
#include <modules/opengl/glformats.h>                                   // for GLFormat, GLFormats
#include <modules/opengl/inviwoopengl.h>                                // for GLsizei, GL_LINEAR
#include <modules/opengl/rendering/meshdrawergl.h>                      // for MeshDrawerGL
#include <modules/opengl/rendering/texturequadrenderer.h>               // for TextureQuadRenderer
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/texture/texture2darray.h>                      // for Texture2DArray
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnit

#include <algorithm>                                                    // for all_of
#include <sstream>                                                      // for basic_stringbuf<>...
#include <string_view>                                                  // for string_view
#include <type_traits>                                                  // for remove_extent_t
#include <unordered_map>                                                // for unordered_map
#include <unordered_set>                                                // for unordered_set
#include <utility>                                                      // for move, pair

#include <glm/vec2.hpp>                                                 // for vec, operator==
#include <glm/vector_relational.hpp>                                    // for any, notEqual

namespace inviwo {

namespace glui {

Renderer::Renderer()
    : uiShader_("renderui.vert", "renderui.frag")
    , textRenderer_(font::getFont(font::FontType::Default, font::FullPath::Yes))
    , textRendererBold_(font::getFont(font::FontType::Bold, font::FullPath::Yes))
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
    rectangleMesh_->addBuffer(BufferType::TexCoordAttrib, texCoordsBuffer);

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
            auto layer = reader->readData(sourcePath + "/" + filename);
            textureLayers.push_back(layer);
        }
    } else {
        throw Exception("Could not find a data reader for texture data (png).", IVW_CONTEXT);
    }

    // Check that all textures has the same dimensions
    const size2_t texDim = textureLayers.front()->getDimensions();
    if (!std::all_of(textureLayers.begin(), textureLayers.end(),
                     [&](const auto& layer) { return layer->getDimensions() == texDim; })) {
        throw Exception("Textures have inconsistent sizes: " + joinString(textureFiles, ", "),
                        IVW_CONTEXT);
    }
    const DataFormatBase* const dataformat = textureLayers.front()->getDataFormat();
    if (!std::all_of(textureLayers.begin(), textureLayers.end(),
                     [&](const auto& layer) { return layer->getDataFormat() == dataformat; })) {
        throw Exception("Textures have inconsistent formats: " + joinString(textureFiles, ", "),
                        IVW_CONTEXT);
    }

    // upload the individual textures, rescale where necessary
    auto texture = std::make_shared<Texture2DArray>(size3_t(texDim, textureLayers.size()), GL_RGBA,
                                                    GL_RGBA8, GL_UNSIGNED_BYTE, GL_LINEAR);
    texture->initialize(nullptr);

    TextureUnit texUnit;
    texUnit.activate();
    texture->bind();
    for (auto [zIndex, texLayer] : util::enumerate(textureLayers)) {
        auto layerRAM = texLayer->getRepresentation<LayerRAM>();
        auto glformat = GLFormats::get(layerRAM->getDataFormat()->getId());
        // upload data into array texture
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<GLsizei>(zIndex),
                        static_cast<GLsizei>(texDim.x), static_cast<GLsizei>(texDim.y), 1,
                        glformat.format, glformat.type, layerRAM->getData());
    }
    return texture;
}

}  // namespace glui

}  // namespace inviwo
