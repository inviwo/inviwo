/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/bufferram.h>         // for createBufferRAM
#include <inviwo/core/datastructures/representationconverter.h>  // for ConverterException
#include <inviwo/core/util/formats.h>                            // for DataFormatBase
#include <inviwo/core/util/sourcecontext.h>                      // for IVW_CONTEXT
#include <modules/opengl/buffer/buffergl.h>                      // for BufferGL

#include <string>       // for operator+, string
#include <type_traits>  // for remove_extent_t

namespace inviwo {

std::shared_ptr<BufferGL> BufferRAM2GLConverter::createFrom(
    std::shared_ptr<const BufferRAM> src) const {
    auto dst = std::make_shared<BufferGL>(src->getSize(), src->getDataFormat(),
                                          src->getBufferUsage(), src->getBufferTarget());

    if (!dst) {
        throw ConverterException(IVW_CONTEXT, "Cannot convert format '{}' from RAM to GL",
                                 *src->getDataFormat());
    }

    dst->upload(src->getData(), src->getSize() * src->getSizeOfElement());
    return dst;
}

void BufferRAM2GLConverter::update(std::shared_ptr<const BufferRAM> src,
                                   std::shared_ptr<BufferGL> dst) const {
    dst->setSize(src->getSize());
    dst->upload(src->getData(), src->getSize() * src->getSizeOfElement());
}

std::shared_ptr<BufferRAM> BufferGL2RAMConverter::createFrom(
    std::shared_ptr<const BufferGL> src) const {
    auto dst = createBufferRAM(src->getSize(), src->getDataFormat(), src->getBufferUsage(),
                               src->getBufferTarget());

    if (!dst) {
        throw ConverterException(IVW_CONTEXT, "Cannot convert format '{}' from GL to RAM",
                                 *src->getDataFormat());
    }
    src->download(dst->getData());
    return dst;
}

void BufferGL2RAMConverter::update(std::shared_ptr<const BufferGL> src,
                                   std::shared_ptr<BufferRAM> dst) const {
    dst->setSize(src->getSize());
    src->download(dst->getData());
}

}  // namespace inviwo
