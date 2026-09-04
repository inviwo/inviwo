/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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
#include <modules/basegl/shadercomponents/backgroundcomponent.h>
#include <modules/basegl/shadercomponents/cameracomponent.h>
#include <modules/basegl/shadercomponents/entryexitcomponent.h>
#include <modules/basegl/shadercomponents/lightcomponent.h>
#include <modules/basegl/shadercomponents/positionindicatorcomponent.h>
#include <modules/basegl/shadercomponents/raycastingcomponent.h>
#include <modules/basegl/shadercomponents/sampletransformcomponent.h>
#include <modules/basegl/shadercomponents/temporalvolumecomponent.h>

namespace inviwo {

/**
 * @brief Raycaster for time-dependent volumetric data.
 *
 * Like the StandardVolumeRaycaster, but consumes a TemporalVolume and performs the time
 * interpolation on the GPU. When linear interpolation is enabled, the two frames bracketing the
 * current time are uploaded as 3D textures and blended in the shader, giving smooth transitions
 * between time steps without a full CPU-side voxel copy.
 *
 * @see TemporalVolume, TemporalVolumeComponent, StandardVolumeRaycaster
 */
class IVW_MODULE_BASEGL_API TemporalVolumeRaycaster : public VolumeRaycasterBase {
public:
    explicit TemporalVolumeRaycaster(std::string_view identifier = "",
                                     std::string_view displayName = "");
    TemporalVolumeRaycaster(const TemporalVolumeRaycaster&) = delete;
    TemporalVolumeRaycaster& operator=(const TemporalVolumeRaycaster&) = delete;
    TemporalVolumeRaycaster(TemporalVolumeRaycaster&&) = delete;
    TemporalVolumeRaycaster& operator=(TemporalVolumeRaycaster&&) = delete;
    virtual ~TemporalVolumeRaycaster() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

private:
    TemporalVolumeComponent volume_;
    EntryExitComponent entryExit_;
    BackgroundComponent background_;
    RaycastingComponent raycasting_;
    CameraComponent camera_;
    LightComponent light_;
    PositionIndicatorComponent positionIndicator_;
    SampleTransformComponent sampleTransform_;
};

}  // namespace inviwo
