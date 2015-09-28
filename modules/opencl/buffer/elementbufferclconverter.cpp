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

#include <modules/opencl/buffer/elementbufferclconverter.h>

namespace inviwo {

std::shared_ptr<ElementBufferCL> ElementBufferRAM2CLConverter::createFrom(
    std::shared_ptr<const BufferRAM> bufferRAM) const {
    return std::make_shared<ElementBufferCL>(bufferRAM->getSize(), bufferRAM->getDataFormat(),
                                             bufferRAM->getBufferUsage(), bufferRAM->getData());
}

void ElementBufferRAM2CLConverter::update(std::shared_ptr<const BufferRAM> src,
                                          std::shared_ptr<ElementBufferCL> dst) const {
    if (src->getSize() != dst->getSize()) {
        dst->setSize(src->getSize());
    }

    dst->upload(src->getData(), src->getSize() * src->getSizeOfElement());
}

std::shared_ptr<BufferRAM> ElementBufferCL2RAMConverter::createFrom(
    std::shared_ptr<const ElementBufferCL> src) const {
    auto dst = createBufferRAM(src->getSize(), src->getDataFormat(), src->getBufferUsage());
    src->download(dst->getData());
    return dst;
}

void ElementBufferCL2RAMConverter::update(std::shared_ptr<const ElementBufferCL> src,
                                          std::shared_ptr<BufferRAM> dst) const {
    if (src->getSize() != dst->getSize()) {
        dst->setSize(src->getSize());
    }

    src->download(dst->getData());
}

}  // end namespace