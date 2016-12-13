/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/util/exception.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/buffer/bufferobjectarray.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include "textrenderer.h"

namespace inviwo {

TextRenderer::TextRenderer(const std::string &fontPath)
    : fontface_(nullptr)
    , fontSize_(10)
    , lineSpacing_(0.2)
    , textShader_("fontrendering_freetype.vert", "fontrendering_freetype.frag", true)
{

    if (FT_Init_FreeType(&fontlib_)) {
        throw Exception("Could not initialize FreeType library");
    }

    setFont(fontPath);

    glGenTextures(1, &texCharacter_);

    initMesh();

    fbo_.activate();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    fbo_.deactivate();
}

TextRenderer::~TextRenderer() {
    FT_Done_Face(fontface_);
    glDeleteTextures(1, &texCharacter_);
}

void TextRenderer::setFont(const std::string &fontPath) {
    // free previous font face
    FT_Done_Face(fontface_);
    fontface_ = nullptr;

    int error = FT_New_Face(fontlib_, fontPath.c_str(), 0, &fontface_);
    if (error == FT_Err_Unknown_File_Format) {
        throw Exception(std::string("Unsupported font format: \"") + fontPath + "\"");
    }
    else if (error) {
        throw FileException(std::string("Could not open font file: \"") + fontPath + "\"");
    }

    FT_Set_Pixel_Sizes(fontface_, 0, fontSize_);
}

void TextRenderer::render(const std::string &str, float x, float y, const vec2 &scale,
                          const vec4 &color) {
    TextureUnit texUnit;
    texUnit.activate();

    glBindTexture(GL_TEXTURE_2D, texCharacter_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    textShader_.activate();
    textShader_.setUniform("tex", texUnit.getUnitNumber());
    textShader_.setUniform("color", color);

    // account for baseline offset
    // y += getBaseLineOffset() * scale.y;

    float offset = 0;
    float inputX = x;

    // TODO: To make things more reliable ask the system for proper ascii
    const char lf = (char)0xA;   // Line Feed Ascii for std::endl, \n
    const char tab = (char)0x9;  // Tab Ascii

    for (auto p : str) {
        // load glyph to access metric and bitmap of it
        if (FT_Load_Char(fontface_, p, FT_LOAD_RENDER)) {
            LogWarn("FreeType: could not render char: '" << p << "' (0x" << std::hex
                                                         << static_cast<int>(p) << ")");
            continue;
        }
        float w = fontface_->glyph->bitmap.width * scale.x;
        float h = fontface_->glyph->bitmap.rows * scale.y;

        if (p == lf) {
            offset += getLineHeight() * scale.y;
            x = inputX;
            y += (fontface_->glyph->advance.y >> 6) * scale.y;
            continue;
        } else if (p == tab) {
            x += (fontface_->glyph->advance.x >> 6) * scale.x;
            y += (fontface_->glyph->advance.y >> 6) * scale.y;
            x += (4 * w);  // 4 times glyph character width
            continue;
        }

        // load glyph into texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, fontface_->glyph->bitmap.width,
                     fontface_->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                     fontface_->glyph->bitmap.buffer);

        float x2 = x + fontface_->glyph->bitmap_left * scale.x;
        float y2 = -y - fontface_->glyph->bitmap_top * scale.y;
        y2 += offset;
        // Translate quad to correct position and render
        mesh_->setModelMatrix(glm::translate(vec3(x2, -y2, 0.f)) * glm::scale(vec3(w, -h, 1.f)));
        utilgl::setShaderUniforms(textShader_, *mesh_, "geometry_");

        drawer_->draw();
        x += (fontface_->glyph->advance.x >> 6) * scale.x;
        y += (fontface_->glyph->advance.y >> 6) * scale.y;
    }

    textShader_.deactivate();
}

void TextRenderer::renderToTexture(std::shared_ptr<Texture2D> texture, const std::string &str,
                                   const vec4 &color) {
    // disable depth test and writing depth
    utilgl::DepthMaskState depthMask(GL_FALSE);
    utilgl::GlBoolState depth(GL_DEPTH_TEST, GL_FALSE);

    fbo_.activate();
    if (prevTexture_ != texture) {
        // detach previous texture and attach new texture as a render target, no depth texture
        fbo_.detachTexture(GL_COLOR_ATTACHMENT0);
        fbo_.attachTexture(texture.get(), GL_COLOR_ATTACHMENT0);
        prevTexture_ = texture;
    }

    // set up viewport
    ivec2 dim(texture->getDimensions());
    utilgl::ViewportState viewport(0, 0, dim.x, dim.y);
    glClear(GL_COLOR_BUFFER_BIT);

    // render text into texture
    vec2 scale(2.f / vec2(dim));
    render(str, -1.0f, 1.0f - getBaseLineOffset() * scale.y, scale, color);

    fbo_.deactivate();
}

vec2 TextRenderer::computeTextSize(const std::string &str, const vec2 &scale) {
    return computeTextSize(str) * scale;
}

vec2 TextRenderer::computeTextSize(const std::string &str) {
    float x = 0.0f;
    // calculate height of first line
    // for most fonts descend is negative (see FreeType documentation for details)
    float y = static_cast<float>(getFontAscent() + std::max(-getFontDescent(), 0.0));

    float maxx = 0.0f;
    float maxy = 0.0f;

    const char lf = (char)0xA;   // Line Feed Ascii for std::endl, \n
    const char tab = (char)0x9;  // Tab Ascii

    for (auto p : str) {
        if (FT_Load_Char(fontface_, p, FT_LOAD_RENDER)) {
            LogWarn("FreeType: could not render char: '" << p << "' (0x" << std::hex
                                                         << static_cast<int>(p) << ")");
            continue;
        }

        float w = static_cast<float>(fontface_->glyph->bitmap.width);

        if (p == lf) {
            y += getLineHeight();
            y += (fontface_->glyph->advance.y >> 6);
            x = 0.0f;
            continue;
        } else if (p == tab) {
            x += (fontface_->glyph->advance.x >> 6);
            y += (fontface_->glyph->advance.y >> 6);
            x += (4 * w);  // 4 times glyph character width
            maxx = std::max(maxx, x);
            continue;
        }
        x += (fontface_->glyph->advance.x >> 6);
        maxx = std::max(maxx, x);

        y += (fontface_->glyph->advance.y >> 6);
        maxy = std::max(maxy, y);
    }

    // add 2 pixel in vertical direction to prevent cut-off. This is caused by the fact
    // that the font ascend and descend are not necessarily correct
    // (see FreeType documentation for details)
    return vec2(maxx, maxy + 2);
}

void TextRenderer::initMesh() {
    auto verticesBuffer =
        util::makeBuffer<vec2>({{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}});

    auto texCoordsBuffer =
        util::makeBuffer<vec2>({{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}});

    auto indices = util::makeIndexBuffer({0, 1, 2, 3});

    mesh_ = util::make_unique<Mesh>();
    mesh_->addBuffer(BufferType::PositionAttrib, verticesBuffer);
    mesh_->addBuffer(BufferType::TexcoordAttrib, texCoordsBuffer);
    mesh_->addIndicies(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip), indices);

