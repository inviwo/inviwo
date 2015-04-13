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

#include <modules/opencl/buffer/buffercl.h>

namespace inviwo {

BufferCL::BufferCL(size_t size, const DataFormatBase* format, BufferType type, BufferUsage usage,
                   const void* data, cl_mem_flags readWriteFlag)
    : BufferCLBase()
    , BufferRepresentation(format, type, usage)
    , readWriteFlag_(readWriteFlag)
    , size_(size) {

    // Generate a new buffer
    if (data != nullptr) {
        // CL_MEM_COPY_HOST_PTR can be used with CL_MEM_ALLOC_HOST_PTR to initialize the contents of
        // the cl_mem object allocated using host-accessible (e.g. PCIe) memory.
        clBuffer_ = new cl::Buffer(OpenCL::getPtr()->getContext(),
                                   readWriteFlag_ | CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR,
                                   getSize() * getSizeOfElement(), const_cast<void*>(data));
    } else {
        clBuffer_ = new cl::Buffer(OpenCL::getPtr()->getContext(), readWriteFlag_,
                                   getSize() * getSizeOfElement());
    }
}

BufferCL::BufferCL(const BufferCL& rhs)
    : BufferRepresentation(rhs)
    , readWriteFlag_(rhs.readWriteFlag_)
    , size_(rhs.size_) {
    
    clBuffer_ = new cl::Buffer(OpenCL::getPtr()->getContext(), readWriteFlag_,
                                   getSize() * getSizeOfElement());

    OpenCL::getPtr()->getQueue().enqueueCopyBuffer(rhs.getBuffer(), *clBuffer_, 0, 0,
                                                   getSize() * getSizeOfElement());
}

BufferCL* BufferCL::clone() const { return new BufferCL(*this); }

BufferCL::~BufferCL() {
    delete clBuffer_;
    clBuffer_ = nullptr;
}

void BufferCL::setSize(size_t size) {
    size_ = size;
    delete clBuffer_;
    clBuffer_ = new cl::Buffer(OpenCL::getPtr()->getContext(), readWriteFlag_,
                                   getSize() * getSizeOfElement());
}
size_t BufferCL::getSize() const { return size_; }

void BufferCL::upload(const void* data, size_t size) {
    // Resize buffer if necessary
    if (size > size_ * getSizeOfElement()) {
        delete clBuffer_;
        clBuffer_ = new cl::Buffer(OpenCL::getPtr()->getContext(),
                                   readWriteFlag_ | CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR,
                                   getSize() * getSizeOfElement(), const_cast<void*>(data));
    } else {
        OpenCL::getPtr()->getQueue().enqueueWriteBuffer(*clBuffer_, true, 0, size,
                                                        const_cast<void*>(data));
    }
}

void BufferCL::download(void* data) const {
    try {
        OpenCL::getPtr()->getQueue().enqueueReadBuffer(*clBuffer_, true, 0,
                                                       getSize() * getSizeOfElement(), data);
    } catch (cl::Error& err) {
        LogError(getCLErrorString(err));
    }
}



}  // namespace inviwo

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::BufferCL& value) {
    return setArg(index, value.getBuffer());
}

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::Buffer& value) {
    return setArg(index, value.getRepresentation<inviwo::BufferCL>()->getBuffer());
}

}  // namespace cl