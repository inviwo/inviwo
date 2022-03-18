/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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

#include <inviwo/core/util/exception.h>

namespace inviwo {

const BufferObjectArray::Warn BufferObjectArray::Warn::Yes{};
const BufferObjectArray::Warn BufferObjectArray::Warn::No{nullptr};

size_t BufferObjectArray::maxSize() const {
    static size_t size = []() {
        GLint glsize = 0;
        // usually about 16 should be more than BufferType::NumberOfBufferTypes.
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, (GLint*)&glsize);
        return static_cast<size_t>(glsize);
    }();

    return size;
}

BufferObjectArray::BufferObjectArray()
    : attachedBuffers_(maxSize(), {BindingType::Native, nullptr}) {}

BufferObjectArray::BufferObjectArray(const BufferObjectArray& rhs)
    : attachedBuffers_(rhs.attachedBuffers_) {}

BufferObjectArray& BufferObjectArray::operator=(const BufferObjectArray& that) {
    if (this != &that) {
        if (id_ != 0) {
            // avoid creating the vertex array if is not already initialized
            clear();
            unbind();
            reattach_ = true;
        }
        attachedBuffers_ = that.attachedBuffers_;
    }
    return *this;
}

BufferObjectArray::~BufferObjectArray() { glDeleteVertexArrays(1, &id_); }

GLuint BufferObjectArray::getId() const { return id_; }

void BufferObjectArray::clear() {
    if (id_ == 0) {
        return;
    }

    glBindVertexArray(id_);
    for (GLuint i = 0; i < static_cast<GLuint>(attachedBuffers_.size()); ++i) {
        if (attachedBuffers_[i].second) {
            glDisableVertexAttribArray(i);
            attachedBuffers_[i].second = nullptr;
        }
    }
}

void BufferObjectArray::bind() const {
    if (id_ == 0) {
        glGenVertexArrays(1, &id_);
    }
    glBindVertexArray(id_);
    if (reattach_) {
        for (GLuint location = 0; location < attachedBuffers_.size(); ++location) {
            if (auto bufferObject = attachedBuffers_[location].second) {
                glEnableVertexAttribArray(location);
                bufferObject->bindAndSetAttribPointer(location, attachedBuffers_[location].first);
            } else {
                glDisableVertexAttribArray(location);
            }
        }
        reattach_ = false;
    }
}

void BufferObjectArray::unbind() const { glBindVertexArray(0); }

void BufferObjectArray::attachBufferObject(const BufferObject* bufferObject, GLuint location,
                                           BindingType bindingType, Warn warn) {
    if (location < attachedBuffers_.size()) {
        // print warning if a different buffer is already attached to this location
        if (warn && attachedBuffers_[location].second && bufferObject &&
            (attachedBuffers_[location].second != bufferObject)) {
            LogWarn(fmt::format(
                "BufferObjectArray ({}): location {} is already bound to different buffer object "
                "(id {}). Replacing with new buffer object (id {}).",
                id_, location, attachedBuffers_[location].second->getId(), bufferObject->getId()));
        }
        attachedBuffers_[location] = {bindingType, bufferObject};
    } else {
        LogError("Error: VertexAttribArray location exceeds maximum allowed range");
    }
    reattach_ = true;
}

void BufferObjectArray::detachBufferObject(GLuint location) {
    if (location >= attachedBuffers_.size()) {
        throw RangeException(IVW_CONTEXT, "Invalid buffer location {}", location);
    }
    attachedBuffers_[location].second = nullptr;
    reattach_ = true;
}

const BufferObject* BufferObjectArray::getBufferObject(size_t location) const {
    if (location < attachedBuffers_.size())
        return attachedBuffers_[location].second;
    else
        return nullptr;
}

BufferObjectArray::BindingType BufferObjectArray::getBindingType(size_t location) const {
    return attachedBuffers_[location].first;
}

void BufferObjectArray::setBindingType(size_t location, BindingType bindingType) {
    attachedBuffers_[location].first = bindingType;
}

}  // namespace inviwo
