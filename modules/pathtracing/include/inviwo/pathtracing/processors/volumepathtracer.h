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

#include <inviwo/core/properties/raycastingproperty.h>          // for RaycastingProperty
#include <inviwo/core/properties/cameraproperty.h>              // for CameraProperty
#include <inviwo/core/properties/optionproperty.h>              // for OptionPropertyInt
#include <inviwo/core/properties/volumeindicatorproperty.h>     // for VolumeIndicatorProperty
#include <inviwo/core/properties/transferfunctionproperty.h>    // for TransferFunctionProperty
#include <inviwo/core/datastructures/light/baselightsource.h>   // for Lights
#include <inviwo/core/ports/bufferport.h>                       // for Lights

// outsider of include/inviwo
#include <../modules/opengl/include/modules/opengl/volume/volumegl.h>
#include <../modules/opengl/include/modules/opengl/volume/volumeutils.h>
#include <../modules/opengl/include/modules/opengl/shader/shader.h>
#include <../modules/opengl/include/modules/opengl/image/layergl.h>
#include <../modules/opengl/include/modules/opengl/buffer/buffergl.h>

namespace inviwo {

class IVW_MODULE_PATHTRACING_API VolumePathTracer : public Processor {
public:
    VolumePathTracer();
    ~VolumePathTracer() = default;

    virtual void initializeResources() override;
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:

    // What helper functions are used in CL version? Do we need equivalances?
    
    void dispatchPathTracerComputeShader(LayerGL* entryGL, LayerGL* exitGL, LayerGL* outportGL); //Needed args; dimension of workgroups

    void numSamplesChanged(); // Creates new samples if changed
    void updateLightSources(); // Updates light sources 
    void invalidateProgressiveRendering(); // Restarts progressive render
    void evaluateProgressiveRefinement(); // Invalidates processor, and you know what that means, its a message.
    void progressiveRefinementChanged(); // does something with the progressivetimer.
    //void phaseFunctionChanged(); // related to advanced material
    //void buildKernel(); // OCL shit
    //void kernelArgChanged(); // Calls invalidateRendering
    //void onTimerEvent(); // Button to call invalidateRendering (I think)

private:
    // Ports
    VolumeInport volumePort_;
    ImageInport entryPort_; // Entry and Exit determine the direction of each sample.
    ImageInport exitPort_; // Imagine worldcoords - screenspace in world coords 
    
    MultiDataInport<LightSource> lights_;

    //OpacityMinMaxUniformGrid3DInport minMaxOpacity_; // Cant find module. Is it even relevant?

    ImageOutport outport_;
    Shader shader_;

    // Properties
    
    OptionPropertyInt channel_;
    RaycastingProperty raycasting_;
    TransferFunctionProperty transferFunction_;

    CameraProperty camera_;

    VolumeIndicatorProperty positionIndicator_;
    
    // What internal data types would I need?
    BufferGL lightSources_;
    int nLightSources_ = 1; // Number of light sources

    std::chrono::time_point<std::chrono::high_resolution_clock> time_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> time_now;

    Buffer<glm::uvec2> randomState_; //arguably not needed
};

}  // namespace inviwo
