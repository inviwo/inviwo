/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <inviwo/core/util/assertion.h>          // for assertion, IVW_ASSERT, assertions
#include <inviwo/core/util/canvas.h>             // for Canvas, Canvas::ContextID
#include <inviwo/core/util/exception.h>          // for RangeException, Exception
#include <inviwo/core/util/rendercontext.h>      // for RenderContext
#include <inviwo/core/util/sourcecontext.h>      // for IVW_CONTEXT, SourceLocation, IVW_SOURCE_...
#include <modules/opengl/buffer/bufferobject.h>  // for BufferObject, BufferObject::BindingType

#include <chrono>       // for literals
#include <string>       // for basic_string
#include <string_view>  // for operator""sv, string_view

#include <fmt/core.h>  // for format

namespace inviwo {

using namespace std::literals;

inline void checkContext(std::string_view error, Canvas::ContextID org, SourceLocation loc) {
    if constexpr (cfg::assertions) {
        auto rc = RenderContext::getPtr();
        Canvas::ContextID curr = rc->activeContext();
        if (org != curr) {
            const auto message =
                fmt::format("{}: '{}' ({}) than it was created: '{}' ({})", error,
                            rc->getContextName(curr), curr, rc->getContextName(org), org);

            assertion(loc.getFile(), loc.getFunction(), loc.getLine(), message);
        }
    }
}

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
    : id_(0u), attachedBuffers_(maxSize(), {BindingType::Native, nullptr}) {

    creationContext_ = RenderContext::getPtr()->activeContext();
    IVW_ASSERT(creationContext_, "An OpenGL Context has to be active");

    glGenVertexArrays(1, &id_);
}

BufferObjectArray::BufferObjectArray(const BufferObjectArray& rhs)
    : id_(0u), attachedBuffers_(rhs.attachedBuffers_) {

    creationContext_ = RenderContext::getPtr()->activeContext();
    IVW_ASSERT(creationContext_, "An OpenGL Context has to be active");

    glGenVertexArrays(1, &id_);

    bind();
    for (GLuint location = 0; location < attachedBuffers_.size(); ++location) {
        if (auto bufferObject = attachedBuffers_[location].second) {
            glEnableVertexAttribArray(location);
            bufferObject->bindAndSetAttribPointer(location, attachedBuffers_[location].first);
        }
    }
    unbind();
}

BufferObjectArray& BufferObjectArray::operator=(const BufferObjectArray& that) {
    if (this != &that) {
        bind();
        clear();
        attachedBuffers_ = that.attachedBuffers_;
        for (GLuint location = 0; location < attachedBuffers_.size(); ++location) {
            if (auto bufferObject = attachedBuffers_[location].second) {
                glEnableVertexAttribArray(location);
                bufferObject->bindAndSetAttribPointer(location, attachedBuffers_[location].first);
            }
        }
        unbind();
    }
    return *this;
}

BufferObjectArray::~BufferObjectArray() {
    if (id_ != 0) {
        checkContext("VAO deleted in a different context"sv, creationContext_, IVW_SOURCE_LOCATION);
        glDeleteVertexArrays(1, &id_);
    }
}

GLuint BufferObjectArray::getId() const { return id_; }

void BufferObjectArray::clear() {
    for (GLuint i = 0; i < static_cast<GLuint>(attachedBuffers_.size()); ++i) {
        if (attachedBuffers_[i].second) {
            glDisableVertexArrayAttrib(id_, i);
            attachedBuffers_[i].second = nullptr;
        }
    }
}

void BufferObjectArray::bind() const {
    checkContext("VAO bound in a different context"sv, creationContext_, IVW_SOURCE_LOCATION);
    glBindVertexArray(id_);
}

void BufferObjectArray::unbind() const { glBindVertexArray(0); }

bool BufferObjectArray::isActive() const {
    checkContext("VAO used in a different context"sv, creationContext_, IVW_SOURCE_LOCATION);
    return glIsVertexArray(id_);
}

void BufferObjectArray::attachBufferObject(const BufferObject* bufferObject, GLuint location,
                                           BindingType bindingType) {
    if (location >= attachedBuffers_.size()) {
        throw RangeException(IVW_CONTEXT,
                             "VertexAttribArray location {} exceeds maximum allowed range ({})",
                             location, attachedBuffers_.size());
    }
    if (attachedBuffers_[location].second && bufferObject &&
        (attachedBuffers_[location].second != bufferObject)) {
        throw Exception(IVW_CONTEXT,
                        "BufferObjectArray ({}): Cannot bind buffer object (id {}). Location "
                        "{} is already bound to different buffer object "
                        "(id {}).",
                        id_, bufferObject->getId(), location,
                        attachedBuffers_[location].second->getId());
    }
    attachedBuffers_[location] = {bindingType, bufferObject};
    glEnableVertexAttribArray(location);
    bufferObject->bindAndSetAttribPointer(location, bindingType);
}

void BufferObjectArray::attachBufferObjectEnforce(const BufferObject* bufferObject, GLuint location,
                                                  BindingType bindingType) {
    if (location >= attachedBuffers_.size()) {
        throw RangeException(IVW_CONTEXT,
                             "VertexAttribArray location {} exceeds maximum allowed range ({})",
                             location, attachedBuffers_.size());
    }
    attachedBuffers_[location] = {bindingType, bufferObject};
    glEnableVertexAttribArray(location);
    bufferObject->bindAndSetAttribPointer(location, bindingType);
}

void BufferObjectArray::detachBufferObject(GLuint location) {
    if (location >= attachedBuffers_.size()) {
        throw RangeException(IVW_CONTEXT, "Invalid buffer location {}", location);
    }
    if (attachedBuffers_[location].second) {
        attachedBuffers_[location].second = nullptr;
        glDisableVertexAttribArray(location);
    }
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
