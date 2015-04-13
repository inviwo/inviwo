/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/ports/bufferport.h>


namespace inviwo {

uvec3 BufferInport::colorCode = uvec3(255,113,0);

// Buffer Inport
BufferInport::BufferInport(std::string identifier, InvalidationLevel invalidationLevel)
    : DataInport<Buffer>(identifier, invalidationLevel)
{}

BufferInport::~BufferInport() {
}

uvec3 BufferInport::getColorCode() const {
    return BufferInport::colorCode;
}

// Buffer Outport
BufferOutport::BufferOutport(std::string identifier, InvalidationLevel invalidationLevel)
    : DataOutport<Buffer>(identifier, invalidationLevel) {

}

BufferOutport::BufferOutport(std::string identifier, size_t size, const DataFormatBase* format, InvalidationLevel invalidationLevel)
    : DataOutport<Buffer>(identifier, invalidationLevel) {
    setData(new Buffer(size, format), true);
}

BufferOutport::~BufferOutport() {}

uvec3 BufferOutport::getColorCode() const {
    return BufferInport::colorCode;
}

} // namespace