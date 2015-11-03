/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <modules/basecl/processors/volumefirsthitclprocessor.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/syncclgl.h>
#include <modules/opencl/image/imagecl.h>
#include <modules/opencl/image/imageclgl.h>
#include <modules/opencl/volume/volumecl.h>
#include <modules/opencl/volume/volumeclgl.h>

namespace inviwo {

const ProcessorInfo VolumeFirstHitCLProcessor::processorInfo_{
    "org.inviwo.VolumeFirstHitCL",  // Class identifier
    "Volume First Hit",             // Display name
    "Volume Rendering",             // Category
    CodeState::Experimental,        // Code state
    Tags::CL,                       // Tags
};
const ProcessorInfo VolumeFirstHitCLProcessor::getProcessorInfo() const {
    return processorInfo_;
}

VolumeFirstHitCLProcessor::VolumeFirstHitCLProcessor()
    : Processor()
    , ProcessorKernelOwner(this)
    , volumePort_("volume")
    , entryPort_("entry-points")
    , exitPort_("exit-points")
    , outport_("outport", DataVec4Float32::get())
    , samplingRate_("samplingRate", "Sampling rate", 1.0f, 1.0f, 15.0f)
    , transferFunction_("transferFunction", "Transfer function", TransferFunction())
    , workGroupSize_("wgsize", "Work group size", ivec2(8, 8), ivec2(0), ivec2(256))
    , useGLSharing_("glsharing", "Use OpenGL sharing", true)
    , kernel_(nullptr) {
    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");
    addProperty(samplingRate_);
    addProperty(transferFunction_);
    addProperty(workGroupSize_);
    addProperty(useGLSharing_);

    kernel_ = addKernel("volumefirsthit.cl",
        "volumeFirstHit");
}

void VolumeFirstHitCLProcessor::process() {
    if (kernel_ == nullptr) {
        return;
    }

    auto entryImage = entryPort_.getData();
    auto exitImage = exitPort_.getData();

    //mat4 volumeTextureToWorld =
    //    volumePort_.getData()->getCoordinateTransformer().getTextureToWorldMatrix();
    const size3_t volumeDim{volumePort_.getData()->getDimensions()};
    float stepSize =
        1.f / (samplingRate_.get() *
               static_cast<float>(std::max(volumeDim.x, std::max(volumeDim.y, volumeDim.z))));
    size2_t localWorkGroupSize(workGroupSize_.get());
    size2_t globalWorkGroupSize(
        getGlobalWorkGroupSize(entryImage->getDimensions().x, localWorkGroupSize.x),
        getGlobalWorkGroupSize(entryImage->getDimensions().y, localWorkGroupSize.y));
    IVW_OPENCL_PROFILING(profilingEvent, "")

    if (useGLSharing_.get()) {
        // Will synchronize with OpenGL upon creation and destruction
        SyncCLGL glSync;
        auto entry = entryImage->getRepresentation<ImageCLGL>();
        auto exit = exitImage->getRepresentation<ImageCLGL>();
        auto output = outport_.getEditableData()->getEditableRepresentation<ImageCLGL>();
        auto volume = volumePort_.getData()->getRepresentation<VolumeCLGL>();
        auto tfCL = transferFunction_.get().getData()->getRepresentation<LayerCLGL>();
        volume->aquireGLObject(glSync.getGLSyncEvent());
        entry->getLayerCL()->aquireGLObject();
        exit->getLayerCL()->aquireGLObject();
        output->getLayerCL()->aquireGLObject();
        tfCL->aquireGLObject();
        const cl::Image& volumeCL = volume->get();
        const cl::Image& entryCL = entry->getLayerCL()->get();
        const cl::Image& exitCL = exit->getLayerCL()->get();
        const cl::Image& outImageCL = output->getLayerCL()->get();
        const cl::Image& tf = tfCL->get();
        firstHit(volumeCL, entryCL, exitCL, tf, outImageCL, stepSize, globalWorkGroupSize,
                 localWorkGroupSize, profilingEvent);
        entry->getLayerCL()->releaseGLObject();
        exit->getLayerCL()->releaseGLObject();
        output->getLayerCL()->releaseGLObject();
        tfCL->releaseGLObject();
        volume->releaseGLObject(nullptr, glSync.getLastReleaseGLEvent());
    } else {
        auto entry = entryImage->getRepresentation<ImageCL>();
        auto exit = exitImage->getRepresentation<ImageCL>();
        auto output = outport_.getEditableData()->getEditableRepresentation<ImageCL>();
        auto volume = volumePort_.getData()->getRepresentation<VolumeCL>();
        auto tfCL = transferFunction_.get().getData()->getRepresentation<LayerCL>();
        const cl::Image& volumeCL = volume->get();
        const cl::Image& entryCL = entry->getLayerCL()->get();
        const cl::Image& exitCL = exit->getLayerCL()->get();
        const cl::Image& outImageCL = output->getLayerCL()->get();
        const cl::Image& tf = tfCL->get();
        firstHit(volumeCL, entryCL, exitCL, tf, outImageCL, stepSize, globalWorkGroupSize,
                 localWorkGroupSize, profilingEvent);
    }

    
}

void VolumeFirstHitCLProcessor::firstHit(const cl::Image& volumeCL, const cl::Image& entryPoints,
                                const cl::Image& exitPoints, const cl::Image& transferFunctionCL,
                                const cl::Image& output, float stepSize, size2_t globalWorkGroupSize,
                                size2_t localWorkGroupSize, cl::Event* profilingEvent) {
    try {
        cl_uint arg = 0;
        kernel_->setArg(arg++, volumeCL);
        kernel_->setArg(arg++, entryPoints);
        kernel_->setArg(arg++, exitPoints);
        kernel_->setArg(arg++, transferFunctionCL);
        kernel_->setArg(arg++, stepSize);
        kernel_->setArg(arg++, output);
        OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(
            *kernel_, cl::NullRange, globalWorkGroupSize, localWorkGroupSize, nullptr, profilingEvent);
    }
    catch (cl::Error& err) {
        LogError(getCLErrorString(err));
    }
}

}  // namespace

