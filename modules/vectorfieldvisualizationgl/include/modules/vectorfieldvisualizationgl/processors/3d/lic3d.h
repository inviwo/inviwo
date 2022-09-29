/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/vectorfieldvisualizationglmoduledefine.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>

#include <memory>

namespace inviwo {
class TextureUnitContainer;

class IVW_MODULE_VECTORFIELDVISUALIZATIONGL_API LIC3D : public VolumeGLProcessor {
public:
    LIC3D();
    virtual ~LIC3D() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void preProcess(TextureUnitContainer& cont) override;
    virtual void postProcess() override;

protected:
    VolumeInport vectorField_;
    IntProperty samples_;
    FloatProperty stepLength_;
    BoolProperty normalizeVectors_;
    BoolProperty intensityMapping_;

    FloatProperty noiseRepeat_;

    TransferFunctionProperty tf_;
    FloatProperty velocityScale_;

    FloatProperty alphaScale_;

    std::unique_ptr<Volume> noiseVolume_;
};

}  // namespace inviwo
