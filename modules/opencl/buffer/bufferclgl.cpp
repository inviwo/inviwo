/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#include <modules/opencl/buffer/bufferclgl.h>
#include <modules/opencl/openclsharing.h>
#include <inviwo/core/util/assertion.h>

namespace inviwo {

BufferCLGL::BufferCLGL(size_t size, const DataFormatBase* format, BufferType type, BufferUsage usage, BufferObject* data,
                       cl_mem_flags readWriteFlag)
    : BufferCLBase(), BufferRepresentation(size, format, type, usage), bufferObject_(data), readWriteFlag_(readWriteFlag)
{
    if (data) {
        initialize(data);
    }
}

BufferCLGL::BufferCLGL(const BufferCLGL& rhs)
    : BufferRepresentation(rhs), bufferObject_(rhs.getBufferGL()), readWriteFlag_(rhs.readWriteFlag_)
{
    initialize(bufferObject_);
}

BufferCLGL::~BufferCLGL() {
    deinitialize();
}

void BufferCLGL::initialize(BufferObject* data) {
    ivwAssert(data != 0, "Cannot initialize with null OpenGL buffer");
    data->increaseRefCount();
    CLBufferSharingMap::iterator it = OpenCLBufferSharing::clBufferSharingMap_.find(data);

    if (it == OpenCLBufferSharing::clBufferSharingMap_.end()) {
        clBuffer_ = new cl::BufferGL(OpenCL::getPtr()->getContext(), readWriteFlag_, data->getId());
        OpenCLBufferSharing::clBufferSharingMap_.insert(BufferSharingPair(data, new OpenCLBufferSharing(clBuffer_)));
    } else {
        clBuffer_ = static_cast<cl::BufferGL*>(it->second->sharedMemory_);
        it->second->increaseRefCount();
    }

    data->addObserver(this);
    BufferCLGL::initialize();
}

BufferCLGL* BufferCLGL::clone() const {
    return new BufferCLGL(*this);
}

void BufferCLGL::deinitialize() {
    CLBufferSharingMap::iterator it = OpenCLBufferSharing::clBufferSharingMap_.find(bufferObject_);

    if (it != OpenCLBufferSharing::clBufferSharingMap_.end()) {
        if (it->second->decreaseRefCount() == 0) {
            delete it->second->sharedMemory_;
            it->second->sharedMemory_ = 0;
            delete it->second;
            OpenCLBufferSharing::clBufferSharingMap_.erase(it);
        }
    }

    if (bufferObject_ && bufferObject_->decreaseRefCount() <= 0) {
        delete bufferObject_;
        bufferObject_ = NULL;
    }
}

void BufferCLGL::setSize(size_t size) {
    if (size == getSize()) {
        return;
    }

    // Make sure that the OpenCL buffer is deleted before changing the size.
    // By observing the BufferObject we will make sure that the shared OpenCL buffer is
    // deleted and reattached after resizing is done.
    bufferObject_->setSize(size*getSizeOfElement());
    BufferRepresentation::setSize(size);
}

void BufferCLGL::onBeforeBufferInitialization() {
    CLBufferSharingMap::iterator it = OpenCLBufferSharing::clBufferSharingMap_.find(bufferObject_);

    if (it != OpenCLBufferSharing::clBufferSharingMap_.end()) {
        if (it->second->decreaseRefCount() == 0) {
            delete it->second->sharedMemory_;
            it->second->sharedMemory_ = 0;
        }
    }

    clBuffer_ = 0;
}

void BufferCLGL::onAfterBufferInitialization() {
    CLBufferSharingMap::iterator it = OpenCLBufferSharing::clBufferSharingMap_.find(bufferObject_);

    if (it != OpenCLBufferSharing::clBufferSharingMap_.end()) {
        if (it->second->getRefCount() == 0) {
            it->second->sharedMemory_ = new cl::BufferGL(OpenCL::getPtr()->getContext(), readWriteFlag_, bufferObject_->getId());
        }

        clBuffer_ = static_cast<cl::BufferGL*>(it->second->sharedMemory_);
        it->second->increaseRefCount();
    }
}

} // namespace

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::BufferCLGL& value)
{
    return setArg(index, value.getBuffer());
}


} // namespace cl
