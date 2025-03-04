/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/volume/volumeclbase.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API VolumeCL : public VolumeCLBase, public VolumeRepresentation {
public:
    VolumeCL(size3_t dimensions, const DataFormatBase* format = DataFormatBase::get(),
             const void* data = nullptr, const SwizzleMask& swizzleMask = swizzlemasks::rgba,
             InterpolationType interpolation = InterpolationType::Linear,
             const Wrapping3D& wrapping = wrapping3d::clampAll);
    explicit VolumeCL(const VolumeReprConfig& config, const void* data = nullptr);
    virtual ~VolumeCL();
    VolumeCL(const VolumeCL& rhs);

    virtual VolumeCL* clone() const override;

    virtual const DataFormatBase* getDataFormat() const override { return dataFormatBase_; }

    virtual const size3_t& getDimensions() const override;
    virtual void setDimensions(size3_t dimensions) override;

    void upload(const void* data);
    /**
     * Download data to preallocated memory.
     *
     * @param data (void *) Preallocated pointer that will contain data after function returns.
     * @return (void)
     */
    void download(void* data) const;
    cl::ImageFormat getFormat() const;

    virtual cl::Image3D& getEditable() override;
    virtual const cl::Image3D& get() const override;

    virtual std::type_index getTypeIndex() const override final;

    /**
     * \brief update the swizzle mask of the color channels when sampling the volume
     *
     * @param mask new swizzle mask
     */
    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual void setInterpolation(InterpolationType interpolation) override;
    virtual InterpolationType getInterpolation() const override;

    virtual void setWrapping(const Wrapping3D& wrapping) override;
    virtual Wrapping3D getWrapping() const override;

protected:
    void initialize(const void* voxels);
    const DataFormatBase* dataFormatBase_;
    size3_t dimensions_;
    cl::ImageFormat imageFormat_;
    std::unique_ptr<cl::Image3D> clImage_;
    SwizzleMask swizzleMask_;
    InterpolationType interpolation_;
    Wrapping3D wrapping_;
};

}  // namespace inviwo

namespace cl {

// Kernel argument specializations for VolumeCL type
// (enables calling cl::Queue::setArg with VolumeCL
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::VolumeCL& value);

}  // namespace cl
