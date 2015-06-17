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

#ifndef IVW_VOLUME_RAYCASTER_CL_H
#define IVW_VOLUME_RAYCASTER_CL_H

#include <modules/basecl/baseclmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/cameraproperty.h>

#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/buffer/buffercl.h>
#include <modules/opencl/image/layerclbase.h>
#include <modules/opencl/kernelowner.h>
#include <modules/opencl/utilcl.h>
#include <modules/opencl/volume/volumeclbase.h>

namespace inviwo {

/**
 * \brief Perform volume rendering on the input volume.
 *
 */
class IVW_MODULE_BASECL_API VolumeRaycasterCL : public KernelOwner {
public:
    VolumeRaycasterCL();
    ~VolumeRaycasterCL();

    bool isValid() const { return kernel_ != nullptr; }

    /**
     * \brief Perform volume rendering on the input volume.
     *
     * @param volume
     * @param entryPoints Start point of ray in texture space.
     * @param exitPoints End point of ray in texture space.
     * @param transferFunction Transfer function, mapping value to color and opacity.
     * @param outImage Output image
     * @param waitForEvents
     * @param event
     */
    void volumeRaycast(const Volume* volume, const Layer* entryPoints, const Layer* exitPoints,
                       const Layer* transferFunction, Layer* outImage,
                       const VECTOR_CLASS<cl::Event>* waitForEvents = nullptr,
                       cl::Event* event = nullptr);

    void volumeRaycast(const Volume* volume, const VolumeCLBase* volumeCL,
                       const LayerCLBase* background, const LayerCLBase* entryCL,
                       const LayerCLBase* exitCL, const LayerCLBase* transferFunctionCL,
                       LayerCLBase* outImageCL, size2_t globalWorkGroupSize, size2_t localWorkGroupSize,
                       const VECTOR_CLASS<cl::Event>* waitForEvents = nullptr,
                       cl::Event* event = nullptr);

    void samplingRate(float samplingRate);
    float samplingRate() const { return samplingRate_; }

    const CameraProperty* getCamera() const { return camera_; }
    void setCamera(CameraProperty* camera) { camera_ = camera; }
    void setLightingProperties(const SimpleLightingProperty& light);
    void setLightingProperties(ShadingMode::Modes mode, const vec3& lightPosition,
                               const vec3& ambientColor, const vec3& diffuseColor,
                               const vec3& specularColor, float specularExponent);

    const Layer* getBackground() const {
        if (background_)
            return background_;
        else
            return &defaultBackground_;
    }
    /**
     * \brief Set background to use in the rendering.
     *
     * The default background, black, will be used if layer is a nullptr.
     * @param val Layer to use as background. Will not take ownership.
     */
    void setBackground(const Layer* val) { background_ = val; }
    /**
    * \brief Set the default background color to use in the rendering.
    * Will only be used if background layer is set to nullptr.
    *
    * @param RGBA color in [0 1]^4
    */
    void setDefaultBackgroundColor(const vec4 color);

    size2_t workGroupSize() const { return workGroupSize_; }
    void workGroupSize(const size2_t& val) { workGroupSize_ = val; }

    bool useGLSharing() const { return useGLSharing_; }
    void useGLSharing(bool val) { useGLSharing_ = val; }

    ivec2 outputOffset() const { return outputOffset_; }
    void outputOffset(ivec2 val);
    ivec2 outputSize() const { return outputSize_; }
    void outputSize(ivec2 val);

    /**
    * Called when kernel has compiled. Set all parameters and notify observers
    */
    virtual void notifyObserversKernelCompiled(const cl::Kernel* kernel);

    void setKernelArguments();

private:
    void compileKernel();
    // Parameters
    size2_t workGroupSize_;
    bool useGLSharing_;
    ivec2 outputOffset_;
    ivec2 outputSize_;
    CameraProperty* camera_;
    utilcl::LightParameters light_;
    float samplingRate_;

    const Layer* background_;
    Layer defaultBackground_;

    BufferCL lightStruct_;
    cl::Kernel* kernel_;
};

}  // namespace

#endif  // IVW_VOLUME_RAYCASTER_CL_H
