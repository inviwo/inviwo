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

#ifndef IVW_VOLUMECLCONVERTER_H
#define IVW_VOLUMECLCONVERTER_H

#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramconverter.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <modules/opengl/volume/volumeglconverter.h>
#include <modules/opencl/volume/volumecl.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API VolumeRAM2CLConverter : public RepresentationConverterType<VolumeCL> {

public:
    VolumeRAM2CLConverter();
    virtual ~VolumeRAM2CLConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const VolumeRAM*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API VolumeDisk2CLConverter : public RepresentationConverterPackage<VolumeCL> {

public:
    VolumeDisk2CLConverter() : RepresentationConverterPackage<VolumeCL>() {
        addConverter(new VolumeDisk2RAMConverter());
        addConverter(new VolumeRAM2CLConverter());
    };
    virtual ~VolumeDisk2CLConverter() {};
};

class IVW_MODULE_OPENCL_API VolumeCL2RAMConverter : public RepresentationConverterType<VolumeRAM> {

public:
    VolumeCL2RAMConverter();
    virtual ~VolumeCL2RAMConverter() {};

    inline bool canConvertFrom(const DataRepresentation* source) const {
        return dynamic_cast<const VolumeCL*>(source) != NULL;
    }
    DataRepresentation* createFrom(const DataRepresentation* source);
    void update(const DataRepresentation* source, DataRepresentation* destination);
};

class IVW_MODULE_OPENCL_API VolumeGL2CLConverter : public RepresentationConverterPackage<VolumeCL> {

public:
    VolumeGL2CLConverter() : RepresentationConverterPackage<VolumeCL>() {
        addConverter(new VolumeGL2RAMConverter());
        addConverter(new VolumeRAM2CLConverter());
    };
    virtual ~VolumeGL2CLConverter() {};
};

class IVW_MODULE_OPENCL_API VolumeCL2GLConverter : public RepresentationConverterPackage<VolumeGL> {

public:
    VolumeCL2GLConverter() : RepresentationConverterPackage<VolumeGL>() {
        addConverter(new VolumeCL2RAMConverter());
        addConverter(new VolumeRAM2GLConverter());
    };
    virtual ~VolumeCL2GLConverter() {};
};

} // namespace

#endif // IVW_VOLUMECLCONVERTER_H
