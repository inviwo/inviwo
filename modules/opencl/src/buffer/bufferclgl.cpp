/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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
#include <modules/opencl/syncclgl.h>

namespace inviwo {
CLBufferSharingMap BufferCLGL::clBufferSharingMap_;

BufferCLGL::BufferCLGL(std::shared_ptr<BufferObject> data, BufferUsage usage,
                       cl_mem_flags readWriteFlag)
    : BufferCLBase()
    , BufferRepresentation(usage)
    , bufferObject_(data)
    , readWriteFlag_(readWriteFlag) {

    IVW_ASSERT(bufferObject_, "bufferObject_ should never be nullptr");

    auto it = BufferCLGL::clBufferSharingMap_.find(bufferObject_);
    if (it == BufferCLGL::clBufferSharingMap_.end()) {
        clBuffer_ = std::make_shared<cl::BufferGL>(OpenCL::getPtr()->getContext(), readWriteFlag_,
                                                   bufferObject_->getId());
        BufferCLGL::clBufferSharingMap_.insert(BufferSharingPair(bufferObject_, clBuffer_));
    } else {
        clBuffer_ = it->second;
    }

    bufferObject_->addObserver(this);
}

BufferCLGL::BufferCLGL(const BufferCLGL& rhs)
    : BufferCLBase(rhs)
    , BufferRepresentation(rhs)
    , bufferObject_(std::make_shared<BufferObject>(*rhs.getBufferGL().get()))
    , readWriteFlag_(rhs.readWriteFlag_) {
    // Share the copied BufferObject
    clBuffer_ = std::make_shared<cl::BufferGL>(OpenCL::getPtr()->getContext(), readWriteFlag_,
                                               bufferObject_->getId());
    BufferCLGL::clBufferSharingMap_.insert(BufferSharingPair(bufferObject_, clBuffer_));

    bufferObject_->addObserver(this);
}

BufferCLGL::~BufferCLGL() {
    CLBufferSharingMap::iterator it = BufferCLGL::clBufferSharingMap_.find(bufferObject_);
    // Release
    clBuffer_.reset();
    if (it != BufferCLGL::clBufferSharingMap_.end()) {
        if (it->second.use_count() == 1) {
            BufferCLGL::clBufferSharingMap_.erase(it);
        }
    }
}

BufferCLGL* BufferCLGL::clone() const { return new BufferCLGL(*this); }

const DataFormatBase* BufferCLGL::getDataFormat() const { return bufferObject_->getDataFormat(); }

size_t BufferCLGL::getSize() const {
    return bufferObject_->getSizeInBytes() / bufferObject_->getDataFormat()->getSizeInBytes();
}

void BufferCLGL::setSize(size_t size) {
    // Make sure that the OpenCL buffer is deleted before changing the size.
    // By observing the BufferObject we will make sure that the shared OpenCL buffer is
    // deleted and reattached after resizing is done.
    bufferObject_->setSizeInBytes(size * getSizeOfElement());
}

void BufferCLGL::upload(const void* data, size_t sizeInBytes) {
    // Resize buffer if necessary
    bufferObject_->setSizeInBytes(sizeInBytes);

    try {
        SyncCLGL glSync;
        glSync.addToAquireGLObjectList(this);
        glSync.aquireAllObjects();
        OpenCL::getPtr()->getQueue().enqueueWriteBuffer(*clBuffer_, true, 0, sizeInBytes,
                                                        const_cast<void*>(data));
    } catch (cl::Error& err) {
        LogError(getCLErrorString(err));
        throw err;
    }
}

void BufferCLGL::download(void* data) const {
    try {
        SyncCLGL glSync;
        glSync.addToAquireGLObjectList(this);
        glSync.aquireAllObjects();
        OpenCL::getPtr()->getQueue().enqueueReadBuffer(*clBuffer_, true, 0,
                                                       getSize() * getSizeOfElement(), data);
    } catch (cl::Error& err) {
        LogError(getCLErrorString(err));
        throw err;
    }
}

std::type_index BufferCLGL::getTypeIndex() const { return std::type_index(typeid(BufferCLGL)); }

void BufferCLGL::onBeforeBufferInitialization() {
    const auto it = BufferCLGL::clBufferSharingMap_.find(bufferObject_);
    // Release
    clBuffer_.reset();
    if (it != BufferCLGL::clBufferSharingMap_.end()) {
        if (it->second.use_count() == 1) {
            BufferCLGL::clBufferSharingMap_.erase(it);
        }
    }
}

void BufferCLGL::onAfterBufferInitialization() {
    const auto it = BufferCLGL::clBufferSharingMap_.find(bufferObject_);

    if (it != BufferCLGL::clBufferSharingMap_.end()) {
        clBuffer_ = it->second;
    } else {
        clBuffer_ = std::make_shared<cl::BufferGL>(OpenCL::getPtr()->getContext(), readWriteFlag_,
                                                   bufferObject_->getId());
        BufferCLGL::clBufferSharingMap_.insert(BufferSharingPair(bufferObject_, clBuffer_));
    }
}

}  // namespace inviwo

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::BufferCLGL& value) {
    return setArg(index, value.get());
}

}  // namespace cl
