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

#ifndef IVW_BUFFER_RAM_H
#define IVW_BUFFER_RAM_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>
#include <inviwo/core/util/formats.h>

namespace inviwo {

class IVW_CORE_API BufferRAM : public BufferRepresentation {
public:
    BufferRAM(const DataFormatBase* format = DataFormatBase::get(),
              BufferUsage usage = BufferUsage::Static);
    BufferRAM(const BufferRAM& rhs) = default;
    BufferRAM& operator=(const BufferRAM& that) = default;
    virtual BufferRAM* clone() const override = 0;
    virtual ~BufferRAM() = default;

    virtual void* getData() = 0;
    virtual const void* getData() const = 0;

    virtual void setValueFromSingleDouble(size_t index, double val) = 0;
    virtual void setValueFromVec2Double(size_t index, dvec2 val) = 0;
    virtual void setValueFromVec3Double(size_t index, dvec3 val) = 0;
    virtual void setValueFromVec4Double(size_t index, dvec4 val) = 0;

    virtual double getValueAsSingleDouble(size_t index) const = 0;
    virtual dvec2 getValueAsVec2Double(size_t index) const = 0;
    virtual dvec3 getValueAsVec3Double(size_t index) const = 0;
    virtual dvec4 getValueAsVec4Double(size_t index) const = 0;

    virtual std::type_index getTypeIndex() const override final;
};

/**
 * Factory for buffers.
 * Creates a BufferRAM with data type specified by format.
 *
 * @param size of buffer to create.
 * @param format of buffer to create.
 * @return nullptr if no valid format was specified.
 */
IVW_CORE_API std::shared_ptr<BufferRAM> createBufferRAM(size_t size, const DataFormatBase* format,
                                                        BufferUsage usage);

template <typename T>
class BufferRAMPrecision;

template <BufferUsage U = BufferUsage::Static, typename T = vec3>
std::shared_ptr<BufferRAMPrecision<T>> createBufferRAM(std::vector<T> data) {
    return std::make_shared<BufferRAMPrecision<T>>(std::move(data), DataFormat<T>::get(), U);
}

struct BufferRamDispatcher {
    using type = std::shared_ptr<BufferRAM>;
    template <class T>
    std::shared_ptr<BufferRAM> dispatch(size_t size, BufferUsage usage) {
        typedef typename T::type F;
        return std::make_shared<BufferRAMPrecision<F>>(size, usage);
    }
};

}  // namespace

#endif  // IVW_BUFFER_RAM_H
