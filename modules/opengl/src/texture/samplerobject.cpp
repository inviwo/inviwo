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

namespace inviwo {

SamplerObject::SamplerObject() : currentlyBoundTextureUnit_{0} { glGenSamplers(1, &id_); }

SamplerObject::~SamplerObject() {
    /*if (id_ != 0) {
        glDeleteSamplers(1, &id_);
    }*/ // Causes "unable to upload renderui.vert"
}

// SamplerObject::SamplerObject(const Texture& tex) {
//	SamplerObject();
//	bind(tex.getID); }

void SamplerObject::bind(GLuint textureUnitNumber) {
    glBindSampler(textureUnitNumber, id_);
    currentlyBoundTextureUnit_ = textureUnitNumber;
}

void SamplerObject::setMinFilterMode(GLenum type) {
    glSamplerParameteri(id_, GL_TEXTURE_MIN_FILTER, type);
}

void SamplerObject::setMagFilterMode(GLenum type) {
    glSamplerParameteri(id_, GL_TEXTURE_MAG_FILTER, type);
}

void SamplerObject::setFilterModeAll(GLenum type) {
    glSamplerParameteri(id_, GL_TEXTURE_MIN_FILTER, type);
    glSamplerParameteri(id_, GL_TEXTURE_MAG_FILTER, type);
}

void SamplerObject::setWrapMode_S(GLenum type) {
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
}

void SamplerObject::setWrapMode_T(GLenum type) {
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void SamplerObject::setWrapMode_R(GLenum type) {
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_R, GL_REPEAT);
}

void SamplerObject::setWrapModeAll(GLenum type) {
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_R, GL_REPEAT);
}

}  // namespace inviwo
