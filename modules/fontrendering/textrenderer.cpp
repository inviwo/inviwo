/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include "textrenderer.h"
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <modules/opengl/buffer/bufferobjectarray.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

TextRenderer::TextRenderer(const std::string& fontPath)
    : textShader_("fontrendering_freetype.vert", "fontrendering_freetype.frag", true) {
    
    if (FT_Init_FreeType(&fontlib_)) LogWarnCustom("TextRenderer", "FreeType: Major error.");

    int error = 0;
    error = FT_New_Face(fontlib_, fontPath.c_str(), 0, &fontface_);
    if (error == FT_Err_Unknown_File_Format) {
        LogWarnCustom("TextRenderer", "FreeType: File opened and read, format unsupported.");
    } else if (error) {
        LogWarnCustom("TextRenderer", "FreeType:  Could not read/open the font file.");
    }

    glGenTextures(1, &texCharacter_);

    initMesh();
    setFontSize(12);
}

TextRenderer::~TextRenderer() {
    FT_Done_Face(fontface_);
    glDeleteTextures(1, &texCharacter_);
}

void TextRenderer::render(const char* text, float x, float y, const vec2& scale, vec4 color) {
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

    const char* p;

    float offset = 0;
    float inputX = x;

    // TODO: To make things more reliable ask the system for proper ascii
    char lf = (char)0xA;  // Line Feed Ascii for std::endl, \n
    char tab = (char)0x9;  // Tab Ascii 

    BufferObjectArray rectArray;

    for (p = text; *p; p++) {
        if (FT_Load_Char(fontface_, *p, FT_LOAD_RENDER)) {
            LogWarn("FreeType: could not render char");
            continue;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, fontface_->glyph->bitmap.width,
            fontface_->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
            fontface_->glyph->bitmap.buffer);

        float x2 = x + fontface_->glyph->bitmap_left * scale.x;
        float y2 = -y - fontface_->glyph->bitmap_top * scale.y;
        float w = fontface_->glyph->bitmap.width * scale.x;
        float h = fontface_->glyph->bitmap.rows * scale.y;

        if (*p == lf) {
            offset += (2 * h);
            x = inputX;
            y += (fontface_->glyph->advance.y >> 6) * scale.y;
            continue;
        } else if (*p == tab) {
            x += (fontface_->glyph->advance.x >> 6) * scale.x;
            y += (fontface_->glyph->advance.y >> 6) * scale.y;
            x += (4 * w);  // 4 times glyph character width
            continue;
        }

        y2 += offset;
        // Translate quad to correct position and render
        mesh_->setModelMatrix(glm::translate(vec3(x2, -y2, 0.f))*glm::scale(vec3(w, -h, 1.f)));
        utilgl::setShaderUniforms(textShader_, *mesh_, "geometry_");                
        
        drawer_->draw();
        x += (fontface_->glyph->advance.x >> 6) * scale.x;
        y += (fontface_->glyph->advance.y >> 6) * scale.y;
    }

    textShader_.deactivate();
}

vec2 TextRenderer::computeTextSize(const char* text, const vec2& scale) {
    const char* p;

    float x = 0.0f;
    float y = 0.0f;

    float maxx = 0.0f;
    float maxy = 0.0f;

    char lf = (char)0xA;  // Line Feed Ascii for std::endl, \n
    char tab = (char)0x9;  // Tab Ascii

    for (p = text; *p; p++) {
        if (FT_Load_Char(fontface_, *p, FT_LOAD_RENDER)) {
            LogWarn("FreeType: could not render char");
            continue;
        }

        float w = fontface_->glyph->bitmap.width * scale.x;
        float h = fontface_->glyph->bitmap.rows * scale.y;

        if(y == 0.0f) y+=2*h;

        if (*p == lf) {
            y += (2 * h);
            x += (fontface_->glyph->advance.x >> 6) * scale.x;
            y += (fontface_->glyph->advance.y >> 6) * scale.y;
            maxx = std::max(maxx, x);
            x = 0.0f;
            continue;
        } else if (*p == tab) {
            x += (fontface_->glyph->advance.x >> 6) * scale.x;
            y += (fontface_->glyph->advance.y >> 6) * scale.y;
            x += (4 * w);  // 4 times glyph character width
            maxx = std::max(maxx, x);
            continue;
        }
        x += (fontface_->glyph->advance.x >> 6) * scale.x;
        maxx = std::max(maxx, x);

        y += (fontface_->glyph->advance.y >> 6) * scale.y;
        maxy = std::max(maxy, y);
    }


    return vec2(maxx, maxy);
}

void TextRenderer::initMesh() {
    Position2dBuffer* verticesBuffer = new Position2dBuffer();
    Position2dBufferRAM* verticesBufferRAM =
        verticesBuffer->getEditableRepresentation<Position2dBufferRAM>();
    verticesBufferRAM->add(vec2(0.0f, 0.0f));
    verticesBufferRAM->add(vec2(1.0f, 0.0f));
    verticesBufferRAM->add(vec2(0.0f, 1.0f));
    verticesBufferRAM->add(vec2(1.0f, 1.0f));
    TexCoord2dBuffer* texCoordsBuffer = new TexCoord2dBuffer();
    TexCoord2dBufferRAM* texCoordsBufferRAM =
        texCoordsBuffer->getEditableRepresentation<TexCoord2dBufferRAM>();
    texCoordsBufferRAM->add(vec2(0.0f, 0.0f));
    texCoordsBufferRAM->add(vec2(1.0f, 0.0f));
    texCoordsBufferRAM->add(vec2(0.0f, 1.0f));
    texCoordsBufferRAM->add(vec2(1.0f, 1.0f));

    IndexBuffer* indices_ = new IndexBuffer();
    Mesh::AttributesInfo(GeometryEnums::TRIANGLES, GeometryEnums::STRIP);
    IndexBufferRAM* indexBufferRAM = indices_->getEditableRepresentation<IndexBufferRAM>();
    indexBufferRAM->add(0);
    indexBufferRAM->add(1);
    indexBufferRAM->add(2);
    indexBufferRAM->add(3);

    mesh_.reset(new Mesh());
    mesh_->addAttribute(verticesBuffer);
    mesh_->addAttribute(texCoordsBuffer);
    mesh_->addIndicies(Mesh::AttributesInfo(GeometryEnums::TRIANGLES, GeometryEnums::STRIP), indices_);

    drawer_.reset(new MeshDrawerGL(mesh_.get()));
}

void TextRenderer::setFontSize(int val) {
    fontSize_ = val;
    FT_Set_Pixel_Sizes(fontface_, 0, val);
}



} // namespace

