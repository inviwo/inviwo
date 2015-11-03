/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#include "grayscaleclprocessor.h"
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/image/imagecl.h>
#include <modules/opencl/image/imageclgl.h>
#include <modules/opencl/settings/openclsettings.h>
#include <modules/opencl/syncclgl.h>
#include <modules/opencl/volume/volumecl.h>
#include <modules/opencl/kernelmanager.h>

namespace inviwo {

const ProcessorInfo GrayscaleCLProcessor::processorInfo_{
    "org.inviwo.GrayscaleCL",  // Class identifier
    "Image Grayscale",         // Display name
    "Image Operation",         // Category
    CodeState::Stable,         // Code state
    Tags::CL,                  // Tags
};
const ProcessorInfo GrayscaleCLProcessor::getProcessorInfo() const {
    return processorInfo_;
}

GrayscaleCLProcessor::GrayscaleCLProcessor()
    : Processor(), ProcessorKernelOwner(this)
    , input_("color image")
    , outport_("outport")
    , useGLSharing_("glsharing", "Use OpenGL sharing", true)
    , kernel_(nullptr)
{
    addPort(input_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");

    addProperty(useGLSharing_);

    kernel_ = addKernel("grayscale.cl", "grayscaleKernel");
    if (!InviwoApplication::getPtr()->getSettingsByType<OpenCLSettings>()->isSharingEnabled()) {
        useGLSharing_.setReadOnly(true);
        useGLSharing_.set(false);
    }
}

void GrayscaleCLProcessor::process() {
    if (kernel_ == nullptr) {
        return;
    }

    auto outImage = outport_.getEditableData();

    //outImage->resize(inImage->getDimensions());
    uvec2 outportDim = outImage->getDimensions();
    auto inImage = input_.getData();
    try {
        if (useGLSharing_.get()) {
            SyncCLGL glSync;

            const ImageCLGL* colorImageCL = inImage->getRepresentation<ImageCLGL>();
            ImageCLGL* outImageCL = outImage->getEditableRepresentation<ImageCLGL>();
            glSync.addToAquireGLObjectList(colorImageCL);
            glSync.addToAquireGLObjectList(outImageCL);
            glSync.aquireAllObjects();
            cl_uint arg = 0;
            kernel_->setArg(arg++, *colorImageCL);
            kernel_->setArg(arg++, *outImageCL);
            OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(*kernel_, cl::NullRange, static_cast<glm::size2_t>(outportDim));
        } else {
            const ImageCL* colorImageCL = inImage->getRepresentation<ImageCL>();
            ImageCL* outImageCL = outImage->getEditableRepresentation<ImageCL>();
            cl_uint arg = 0;
            kernel_->setArg(arg++, *colorImageCL);
            kernel_->setArg(arg++, *outImageCL);
            OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(*kernel_, cl::NullRange, static_cast<glm::size2_t>(outportDim));
        }
    } catch (cl::Error& err) {
        LogError(getCLErrorString(err));
    }
}

} // namespace

