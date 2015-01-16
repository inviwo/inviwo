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

#ifndef IVW_ELEMENTBUFFERCL_CONVERTER_H
#define IVW_ELEMENTBUFFERCL_CONVERTER_H

#include <inviwo/core/common/inviwo.h>
#include <modules/opencl/buffer/elementbuffercl.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opengl/buffer/elementbufferglconverter.h>

namespace inviwo {


class IVW_MODULE_OPENCL_API ElementBufferRAM2CLConverter : public RepresentationConverterType<ElementBufferCL> {

public:
    ElementBufferRAM2CLConverter();
    virtual ~ElementBufferRAM2CLConverter();

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const BufferRAM*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API ElementBufferCL2RAMConverter : public RepresentationConverterType<BufferRAM> {

public:
    ElementBufferCL2RAMConverter();
    virtual ~ElementBufferCL2RAMConverter();

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const ElementBufferCL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API ElementBufferGL2CLConverter : public RepresentationConverterPackage<ElementBufferCL> {

public:
    ElementBufferGL2CLConverter() : RepresentationConverterPackage<ElementBufferCL>() {
        addConverter(new ElementBufferGL2RAMConverter());
        addConverter(new ElementBufferRAM2CLConverter());
    };
    virtual ~ElementBufferGL2CLConverter() {};
};

class IVW_MODULE_OPENCL_API ElementBufferCL2GLConverter : public RepresentationConverterPackage<BufferGL> {

public:
    ElementBufferCL2GLConverter() : RepresentationConverterPackage<BufferGL>() {
        addConverter(new ElementBufferCL2RAMConverter());
        addConverter(new ElementBufferRAM2GLConverter());
    };
    virtual ~ElementBufferCL2GLConverter() {};
};


} // namespace inviwo

#endif // IVW_ELEMENTBUFFERCL_CONVERTER_H