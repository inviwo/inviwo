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

class IVW_CORE_API Buffer : public Data<BufferRepresentation> {
public:
    Buffer(size_t size, const DataFormatBase* format = DataFormatBase::get(),
           BufferType type = BufferType::POSITION_ATTRIB, BufferUsage usage = BufferUsage::STATIC);

    template <typename T>
    Buffer(std::shared_ptr<BufferRAMPrecision<T>> repr);

    Buffer(const Buffer& rhs) = default;
    Buffer& operator=(const Buffer& that) = default;
    virtual Buffer* clone() const override;
    virtual ~Buffer() = default;

    /**
     * Set the number of elements in the buffer. This is destructive, the data will not be
     * preserved.
     * @note Resizes the last valid representation and erases all representations.
     * Last valid representation will remain valid after changing the size.
     */
    void setSize(size_t size);
    size_t getSize() const;

    size_t getSizeInBytes();
    BufferType getBufferType() const { return type_; }
    BufferUsage getBufferUsage() const { return usage_; }

    static uvec3 COLOR_CODE;
    static const std::string CLASS_IDENTIFIER;

protected:
    virtual std::shared_ptr<BufferRepresentation> createDefaultRepresentation() const override;

private:
    size_t size_;
    BufferType type_;
    BufferUsage usage_;
};

template <typename T>
inviwo::Buffer::Buffer(std::shared_ptr<BufferRAMPrecision<T>> repr)
    : Data<BufferRepresentation>(repr->getDataFormat())
    , size_(repr->getSize())
    , type_(repr->getBufferType())
    , usage_(repr->getBufferUsage()) {
    addRepresentation(repr);
}

template <typename T, BufferType A = BufferType::POSITION_ATTRIB>
class BufferPrecision : public Buffer {
public:
    BufferPrecision(size_t size = 0, BufferUsage usage = BufferUsage::STATIC)
        : Buffer(size, DataFormat<T>::get(), A, usage) {}

    BufferPrecision(BufferUsage usage) : Buffer(0, DataFormat<T>::get(), A, usage) {}
    BufferPrecision(std::shared_ptr<BufferRAMPrecision<T>> repr);

    BufferPrecision(const BufferPrecision& rhs) = default;
    BufferPrecision& operator=(const BufferPrecision& that) = default;
    virtual BufferPrecision<T, A>* clone() const { return new BufferPrecision<T, A>(*this); }
    virtual ~BufferPrecision() = default;

    BufferRAMPrecision<T>* getEditableRAMRepresentation();
    const BufferRAMPrecision<T>* getRAMRepresentation() const;

private:
    static const DataFormatBase* defaultformat() { return DataFormat<T>::get(); }
};

template <typename T, BufferType A>
inviwo::BufferPrecision<T, A>::BufferPrecision(std::shared_ptr<BufferRAMPrecision<T>> repr)
    : Buffer(repr->getSize(), repr->getDataFormat(), repr->getBufferType(),
             repr->getBufferUsage()) {
    addRepresentation(repr);
}

template <typename T, BufferType A>
const BufferRAMPrecision<T>* inviwo::BufferPrecision<T, A>::getRAMRepresentation() const {
    if (auto res = dynamic_cast<const BufferRAMPrecision<T>*>(getRepresentation<BufferRAM>())) {
        return res;
    } else {
        throw Exception("Unable to create requested RAM representation", IvwContext);
    }
}

template <typename T, BufferType A>
BufferRAMPrecision<T>* inviwo::BufferPrecision<T, A>::getEditableRAMRepresentation() {
    if (auto res = dynamic_cast<BufferRAMPrecision<T>*>(getEditableRepresentation<BufferRAM>())) {
        return res;
    } else {
        throw Exception("Unable to create requested RAM representation", IvwContext);
    }
}

typedef BufferPrecision<vec2, BufferType::POSITION_ATTRIB> Position2dBuffer;
typedef BufferPrecision<vec2, BufferType::TEXCOORD_ATTRIB> TexCoord2dBuffer;
typedef BufferPrecision<vec3, BufferType::POSITION_ATTRIB> Position3dBuffer;
typedef BufferPrecision<vec4, BufferType::COLOR_ATTRIB> ColorBuffer;
typedef BufferPrecision<vec3, BufferType::NORMAL_ATTRIB> NormalBuffer;
typedef BufferPrecision<vec3, BufferType::TEXCOORD_ATTRIB> TexCoord3dBuffer;
typedef BufferPrecision<float, BufferType::CURVATURE_ATTRIB> CurvatureBuffer;
typedef BufferPrecision<std::uint32_t, BufferType::INDEX_ATTRIB> IndexBuffer;

// Scalar buffers
typedef BufferPrecision<std::uint8_t> BufferUInt8;
typedef BufferPrecision<float, BufferType::POSITION_ATTRIB> BufferFloat32;
typedef BufferPrecision<std::int32_t, BufferType::INDEX_ATTRIB> BufferInt32;
typedef BufferPrecision<std::uint32_t, BufferType::INDEX_ATTRIB> BufferUInt32;
typedef BufferPrecision<double, BufferType::POSITION_ATTRIB> BufferFloat64;

// Vector buffers
typedef BufferPrecision<vec2, BufferType::TEXCOORD_ATTRIB> BufferVec2Float32;
typedef BufferPrecision<vec3, BufferType::POSITION_ATTRIB> BufferVec3Float32;
typedef BufferPrecision<vec4, BufferType::COLOR_ATTRIB> BufferVec4Float32;

typedef BufferPrecision<dvec2, BufferType::TEXCOORD_ATTRIB> BufferVec2Float64;
typedef BufferPrecision<dvec3, BufferType::POSITION_ATTRIB> BufferVec3Float64;
typedef BufferPrecision<dvec4, BufferType::COLOR_ATTRIB> BufferVec4Float64;

typedef BufferPrecision<ivec2, BufferType::POSITION_ATTRIB> BufferVec2Int32;
typedef BufferPrecision<ivec3, BufferType::POSITION_ATTRIB> BufferVec3Int32;
typedef BufferPrecision<ivec4, BufferType::POSITION_ATTRIB> BufferVec4Int32;

typedef BufferPrecision<uvec2, BufferType::POSITION_ATTRIB> BufferVec2UInt32;
typedef BufferPrecision<uvec3, BufferType::POSITION_ATTRIB> BufferVec3UInt32;
typedef BufferPrecision<uvec4, BufferType::POSITION_ATTRIB> BufferVec4UInt32;

namespace util {
inline std::shared_ptr<IndexBuffer> makeIndexBuffer(std::initializer_list<std::uint32_t> data) {
    auto indexBufferRAM =
        std::make_shared<IndexBufferRAM>(std::vector<std::uint32_t>(std::move(data)));
    auto indices = std::make_shared<IndexBuffer>(indexBufferRAM);
    return indices;
}

template <typename T = vec3, BufferType type = BufferType::POSITION_ATTRIB,
          BufferUsage usage = BufferUsage::STATIC>
std::shared_ptr<Buffer> makeBuffer(std::initializer_list<T> data) {
    auto repr = std::make_shared<BufferRAMPrecision<T>>(std::vector<T>(std::move(data)),
                                                        DataFormat<T>::get(), type, usage);
    auto buffer = std::make_shared<Buffer>(repr);
    return buffer;
}
}

}  // namespace

#endif  // IVW_BUFFER_H
