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

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

namespace inviwo {

BufferBase::BufferBase(size_t size, const DataFormatBase* format, BufferUsage usage)
    : Data<BufferRepresentation>(format), size_(size), usage_(usage) {}

size_t BufferBase::getSizeInBytes() const { return size_ * dataFormatBase_->getSize(); }

inviwo::BufferUsage BufferBase::getBufferUsage() const { return usage_; }

inviwo::uvec3 BufferBase::COLOR_CODE = uvec3(255, 113, 0);

const std::string BufferBase::CLASS_IDENTIFIER = "org.inviwo.Buffer";

void BufferBase::setSize(size_t size) {
    if (size != size_) {
        size_ = size;

        if (lastValidRepresentation_) {
            // Resize last valid representation
            lastValidRepresentation_->setSize(size);
            removeOtherRepresentations(lastValidRepresentation_.get());
        }
    }
}

size_t BufferBase::getSize() const {
    // We need to update the size if a representation has changed size
    if (lastValidRepresentation_)
        const_cast<BufferBase*>(this)->size_ = lastValidRepresentation_->getSize();

    return size_;
}



}
