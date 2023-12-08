/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <chrono>

#include <inviwo/pathtracing/pathtracingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/light/baselightsource.h>
#include <inviwo/core/ports/bufferport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/util/timer.h>

#include <inviwo/core/properties/raycastingproperty.h>         // for RaycastingProperty
#include <inviwo/core/properties/cameraproperty.h>             // for CameraProperty
#include <inviwo/core/properties/optionproperty.h>             // for OptionPropertyInt
#include <inviwo/core/properties/volumeindicatorproperty.h>    // for VolumeIndicatorProperty
#include <inviwo/core/properties/transferfunctionproperty.h>   // for TransferFunctionProperty
#include <inviwo/core/datastructures/light/baselightsource.h>  // for Lights
#include <inviwo/core/ports/bufferport.h>                      // for Lights
#include <inviwo/core/properties/simplelightingproperty.h>     // for SimpleLightingProperty
#include <inviwo/core/properties/buttonproperty.h>             // for IterativeRendering
#include <inviwo/core/util/timer.h>                            // for IterativeRendering

// outside of include/inviwo
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/buffer/buffergl.h>

namespace inviwo {

class IVW_MODULE_PATHTRACING_API VolumePathTracer : public Processor {
public:
    VolumePathTracer();
    ~VolumePathTracer() = default;

    virtual void initializeResources() override { invalidateProgressiveRendering(); }
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    void numSamplesChanged();
    void updateLightSources();
    void invalidateProgressiveRendering();
    void evaluateProgressiveRefinement();
    void progressiveRefinementChanged();
    // void phaseFunctionChanged();
    void onTimerEvent();

private:
    // Ports
    VolumeInport volumePort_;
    ImageInport entryPort_;
    ImageInport exitPort_;

    ImageOutport outport_;

    // Properties and Internals
    Shader shader_;
    OptionPropertyInt channel_;
    RaycastingProperty raycasting_;
    TransferFunctionProperty transferFunction_;

    CameraProperty camera_;

    VolumeIndicatorProperty positionIndicator_;

    SimpleLightingProperty light_;
    std::chrono::time_point<std::chrono::high_resolution_clock> time_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> time_now;

    // Progressive Refinement
    ButtonProperty invalidateRendering_;
    BoolProperty enableProgressiveRefinement_;

    // Timer
    Timer progressiveTimer_;
    int iteration_ = 0;
};

}  // namespace inviwo
