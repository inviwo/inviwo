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

#ifndef IVW_BUFFERCLGL_H
#define IVW_BUFFERCLGL_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>

#include <modules/opencl/buffer/bufferclbase.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opengl/buffer/bufferobjectobserver.h>
#include <modules/opengl/glwrap/bufferobject.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API BufferCLGL : public BufferCLBase, public BufferRepresentation, public BufferObjectObserver {

public:
    BufferCLGL(size_t size, const DataFormatBase* format, BufferType type, BufferUsage usage = STATIC, BufferObject* data = NULL,
               cl_mem_flags readWriteFlag = CL_MEM_READ_WRITE);
    BufferCLGL(const BufferCLGL& rhs);
    virtual ~BufferCLGL();
    virtual void initialize() {};
    virtual void deinitialize();
    virtual BufferCLGL* clone() const;

    virtual void setSize(size_t size);

    void initialize(BufferObject* data);

    const cl::Buffer& getBuffer() const { return *(clBuffer_); }
    BufferObject* getBufferGL() const { return bufferObject_; }

    void aquireGLObject(std::vector<cl::Event>* syncEvents = NULL) const {
        std::vector<cl::Memory> syncBuffers(1, *clBuffer_);
        OpenCL::getPtr()->getQueue().enqueueAcquireGLObjects(&syncBuffers, syncEvents);
    }

    void releaseGLObject(std::vector<cl::Event>* syncEvents = NULL, cl::Event* event= NULL) const {
        std::vector<cl::Memory> syncBuffers(1, *clBuffer_);
        OpenCL::getPtr()->getQueue().enqueueReleaseGLObjects(&syncBuffers, syncEvents, event);
    }

    /**
     * Release shared object before it is initialized.
     */
    void onBeforeBufferInitialization();
    /**
     * Reattach shared object after it has been initialized.
     */
    void onAfterBufferInitialization();

protected:
    BufferObject* bufferObject_;
    cl_mem_flags readWriteFlag_;
};

} // namespace

namespace cl {

// Kernel argument specializations for BufferCLGL type
// (enables calling cl::Queue::setArg with BufferCLGL)
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::BufferCLGL& value);

} // namespace cl



#endif // IVW_BUFFERCLGL_H
