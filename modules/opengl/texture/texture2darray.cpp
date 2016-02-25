/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "texture2darray.h"
#include <modules/opengl/openglcapabilities.h>

namespace inviwo {

Texture2DArray::Texture2DArray(size3_t dimensions, GLFormats::GLFormat glFormat, GLenum filtering,
                               GLint level)
    : Texture(GL_TEXTURE_2D_ARRAY, glFormat, filtering, level), dimensions_(dimensions) {
    setTextureParameters(&Texture2DArray::default2DArrayTextureParameterFunction);
}

Texture2DArray::Texture2DArray(size3_t dimensions, GLint format, GLint internalformat,
                               GLenum dataType, GLenum filtering, GLint level)
    : Texture(GL_TEXTURE_2D_ARRAY, format, internalformat, dataType, filtering, level)
    , dimensions_(dimensions) {
    setTextureParameters(&Texture2DArray::default2DArrayTextureParameterFunction);
}

Texture2DArray::Texture2DArray(const Texture2DArray& rhs)
    : Texture(rhs), dimensions_(rhs.dimensions_) {
    setTextureParameters(&Texture2DArray::default2DArrayTextureParameterFunction);
    initialize(nullptr);
    if (OpenGLCapabilities::getOpenGLVersion() >= 430) {
        // GPU memcpy
        glCopyImageSubData(rhs.getID(), rhs.getTarget(), 0, 0, 0, 0, getID(), target_, 0, 0, 0, 0,
                           static_cast<GLsizei>(dimensions_.x), static_cast<GLsizei>(dimensions_.y),
                           static_cast<GLsizei>(dimensions_.z));
    } else {
        // Copy data through PBO
        loadFromPBO(&rhs);
    }
}

Texture2DArray& Texture2DArray::operator=(const Texture2DArray& rhs) {
    if (this != &rhs) {
        Texture::operator=(rhs);
        dimensions_ = rhs.dimensions_;
        setTextureParameters(&Texture2DArray::default2DArrayTextureParameterFunction);
        initialize(nullptr);
        if (OpenGLCapabilities::getOpenGLVersion() >= 430) {
            // GPU memcpy
            glCopyImageSubData(rhs.getID(), rhs.getTarget(), 0, 0, 0, 0, getID(), target_, 0, 0, 0,
                               0, static_cast<GLsizei>(rhs.dimensions_.x),
                               static_cast<GLsizei>(rhs.dimensions_.y),
                               static_cast<GLsizei>(rhs.dimensions_.z));
        } else {
            // Copy data through PBO
            loadFromPBO(&rhs);
        }
    }

    return *this;
}

Texture2DArray* Texture2DArray::clone() const { return new Texture2DArray(*this); }

void Texture2DArray::initialize(const void* data) {
    // Notify observers
    for (auto o : observers_) o->notifyBeforeTextureInitialization();

    // Allocate data
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, level_, internalformat_, static_cast<GLsizei>(dimensions_.x),
                 static_cast<GLsizei>(dimensions_.y), static_cast<GLsizei>(dimensions_.z), 0,
                 format_, dataType_, data);
    LGL_ERROR;

    for (auto o : observers_) o->notifyAfterTextureInitialization();
}

size_t Texture2DArray::getNumberOfValues() const {
    return dimensions_.x * dimensions_.y * dimensions_.z;
}

void Texture2DArray::upload(const void* data) {
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, static_cast<GLsizei>(dimensions_.x),
                    static_cast<GLsizei>(dimensions_.y), static_cast<GLsizei>(dimensions_.z),
                    format_, dataType_, data);
    LGL_ERROR;
}

void Texture2DArray::uploadAndResize(const void* data, const size3_t& dim) {
    dimensions_ = dim;
    setPBOAsInvalid();
    initialize(data);
}

void Texture2DArray::default2DArrayTextureParameterFunction(Texture* tex) {
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, tex->getFiltering());
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, tex->getFiltering());
}

}  // namespace
