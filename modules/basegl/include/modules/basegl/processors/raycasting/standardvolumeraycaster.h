/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>
#include <modules/basegl/processors/raycasting/volumeraycasterbase.h>

#include <modules/basegl/raycasting/raycastingcomponent.h>
#include <modules/basegl/raycasting/backgroundcomponent.h>
#include <modules/basegl/raycasting/cameracomponent.h>
#include <modules/basegl/raycasting/classifycomponent.h>
#include <modules/basegl/raycasting/isotfcomponent.h>
#include <modules/basegl/raycasting/lightcomponent.h>
#include <modules/basegl/raycasting/positionindicatorcomponent.h>
#include <modules/basegl/raycasting/sampletransformcomponent.h>
#include <modules/basegl/raycasting/volumecomponent.h>
#include <modules/basegl/raycasting/entryexitcomponent.h>

namespace inviwo {

class IVW_MODULE_BASEGL_API StandardVolumeRaycaster : public VolumeRaycasterBase {
public:
    StandardVolumeRaycaster(std::string_view identifier = "", std::string_view displayName = "");
    virtual ~StandardVolumeRaycaster() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeComponent volume_;
    EntryExitComponent entryExit_;
    ClassifyComponent classify_;
    BackgroundComponent background_;
    RaycastingComponent raycasting_;
    IsoTFComponent isoTF_;
    CameraComponent camera_;
    LightComponent light_;
    PositionIndicatorComponent positionIndicator_;
    SampleTransformComponent sampleTransform_;
};

}  // namespace inviwo
