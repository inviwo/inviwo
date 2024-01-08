/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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
#pragma once

#include <inviwo/python3gl/python3glmoduledefine.h>

#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <inviwo/core/datastructures/representationconverter.h>

#include <modules/python3/volumepy.h>
#include <modules/opengl/volume/volumegl.h>

namespace inviwo {

class IVW_MODULE_PYTHON3GL_API VolumeGL2PyConverter
    : public RepresentationConverterType<VolumeRepresentation, VolumeGL, VolumePy> {
public:
    virtual std::shared_ptr<VolumePy> createFrom(
        std::shared_ptr<const VolumeGL> source) const override;
    virtual void update(std::shared_ptr<const VolumeGL> source,
                        std::shared_ptr<VolumePy> destination) const override;
};

class IVW_MODULE_PYTHON3GL_API VolumePy2GLConverter
    : public RepresentationConverterType<VolumeRepresentation, VolumePy, VolumeGL> {
public:
    virtual std::shared_ptr<VolumeGL> createFrom(
        std::shared_ptr<const VolumePy> source) const override;
    virtual void update(std::shared_ptr<const VolumePy> source,
                        std::shared_ptr<VolumeGL> destination) const override;
};

}  // namespace inviwo
