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

#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>
#include <modules/opencl/buffer/bufferclconverter.h>
#include <modules/opencl/buffer/elementbufferclglconverter.h>
#include <modules/opencl/syncclgl.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opengl/buffer/elementbufferglconverter.h>

namespace inviwo {

std::shared_ptr<ElementBufferGL> ElementBufferCLGL2GLConverter::createFrom(
    std::shared_ptr<const ElementBufferCLGL> src) const {
    return std::make_shared<ElementBufferGL>(src->getSize(), src->getDataFormat(),
                                             src->getBufferUsage(), src->getElementBufferObject());
}

void ElementBufferCLGL2GLConverter::update(std::shared_ptr<const ElementBufferCLGL> source,
                                           std::shared_ptr<ElementBufferGL> destination) const {
    // Do nothing since they share data
}

std::shared_ptr<ElementBufferCLGL> ElementBufferGL2CLGLConverter::createFrom(
    std::shared_ptr<const ElementBufferGL> src) const {
    return std::make_shared<ElementBufferCLGL>(
        src->getSize(), src->getDataFormat(), src->getBufferUsage(), src->getElementBufferObject());
}

void ElementBufferGL2CLGLConverter::update(std::shared_ptr<const ElementBufferGL> src,
                                           std::shared_ptr<ElementBufferCLGL> dst) const {
    if (src->getSize() != dst->getSize()) {
        dst->setSize(src->getSize());
    }
}

}  // namespace
