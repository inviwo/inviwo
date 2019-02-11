/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_GLFORMATS_H
#define IVW_GLFORMATS_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/assertion.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglexception.h>

namespace inviwo {

class IVW_MODULE_OPENGL_API GLFormats {
public:
    enum class Normalization { None, Normalized, SignNormalized };

    struct GLFormat {
        GLFormat();
        GLFormat(GLint f, GLint i, GLenum t, GLuint c, GLuint s, Normalization n, GLfloat sc = 0.f);

        GLint format;
        GLint internalFormat;
        GLenum type;
        GLuint channels;
        GLuint typeSize;
        Normalization normalization;
        GLdouble scaling;
        bool valid;
    };

    GLFormats();
    const GLFormat& getGLFormat(DataFormatId id) const;

    static const GLFormat& get(DataFormatId id);

private:
    GLFormat glFormatArray_[static_cast<size_t>(DataFormatId::NumberOfFormats)];
};

IVW_MODULE_OPENGL_API bool operator==(const GLFormats::GLFormat& a, const GLFormats::GLFormat& b);
IVW_MODULE_OPENGL_API bool operator!=(const GLFormats::GLFormat& a, const GLFormats::GLFormat& b);

}  // namespace inviwo

#endif
