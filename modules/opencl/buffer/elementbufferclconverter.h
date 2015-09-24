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

#ifndef IVW_ELEMENTBUFFERCL_CONVERTER_H
#define IVW_ELEMENTBUFFERCL_CONVERTER_H

#include <inviwo/core/common/inviwo.h>
#include <modules/opencl/buffer/elementbuffercl.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opengl/buffer/elementbufferglconverter.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API ElementBufferRAM2CLConverter
    : public RepresentationConverterType<BufferRAM, ElementBufferCL> {
public:
    virtual std::shared_ptr<ElementBufferCL> createFrom(
        std::shared_ptr<const BufferRAM> source) const override;
    virtual void update(std::shared_ptr<const BufferRAM> source,
                        std::shared_ptr<ElementBufferCL> destination) const override;
};

class IVW_MODULE_OPENCL_API ElementBufferCL2RAMConverter
    : public RepresentationConverterType<ElementBufferCL, BufferRAM> {
public:
    virtual std::shared_ptr<BufferRAM> createFrom(
        std::shared_ptr<const ElementBufferCL> source) const override;
    virtual void update(std::shared_ptr<const ElementBufferCL> source,
                        std::shared_ptr<BufferRAM> destination) const override;
};

}  // namespace inviwo

#endif  // IVW_ELEMENTBUFFERCL_CONVERTER_H