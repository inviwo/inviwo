/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/geometry/plane.h>  // for Plane
#include <inviwo/core/ports/dataoutport.h>              // for DataOutport
#include <inviwo/core/ports/meshport.h>                 // for MeshInport, MeshOutport
#include <inviwo/core/processors/processor.h>           // for Processor
#include <inviwo/core/processors/processorinfo.h>       // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>        // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>      // for ButtonProperty
#include <inviwo/core/properties/cameraproperty.h>      // for CameraProperty
#include <inviwo/core/properties/ordinalproperty.h>     // for FloatVec3Property, FloatProperty
#include <inviwo/core/properties/optionproperty.h>

#include <string>  // for operator+, string

#include <fmt/core.h>  // for format

namespace inviwo {

class IVW_MODULE_BASE_API MeshClipping : public Processor {
public:
    MeshClipping();
    ~MeshClipping();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    void onAlignPlaneNormalToCameraNormalPressed();

    enum class ClipSide { Back, Front };

    MeshInport inport_;
    MeshOutport outport_;
    DataOutport<Plane> clippingPlane_;

    BoolProperty clippingEnabled_;
    OptionProperty<ClipSide> clipSide_;
    BoolProperty movePointAlongNormal_;
    BoolProperty moveCameraAlongNormal_;
    FloatProperty pointPlaneMove_;

    BoolProperty capClippedHoles_;

    FloatVec3Property planePoint_;   ///< World space plane position
    FloatVec3Property planeNormal_;  ///< World space plane normal
    ButtonProperty alignPlaneNormalToCameraNormal_;
    CameraProperty camera_;

    float previousPointPlaneMove_;
};
}  // namespace inviwo
