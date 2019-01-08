/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_BUFFERCL_H
#define IVW_BUFFERCL_H

#include <modules/opencl/buffer/bufferclbase.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/inviwoopencl.h>

#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/util/formats.h>
#include <memory>

namespace inviwo {

class IVW_MODULE_OPENCL_API BufferCL : public BufferCLBase, public BufferRepresentation {
public:
    /**
     * @param size
     * @param type
     * @param format
     * @param data Data to transfer. Does not transfer data if data is nullptr.
     * @param readWriteFlag Determine how memory will be used by Kernels: CL_MEM_READ_ONLY,
     * CL_MEM_WRITE_ONLY, CL_MEM_READ_WRITE
     */
    BufferCL(size_t size, const DataFormatBase* format, BufferUsage usage = BufferUsage::Static,
             const void* data = nullptr, cl_mem_flags readWriteFlag = CL_MEM_READ_WRITE);
    BufferCL(const BufferCL& rhs);
    virtual ~BufferCL();

    virtual BufferCL* clone() const override;
    virtual size_t getSize() const override;
    virtual void setSize(size_t size) override;

    cl::Buffer& getEditable() override { return *clBuffer_; }
    const cl::Buffer& get() const override { return *clBuffer_; }
    virtual std::type_index getTypeIndex() const override final;
    /**
     * \brief Copies data from RAM to OpenCL.
     *
     * @param data Pointer to data
     * @param size Size in bytes to copy
     */
    void upload(const void* data, size_t size);
    /**
     * \brief Copies the entire buffer to RAM memory.
     *
     * Copies getSize()*getSizeOfElement() bytes into data.
     * Pointer needs to be allocated beforehand.
     *
     * @param data Pointer to data
     */
    void download(void* data) const;

protected:
    cl_mem_flags readWriteFlag_;
    size_t size_;
    std::unique_ptr<cl::Buffer> clBuffer_;
};

}  // namespace inviwo

namespace cl {

// Kernel argument specializations for BufferCL type
// (enables calling cl::Queue::setArg with BufferCL)
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::BufferCL& value);

// Kernel argument specializations for BufferBase type
// (enables calling cl::Queue::setArg with BufferBase).
// Calls value.getRepresentation<BufferCL>().
// @note This function is only valid for buffers
// that does not change the buffer data.
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::BufferBase& value);

// Kernel argument specializations for Buffer<T> type
// (enables calling cl::Queue::setArg with Buffer<T>).
// Calls value.getRepresentation<BufferCL>().
// @note This function is only valid for buffers
// that does not change the buffer data.
#define DataFormatIdMacro(i)                     \
    template <>                                  \
    IVW_MODULE_OPENCL_API cl_int Kernel::setArg( \
        cl_uint index, const inviwo::Buffer<inviwo::Data##i::type>& value);
#include <inviwo/core/util/formatsdefinefunc.h>

}  // namespace cl

#endif  // IVW_BUFFERCL_H