    drawer_ = util::make_unique<MeshDrawerGL>(mesh_.get());
}

void TextRenderer::setFontSize(int val) {
    if (fontSize_ != val) {
        fontSize_ = val;
        FT_Set_Pixel_Sizes(fontface_, 0, val);
    }
}

void TextRenderer::setLineSpacing(double lineSpacing) { lineSpacing_ = lineSpacing; }

double TextRenderer::getLineSpacing() const { return lineSpacing_; }

void TextRenderer::setLineHeight(int lineHeight) {
    lineSpacing_ = static_cast<double>(lineHeight) / static_cast<double>(fontSize_) - 1.0;
}

int TextRenderer::getLineHeight() const {
    return static_cast<int>(fontSize_ * (1.0 + lineSpacing_));
}

int TextRenderer::getBaseLineOffset() const { return static_cast<int>(getFontAscent()); }

int TextRenderer::getBaseLineDescent() const { return static_cast<int>(getFontDescent()); }

double TextRenderer::getFontAscent() const {
    return (fontface_->ascender * fontSize_ / static_cast<double>(fontface_->units_per_EM));
}

double TextRenderer::getFontDescent() const {
    return (fontface_->descender * fontSize_ / static_cast<double>(fontface_->units_per_EM));
}

}  // namespace
