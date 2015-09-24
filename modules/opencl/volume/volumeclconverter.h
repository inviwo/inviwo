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

#ifndef IVW_VOLUMECLCONVERTER_H
#define IVW_VOLUMECLCONVERTER_H

#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramconverter.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <modules/opengl/volume/volumeglconverter.h>
#include <modules/opencl/volume/volumecl.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API VolumeRAM2CLConverter
    : public RepresentationConverterType<VolumeRAM, VolumeCL> {
public:
    virtual std::shared_ptr<VolumeCL> createFrom(
        std::shared_ptr<const VolumeRAM> source) const override;
    virtual void update(std::shared_ptr<const VolumeRAM> source,
                        std::shared_ptr<VolumeCL> destination) const override;
};

class IVW_MODULE_OPENCL_API VolumeCL2RAMConverter
    : public RepresentationConverterType<VolumeCL, VolumeRAM> {
public:
    virtual std::shared_ptr<VolumeRAM> createFrom(
        std::shared_ptr<const VolumeCL> source) const override;
    virtual void update(std::shared_ptr<const VolumeCL> source,
                        std::shared_ptr<VolumeRAM> destination) const override;
};

}  // namespace

#endif  // IVW_VOLUMECLCONVERTER_H
