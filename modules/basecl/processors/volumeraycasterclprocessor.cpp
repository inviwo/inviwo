/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "volumeraycasterclprocessor.h"
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/kernelmanager.h>
#include <modules/opencl/buffer/buffercl.h>
#include <modules/opencl/image/imagecl.h>
#include <modules/opencl/image/imageclgl.h>
#include <modules/opencl/settings/openclsettings.h> // To check if the we can use sharing
#include <modules/opencl/syncclgl.h>
#include <modules/opencl/volume/volumecl.h>
#include <modules/opencl/volume/volumeclgl.h>


namespace inviwo {

ProcessorClassIdentifier(VolumeRaycasterCLProcessor, "org.inviwo.VolumeRaycasterCL");
ProcessorDisplayName(VolumeRaycasterCLProcessor,  "Volume Raycaster");
ProcessorTags(VolumeRaycasterCLProcessor, Tags::CL);
ProcessorCategory(VolumeRaycasterCLProcessor, "Volume Rendering");
ProcessorCodeState(VolumeRaycasterCLProcessor, CODE_STATE_EXPERIMENTAL);

VolumeRaycasterCLProcessor::VolumeRaycasterCLProcessor()
    : Processor(), KernelObserver()
    , volumePort_("volume")
    , entryPort_("entry-points")
    , exitPort_("exit-points")
    , outport_("outport")
    , samplingRate_("samplingRate", "Sampling rate", 1.0f, 1.0f, 15.0f)
    , transferFunction_("transferFunction", "Transfer function", TransferFunction())
    , workGroupSize_("wgsize", "Work group size", ivec2(8, 8), ivec2(0), ivec2(256))
    , useGLSharing_("glsharing", "Use OpenGL sharing", true)
    , lighting_("lighting", "Lighting")
    , camera_("camera", "Camera")
{
    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");
    addProperty(samplingRate_);
    addProperty(transferFunction_);
    addProperty(workGroupSize_);
    addProperty(useGLSharing_);
    addProperty(lighting_);
    addProperty(camera_);

    samplingRate_.onChange(this, &VolumeRaycasterCLProcessor::onParameterChanged);
    workGroupSize_.onChange(this, &VolumeRaycasterCLProcessor::onParameterChanged);
    useGLSharing_.onChange(this, &VolumeRaycasterCLProcessor::onParameterChanged);
    lighting_.onChange(this, &VolumeRaycasterCLProcessor::onParameterChanged);

    volumeRaycaster_.addObserver(this);
    volumeRaycaster_.setCamera(&camera_);
}

VolumeRaycasterCLProcessor::~VolumeRaycasterCLProcessor() {}

void VolumeRaycasterCLProcessor::initialize() {
    Processor::initialize();


    if (!InviwoApplication::getPtr()->getSettingsByType<OpenCLSettings>()->isSharingEnabled()) {
        useGLSharing_.setReadOnly(true);
        useGLSharing_.set(false);
    }
    onParameterChanged();
}

void VolumeRaycasterCLProcessor::deinitialize() {
    Processor::deinitialize();
}

bool VolumeRaycasterCLProcessor::isReady() const {
    return Processor::isReady() && volumeRaycaster_.isValid();
}

void VolumeRaycasterCLProcessor::process() {


    try {
        // This macro will create an event called profilingEvent if IVW_PROFILING is enabled,
        // otherwise the profilingEvent will be declared as a null pointer
        IVW_OPENCL_PROFILING(profilingEvent, "")

        volumeRaycaster_.outputSize(outport_.getDimensions());
        volumeRaycaster_.volumeRaycast(volumePort_.getData(), entryPort_.getData(), exitPort_.getData(), transferFunction_.get().getData(), outport_.getData(), NULL, profilingEvent);
    } catch (cl::Error& err) {
        LogError(getCLErrorString(err));
    }

}

void VolumeRaycasterCLProcessor::onParameterChanged() {
    volumeRaycaster_.setLightingProperties(lighting_);
    volumeRaycaster_.samplingRate(samplingRate_.get());
    volumeRaycaster_.workGroupSize(workGroupSize_.get());
    volumeRaycaster_.useGLSharing(useGLSharing_.get());

}

} // namespace
