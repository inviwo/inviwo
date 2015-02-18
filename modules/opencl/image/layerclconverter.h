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

#ifndef IVW_LAYERCLCONVERTER_H
#define IVW_LAYERCLCONVERTER_H

#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramconverter.h>
#include <inviwo/core/datastructures/image/layerdisk.h>
#include <modules/opengl/image/layerglconverter.h>
#include <modules/opencl/image/layercl.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API LayerRAM2CLConverter : public RepresentationConverterType<LayerCL> {

public:
    LayerRAM2CLConverter();
    virtual ~LayerRAM2CLConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const LayerRAM*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API LayerDisk2CLConverter : public RepresentationConverterPackage<LayerCL> {

public:
    LayerDisk2CLConverter() : RepresentationConverterPackage<LayerCL>() {
        addConverter(new LayerDisk2RAMConverter());
        addConverter(new LayerRAM2CLConverter());
    };
    virtual ~LayerDisk2CLConverter() {};
};

class IVW_MODULE_OPENCL_API LayerCL2RAMConverter : public RepresentationConverterType<LayerRAM> {

public:
    LayerCL2RAMConverter();
    virtual ~LayerCL2RAMConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const LayerCL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API LayerGL2CLConverter : public RepresentationConverterPackage<LayerCL> {

public:
    LayerGL2CLConverter() : RepresentationConverterPackage<LayerCL>() {
        addConverter(new LayerGL2RAMConverter());
        addConverter(new LayerRAM2CLConverter());
    };
    virtual ~LayerGL2CLConverter() {};
};

class IVW_MODULE_OPENCL_API LayerCL2GLConverter : public RepresentationConverterPackage<LayerGL> {

public:
    LayerCL2GLConverter() : RepresentationConverterPackage<LayerGL>() {
        addConverter(new LayerCL2RAMConverter());
        addConverter(new LayerRAM2GLConverter());
    };
    virtual ~LayerCL2GLConverter() {};
};



} // namespace

#endif // IVW_LAYERCLCONVERTER_H
