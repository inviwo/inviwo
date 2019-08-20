/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/basecl/processors/volumeraycasterclprocessor.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/kernelmanager.h>
#include <modules/opencl/buffer/buffercl.h>
#include <modules/opencl/image/imagecl.h>
#include <modules/opencl/image/imageclgl.h>
#include <modules/opencl/settings/openclsettings.h>  // To check if the we can use sharing
#include <modules/opencl/syncclgl.h>
#include <modules/opencl/volume/volumecl.h>
#include <modules/opencl/volume/volumeclgl.h>
#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

const ProcessorInfo VolumeRaycasterCLProcessor::processorInfo_{
    "org.inviwo.VolumeRaycasterCL",  // Class identifier
    "Volume Raycaster",              // Display name
    "Volume Rendering",              // Category
    CodeState::Experimental,         // Code state
    Tags::CL,                        // Tags
};
const ProcessorInfo VolumeRaycasterCLProcessor::getProcessorInfo() const { return processorInfo_; }

VolumeRaycasterCLProcessor::VolumeRaycasterCLProcessor()
    : Processor()
    , KernelObserver()
    , volumePort_("volume")
    , entryPort_("entry-points")
    , exitPort_("exit-points")
    , backgroundPort_("background")
    , outport_("outport")
    , samplingRate_("samplingRate", "Sampling rate", 1.0f, 1.0f, 15.0f)
    , transferFunction_("transferFunction", "Transfer function")
    , workGroupSize_("wgsize", "Work group size", ivec2(8, 8), ivec2(0), ivec2(256))
    , useGLSharing_("glsharing", "Use OpenGL sharing", true)
    , lighting_("lighting", "Lighting")
    , camera_("camera", "Camera", util::boundingBox(volumePort_)) {

    backgroundPort_.setOptional(true);

    isReady_.setUpdate([this]() { return allInportsAreReady() && volumeRaycaster_.isValid(); });

    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(backgroundPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");
    addProperty(samplingRate_);
    addProperty(transferFunction_);
    addProperty(workGroupSize_);
    addProperty(useGLSharing_);
    addProperty(lighting_);
    addProperty(camera_);

    samplingRate_.onChange([this]() { onParameterChanged(); });
    workGroupSize_.onChange([this]() { onParameterChanged(); });
    useGLSharing_.onChange([this]() { onParameterChanged(); });
    lighting_.onChange([this]() { onParameterChanged(); });

    volumeRaycaster_.addObserver(this);
    volumeRaycaster_.setCamera(&camera_);

    if (!InviwoApplication::getPtr()->getSettingsByType<OpenCLSettings>()->isSharingEnabled()) {
        useGLSharing_.setReadOnly(true);
        useGLSharing_.set(false);
    }
    // Set kernel parameters
    onParameterChanged();
}

void VolumeRaycasterCLProcessor::process() {
    try {
        // This macro will create an event called profilingEvent if IVW_PROFILING is enabled,
        // otherwise the profilingEvent will be declared as a null pointer
        IVW_OPENCL_PROFILING(profilingEvent, "")
        if (backgroundPort_.isReady()) {
            volumeRaycaster_.setBackground(backgroundPort_.getData()->getColorLayer());
        } else {
            // Use default background
            volumeRaycaster_.setBackground(nullptr);
        }
        volumeRaycaster_.outputSize(outport_.getDimensions());
        volumeRaycaster_.volumeRaycast(
            volumePort_.getData().get(), entryPort_.getData()->getColorLayer(),
            exitPort_.getData()->getColorLayer(), transferFunction_.get().getData(),
            outport_.getEditableData()->getColorLayer(), nullptr, profilingEvent);
    } catch (cl::Error& err) {
        LogError(getCLErrorString(err));
    }
}

void VolumeRaycasterCLProcessor::onKernelCompiled(const cl::Kernel*) {
    isReady_.update();
    invalidate(InvalidationLevel::InvalidResources);
}

void VolumeRaycasterCLProcessor::onParameterChanged() {
    volumeRaycaster_.setLightingProperties(lighting_);
    volumeRaycaster_.samplingRate(samplingRate_.get());
    volumeRaycaster_.workGroupSize(workGroupSize_.get());
    volumeRaycaster_.useGLSharing(useGLSharing_.get());
}

}  // namespace inviwo
