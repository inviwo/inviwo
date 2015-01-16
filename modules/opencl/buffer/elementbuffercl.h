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

#ifndef IVW_ELEMENTBUFFERCL_H
#define IVW_ELEMENTBUFFERCL_H

#include <modules/opencl/buffer/buffercl.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/inviwoopencl.h>

#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>

namespace inviwo {
/** \class ElementBufferCL
 *
 * Class to enable proper conversion to ElementBufferGL.
 * This class does not add any other functionality to BufferCL
 * but needs to be used so that the representation converter
 * can create an ElementBufferGL object.
 */
class IVW_MODULE_OPENCL_API ElementBufferCL: public BufferCL {

public:
    /**
     * .
     *
     * @param size
     * @param type
     * @param format
     * @param data Data to transfer. Does not transfer data if data is NULL.
     * @param readWriteFlag Determine how memory will be used by Kernels: CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY, CL_MEM_READ_WRITE
     */
    ElementBufferCL(size_t size, const DataFormatBase* format, BufferType type, BufferUsage usage = STATIC, const void* data = NULL,
             cl_mem_flags readWriteFlag = CL_MEM_READ_WRITE);
    ElementBufferCL(const ElementBufferCL& rhs);
    virtual ~ElementBufferCL();



protected:
};

} // namespace

namespace cl {

// Kernel argument specializations for ElementBufferCL type
// (enables calling cl::Queue::setArg with ElementBufferCL)
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::ElementBufferCL& value);

} // namespace cl

#endif // IVW_ELEMENTBUFFERCL_H
