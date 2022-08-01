/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/opengl/texture/samplerobject.h>

#include <modules/opengl/openglutils.h>

namespace inviwo {

SamplerObject::SamplerObject(InterpolationType type, Wrapping3D wrap) {
    glGenSamplers(1, &id_);
    setFilterModeAll(type);
    setWrapMode(wrap);
}

SamplerObject::SamplerObject(SamplerObject&& rhs) noexcept : id_{rhs.id_} { rhs.id_ = 0; }

SamplerObject& SamplerObject::operator=(SamplerObject&& that) noexcept {
    if (this != &that) {
        if (id_ != 0) {
            glDeleteSamplers(1, &id_);
        }
        id_ = that.id_;
        that.id_ = 0;
    }
    return *this;
}

SamplerObject::~SamplerObject() {
    if (id_ != 0) {
        glDeleteSamplers(1, &id_);
    }
}

void SamplerObject::setMinFilterMode(GLenum interpolation) {
    glSamplerParameteri(id_, GL_TEXTURE_MIN_FILTER, interpolation);
}

void SamplerObject::setMinFilterMode(InterpolationType interpolation) {
    auto filtering = utilgl::convertInterpolationToGL(interpolation);
    glSamplerParameteri(id_, GL_TEXTURE_MIN_FILTER, filtering);
}

void SamplerObject::setMagFilterMode(GLenum interpolation) {
    glSamplerParameteri(id_, GL_TEXTURE_MAG_FILTER, interpolation);
}

void SamplerObject::setMagFilterMode(InterpolationType interpolation) {
    auto filtering = utilgl::convertInterpolationToGL(interpolation);
    glSamplerParameteri(id_, GL_TEXTURE_MAG_FILTER, filtering);
}

void SamplerObject::setFilterModeAll(GLenum interpolation) {
    glSamplerParameteri(id_, GL_TEXTURE_MIN_FILTER, interpolation);
    glSamplerParameteri(id_, GL_TEXTURE_MAG_FILTER, interpolation);
}

void SamplerObject::setFilterModeAll(InterpolationType interpolation) {
    auto filtering = utilgl::convertInterpolationToGL(interpolation);
    glSamplerParameteri(id_, GL_TEXTURE_MIN_FILTER, filtering);
    glSamplerParameteri(id_, GL_TEXTURE_MAG_FILTER, filtering);
}

void SamplerObject::setWrapMode_S(GLenum wrap) {
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_S, wrap);
}

void SamplerObject::setWrapMode_T(GLenum wrap) {
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_T, wrap);
}

void SamplerObject::setWrapMode_R(GLenum wrap) {
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_R, wrap);
}

void SamplerObject::setWrapModeAll(GLenum wrap) {
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_S, wrap);
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_T, wrap);
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_R, wrap);
}

void SamplerObject::setWrapMode(Wrapping3D wrap) {
    auto wrapMode = utilgl::convertWrappingToGL(wrap);
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_S, wrapMode[0]);
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_T, wrapMode[1]);
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_R, wrapMode[2]);
}

void SamplerObject::setWrapMode_S(Wrapping wrap) {
    auto wrapMode = utilgl::convertWrappingToGL(wrap);
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_S, wrapMode);
}

void SamplerObject::setWrapMode_T(Wrapping wrap) {
    auto wrapMode = utilgl::convertWrappingToGL(wrap);
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_T, wrapMode);
}

void SamplerObject::setWrapMode_R(Wrapping wrap) {
    auto wrapMode = utilgl::convertWrappingToGL(wrap);
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_R, wrapMode);
}

GLint SamplerObject::getID() const { return id_; }

}  // namespace inviwo
