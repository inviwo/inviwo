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

#include <modules/opencl/buffer/bufferclconverter.h>
#include <modules/opencl/buffer/bufferclglconverter.h>
#include <modules/opencl/syncclgl.h>
#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {

std::shared_ptr<BufferRAM> BufferCLGL2RAMConverter::createFrom(
    std::shared_ptr<const BufferCLGL> src) const {
    size_t size = src->getSize();
    auto destination = createBufferRAM(size, src->getDataFormat(), src->getBufferUsage());

    if (destination) {
        src->getBufferGL()->download(destination->getData());
    } else {
        LogError("Invalid conversion or not implemented");
    }

    return destination;
}

void BufferCLGL2RAMConverter::update(std::shared_ptr<const BufferCLGL> src,
                                     std::shared_ptr<BufferRAM> dst) const {
    if (src->getSize() != dst->getSize()) {
        dst->setSize(src->getSize());
    }

    src->getBufferGL()->download(dst->getData());
}

std::shared_ptr<BufferGL> BufferCLGL2GLConverter::createFrom(
    std::shared_ptr<const BufferCLGL> src) const {
    return std::make_shared<BufferGL>(src->getSize(), src->getDataFormat(), src->getBufferUsage(),
                                      src->getBufferGL());
}

void BufferCLGL2GLConverter::update(std::shared_ptr<const BufferCLGL> source,
                                    std::shared_ptr<BufferGL> destination) const {
    // Do nothing since they share data
}

std::shared_ptr<BufferCLGL> BufferGL2CLGLConverter::createFrom(
    std::shared_ptr<const BufferGL> src) const {
    return std::make_shared<BufferCLGL>(src->getSize(), src->getDataFormat(), src->getBufferUsage(),
                                        src->getBufferObject());
}

void BufferGL2CLGLConverter::update(std::shared_ptr<const BufferGL> src,
                                    std::shared_ptr<BufferCLGL> dst) const {
    if (src->getSize() != dst->getSize()) {
        dst->setSize(src->getSize());
    }
}

std::shared_ptr<BufferCL> BufferCLGL2CLConverter::createFrom(
    std::shared_ptr<const BufferCLGL> src) const {
    size_t size = src->getSize();
    auto destination =
        std::make_shared<BufferCL>(size, src->getDataFormat(), src->getBufferUsage());
    {
        SyncCLGL glSync;
        glSync.addToAquireGLObjectList(src.get());
        glSync.aquireAllObjects();
        OpenCL::getPtr()->getQueue().enqueueCopyBuffer(src->get(), destination->get(), 0, 0,
                                                       src->getSize() * src->getSizeOfElement());
    }
    return destination;
}
void BufferCLGL2CLConverter::update(std::shared_ptr<const BufferCLGL> src,
                                    std::shared_ptr<BufferCL> dst) const {
    if (src->getSize() != dst->getSize()) {
        dst->setSize(src->getSize());
    }

    {
        SyncCLGL glSync;
        glSync.addToAquireGLObjectList(src.get());
        glSync.aquireAllObjects();
        OpenCL::getPtr()->getQueue().enqueueCopyBuffer(src->get(), dst->get(), 0, 0,
                                                       src->getSize() * src->getSizeOfElement());
    }
}

}  // namespace
