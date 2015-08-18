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

namespace inviwo {

enum BufferType {
    POSITION_ATTRIB,
    NORMAL_ATTRIB,
    COLOR_ATTRIB,
    TEXCOORD_ATTRIB,
    CURVATURE_ATTRIB,
    INDEX_ATTRIB,
    NUMBER_OF_BUFFER_TYPES
};

enum BufferUsage { STATIC, DYNAMIC };

class IVW_CORE_API Buffer : public Data {
public:
    Buffer(size_t size, const DataFormatBase* format = DataFormatBase::get(),
           BufferType type = POSITION_ATTRIB, BufferUsage usage = STATIC);
    Buffer(const Buffer& rhs);
    Buffer& operator=(const Buffer& that);
    virtual Buffer* clone() const;
    virtual ~Buffer();

    void resizeBufferRepresentations(Buffer* targetBuffer, size_t targetSize);

    size_t getSize() const;

    /**
     * Set the number of elements in the buffer. This is destructive, the data will not be
     * preserved.
     * @note Resizes the last valid representation and erases all representations.
     * Last valid representation will remain valid after changing the size.
     */
    void setSize(size_t size);

    size_t getSizeInBytes();
    BufferType getBufferType() const { return type_; }

    static uvec3 COLOR_CODE;
    static const std::string CLASS_IDENTIFIER;

protected:
    virtual DataRepresentation* createDefaultRepresentation();

private:
    size_t size_;
    BufferType type_;
    BufferUsage usage_;
};

template <typename T, BufferType A = POSITION_ATTRIB>
class BufferPrecision : public Buffer {
public:
    BufferPrecision(size_t size = 0, BufferUsage usage = STATIC)
        : Buffer(size, DataFormat<T>::get(), A, usage) {}
    BufferPrecision(BufferUsage usage) : Buffer(0, DataFormat<T>::get(), A, usage) {}
    BufferPrecision(const BufferPrecision& rhs) : Buffer(rhs) {}
    BufferPrecision& operator=(const BufferPrecision& that) {
        if (this != &that) {
            Buffer::operator=(that);
        }
        return *this;
    }
    virtual BufferPrecision<T, A>* clone() const { return new BufferPrecision<T, A>(*this); }

    virtual ~BufferPrecision() {}

private:
    static const DataFormatBase* defaultformat() { return DataFormat<T>::get(); }
};

typedef BufferPrecision<vec2, POSITION_ATTRIB> Position2dBuffer;
typedef BufferPrecision<vec2, TEXCOORD_ATTRIB> TexCoord2dBuffer;
typedef BufferPrecision<vec3, POSITION_ATTRIB> Position3dBuffer;
typedef BufferPrecision<vec4, COLOR_ATTRIB> ColorBuffer;
typedef BufferPrecision<vec3, NORMAL_ATTRIB> NormalBuffer;
typedef BufferPrecision<vec3, TEXCOORD_ATTRIB> TexCoord3dBuffer;
typedef BufferPrecision<float, CURVATURE_ATTRIB> CurvatureBuffer;
typedef BufferPrecision<std::uint32_t, INDEX_ATTRIB> IndexBuffer;

// Scalar buffers
typedef BufferPrecision<float, POSITION_ATTRIB> BufferFloat32;
typedef BufferPrecision<std::int32_t, INDEX_ATTRIB> BufferInt32;
typedef BufferPrecision<std::uint32_t, INDEX_ATTRIB> BufferUInt32;
typedef BufferPrecision<double, POSITION_ATTRIB> BufferFloat64;

// Vector buffers
typedef BufferPrecision<vec2, TEXCOORD_ATTRIB> BufferVec2Float32;
typedef BufferPrecision<vec3, POSITION_ATTRIB> BufferVec3Float32;
typedef BufferPrecision<vec4, COLOR_ATTRIB>    BufferVec4Float32;

typedef BufferPrecision<dvec2, TEXCOORD_ATTRIB> BufferVec2Float64;
typedef BufferPrecision<dvec3, POSITION_ATTRIB> BufferVec3Float64;
typedef BufferPrecision<dvec4, COLOR_ATTRIB>    BufferVec4Float64;

typedef BufferPrecision<ivec2, POSITION_ATTRIB> BufferVec2Int32;
typedef BufferPrecision<ivec3, POSITION_ATTRIB> BufferVec3Int32;
typedef BufferPrecision<ivec4, POSITION_ATTRIB> BufferVec4Int32;

typedef BufferPrecision<uvec2, POSITION_ATTRIB> BufferVec2UInt32;
typedef BufferPrecision<uvec3, POSITION_ATTRIB> BufferVec3UInt32;
typedef BufferPrecision<uvec4, POSITION_ATTRIB> BufferVec4UInt32;

}  // namespace

#endif  // IVW_BUFFER_H
