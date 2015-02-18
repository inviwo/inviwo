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

#ifndef IVW_BUFFERCL_BASE_H
#define IVW_BUFFERCL_BASE_H

#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>

namespace inviwo {
// Parent classes are responsible for creating the appropriate cl::Buffer type (Buffer/BufferGL and so forth)
// This class enables inviwo to use cl::Buffer(s) in a generic way (i.e. not caring if it is an Buffer or BufferGL).
class IVW_MODULE_OPENCL_API BufferCLBase {

public:
    BufferCLBase();
    BufferCLBase(const BufferCLBase& other);
    virtual ~BufferCLBase();

    virtual cl::Buffer& getEditable() { return *clBuffer_; }
    virtual const cl::Buffer& get() const { return *const_cast<const cl::Buffer*>(clBuffer_); }

protected:
    cl::Buffer* clBuffer_; // Derived class is responsible for allocating and deallocating this
};

} // namespace

namespace cl {

// Kernel argument specializations for BufferCLBase type
// (enables calling cl::Queue::setArg with BufferCLBase)
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::BufferCLBase& value);

} // namespace cl



#endif // IVW_BUFFERCL_BASE_H
