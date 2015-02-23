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

#include <modules/opengl/buffer/bufferglconverter.h>

namespace inviwo {

BufferRAM2GLConverter::BufferRAM2GLConverter()
    : RepresentationConverterType<BufferGL>() {
}

BufferRAM2GLConverter::~BufferRAM2GLConverter() {}

DataRepresentation* BufferRAM2GLConverter::createFrom(const DataRepresentation* source) {
    const BufferRAM* bufferRAM = static_cast<const BufferRAM*>(source);
    BufferGL* bufferGL = new BufferGL(bufferRAM->getSize(), bufferRAM->getDataFormat(), bufferRAM->getBufferType(),
                                      bufferRAM->getBufferUsage());
    bufferGL->initialize(bufferRAM->getData(), bufferRAM->getSize()*bufferRAM->getSizeOfElement());
    return bufferGL;
}
void BufferRAM2GLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const BufferRAM* src = static_cast<const BufferRAM*>(source);
    BufferGL* dst = static_cast<BufferGL*>(destination);

    if (src->getSize() != dst->getSize()) 
        dst->initialize(src->getData(), src->getSize()*src->getSizeOfElement());
    else
        dst->upload(src->getData(), src->getSize()*src->getSizeOfElement());
}


BufferGL2RAMConverter::BufferGL2RAMConverter()
    : RepresentationConverterType<BufferRAM>()
{}

BufferGL2RAMConverter::~BufferGL2RAMConverter() {}

DataRepresentation* BufferGL2RAMConverter::createFrom(const DataRepresentation* source) {
    const BufferGL* src = static_cast<const BufferGL*>(source);
    BufferRAM* dst = createBufferRAM(src->getSize(), src->getDataFormat(), src->getBufferType(), src->getBufferUsage());

    if (dst) {
        src->download(dst->getData());
        return dst;
    } else {
        LogError("Cannot convert format from GL to RAM:" << src->getDataFormat()->getString());
    }

    return nullptr;
}

void BufferGL2RAMConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const BufferGL* src = static_cast<const BufferGL*>(source);
    BufferRAM* dst = static_cast<BufferRAM*>(destination);

    if (src->getSize() != dst->getSize()) {
        dst->setSize(src->getSize());
    }

    src->download(dst->getData());
}


} // namespace