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
   
size_t BufferObjectArray::maxSize() const {
    static size_t size = [](){
        GLint glsize = 0;
        // usually about 16 should be more than BufferType::NumberOfBufferTypes. 
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, (GLint*)&glsize);
        return static_cast<size_t>(glsize);
    }();
    
    return size;
}

BufferObjectArray::BufferObjectArray()
    : attachedBuffers_(maxSize(), nullptr) {
}

BufferObjectArray::BufferObjectArray(const BufferObjectArray& rhs)
    : attachedBuffers_(rhs.attachedBuffers_) {
}

BufferObjectArray& BufferObjectArray::operator=(const BufferObjectArray& that) {
    if (this != &that) {
        bind();
        clear();
        unbind();
        attachedBuffers_ = that.attachedBuffers_;
    }
    return *this;
}

BufferObjectArray::~BufferObjectArray() { 
    glDeleteVertexArrays(1, &id_); 
}

GLuint BufferObjectArray::getId() const { return id_; }

void BufferObjectArray::clear() {
    for (GLuint i = 0; i < static_cast<GLuint>(attachedBuffers_.size()); ++i) {
        if (attachedBuffers_[i]) {
            glDisableVertexAttribArray(i);
            attachedBuffers_[i] = nullptr;
        }
    }
    reattach_ = true;
}

void BufferObjectArray::bind() const {
    if (id_ == 0) {
        glGenVertexArrays(1, &id_);
    }
    glBindVertexArray(id_);
    if (reattach_) {
        
        GLuint location = 0;
        for (auto& bo : attachedBuffers_) {
            if (bo) {
                glEnableVertexAttribArray(location);
                bo->bind();
                glVertexAttribPointer(location, bo->getGLFormat().channels, bo->getGLFormat().type,
                                      GL_FALSE, 0, (void*)nullptr);
            }
            location++;
        }
        reattach_ = false;
    }
}

void BufferObjectArray::unbind() const { glBindVertexArray(0); }

void BufferObjectArray::attachBufferObject(const BufferObject* bo, GLuint location) {
    if (location < attachedBuffers_.size()) {
        attachedBuffers_[location] = bo;
    } else {
        LogError("Error: VertexAttribArray location exceeds maximum allowed range");
    }
}

const BufferObject* BufferObjectArray::getBufferObject(size_t location) const {
    if (location < attachedBuffers_.size())
        return attachedBuffers_[location];
    else
        return nullptr;
}

}  // namespace
