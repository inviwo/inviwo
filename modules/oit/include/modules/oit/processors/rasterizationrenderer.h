/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/oit/oitmoduledefine.h>  // for IVW_MODULE_MESHRENDE...

#include <inviwo/core/datastructures/image/image.h>        // for Image
#include <inviwo/core/interaction/cameratrackball.h>       // for CameraTrackball
#include <inviwo/core/ports/imageport.h>                   // for ImageInport, ImageOu...
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/processors/processorinfo.h>          // for ProcessorInfo
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/cameraproperty.h>         // for CameraProperty
#include <inviwo/core/properties/ordinalproperty.h>        // for FloatProperty, Float...
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/util/dispatcher.h>                             // for Dispatcher, Dispatch...
#include <modules/oit/ports/rasterizationport.h>         // for RasterizationInport
#include <modules/oit/rendering/fragmentlistrenderer.h>  // for FragmentListRenderer

#include <optional>

namespace inviwo {
class RasterizeEvent;
class RasterizeHandle;

class IVW_MODULE_OIT_API RasterizationRenderer : public Processor {
public:
    RasterizationRenderer();
    virtual ~RasterizationRenderer() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override;

    virtual void process() override;
    
protected:
    friend RasterizeHandle;
    friend RasterizeEvent;

    void configureShader(Shader& shader) const;
    void setUniforms(Shader& shader, UseFragmentList useFragmentList) const;

    std::optional<mat4> boundingBox() const;

    RasterizationInport rasterizations_;
    ImageInport background_;
    ImageOutport outport_;
    Image intermediateImage_;

    CameraProperty camera_;
    SimpleLightingProperty lighting_;
    CameraTrackball trackball_;

    struct IllustrationSettings {
        IllustrationSettings();

        BoolCompositeProperty enabled_;
        FloatVec3Property edgeColor_;
        FloatProperty edgeStrength_;
        FloatProperty haloStrength_;
        IntProperty smoothingSteps_;
        FloatProperty edgeSmoothing_;
        FloatProperty haloSmoothing_;

        FragmentListRenderer::IllustrationSettings getSettings() const;
    };
    IllustrationSettings illustrationSettings_;

    std::optional<FragmentListRenderer> flr_;
    DispatcherHandle<void()> flrReload_;
    DispatcherHandle<void(Outport*)> onConnect_;

    Dispatcher<void()> initializeShader_;
};

}  // namespace inviwo
