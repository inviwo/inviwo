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

#ifndef IVW_LAYERCLGLCONVERTER_H
#define IVW_LAYERCLGLCONVERTER_H

#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramconverter.h>
#include <modules/opengl/image/layerglconverter.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/image/layercl.h>
#include <modules/opencl/image/layerclgl.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API LayerRAM2CLGLConverter : public RepresentationConverterPackage<LayerCLGL> {

public:
    LayerRAM2CLGLConverter();
    virtual ~LayerRAM2CLGLConverter() {};
};



class IVW_MODULE_OPENCL_API LayerCLGL2RAMConverter : public RepresentationConverterType<LayerRAM> {

public:
    LayerCLGL2RAMConverter();
    virtual ~LayerCLGL2RAMConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const LayerCLGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API LayerCLGL2GLConverter : public RepresentationConverterType<LayerGL> {

public:
    LayerCLGL2GLConverter();
    virtual ~LayerCLGL2GLConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const LayerCLGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API LayerCLGL2CLConverter : public RepresentationConverterType<LayerCL> {

public:
    LayerCLGL2CLConverter() : RepresentationConverterType<LayerCL>() {};
    virtual ~LayerCLGL2CLConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const LayerCLGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API LayerGL2CLGLConverter : public RepresentationConverterType<LayerCLGL> {

public:
    LayerGL2CLGLConverter() : RepresentationConverterType<LayerCLGL>() {};
    virtual ~LayerGL2CLGLConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const LayerGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API LayerCL2CLGLConverter : public RepresentationConverterPackage<LayerCLGL> {
public:
    LayerCL2CLGLConverter();
    virtual ~LayerCL2CLGLConverter() {};
};

class IVW_MODULE_OPENCL_API LayerDisk2CLGLConverter : public RepresentationConverterPackage<LayerCLGL> {

public:
    LayerDisk2CLGLConverter() : RepresentationConverterPackage<LayerCLGL>() {
        addConverter(new LayerDisk2RAMConverter());
        addConverter(new LayerRAM2GLConverter());
        addConverter(new LayerGL2CLGLConverter());
    };
    virtual ~LayerDisk2CLGLConverter() {};
};

} // namespace

#endif // IVW_LAYERCLGLCONVERTER_H
