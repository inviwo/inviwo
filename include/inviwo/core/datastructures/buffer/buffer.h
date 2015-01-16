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

enum BufferUsage {
    STATIC,
    DYNAMIC
};

class IVW_CORE_API Buffer : public Data {

public:
    Buffer(size_t size,  const DataFormatBase* format = DataFormatBase::get(), BufferType type = POSITION_ATTRIB, BufferUsage usage = STATIC);
    Buffer(const Buffer& rhs);
    Buffer& operator=(const Buffer& that);
    virtual Buffer* clone() const;
    virtual ~Buffer();

    void resizeBufferRepresentations(Buffer* targetBuffer, size_t targetSize);

    size_t getSize() const;
    void setSize(size_t size);

    size_t getSizeInBytes();
    BufferType getBufferType() const { return type_; }

protected:
    virtual DataRepresentation* createDefaultRepresentation();
private:
    size_t size_;
    BufferType type_;
    BufferUsage usage_;
};

template<typename T, size_t B, BufferType A>
class BufferPrecision : public Buffer {

public:
    BufferPrecision(size_t size = 0, BufferUsage usage = STATIC)
        : Buffer(size, DataFormat<T,B>::get(), A, usage) {
    }
    BufferPrecision(BufferUsage usage)
        : Buffer(0, DataFormat<T,B>::get(), A, usage) {
    }
    BufferPrecision(const BufferPrecision& rhs)
        : Buffer(rhs) {
    }
    BufferPrecision& operator=(const BufferPrecision& that) {
        if (this != &that) {
            Buffer::operator=(that);
        }

        return *this;
    }
    virtual BufferPrecision<T, B, A>* clone() const {
        return new BufferPrecision<T, B, A>(*this);
    }

    virtual ~BufferPrecision() { }

private:
    static const DataFormatBase* defaultformat() {
        return  DataFormat<T, B>::get();
    }

};

#define DataFormatBuffers(D, BUFFER_TYPE) BufferPrecision<D::type, D::bits, BUFFER_TYPE>

typedef DataFormatBuffers(DataVec2FLOAT32, POSITION_ATTRIB) Position2dBuffer;
typedef DataFormatBuffers(DataVec2FLOAT32, TEXCOORD_ATTRIB) TexCoord2dBuffer;
typedef DataFormatBuffers(DataVec3FLOAT32, POSITION_ATTRIB) Position3dBuffer;
typedef DataFormatBuffers(DataVec4FLOAT32, COLOR_ATTRIB) ColorBuffer;
typedef DataFormatBuffers(DataVec3FLOAT32, NORMAL_ATTRIB) NormalBuffer;
typedef DataFormatBuffers(DataVec3FLOAT32, TEXCOORD_ATTRIB) TexCoord3dBuffer;
typedef DataFormatBuffers(DataFLOAT32, CURVATURE_ATTRIB) CurvatureBuffer;
typedef DataFormatBuffers(DataUINT32, INDEX_ATTRIB) IndexBuffer;

#define DataFormatIdMacro(i) typedef BufferPrecision<Data##i::type, Data##i::bits, POSITION_ATTRIB> Buffer_##i;
#include <inviwo/core/util/formatsdefinefunc.h>


} // namespace

#endif // IVW_BUFFER_H
