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

#include <modules/opengl/buffer/bufferobject.h>

namespace inviwo {

BufferObject::BufferObject(size_t sizeInBytes, const DataFormatBase* format, BufferType type,
                           BufferUsage usage, GLenum target /*= GL_ARRAY_BUFFER*/)
    : Observable<BufferObjectObserver>()
    , target_(target)
    , glFormat_(getGLFormats()->getGLFormat(format->getId()))
    , type_(type) {
    switch (usage) {
        case DYNAMIC:
            usageGL_ = GL_DYNAMIC_DRAW;
            break;

        case STATIC:
        default:
            usageGL_ = GL_STATIC_DRAW;
            break;
    }

    glGenBuffers(1, &id_);

    initialize(nullptr, sizeInBytes);

    LGL_ERROR_SUPPRESS;
}

BufferObject::BufferObject(const BufferObject& rhs)
    : Observable<BufferObjectObserver>()
    , usageGL_(rhs.usageGL_)
    , target_(rhs.target_)
    , glFormat_(rhs.glFormat_)
    , type_(rhs.type_) {
    glGenBuffers(1, &id_);
    *this = rhs;
}

BufferObject::BufferObject(BufferObject&& rhs)
    : Observable<BufferObjectObserver>(std::move(rhs))
    , usageGL_(rhs.usageGL_)
    , target_(rhs.target_)
    , glFormat_(rhs.glFormat_)
    , type_(rhs.type_)
    // Steal buffer
    , id_(rhs.id_)
{
    // Free resources from other
    rhs.id_ = 0;
}

BufferObject& BufferObject::operator=(const BufferObject& rhs) {
    if (this != &rhs) {
        Observable<BufferObjectObserver>::operator=(rhs);
        usageGL_ = rhs.usageGL_;
        target_ = rhs.target_;
        glFormat_ = rhs.glFormat_;
        type_ = rhs.type_;
        
        // TODO: Verify that data copying works. What about backwards compability?
        // Initialize size of buffer
        initialize(nullptr, rhs.sizeInBytes_);
        // Now bind the second buffer, this buffer is already bound
        glBindBuffer(GL_COPY_READ_BUFFER, rhs.getId());
        // Copy data (OpenGL 3.1 functionality...)
        glCopyBufferSubData(GL_COPY_READ_BUFFER, target_, 0, 0, sizeInBytes_);
    }
    return *this;
}

BufferObject& BufferObject::operator=(BufferObject&& rhs) {
    if (this != &rhs) {
        // Free existing resources
        glDeleteBuffers(1, &id_);

        // Steal resources
        Observable<BufferObjectObserver>::operator=(std::move(rhs));

        id_ = rhs.id_;
        target_ = rhs.target_;
        usageGL_ = rhs.usageGL_;
        glFormat_ = rhs.glFormat_;
        type_ = rhs.type_;
        sizeInBytes_ = rhs.sizeInBytes_;

        // Release resources from source object
        rhs.id_ = 0;
    }
    return *this;
}

BufferObject::~BufferObject() { glDeleteBuffers(1, &id_); }

BufferObject* BufferObject::clone() const { return new BufferObject(*this); }

GLenum BufferObject::getFormatType() const { return glFormat_.type; }

GLuint BufferObject::getId() const { return id_; }

void BufferObject::bind() const { glBindBuffer(target_, id_); }

void BufferObject::unbind() const { glBindBuffer(target_, 0); }

void BufferObject::initialize(const void* data, GLsizeiptr sizeInBytes) {
    sizeInBytes_ = sizeInBytes;

    // Notify observers
    for (auto observer : observers_) {
        observer->onBeforeBufferInitialization();
    }

    bind();
    // Allocate and transfer possible data
    glBufferData(target_, sizeInBytes, data, usageGL_);

    for (auto observer : observers_) {
        observer->onAfterBufferInitialization();
    }
}

void BufferObject::upload(const void* data, GLsizeiptr sizeInBytes) {
    bind();
    glBufferSubData(target_, 0, sizeInBytes, data);
}

void BufferObject::download(void* data) const {
    bind();
    // Map data
    void* gldata = glMapBuffer(target_, GL_READ_ONLY);

    // Copy data if valid pointer
    if (gldata) {
        memcpy(data, gldata, sizeInBytes_);
        // Unmap buffer after using it
        glUnmapBufferARB(target_);
    } else {
        LogError("Unable to map data");
    }
}

}  // namespace
