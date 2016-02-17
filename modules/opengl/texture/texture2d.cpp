/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#include "texture2d.h"
#include <modules/opengl/openglcapabilities.h>

namespace inviwo {

Texture2D::Texture2D(size2_t dimensions, GLFormats::GLFormat glFormat, GLenum filtering,
                     GLint level)
    : Texture(GL_TEXTURE_2D, glFormat, filtering), dimensions_(dimensions) {
    setTextureParameters(&Texture2D::default2DTextureParameterFunction);
}

Texture2D::Texture2D(size2_t dimensions, GLint format, GLint internalformat, GLenum dataType,
                     GLenum filtering, GLint level)
    : Texture(GL_TEXTURE_2D, format, internalformat, dataType, filtering), dimensions_(dimensions) {
    setTextureParameters(&Texture2D::default2DTextureParameterFunction);
}

Texture2D::Texture2D(const Texture2D& rhs) : Texture(rhs), dimensions_(rhs.dimensions_) {
    setTextureParameters(&Texture2D::default2DTextureParameterFunction);
    initialize(nullptr);
    if (OpenGLCapabilities::getOpenGLVersion() >= 430) {
        // GPU memcpy
        glCopyImageSubData(rhs.getID(), rhs.getTarget(), 0, 0, 0, 0, getID(), target_, 0, 0, 0, 0,
                           static_cast<GLsizei>(dimensions_.x), static_cast<GLsizei>(dimensions_.y),
                           1);
    } else {
        // Copy data through PBO
        loadFromPBO(&rhs);
    }
}

Texture2D::Texture2D(Texture2D&& rhs)
    : Texture(std::move(rhs)), dimensions_(std::move(rhs.dimensions_)) {}

Texture2D& Texture2D::operator=(const Texture2D& rhs) {
    if (this != &rhs) {
        Texture::operator=(rhs);
        dimensions_ = rhs.dimensions_;
        setTextureParameters(&Texture2D::default2DTextureParameterFunction);
        initialize(nullptr);

        if (OpenGLCapabilities::getOpenGLVersion() >= 430) {
            // GPU memcpy
            glCopyImageSubData(rhs.getID(), rhs.getTarget(), 0, 0, 0, 0, getID(), target_, 0, 0, 0,
                               0, static_cast<GLsizei>(rhs.dimensions_.x),
                               static_cast<GLsizei>(rhs.dimensions_.y), 1);
        } else {
            // Copy data through PBO
            loadFromPBO(&rhs);
        }
    }

    return *this;
}

Texture2D& Texture2D::operator=(Texture2D&& rhs) {
    if (this != &rhs) {
        Texture::operator=(std::move(rhs));
        dimensions_ = std::move(rhs.dimensions_);
    }
    return *this;
}

Texture2D* Texture2D::clone() const { return new Texture2D(*this); }

void Texture2D::initialize(const void* data) {
    // Notify observers
    for (auto o : observers_) o->notifyBeforeTextureInitialization();
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, level_, internalformat_, static_cast<GLsizei>(dimensions_.x),
                 static_cast<GLsizei>(dimensions_.y), 0, format_, dataType_, data);
    LGL_ERROR;
    for (auto o : observers_) o->notifyAfterTextureInitialization();
}

size_t Texture2D::getNumberOfValues() const { return dimensions_.x * dimensions_.y; }

void Texture2D::upload(const void* data) {
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, static_cast<GLsizei>(dimensions_.x),
                    static_cast<GLsizei>(dimensions_.y), format_, dataType_, data);
    LGL_ERROR_SUPPRESS;
}

void Texture2D::resize(size2_t dimensions) {
    dimensions_ = dimensions;
    setPBOAsInvalid();
    initialize(nullptr);
}

void Texture2D::default2DTextureParameterFunction(Texture* tex) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex->getFiltering());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex->getFiltering());
}

}  // namespace
