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

#include "volumeraycastercl.h"
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/kernelmanager.h>
#include <modules/opencl/image/imagecl.h>
#include <modules/opencl/image/imageclgl.h>
#include <modules/opencl/settings/openclsettings.h>  // To check if the we can use sharing
#include <modules/opencl/syncclgl.h>
#include <modules/opencl/volume/volumecl.h>
#include <modules/opencl/volume/volumeclgl.h>

namespace inviwo {

VolumeRaycasterCL::VolumeRaycasterCL()
    : KernelOwner()
    , workGroupSize_(size2_t(8, 8))
    , useGLSharing_(true)
    , outputOffset_(0)
    , outputSize_(1)
    , camera_(nullptr)
    , samplingRate_(2.f)
    , background_(nullptr)
    , defaultBackground_(uvec2(1), DataVec4UINT8::get())
    , lightStruct_(sizeof(utilcl::LightParameters), DataUINT8::get(), BufferUsage::STATIC, nullptr,
                   CL_MEM_READ_ONLY)
    , kernel_(nullptr) {
    light_.ambientColor = vec4(1.f);
    light_.diffuseColor = vec4(1.f);
    light_.specularColor = vec4(1.f);
    light_.specularExponent = 110.f;
    light_.position = vec4(0.7f);
    light_.shadingMode = ShadingMode::Phong;

    compileKernel();
}

VolumeRaycasterCL::~VolumeRaycasterCL() {}

void VolumeRaycasterCL::volumeRaycast(const Volume* volume, const Layer* entryPoints,
                                      const Layer* exitPoints, const Layer* transferFunction,
                                      Layer* outImage,
                                      const VECTOR_CLASS<cl::Event>* waitForEvents /*= nullptr*/,
                                      cl::Event* event /*= nullptr*/) {
    size2_t localWorkGroupSize(workGroupSize_);
    size2_t globalWorkGroupSize(getGlobalWorkGroupSize(outputSize_.x, localWorkGroupSize.x),
                                getGlobalWorkGroupSize(outputSize_.y, localWorkGroupSize.y));

    if (useGLSharing_) {
        // SyncCLGL will synchronize with OpenGL upon creation and destruction
        SyncCLGL glSync;
        const LayerCLGL* entryCL = entryPoints->getRepresentation<LayerCLGL>();
        const LayerCLGL* exitCL = exitPoints->getRepresentation<LayerCLGL>();
        LayerCLGL* outImageCL = outImage->getEditableRepresentation<LayerCLGL>();
        const VolumeCLGL* volumeCL = volume->getRepresentation<VolumeCLGL>();
        const LayerCLGL* transferFunctionCL = transferFunction->getRepresentation<LayerCLGL>();
        const LayerCLBase* background;
        if (background_) {
            background = background_->getRepresentation<LayerCLGL>();
            glSync.addToAquireGLObjectList(static_cast<const LayerCLGL*>(background));
        } else {
            background = defaultBackground_.getRepresentation<LayerCL>();
        }
        // Shared objects must be acquired before use.
        glSync.addToAquireGLObjectList(entryCL);
        glSync.addToAquireGLObjectList(exitCL);
        glSync.addToAquireGLObjectList(outImageCL);
        glSync.addToAquireGLObjectList(volumeCL);
        glSync.addToAquireGLObjectList(transferFunctionCL);

        // Acquire all of the objects at once
        glSync.aquireAllObjects();

        volumeRaycast(volume, volumeCL, background, entryCL, exitCL, transferFunctionCL, outImageCL,
                      globalWorkGroupSize, localWorkGroupSize, waitForEvents, event);
    } else {
        const LayerCL* entryCL = entryPoints->getRepresentation<LayerCL>();
        const LayerCL* exitCL = exitPoints->getRepresentation<LayerCL>();
        LayerCL* outImageCL = outImage->getEditableRepresentation<LayerCL>();
        const VolumeCL* volumeCL = volume->getRepresentation<VolumeCL>();
        const LayerCL* transferFunctionCL = transferFunction->getRepresentation<LayerCL>();
        // const LayerCL* background = background_->getRepresentation<LayerCL>();
        const LayerCL* background;
        if (background_) {
            background = background_->getRepresentation<LayerCL>();
        } else {
            background = defaultBackground_.getRepresentation<LayerCL>();
        }
        volumeRaycast(volume, volumeCL, background, entryCL, exitCL, transferFunctionCL, outImageCL,
                      globalWorkGroupSize, localWorkGroupSize, waitForEvents, event);
    }
}

void VolumeRaycasterCL::volumeRaycast(const Volume* volume, const VolumeCLBase* volumeCL,
                                      const LayerCLBase* background, const LayerCLBase* entryCLGL,
                                      const LayerCLBase* exitCLGL,
                                      const LayerCLBase* transferFunctionCL,
                                      LayerCLBase* outImageCL, size2_t globalWorkGroupSize,
                                      size2_t localWorkGroupSize,
                                      const VECTOR_CLASS<cl::Event>* waitForEvents /*= nullptr*/,
                                      cl::Event* event /*= nullptr*/) {
    // Note that the overloaded setArg methods requires
    // the reference to an object (not the pointer),
    // which is why we need to dereference the pointers
    kernel_->setArg(0, *volumeCL);
    kernel_->setArg(1, *(volumeCL->getVolumeStruct(volume)
                             .getRepresentation<BufferCL>()));  // Scaling for 12-bit data
    kernel_->setArg(2, *background);
    kernel_->setArg(3, *entryCLGL);
    kernel_->setArg(4, *exitCLGL);
    kernel_->setArg(5, *transferFunctionCL);
    kernel_->setArg(6, camera_->getLookFrom());
    kernel_->setArg(9, *outImageCL);
    //
    OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(*kernel_, cl::NullRange, globalWorkGroupSize,
                                                      localWorkGroupSize, waitForEvents, event);
}

void VolumeRaycasterCL::samplingRate(float samplingRate) {
    samplingRate_ = samplingRate;
    if (kernel_) {
        try {
            kernel_->setArg(7, samplingRate);
        } catch (cl::Error& err) {
            LogError(getCLErrorString(err));
        }
    }
}

void VolumeRaycasterCL::outputOffset(ivec2 val) {
    if (kernel_) {
        try {
            kernel_->setArg(10, val);
        } catch (cl::Error& err) {
            LogError(getCLErrorString(err));
        }
    }

    outputOffset_ = val;
}

void VolumeRaycasterCL::outputSize(ivec2 val) {
    if (kernel_) {
        try {
            kernel_->setArg(11, val);
        } catch (cl::Error& err) {
            LogError(getCLErrorString(err));
        }
    }

    outputSize_ = val;
}

void VolumeRaycasterCL::notifyObserversKernelCompiled(const cl::Kernel* kernel) {
    // Update kernel arguments that are only set once they are changed
    setKernelArguments();
    // Notify observers
    KernelObservable::notifyObserversKernelCompiled(kernel);
}

void VolumeRaycasterCL::setKernelArguments() {
    outputOffset(outputOffset_);
    outputSize(outputSize_);
    samplingRate(samplingRate());
    setLightingProperties(light_.shadingMode, light_.position.xyz(), light_.ambientColor.xyz(),
                          light_.diffuseColor.xyz(), light_.specularColor.xyz(),
                          light_.specularExponent);
}

void VolumeRaycasterCL::setLightingProperties(ShadingMode::Modes mode, const vec3& lightPosition,
                                              const vec3& ambientColor, const vec3& diffuseColor,
                                              const vec3& specularColor, float specularExponent) {
    light_.position = vec4(lightPosition, 1.f);
    light_.ambientColor = vec4(ambientColor, 1.f);
    light_.diffuseColor = vec4(diffuseColor, 1.f);
    light_.specularColor = vec4(specularColor, 1.f);
    light_.specularExponent = specularExponent;
    if (mode != light_.shadingMode) {
        light_.shadingMode = mode;
        compileKernel();
    }
    if (kernel_) {
        try {
            // Update data before returning it
            lightStruct_.upload(&light_, sizeof(utilcl::LightParameters));

            kernel_->setArg(8, lightStruct_);
        } catch (cl::Error& err) {
            LogError(getCLErrorString(err));
        }
    }
}

void VolumeRaycasterCL::setLightingProperties(const SimpleLightingProperty& light) {
    setLightingProperties(ShadingMode::Modes(light.shadingMode_.get()), light.lightPosition_.get(),
                          light.ambientColor_.get(), light.diffuseColor_.get(),
                          light.specularColor_.get(), light.specularExponent_.get());
}

void VolumeRaycasterCL::setDefaultBackgroundColor(const vec4 color) {
    LayerRAM_Vec4UINT8* defaultBGRAM =
        defaultBackground_.getEditableRepresentation<LayerRAM_Vec4UINT8>();
    unsigned char* data = static_cast<unsigned char*>(defaultBGRAM->getData());
    for (int i = 0; i < 4; ++i) {
        data[i] = static_cast<unsigned char>(glm::floor(color[i] * 255.f + 0.5f));
    }
}

void VolumeRaycasterCL::compileKernel() {
    if (kernel_) {
        removeKernel(kernel_);
    }
    std::stringstream defines;
    if (light_.shadingMode != 0) defines << " -D SHADING_MODE=" << light_.shadingMode;
    // Will compile kernel and make sure that it it
    // recompiled whenever the file changes
    // If the kernel fails to compile it will be set to nullptr
    kernel_ = addKernel("volumeraycaster.cl", "raycaster", defines.str());
    setKernelArguments();
}

}  // namespace
