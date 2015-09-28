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

#include <modules/opengl/buffer/bufferobjectarray.h>
#include <modules/opengl/buffer/bufferobject.h>

namespace inviwo {

int BufferObjectArray::maxVertexAttribSize_ = 0;

BufferObjectArray::BufferObjectArray()
    : id_(0u), attachedBuffers_(static_cast<size_t>(BufferType::NUMBER_OF_BUFFER_TYPES)) {
    initialize();
}

BufferObjectArray::BufferObjectArray(const BufferObjectArray& rhs)
    : id_(0u), attachedBuffers_(static_cast<size_t>(BufferType::NUMBER_OF_BUFFER_TYPES)) {
    initialize();

    bind();

    for (auto it = rhs.attachedBuffers_.cbegin(); it != rhs.attachedBuffers_.cend(); it++) {
        attachBufferObject(*it, static_cast<GLuint>(it - attachedBuffers_.cbegin()));
    }
    unbind();
}

BufferObjectArray* BufferObjectArray::clone() const { return new BufferObjectArray(*this); }

BufferObjectArray::~BufferObjectArray() { deinitialize(); }

void BufferObjectArray::initialize() {
    glGenVertexArrays(1, &id_);

#ifdef GL_VERSION_2_0
    if (maxVertexAttribSize_ < 1) {
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, (GLint*)&maxVertexAttribSize_);
    }
#endif

    if (maxVertexAttribSize_ > 0) attachedBuffers_.resize(maxVertexAttribSize_);
    std::fill(attachedBuffers_.begin(), attachedBuffers_.end(), nullptr);
}

void BufferObjectArray::deinitialize() { glDeleteVertexArrays(1, &id_); }

GLuint BufferObjectArray::getId() const { return id_; }

void BufferObjectArray::clear() {
    for (GLuint i = 0; i < static_cast<GLuint>(attachedBuffers_.size()); ++i) {
        if (attachedBuffers_[i]) {
            glDisableVertexAttribArray(i);
            attachedBuffers_[i] = nullptr;
        }
    }
}

void BufferObjectArray::bind() const { glBindVertexArray(id_); }

void BufferObjectArray::unbind() const { glBindVertexArray(0); }

void BufferObjectArray::attachBufferObject(const BufferObject* bo, GLuint location) {
    if (bo)
        pointToObject(bo, location);
    else
        LogError("Error: No valid BufferObject");

    attachedBuffers_.at(location) = bo;
}

void BufferObjectArray::pointToObject(const BufferObject* bo, GLuint location) {
    if (location < attachedBuffers_.size()) {
        glEnableVertexAttribArray(location);
        bo->bind();
        glVertexAttribPointer(location, bo->getGLFormat().channels, bo->getGLFormat().type,
                              GL_FALSE, 0, (void*)nullptr);
    } else
        LogError("Error: VertexAttribArray location exceeds maximum allowed range");
}

const BufferObject* BufferObjectArray::getBufferObject(size_t idx) const {
    if (idx < attachedBuffers_.size())
        return attachedBuffers_[idx];
    else
        return nullptr;
}

}  // namespace
