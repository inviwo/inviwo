/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <modules/opengl/buffer/buffergl.h>

namespace inviwo {

BufferGL::BufferGL(size_t size, const DataFormatBase* format, BufferType type, BufferUsage usage, BufferObject* data)
    : BufferRepresentation(size, format, type, usage)
    , buffer_(data)
    , bufferArray_(NULL) {
    initialize();
    LGL_ERROR_SUPPRESS;
}

BufferGL::BufferGL(const BufferGL& rhs): BufferRepresentation(rhs) {
    bufferArray_ = NULL;
    buffer_ = rhs.getBufferObject()->clone();
}

BufferGL::~BufferGL() {
    deinitialize();
}

BufferGL* BufferGL::clone() const {
    return new BufferGL(*this);
}

void BufferGL::setSize( size_t size ) {
    if (size == getSize()) {
        return;
    }
    initialize(NULL, size*getSizeOfElement());
    BufferRepresentation::setSize(size);
}

GLuint BufferGL::getId() const {
    return buffer_->getId();
}

GLenum BufferGL::getFormatType() const {
    return buffer_->getFormatType();
}

void BufferGL::bind() const {
    buffer_->bind();
}

void BufferGL::initialize(const void* data, GLsizeiptr sizeInBytes) {
    BufferRepresentation::setSize(sizeInBytes/getSizeOfElement());
    buffer_->initialize(data, sizeInBytes);
}

void BufferGL::upload(const void* data, GLsizeiptr sizeInBytes) {
    buffer_->upload(data, sizeInBytes);
}

void BufferGL::initialize() {
    if (!buffer_) {
        buffer_ = new BufferObject(getSize(), getDataFormat(), getBufferType(), getBufferUsage());
    }
}

void BufferGL::deinitialize() {
    if (buffer_ && buffer_->decreaseRefCount() <= 0) {
        delete buffer_;
    }
    delete bufferArray_;
    bufferArray_ = NULL;
}

void BufferGL::download(void* data) const {
    buffer_->download(data);
}

void BufferGL::enable() const {
    if(!bufferArray_){
        bufferArray_ = new BufferObjectArray();
        bufferArray_->bind();
        bufferArray_->attachBufferObject(buffer_, 0);
    }
    else
        bufferArray_->bind();
}

void BufferGL::disable() const {
    if(bufferArray_)
        bufferArray_->unbind();
}

} // namespace

