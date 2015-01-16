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

#ifndef IVW_VOLUMECLGLCONVERTER_H
#define IVW_VOLUMECLGLCONVERTER_H

#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramconverter.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <modules/opengl/volume/volumeglconverter.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/volume/volumecl.h>
#include <modules/opencl/volume/volumeclgl.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API VolumeRAM2CLGLConverter : public RepresentationConverterPackage<VolumeCLGL> {

public:
    VolumeRAM2CLGLConverter();
    virtual ~VolumeRAM2CLGLConverter() {};
};



class IVW_MODULE_OPENCL_API VolumeCLGL2RAMConverter : public RepresentationConverterType<VolumeRAM> {

public:
    VolumeCLGL2RAMConverter();
    virtual ~VolumeCLGL2RAMConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const VolumeCLGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API VolumeGL2CLGLConverter : public RepresentationConverterType<VolumeCLGL> {

public:
    VolumeGL2CLGLConverter() : RepresentationConverterType<VolumeCLGL>() {};
    virtual ~VolumeGL2CLGLConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const VolumeGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API VolumeCLGL2CLConverter : public RepresentationConverterType<VolumeCL> {
public:
    VolumeCLGL2CLConverter() : RepresentationConverterType<VolumeCL>() {};
    virtual ~VolumeCLGL2CLConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const VolumeCLGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API VolumeCLGL2GLConverter : public RepresentationConverterType<VolumeGL> {
public:
    VolumeCLGL2GLConverter() : RepresentationConverterType<VolumeGL>() {};
    virtual ~VolumeCLGL2GLConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const VolumeCLGL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API VolumeCL2CLGLConverter : public RepresentationConverterPackage<VolumeCLGL> {
public:
    VolumeCL2CLGLConverter();
    virtual ~VolumeCL2CLGLConverter() {};
};

class IVW_MODULE_OPENCL_API VolumeDisk2CLGLConverter : public RepresentationConverterPackage<VolumeCLGL> {

public:
    VolumeDisk2CLGLConverter() : RepresentationConverterPackage<VolumeCLGL>() {
        addConverter(new VolumeDisk2RAMConverter());
        addConverter(new VolumeRAM2GLConverter());
        addConverter(new VolumeGL2CLGLConverter());
    };
    virtual ~VolumeDisk2CLGLConverter() {};
};



} // namespace

#endif // IVW_VOLUMECLGLCONVERTER_H
