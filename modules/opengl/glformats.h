/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_GLFORMATS_H
#define IVW_GLFORMATS_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/assertion.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglexception.h>

namespace inviwo {

class GLFormats {
public:
    enum class Normalization {
        None,
        Normalized,
        SignNormalized
    };

    struct GLFormat {
        GLFormat()
            : format(0)
            , internalFormat(0)
            , type(0)
            , channels(0)
            , typeSize(0)
            , normalization(Normalization::None)
            , scaling(0.f)
            , valid(false) {}
        GLFormat(GLint f, GLint i, GLenum t, GLuint c, GLuint s, Normalization n, GLfloat sc = 0.f)
            : format(f)
            , internalFormat(i)
            , type(t)
            , channels(c)
            , typeSize(s)
            , normalization(n)
            , scaling(sc)
            , valid(true) {}
        
        GLint format;
        GLint internalFormat;
        GLenum type;
        GLuint channels;
        GLuint typeSize;
        Normalization normalization;
        GLdouble scaling;
        bool valid;
    };

    GLFormats() {
        //1 channel
        glFormatArray_[static_cast<size_t>(DataFormatId::Float16)] = GLFormat(GL_RED, GL_R16F, GL_HALF_FLOAT, 1, 2, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::Float32)] = GLFormat(GL_RED, GL_R32F, GL_FLOAT, 1, 4, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::Int8)] = GLFormat(GL_RED, GL_R8_SNORM, GL_BYTE, 1, 1, Normalization::SignNormalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Int16)] = GLFormat(GL_RED, GL_R16_SNORM, GL_SHORT, 1, 2, Normalization::SignNormalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Int32)] = GLFormat(GL_RED_INTEGER, GL_R32I, GL_INT, 1, 4, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::UInt8)] = GLFormat(GL_RED, GL_R8, GL_UNSIGNED_BYTE, 1, 1, Normalization::Normalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::UInt16)] = GLFormat(GL_RED, GL_R16, GL_UNSIGNED_SHORT, 1, 2, Normalization::Normalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::UInt32)] = GLFormat(GL_RED_INTEGER, GL_R32UI, GL_UNSIGNED_INT, 1, 4, Normalization::None);
        //2 channels
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec2Float16)] = GLFormat(GL_RG, GL_RG16F, GL_HALF_FLOAT, 2, 2, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec2Float32)] = GLFormat(GL_RG, GL_RG32F, GL_FLOAT, 2, 4, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec2Int8)] = GLFormat(GL_RG, GL_RG8_SNORM, GL_BYTE, 2, 1, Normalization::SignNormalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec2Int16)] = GLFormat(GL_RG, GL_RG16_SNORM, GL_SHORT, 2, 2, Normalization::SignNormalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec2Int32)] = GLFormat(GL_RG_INTEGER, GL_RG32I, GL_INT, 2, 4, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec2UInt8)] = GLFormat(GL_RG, GL_RG8, GL_UNSIGNED_BYTE, 2, 1, Normalization::Normalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec2UInt16)] = GLFormat(GL_RG, GL_RG16, GL_UNSIGNED_SHORT, 2, 2, Normalization::Normalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec2UInt32)] = GLFormat(GL_RG_INTEGER, GL_RG32UI, GL_UNSIGNED_INT, 2, 4, Normalization::None);
        //3 channels
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec3Float16)] = GLFormat(GL_RGB, GL_RGB16F, GL_HALF_FLOAT, 3, 2, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec3Float32)] = GLFormat(GL_RGB, GL_RGB32F, GL_FLOAT, 3, 4, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec3Int8)] = GLFormat(GL_RGB, GL_RGB8_SNORM, GL_BYTE, 3, 1, Normalization::SignNormalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec3Int16)] = GLFormat(GL_RGB, GL_RGB16_SNORM, GL_SHORT, 3, 2, Normalization::SignNormalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec3Int32)] = GLFormat(GL_RGB_INTEGER, GL_RGB32I, GL_INT, 3, 4, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec3UInt8)] = GLFormat(GL_RGB, GL_RGB8, GL_UNSIGNED_BYTE, 3, 1, Normalization::Normalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec3UInt16)] = GLFormat(GL_RGB, GL_RGB16, GL_UNSIGNED_SHORT, 3, 2, Normalization::Normalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec3UInt32)] = GLFormat(GL_RGB_INTEGER, GL_RGB32UI, GL_UNSIGNED_INT, 3, 4, Normalization::None);
        //4 channels
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec4Float16)] = GLFormat(GL_RGBA, GL_RGBA16F, GL_HALF_FLOAT, 4, 2, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec4Float32)] = GLFormat(GL_RGBA, GL_RGBA32F, GL_FLOAT, 4, 4, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec4Int8)] = GLFormat(GL_RGBA, GL_RGBA8_SNORM, GL_BYTE, 4, 1, Normalization::SignNormalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec4Int16)] = GLFormat(GL_RGBA, GL_RGBA16_SNORM, GL_SHORT, 4, 2, Normalization::SignNormalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec4Int32)] = GLFormat(GL_RGBA_INTEGER, GL_RGBA32I, GL_INT, 4, 4, Normalization::None);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec4UInt8)] = GLFormat(GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, 4, 1, Normalization::Normalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec4UInt16)] = GLFormat(GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT, 4, 2, Normalization::Normalized);
        glFormatArray_[static_cast<size_t>(DataFormatId::Vec4UInt32)] = GLFormat(GL_RGBA_INTEGER, GL_RGBA32UI, GL_UNSIGNED_INT, 4, 4, Normalization::None);
    };

    GLFormat getGLFormat(DataFormatId id) const {
        if (!glFormatArray_[static_cast<size_t>(id)].valid) {
            throw OpenGLException("Error no OpenGL format available for selected data format: " 
                                  + std::string(DataFormatBase::get(id)->getString()), IvwContext);
        }
        return glFormatArray_[static_cast<size_t>(id)];
    };

private:
    GLFormat glFormatArray_[static_cast<size_t>(DataFormatId::NumberOfFormats)];
};

static const GLFormats glFormats_ = GLFormats();
#include <warn/push>
#include <warn/ignore/unused-function>
static const GLFormats* getGLFormats() {
    return &glFormats_;
}
#include <warn/pop>
}

#endif
