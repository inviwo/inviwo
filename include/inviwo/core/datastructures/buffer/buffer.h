/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <inviwo/core/util/document.h>

namespace inviwo {

class DataFormatBase;

class IVW_CORE_API BufferBase : public Data<BufferBase, BufferRepresentation> {
public:
    BufferBase(size_t defaultSize, const DataFormatBase* defaultFormat, BufferUsage usage,
               BufferTarget target);
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
    BufferTarget getBufferTarget() const;
    /**
     * Set the format of the data.
     * @see DataFormatBase
     * @param format The format of the data.
     */
    // clang-format off
    [[ deprecated("use BufferRepresentation::setDataFormat() instead (deprecated since 2019-06-26)")]]
    void setDataFormat(const DataFormatBase* format);
    const DataFormatBase* getDataFormat() const;
    // clang-format on

    virtual void append(const BufferBase&) = 0;

    virtual Document getInfo() const = 0;
    static uvec3 colorCode;
    static const std::string classIdentifier;
    static const std::string dataName;

protected:
    size_t defaultSize_;
    BufferUsage usage_;
    BufferTarget target_;
    const DataFormatBase* defaultDataFormat_;
};

/**
 * \ingroup datastructures
 */
template <typename T, BufferTarget Target = BufferTarget::Data>
class Buffer : public BufferBase {
public:
    Buffer();
    explicit Buffer(size_t size, BufferUsage usage = BufferUsage::Static);
    explicit Buffer(BufferUsage usage);
    explicit Buffer(std::shared_ptr<BufferRAMPrecision<T, Target>> repr);
    Buffer(const Buffer<T, Target>& rhs) = default;
    Buffer<T, Target>& operator=(const Buffer<T, Target>& that) = default;
    virtual Buffer<T, Target>* clone() const override;
    virtual ~Buffer() = default;

    BufferRAMPrecision<T, Target>* getEditableRAMRepresentation();
    const BufferRAMPrecision<T, Target>* getRAMRepresentation() const;

    virtual void append(const BufferBase&) override;
    void append(const Buffer<T, Target>&);

    virtual Document getInfo() const override;
};

template <typename T, BufferTarget Target>
void Buffer<T, Target>::append(const Buffer<T, Target>& buffer) {
    getEditableRAMRepresentation()->append(buffer.getRAMRepresentation()->getDataContainer());
}

template <typename T, BufferTarget Target>
void Buffer<T, Target>::append(const BufferBase& buffer) {
    if (buffer.getDataFormat() != DataFormat<T>::get()) {
        throw Exception("Mismatched buffers: types does not match", IVW_CONTEXT);
    }
    if (buffer.getBufferTarget() != Target) {
        throw Exception("Mismatched buffers: Targets does not match", IVW_CONTEXT);
    }
    append(static_cast<const Buffer<T, Target>&>(buffer));
}

// Used for index buffers
using IndexBuffer = Buffer<std::uint32_t, BufferTarget::Index>;

namespace util {

inline std::shared_ptr<IndexBuffer> makeIndexBuffer(std::vector<std::uint32_t>&& data) {
    auto indexBufferRAM =
        std::make_shared<IndexBufferRAM>(std::vector<std::uint32_t>(std::move(data)));
    auto indices = std::make_shared<IndexBuffer>(indexBufferRAM);
    return indices;
}

template <typename T = vec3, BufferUsage U = BufferUsage::Static,
          BufferTarget Target = BufferTarget::Data>
std::shared_ptr<Buffer<T, Target>> makeBuffer(std::vector<T>&& data) {
    auto repr = std::make_shared<BufferRAMPrecision<T, Target>>(std::vector<T>(std::move(data)), U);
    auto buffer = std::make_shared<Buffer<T, Target>>(repr);
    return buffer;
}

struct IVW_CORE_API BufferDispatcher {
    using type = std::shared_ptr<BufferBase>;
    template <class T>
    std::shared_ptr<BufferBase> dispatch(size_t size, BufferUsage usage, BufferTarget target) {
        typedef typename T::type F;
        switch (target) {
            case BufferTarget::Index:
                return std::make_shared<Buffer<F, BufferTarget::Index>>(size, usage);
            case BufferTarget::Data:
            default:
                return std::make_shared<Buffer<F, BufferTarget::Data>>(size, usage);
        }
    }
};

}  // namespace util

template <typename T, BufferTarget Target>
Buffer<T, Target>::Buffer() : Buffer(BufferUsage::Static) {}

template <typename T, BufferTarget Target>
Buffer<T, Target>::Buffer(std::shared_ptr<BufferRAMPrecision<T, Target>> repr)
    : BufferBase(repr->getSize(), repr->getDataFormat(), repr->getBufferUsage(), Target) {
    addRepresentation(repr);
}

template <typename T, BufferTarget Target>
Buffer<T, Target>::Buffer(size_t size, BufferUsage usage)
    : BufferBase(size, DataFormat<T>::get(), usage, Target) {}

template <typename T, BufferTarget Target>
Buffer<T, Target>::Buffer(BufferUsage usage) : BufferBase(0, DataFormat<T>::get(), usage, Target) {}

template <typename T, BufferTarget Target>
Buffer<T, Target>* Buffer<T, Target>::clone() const {
    return new Buffer<T, Target>(*this);
}

template <typename T, BufferTarget Target>
const BufferRAMPrecision<T, Target>* Buffer<T, Target>::getRAMRepresentation() const {
    auto bufferRAM = getRepresentation<BufferRAM>();
    ivwAssert(bufferRAM->getDataFormat() == DataFormat<T>::get(),
              "Invalid format for buffer representation");
    ivwAssert(bufferRAM->getBufferTarget() == Target, "Invalid target for buffer representation");

    return static_cast<const BufferRAMPrecision<T, Target>*>(bufferRAM);
}

template <typename T, BufferTarget Target>
BufferRAMPrecision<T, Target>* Buffer<T, Target>::getEditableRAMRepresentation() {
    auto bufferRAM = getEditableRepresentation<BufferRAM>();
    ivwAssert(bufferRAM->getDataFormat() == DataFormat<T>::get(),
              "Invalid format for buffer representation");
    ivwAssert(bufferRAM->getBufferTarget() == Target, "Invalid target for buffer representation");

    return static_cast<BufferRAMPrecision<T, Target>*>(bufferRAM);
}

template <typename T, BufferTarget Target>
Document Buffer<T, Target>::getInfo() const {
    Document doc;
    doc.append("b", "Buffer", {{"style", "color:white;"}});
    return doc;
}

}  // namespace inviwo

#endif  // IVW_BUFFER_H
