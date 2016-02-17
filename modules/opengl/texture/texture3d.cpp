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

#include "texture3d.h"
#include <modules/opengl/openglcapabilities.h>

namespace inviwo {

Texture3D::Texture3D(size3_t dimensions, GLFormats::GLFormat glFormat, GLenum filtering, GLint level)
    : Texture(GL_TEXTURE_3D, glFormat, filtering, level), dimensions_(dimensions) {  
    setTextureParameters(&Texture3D::default3DTextureParameterFunction);
}

Texture3D::Texture3D(size3_t dimensions, GLint format, GLint internalformat, GLenum dataType,
                     GLenum filtering, GLint level)
    : Texture(GL_TEXTURE_3D, format, internalformat, dataType, filtering, level)
    , dimensions_(dimensions) {
    setTextureParameters(&Texture3D::default3DTextureParameterFunction);
}

Texture3D::Texture3D(const Texture3D& rhs) : Texture(rhs), dimensions_(rhs.dimensions_) {
    setTextureParameters(&Texture3D::default3DTextureParameterFunction);
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

Texture3D::Texture3D(Texture3D&& rhs) 
    : Texture(std::move(rhs))
    , dimensions_(std::move(rhs.dimensions_)) {
}

Texture3D& Texture3D::operator=(const Texture3D& rhs) {
    if (this != &rhs) {
        Texture::operator=(rhs);
        dimensions_ = rhs.dimensions_;
        setTextureParameters(&Texture3D::default3DTextureParameterFunction);
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

Texture3D& Texture3D::operator=(Texture3D&& rhs) {
    if (this != &rhs) {
        Texture::operator=(std::move(rhs));
        dimensions_ = std::move(rhs.dimensions_);
    }
    return *this;
}

Texture3D* Texture3D::clone() const { return new Texture3D(*this); }

void Texture3D::initialize(const void* data) {
    // Notify observers
    for (auto o : observers_) o->notifyBeforeTextureInitialization();

    // Allocate data
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage3D(GL_TEXTURE_3D, level_, internalformat_, static_cast<GLsizei>(dimensions_.x),
                 static_cast<GLsizei>(dimensions_.y), static_cast<GLsizei>(dimensions_.z), 0,
                 format_, dataType_, data);
    LGL_ERROR;

    for (auto o : observers_) o->notifyAfterTextureInitialization();  
}

size_t Texture3D::getNumberOfValues() const {
    return dimensions_.x * dimensions_.y * dimensions_.z;
}

void Texture3D::upload(const void* data) {
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, static_cast<GLsizei>(dimensions_.x),
                    static_cast<GLsizei>(dimensions_.y), static_cast<GLsizei>(dimensions_.z),
                    format_, dataType_, data);
    LGL_ERROR;
}

void Texture3D::uploadAndResize(const void* data, const size3_t& dim) {
    dimensions_ = dim;
    setPBOAsInvalid();
    initialize(data);
}

void Texture3D::default3DTextureParameterFunction(Texture* tex) {
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, tex->getFiltering());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, tex->getFiltering());
}

}  // namespace
