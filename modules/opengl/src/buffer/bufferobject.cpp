/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for BufferTarget, BufferUsage
#include <inviwo/core/util/formats.h>                          // for DataFormatBase, NumericType
#include <inviwo/core/util/logcentral.h>                       // for LogCentral, LogError
#include <inviwo/core/util/observer.h>                         // for Observable
#include <inviwo/core/util/sourcecontext.h>                    // for IVW_CONTEXT
#include <modules/opengl/buffer/bufferobjectobserver.h>        // for BufferObjectObserver
#include <modules/opengl/glformats.h>                          // for GLFormat, operator!=, GLFo...
#include <modules/opengl/inviwoopengl.h>                       // for GLenum, GLsizeiptr, glVert...
#include <modules/opengl/openglexception.h>                    // for OpenGLException
#include <inviwo/core/resourcemanager/resource.h>

#include <chrono>       // for literals
#include <cstring>      // for memcpy
#include <type_traits>  // for remove_reference<>::type
#include <utility>      // for move

namespace inviwo {

BufferObject::BufferObject(size_t sizeInBytes, const DataFormatBase* format, BufferUsage usage,
                           BufferTarget target)
    : Observable<BufferObjectObserver>(), glFormat_(GLFormats::get(format->getId())) {
    switch (usage) {
        case BufferUsage::Dynamic:
            usageGL_ = GL_DYNAMIC_DRAW;
            break;

        case BufferUsage::Static:
        default:
            usageGL_ = GL_STATIC_DRAW;
            break;
    }
    switch (target) {
        case BufferTarget::Index:
            target_ = GL_ELEMENT_ARRAY_BUFFER;
            break;
        case BufferTarget::Data:
        default:
            target_ = GL_ARRAY_BUFFER;
            break;
    }

    glGenBuffers(1, &id_);

    initialize(nullptr, sizeInBytes);
}

BufferObject::BufferObject(size_t sizeInBytes, GLFormat format, GLenum usage, GLenum target)
    : Observable<BufferObjectObserver>()
    , usageGL_(usage)
    , target_(target)
    , glFormat_(format)
    , sizeInBytes_(0)
    , capacityInBytes_(0) {

    glGenBuffers(1, &id_);

    initialize(nullptr, sizeInBytes);
}

BufferObject::BufferObject(const BufferObject& rhs)
    : Observable<BufferObjectObserver>()
    , usageGL_(rhs.usageGL_)
    , target_(rhs.target_)
    , glFormat_(rhs.glFormat_)
    , sizeInBytes_(0)
    , capacityInBytes_(0) {
    glGenBuffers(1, &id_);
    *this = rhs;
}

BufferObject::BufferObject(BufferObject&& rhs)
    : Observable<BufferObjectObserver>(std::move(rhs))
    , id_(rhs.id_)  // Steal buffer
    , usageGL_(rhs.usageGL_)
    , target_(rhs.target_)
    , glFormat_(rhs.glFormat_)
    , sizeInBytes_(rhs.sizeInBytes_)
    , capacityInBytes_(rhs.capacityInBytes_) {
    // Free resources from other
    rhs.id_ = 0;
}

BufferObject& BufferObject::operator=(const BufferObject& rhs) {
    if (this != &rhs) {
        // Note: do not copy observers since we are using this object's id_.
        // Observable<BufferObjectObserver>::operator=(rhs);

        // Only do expensive initialization if necessary
        if (sizeInBytes_ != rhs.sizeInBytes_ || usageGL_ != rhs.usageGL_ ||
            target_ != rhs.target_ || glFormat_ != rhs.glFormat_) {
            usageGL_ = rhs.usageGL_;
            target_ = rhs.target_;
            glFormat_ = rhs.glFormat_;
            // Initialize size of buffer
            initialize(nullptr, rhs.sizeInBytes_);
        }

        // Now bind the second buffer, this buffer is already bound
        glBindBuffer(GL_COPY_READ_BUFFER, rhs.getId());
        // Copy data (OpenGL 3.1 functionality...)
        glCopyBufferSubData(GL_COPY_READ_BUFFER, target_, 0, 0, sizeInBytes_);
    }
    return *this;
}

BufferObject& BufferObject::operator=(BufferObject&& rhs) {
    if (this != &rhs) {
        // Notify observers
        forEachObserver([](BufferObjectObserver* o) { o->onBeforeBufferInitialization(); });

        // Free existing resources
        glDeleteBuffers(1, &id_);

        // Steal resources
        Observable<BufferObjectObserver>::operator=(std::move(rhs));

        id_ = rhs.id_;
        target_ = rhs.target_;
        usageGL_ = rhs.usageGL_;
        glFormat_ = rhs.glFormat_;
        sizeInBytes_ = rhs.sizeInBytes_;
        capacityInBytes_ = rhs.capacityInBytes_;

        // Release resources from source object
        rhs.id_ = 0;

        forEachObserver([](BufferObjectObserver* o) { o->onAfterBufferInitialization(); });
    }
    return *this;
}

BufferObject::~BufferObject() {
    resource::remove(resource::GL{id_});
    glDeleteBuffers(1, &id_);
}

BufferObject* BufferObject::clone() const { return new BufferObject(*this); }

GLenum BufferObject::getFormatType() const { return glFormat_.type; }

GLenum BufferObject::getTarget() const { return target_; }

GLuint BufferObject::getId() const { return id_; }

void BufferObject::bind() const { glBindBuffer(target_, id_); }

void BufferObject::unbind() const { glBindBuffer(target_, 0); }

void BufferObject::bindBase(GLuint index) const { glBindBufferBase(target_, index, id_); }

void BufferObject::bindRange(GLuint index, GLintptr offset, GLsizeiptr size) const {
    glBindBufferRange(target_, index, id_, offset, size);
}

void BufferObject::bindAndSetAttribPointer(GLuint location, BindingType bindingType) const {
    bind();
    switch (bindingType) {
        case BindingType::Native:
            if (getDataFormat()->getNumericType() == NumericType::Float) {
                if (getDataFormat()->getPrecision() == static_cast<size_t>(64)) {
                    // double
                    glVertexAttribLPointer(location, glFormat_.channels, GL_DOUBLE, 0,
                                           (void*)nullptr);
                } else {
                    // other floating point types
                    glVertexAttribPointer(location, glFormat_.channels, glFormat_.type, GL_FALSE, 0,
                                          (void*)nullptr);
                }
            } else {
                // integral types
                glVertexAttribIPointer(location, glFormat_.channels, glFormat_.type, 0,
                                       (void*)nullptr);
            }
            break;
        case BindingType::ForceFloat:
            if ((getDataFormat()->getNumericType() == NumericType::Float) &&
                (getDataFormat()->getPrecision() == static_cast<size_t>(64))) {
                // special case for double precision since it is not part of GLFormats
                glVertexAttribPointer(location, glFormat_.channels, GL_DOUBLE, GL_FALSE, 0,
                                      (void*)nullptr);
            } else {
                glVertexAttribPointer(location, glFormat_.channels, glFormat_.type, GL_FALSE, 0,
                                      (void*)nullptr);
            }
            break;
        case BindingType::ForceNormalizedFloat:
            if ((getDataFormat()->getNumericType() == NumericType::Float) &&
                (getDataFormat()->getPrecision() == static_cast<size_t>(64))) {
                // special case for double precision since it is not part of GLFormats
                glVertexAttribPointer(location, glFormat_.channels, GL_DOUBLE, GL_FALSE, 0,
                                      (void*)nullptr);
            } else {
                glVertexAttribPointer(location, glFormat_.channels, glFormat_.type, GL_TRUE, 0,
                                      (void*)nullptr);
            }
            break;
    }
}

void BufferObject::setSizeInBytes(GLsizeiptr sizeInBytes, SizePolicy policy) {
    if (sizeInBytes == getCapacityInBytes() && policy == SizePolicy::ResizeToFit) {
        return;
    }

    if (sizeInBytes <= getCapacityInBytes() && policy == SizePolicy::GrowOnly) {
        // the buffer is large enough for the requested size
        sizeInBytes_ = sizeInBytes;
    } else {
        // re-initialize the buffer since its actual size/capacity needs to be adjusted
        initialize(nullptr, sizeInBytes);
    }
}

GLsizeiptr BufferObject::getSizeInBytes() const { return sizeInBytes_; }

GLsizeiptr BufferObject::getCapacityInBytes() const { return capacityInBytes_; }

void BufferObject::initialize(const void* data, GLsizeiptr sizeInBytes) {
    LGL_ERROR_CLASS;
    sizeInBytes_ = sizeInBytes;
    capacityInBytes_ = sizeInBytes;

    // Notify observers
    forEachObserver([](BufferObjectObserver* o) { o->onBeforeBufferInitialization(); });

    bind();
    // Allocate and transfer possible data
    // Allocation a zero sized buffer may create
    // errors (OpenCL sharing) so ensure at least one byte
    glBufferData(target_, sizeInBytes <= 0 ? 1 : sizeInBytes, data, usageGL_);

    if (auto err = glGetError(); err != GL_NO_ERROR) {
        throw OpenGLException(IVW_CONTEXT, "Unable to create buffer of type: {}. Error: {}",
                              targetName(target_), getGLErrorString(err));
    }

    forEachObserver([](BufferObjectObserver* o) { o->onAfterBufferInitialization(); });

    resource::add(
        resource::GL{id_},
        Resource{
            .dims = glm::size4_t{capacityInBytes_ / getDataFormat()->getSizeInBytes(), 0, 0, 0},
            .format = getDataFormat()->getId(),
            .desc = "BufferObject"});
}

void BufferObject::upload(const void* data, GLsizeiptr sizeInBytes, SizePolicy policy) {
    if ((sizeInBytes == getCapacityInBytes() && policy == SizePolicy::ResizeToFit) ||
        (sizeInBytes <= getCapacityInBytes() && policy == SizePolicy::GrowOnly)) {
        bind();
        glBufferSubData(target_, 0, sizeInBytes, data);
        sizeInBytes_ = sizeInBytes;
    } else {
        initialize(data, sizeInBytes);
    }
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

std::string_view BufferObject::targetName(GLenum target) {
    using namespace std::literals;
    switch (target) {
        case GL_ARRAY_BUFFER:
            return "GL_ARRAY_BUFFER"sv;
        case GL_ATOMIC_COUNTER_BUFFER:
            return "GL_ATOMIC_COUNTER_BUFFER"sv;
        case GL_COPY_READ_BUFFER:
            return "GL_COPY_READ_BUFFER"sv;
        case GL_COPY_WRITE_BUFFER:
            return "GL_COPY_WRITE_BUFFER"sv;
        case GL_DISPATCH_INDIRECT_BUFFER:
            return "GL_DISPATCH_INDIRECT_BUFFER"sv;
        case GL_DRAW_INDIRECT_BUFFER:
            return "GL_DRAW_INDIRECT_BUFFER"sv;
        case GL_ELEMENT_ARRAY_BUFFER:
            return "GL_ELEMENT_ARRAY_BUFFER"sv;
        case GL_PIXEL_PACK_BUFFER:
            return "GL_PIXEL_PACK_BUFFER"sv;
        case GL_PIXEL_UNPACK_BUFFER:
            return "GL_PIXEL_UNPACK_BUFFER"sv;
        case GL_QUERY_BUFFER:
            return "GL_QUERY_BUFFER"sv;
        case GL_SHADER_STORAGE_BUFFER:
            return "GL_SHADER_STORAGE_BUFFER"sv;
        case GL_TEXTURE_BUFFER:
            return "GL_TEXTURE_BUFFER"sv;
        case GL_TRANSFORM_FEEDBACK_BUFFER:
            return "GL_TRANSFORM_FEEDBACK_BUFFER"sv;
        case GL_UNIFORM_BUFFER:
            return "GL_UNIFORM_BUFFER"sv;
        default:
            return "UNKNOW_TARGET"sv;
    }
}

}  // namespace inviwo
