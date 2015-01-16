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

#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

namespace inviwo {

BufferRAM::BufferRAM(size_t size, const DataFormatBase* format, BufferType type, BufferUsage usage)
    : BufferRepresentation(size, format, type, usage) {
    BufferRAM::initialize();
}
BufferRAM::BufferRAM(const BufferRAM& rhs) : BufferRepresentation(rhs) {
    BufferRAM::initialize();
}
BufferRAM& BufferRAM::operator=(const BufferRAM& that) {
    if (this != &that) {
        deinitialize();
        BufferRepresentation::operator=(that);
        initialize();
    }

    return *this;
}
BufferRAM::~BufferRAM() {
    deinitialize();
}

void BufferRAM::initialize() {
}

void BufferRAM::deinitialize() {
    // Make sure that data is deinitialized in
    // child class (should not delete void pointer
    // since destructor will not be called for object).
}

void BufferRAM::setSize(size_t size) {
    size_ = size;
    // Delete and reallocate data_ to new size
    deinitialize();
    initialize();
}

BufferRAM* createBufferRAM(size_t size, const DataFormatBase* format, BufferType type, BufferUsage usage) {
    switch (format->getId())
    {
        case DataFormatEnums::NOT_SPECIALIZED:
            LogErrorCustom("createBufferRAM", "Invalid format");
            return NULL;
#define DataFormatIdMacro(i) case DataFormatEnums::i: return new BufferRAMCustomPrecision<Data##i::type, Data##i::bits>(size, format, type, usage); break;
#include <inviwo/core/util/formatsdefinefunc.h>

        default:
            LogErrorCustom("createBufferRAM", "Invalid format or not implemented");
            return NULL;
    }

    return NULL;
}


}
