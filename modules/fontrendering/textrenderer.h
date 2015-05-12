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

#ifndef IVW_TEXTRENDERER_H
#define IVW_TEXTRENDERER_H

#include <modules/fontrendering/fontrenderingmoduledefine.h>
#include <modules/fontrendering/fontrenderingmodule.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/glwrap/shader.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace inviwo {

/**
 * \class TextRenderer
 *
 * \brief Render text using the FreeType font library
 *
 */
class IVW_MODULE_FONTRENDERING_API TextRenderer { 
public:
    TextRenderer(const std::string& fontPath = InviwoApplication::getPtr()
        ->getModuleByType<FontRenderingModule>()
        ->getPath() + "/fonts/arial.ttf");
    virtual ~TextRenderer();

    void render(const char* text, float x, float y, const vec2& scale, vec4 color);
    /** 
     * \brief Computes width and height of the given text     
     */
    vec2 computeTextSize(const char* text, const vec2& scale);

    int getFontSize() const { return fontSize_; }
    void setFontSize(int val);
protected:
    
    void initMesh();

    FT_Library fontlib_;
    FT_Face fontface_;

    int fontSize_;

    Shader* textShader_;
    GLuint texCharacter_;
    Mesh* mesh_;
    MeshDrawerGL* drawer_;

};

} // namespace

#endif // IVW_TEXTRENDERER_H

