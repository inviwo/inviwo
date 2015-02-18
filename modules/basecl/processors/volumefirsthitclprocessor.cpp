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

ProcessorClassIdentifier(VolumeFirstHitCLProcessor, "org.inviwo.VolumeFirstHitCL");
ProcessorDisplayName(VolumeFirstHitCLProcessor,  "Volume First Hit");
ProcessorTags(VolumeFirstHitCLProcessor, Tags::CL);
ProcessorCategory(VolumeFirstHitCLProcessor, "Volume Rendering");
ProcessorCodeState(VolumeFirstHitCLProcessor, CODE_STATE_EXPERIMENTAL);

VolumeFirstHitCLProcessor::VolumeFirstHitCLProcessor()
    : Processor()
    , ProcessorKernelOwner(this)
    , volumePort_("volume")
    , entryPort_("entry-points")
    , exitPort_("exit-points")
    , outport_("outport", DataVec4FLOAT32::get())
    , samplingRate_("samplingRate", "Sampling rate", 1.0f, 1.0f, 15.0f)
    , transferFunction_("transferFunction", "Transfer function", TransferFunction())
    , workGroupSize_("wgsize", "Work group size", ivec2(8, 8), ivec2(0), ivec2(256))
    , useGLSharing_("glsharing", "Use OpenGL sharing", true)
    , kernel_(NULL) {
    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");
    addProperty(samplingRate_);
    addProperty(transferFunction_);
    addProperty(workGroupSize_);
    addProperty(useGLSharing_);
}

VolumeFirstHitCLProcessor::~VolumeFirstHitCLProcessor() {}

void VolumeFirstHitCLProcessor::initialize() {
    Processor::initialize();
    kernel_ = addKernel("volumefirsthit.cl",
                        "volumeFirstHit");
}

void VolumeFirstHitCLProcessor::deinitialize() { Processor::deinitialize(); }

void VolumeFirstHitCLProcessor::process() {
    if (kernel_ == NULL) {
        return;
    }

    mat4 volumeTextureToWorld =
        volumePort_.getData()->getCoordinateTransformer().getTextureToWorldMatrix();
    uvec3 volumeDim = volumePort_.getData()->getDimensions();
    float stepSize =
        1.f / (samplingRate_.get() *
               static_cast<float>(std::max(volumeDim.x, std::max(volumeDim.y, volumeDim.z))));
    svec2 localWorkGroupSize(workGroupSize_.get());
    svec2 globalWorkGroupSize(
        getGlobalWorkGroupSize(entryPort_.getDimensions().x, localWorkGroupSize.x),
        getGlobalWorkGroupSize(entryPort_.getDimensions().y, localWorkGroupSize.y));
    IVW_OPENCL_PROFILING(profilingEvent, "")

    if (useGLSharing_.get()) {
        // Will synchronize with OpenGL upon creation and destruction
        SyncCLGL glSync;
        const ImageCLGL* entry = entryPort_.getData()->getRepresentation<ImageCLGL>();
        const ImageCLGL* exit = exitPort_.getData()->getRepresentation<ImageCLGL>();
        const ImageCLGL* output = outport_.getData()->getEditableRepresentation<ImageCLGL>();
        const VolumeCLGL* volume = volumePort_.getData()->getRepresentation<VolumeCLGL>();
        const LayerCLGL* tfCL = transferFunction_.get().getData()->getRepresentation<LayerCLGL>();
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
        volume->releaseGLObject(NULL, glSync.getLastReleaseGLEvent());
    } else {
        const ImageCL* entry = entryPort_.getData()->getRepresentation<ImageCL>();
        const ImageCL* exit = exitPort_.getData()->getRepresentation<ImageCL>();
        const ImageCL* output = outport_.getData()->getEditableRepresentation<ImageCL>();
        const VolumeCL* volume = volumePort_.getData()->getRepresentation<VolumeCL>();
        const LayerCL* tfCL = transferFunction_.get().getData()->getRepresentation<LayerCL>();
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
                                const cl::Image& output, float stepSize, svec2 globalWorkGroupSize,
                                svec2 localWorkGroupSize, cl::Event* profilingEvent) {
    try {
        cl_uint arg = 0;
        kernel_->setArg(arg++, volumeCL);
        kernel_->setArg(arg++, entryPoints);
        kernel_->setArg(arg++, exitPoints);
        kernel_->setArg(arg++, transferFunctionCL);
        kernel_->setArg(arg++, stepSize);
        kernel_->setArg(arg++, output);
        OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(
            *kernel_, cl::NullRange, globalWorkGroupSize, localWorkGroupSize, NULL, profilingEvent);
    }
    catch (cl::Error& err) {
        LogError(getCLErrorString(err));
    }
}

}  // namespace
