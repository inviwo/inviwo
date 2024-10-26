/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <modules/oit/oitmoduledefine.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <modules/oit/ports/rasterizationport.h>
#include <modules/oit/rendering/rasterizationrendererbase.h>
#include <modules/oit/rendering/volumefragmentlistrenderer.h>

namespace inviwo {

class Rasterization;

class IVW_MODULE_OIT_API MeshVolumeRenderer : public Processor, public RasterizationRendererBase {
public:
    MeshVolumeRenderer();
    virtual ~MeshVolumeRenderer() override = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    friend RasterizeHandle;
    friend RasterizeEvent;

    virtual void configureShader(Shader& shader) const override;
    virtual void setUniforms(Shader& shader, UseFragmentList useFragmentList,
                             std::string_view rasterizerId) const override;
    virtual DispatcherHandle<void()> addInitializeShaderCallback(
        std::function<void()> callback) override;

    RasterizationInport rasterizations_;
    ImageInport background_;
    ImageOutport outport_;

    Image intermediateImage_;

    CompositeProperty raycastingProps_;
    FloatProperty samplingDistance_;
    CameraProperty camera_;
    SimpleLightingProperty lighting_;
    CameraTrackball trackball_;

    std::optional<VolumeFragmentListRenderer> flr_;
    typename Dispatcher<void()>::Handle flrReload_;
    DispatcherHandle<void(Outport*)> onConnect_;

    Dispatcher<void()> initializeShader_;
    std::unordered_map<std::string_view, int> volumeIds_;
};

}  // namespace inviwo
