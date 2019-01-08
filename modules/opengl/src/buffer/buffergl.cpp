/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <modules/opengl/buffer/buffergl.h>

namespace inviwo {

BufferGL::BufferGL(size_t size, const DataFormatBase* format, BufferUsage usage,
                   BufferTarget target, std::shared_ptr<BufferObject> data)
    : BufferRepresentation(format, usage, target)
    , buffer_(data
                  ? data
                  : std::make_shared<BufferObject>(size * format->getSize(), format, usage, target))
    , bufferArray_(nullptr)
    , size_(size) {}

BufferGL::BufferGL(const BufferGL& rhs)
    : BufferRepresentation(rhs)
    , buffer_(rhs.buffer_->clone())
    , bufferArray_(nullptr)
    , size_(rhs.size_) {}

BufferGL::~BufferGL() = default;

BufferGL* BufferGL::clone() const { return new BufferGL(*this); }

size_t BufferGL::getSize() const { return size_; }

void BufferGL::setSize(size_t size) {
    if (size != size_) {
        size_ = size;
        buffer_->setSize(size * getSizeOfElement());
    }
}

GLuint BufferGL::getId() const { return buffer_->getId(); }

GLenum BufferGL::getFormatType() const { return buffer_->getFormatType(); }

void BufferGL::bind() const { buffer_->bind(); }

void BufferGL::upload(const void* data, GLsizeiptr sizeInBytes) {
    buffer_->upload(data, sizeInBytes);
}

void BufferGL::download(void* data) const { buffer_->download(data); }

void BufferGL::enable() const {
    if (!bufferArray_) {
        bufferArray_ = util::make_unique<BufferObjectArray>();
        bufferArray_->attachBufferObject(buffer_.get(), 0);
    }
    bufferArray_->bind();
}

void BufferGL::disable() const {
    if (bufferArray_) bufferArray_->unbind();
}

std::type_index BufferGL::getTypeIndex() const { return std::type_index(typeid(BufferGL)); }

}  // namespace inviwo
