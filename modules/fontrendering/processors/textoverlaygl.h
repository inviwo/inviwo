/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_TEXTOVERLAYGL_H
#define IVW_TEXTOVERLAYGL_H

#include <modules/fontrendering/fontrenderingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <modules/opengl/inviwoopengl.h>
#include <inviwo/core/properties/baseoptionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace inviwo {

class Shader;
class Mesh;

class IVW_MODULE_FONTRENDERING_API TextOverlayGL : public Processor {
public:
    TextOverlayGL();
    ~TextOverlayGL();

    InviwoProcessorInfo();

    void initialize();
    void deinitialize();

    
protected:
    virtual void process();
    void render_text(const char* text, float x, float y, float sx, float sy);
    vec2 measure_text(const char* text, float sx, float sy);


private:
    void initMesh();

    ImageInport inport_;
    ImageOutport outport_;
    FT_Library fontlib_;
    FT_Face fontface_;
    StringProperty text_;

    unsigned int font_size_;
    float xpos_;
    float ypos_;
    
    FloatVec4Property color_;
    OptionPropertyInt fontSize_;
    FloatVec2Property fontPos_;
    FloatVec2Property refPos_;
    Shader* textShader_;

    GLuint texCharacter_;
    Mesh* mesh_;
};

} // namespace

#endif // IVW_TEXTOVERLAYGL_H