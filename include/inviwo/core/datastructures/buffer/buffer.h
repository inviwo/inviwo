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

#ifndef IVW_BUFFER_H
#define IVW_BUFFER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <initializer_list>

namespace inviwo {

class IVW_CORE_API BufferBase : public Data<BufferRepresentation> {
public:
    BufferBase(size_t size, const DataFormatBase* format, BufferUsage usage);
    BufferBase(const BufferBase& rhs) = default;
    BufferBase& operator=(const BufferBase& that) = default;

    virtual BufferBase* clone() const override = 0;
    virtual ~BufferBase() = default;

    /**
     * Set the number of elements in the buffer. This is destructive, the data will not be
     * preserved.
     * @note Resizes the last valid representation and erases all representations.
     * Last valid representation will remain valid after changing the size.
     */
    void setSize(size_t size);
    size_t getSize() const;

    size_t getSizeInBytes() const;
    BufferUsage getBufferUsage() const;

    static uvec3 COLOR_CODE;
    static const std::string CLASS_IDENTIFIER;

protected:
    size_t size_;
    BufferUsage usage_;
};

template <typename T>
class Buffer : public BufferBase {
public:
    Buffer(size_t size, BufferUsage usage = BufferUsage::STATIC);
    Buffer(BufferUsage usage = BufferUsage::STATIC);
    Buffer(std::shared_ptr<BufferRAMPrecision<T>> repr);
    Buffer(const Buffer<T>& rhs) = default;
    Buffer<T>& operator=(const Buffer<T>& that) = default;
    virtual Buffer<T>* clone() const override;
    virtual ~Buffer() = default;

    BufferRAMPrecision<T>* getEditableRAMRepresentation();
    const BufferRAMPrecision<T>* getRAMRepresentation() const;

protected:
    virtual std::shared_ptr<BufferRepresentation> createDefaultRepresentation() const override;
};

// Used for index buffers
using IndexBuffer = Buffer<std::uint32_t>;

namespace util {
inline std::shared_ptr<IndexBuffer> makeIndexBuffer(std::initializer_list<std::uint32_t> data) {
    auto indexBufferRAM =
        std::make_shared<IndexBufferRAM>(std::vector<std::uint32_t>(std::move(data)));
    auto indices = std::make_shared<IndexBuffer>(indexBufferRAM);
    return indices;
}

template <typename T = vec3, BufferUsage U = BufferUsage::STATIC>
std::shared_ptr<Buffer<T>> makeBuffer(std::initializer_list<T> data) {
    auto repr = std::make_shared<BufferRAMPrecision<T>>(std::vector<T>(std::move(data)), U);
    auto buffer = std::make_shared<Buffer<T>>(repr);
    return buffer;
}

}  // namespace

template <typename T>
Buffer<T>::Buffer(std::shared_ptr<BufferRAMPrecision<T>> repr)
    : BufferBase(repr->getSize(), repr->getDataFormat(), repr->getBufferUsage()) {
    addRepresentation(repr);
}

template <typename T>
Buffer<T>::Buffer(size_t size, BufferUsage usage /*= BufferUsage::STATIC*/)
    : BufferBase(size, DataFormat<T>::get(), usage) {}

template <typename T>
Buffer<T>::Buffer(BufferUsage usage /*= BufferUsage::STATIC*/)
    : BufferBase(0, DataFormat<T>::get(), usage) {}

template <typename T>
Buffer<T>* Buffer<T>::clone() const {
    return new Buffer<T>(*this);
}

template <typename T>
const BufferRAMPrecision<T>* Buffer<T>::getRAMRepresentation() const {
    if (auto res = dynamic_cast<const BufferRAMPrecision<T>*>(getRepresentation<BufferRAM>())) {
        return res;
    } else {
        throw Exception("Unable to create requested RAM representation", IvwContext);
    }
}

template <typename T>
BufferRAMPrecision<T>* Buffer<T>::getEditableRAMRepresentation() {
    if (auto res = dynamic_cast<BufferRAMPrecision<T>*>(getEditableRepresentation<BufferRAM>())) {
        return res;
    } else {
        throw Exception("Unable to create requested RAM representation", IvwContext);
    }
}

template <typename T>
std::shared_ptr<BufferRepresentation> inviwo::Buffer<T>::createDefaultRepresentation() const {
    return std::make_shared<BufferRAMPrecision<T>>(size_, usage_);
}

}  // namespace

#endif  // IVW_BUFFER_H
