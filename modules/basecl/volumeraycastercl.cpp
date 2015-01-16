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

#include "volumeraycastercl.h"
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

VolumeRaycasterCL::VolumeRaycasterCL()
    : KernelOwner()
    , workGroupSize_(svec2(8, 8))
    , useGLSharing_(true)
    , outputOffset_(0)
    , outputSize_(1)
    , kernel_(NULL)
{
    // Will compile kernel and make sure that it it
    // recompiled whenever the file changes
    // If the kernel fails to compile it will be set to NULL
    kernel_ = addKernel("volumeraycaster.cl", "raycaster");
    outputOffset(outputOffset_);
    outputSize(outputSize_);
    samplingRate(1.f); 
}

VolumeRaycasterCL::~VolumeRaycasterCL() {}


void VolumeRaycasterCL::volumeRaycast(const Volume* volume, const Image* entryPoints, const Image* exitPoints, const Layer* transferFunction, Image* outImage, const VECTOR_CLASS<cl::Event> *waitForEvents /*= NULL*/, cl::Event *event /*= NULL*/) {

    svec2 localWorkGroupSize(workGroupSize_);
    svec2 globalWorkGroupSize(getGlobalWorkGroupSize(outputSize_.x, localWorkGroupSize.x), getGlobalWorkGroupSize(outputSize_.y,
        localWorkGroupSize.y));

    try {
        // This macro will create an event called profilingEvent if IVW_PROFILING is enabled,
        // otherwise the profilingEvent will be declared as a null pointer
        IVW_OPENCL_PROFILING(profilingEvent, "")

            if (useGLSharing_) {
                // SyncCLGL will synchronize with OpenGL upon creation and destruction
                SyncCLGL glSync;
                const ImageCLGL* entryCL = entryPoints->getRepresentation<ImageCLGL>();
                const ImageCLGL* exitCL = exitPoints->getRepresentation<ImageCLGL>();
                ImageCLGL* outImageCL = outImage->getEditableRepresentation<ImageCLGL>();
                const VolumeCLGL* volumeCL = volume->getRepresentation<VolumeCLGL>();
                const LayerCLGL* transferFunctionCL = transferFunction->getRepresentation<LayerCLGL>();
                // Shared objects must be acquired before use.
                glSync.addToAquireGLObjectList(entryCL);
                glSync.addToAquireGLObjectList(exitCL);
                glSync.addToAquireGLObjectList(outImageCL);
                glSync.addToAquireGLObjectList(volumeCL);
                glSync.addToAquireGLObjectList(transferFunctionCL);
                // Acquire all of the objects at once
                glSync.aquireAllObjects();

                volumeRaycast(volume, volumeCL, entryCL->getLayerCL(), exitCL->getLayerCL(), transferFunctionCL, outImageCL->getLayerCL(), globalWorkGroupSize, localWorkGroupSize, waitForEvents, event);
            } else {
                const ImageCL* entryCL = entryPoints->getRepresentation<ImageCL>();
                const ImageCL* exitCL = exitPoints->getRepresentation<ImageCL>();
                ImageCL* outImageCL = outImage->getEditableRepresentation<ImageCL>();
                const VolumeCL* volumeCL = volume->getRepresentation<VolumeCL>();
                const LayerCL* transferFunctionCL = transferFunction->getRepresentation<LayerCL>();

                volumeRaycast(volume, volumeCL, entryCL->getLayerCL(), exitCL->getLayerCL(), transferFunctionCL, outImageCL->getLayerCL(), globalWorkGroupSize, localWorkGroupSize, waitForEvents, event);
            }

            // This macro will destroy the profiling event and print the timing in the console if IVW_PROFILING is enabled

    } catch (cl::Error& err) {
        LogError(getCLErrorString(err));
    }
}


void VolumeRaycasterCL::volumeRaycast(const Volume* volume, const VolumeCLBase* volumeCL, const LayerCLBase* entryCLGL, const LayerCLBase* exitCLGL, const LayerCLBase* transferFunctionCL, LayerCLBase* outImageCL, svec2 globalWorkGroupSize, svec2 localWorkGroupSize, const VECTOR_CLASS<cl::Event> *waitForEvents /*= NULL*/, cl::Event *event /*= NULL*/) {
    // Note that the overloaded setArg methods requires 
    // the reference to an object (not the pointer), 
    // which is why we need to dereference the pointers
    kernel_->setArg(0, *volumeCL);
    kernel_->setArg(1, *(volumeCL->getVolumeStruct(volume).getRepresentation<BufferCL>())); // Scaling for 12-bit data
    kernel_->setArg(2, *entryCLGL);
    kernel_->setArg(3, *exitCLGL);
    kernel_->setArg(4, *transferFunctionCL);
    kernel_->setArg(6, *outImageCL);
    //
    OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(*kernel_, cl::NullRange, globalWorkGroupSize, localWorkGroupSize, waitForEvents, event);
}

void VolumeRaycasterCL::samplingRate(float samplingRate) {
    if (kernel_)
        kernel_->setArg(5, samplingRate);
}

void VolumeRaycasterCL::outputOffset(ivec2 val) {
    if (kernel_)
        kernel_->setArg(7, val);
    outputOffset_ = val;
}

void VolumeRaycasterCL::outputSize(ivec2 val) {
    if (kernel_)
        kernel_->setArg(8, val);
    outputSize_ = val;
}

} // namespace
