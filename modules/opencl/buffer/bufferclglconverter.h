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

#ifndef IVW_BUFFERCLGLCONVERTER_H
#define IVW_BUFFERCLGLCONVERTER_H

#include <inviwo/core/common/inviwo.h>
#include <modules/opencl/buffer/buffercl.h>
#include <modules/opencl/buffer/bufferclgl.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opengl/buffer/bufferglconverter.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API BufferRAM2CLGLConverter : public RepresentationConverterPackage<BufferCLGL> {

public:
    BufferRAM2CLGLConverter();
    virtual ~BufferRAM2CLGLConverter() {};
};

class IVW_MODULE_OPENCL_API BufferCLGL2RAMConverter : public RepresentationConverterType<BufferRAM> {

public:
    BufferCLGL2RAMConverter();
    virtual ~BufferCLGL2RAMConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const BufferCLGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API BufferCLGL2GLConverter : public RepresentationConverterType<BufferGL> {
public:
    BufferCLGL2GLConverter(): RepresentationConverterType<BufferGL>() {};
    virtual ~BufferCLGL2GLConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const BufferCLGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API BufferGL2CLGLConverter : public RepresentationConverterType<BufferCLGL> {

public:
    BufferGL2CLGLConverter() : RepresentationConverterType<BufferCLGL>() {};
    virtual ~BufferGL2CLGLConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const BufferGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API BufferCLGL2CLConverter : public RepresentationConverterType<BufferCL> {
public:
    BufferCLGL2CLConverter() : RepresentationConverterType<BufferCL>() {};
    virtual ~BufferCLGL2CLConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const BufferCLGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API BufferCL2CLGLConverter : public RepresentationConverterPackage<BufferCLGL> {
public:
    BufferCL2CLGLConverter();
    virtual ~BufferCL2CLGLConverter() {};
};

//class IVW_MODULE_OPENCL_API BufferDisk2CLGLConverter : public RepresentationConverterPackage<BufferCLGL> {
//
//public:
//    BufferDisk2CLGLConverter() : RepresentationConverterPackage<BufferCLGL>() {
//        addConverter(new BufferDisk2RAMConverter());
//        addConverter(new BufferRAM2GLConverter());
//        addConverter(new BufferGL2CLGLConverter());
//    };
//    virtual ~BufferDisk2CLGLConverter() {};
//};

} // namespace

#endif // IVW_BUFFERCLGLCONVERTER_H
