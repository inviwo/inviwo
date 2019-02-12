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

#ifndef IVW_BUFFER_REPRESENTATION_H
#define IVW_BUFFER_REPRESENTATION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/datarepresentation.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>

namespace inviwo {

class BufferBase;

/**
 * \ingroup datastructures
 */
class IVW_CORE_API BufferRepresentation : public DataRepresentation<BufferBase> {
public:
    virtual BufferRepresentation* clone() const override = 0;
    virtual ~BufferRepresentation() = default;
    virtual void setSize(size_t size) = 0;

    /**
     * Return the number of elements in the buffer.
     */
    virtual size_t getSize() const = 0;
    /**
     * Return size of buffer element in bytes.
     */
    virtual size_t getSizeOfElement() const;
    BufferUsage getBufferUsage() const;
    BufferTarget getBufferTarget() const;

protected:
    BufferRepresentation(const DataFormatBase* format, BufferUsage usage = BufferUsage::Static,
                         BufferTarget target = BufferTarget::Data);
    BufferRepresentation(const BufferRepresentation& rhs) = default;
    BufferRepresentation& operator=(const BufferRepresentation& that) = default;

    BufferUsage usage_;
    BufferTarget target_;
};

}  // namespace inviwo

#endif  // IVW_BUFFER_REPRESENTATION_H
