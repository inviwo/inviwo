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

#include <modules/opencl/volume/volumecl.h>
#include <modules/opencl/clformats.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/stdextensions.h>  // make_unique is c++14 but works on some compilers

namespace inviwo {

VolumeCL::VolumeCL(size3_t dimensions, const DataFormatBase* format, const void* data,
                   const SwizzleMask& swizzleMask, InterpolationType interpolation,
                   const Wrapping3D& wrapping)
    : VolumeCLBase{}
    , VolumeRepresentation{}
    , dataFormatBase_{format}
    , dimensions_{dimensions}
    , imageFormat_{dataFormatToCLImageFormat(format->getId())}
    , swizzleMask_{swizzleMask}
    , interpolation_{interpolation}
    , wrapping_{wrapping} {

    initialize(data);
}

VolumeCL::VolumeCL(const VolumeReprConfig& config, const void* data)
    : VolumeCL{config.dimensions.value_or(VolumeConfig::defaultDimensions),
               config.format ? config.format : VolumeConfig::defaultFormat,
               data,
               config.swizzleMask.value_or(VolumeConfig::defaultSwizzleMask),
               config.interpolation.value_or(VolumeConfig::defaultInterpolation),
               config.wrapping.value_or(VolumeConfig::defaultWrapping)} {}

VolumeCL::VolumeCL(const VolumeCL& rhs)
    : VolumeCLBase{rhs}
    , VolumeRepresentation{rhs}
    , dataFormatBase_{rhs.dataFormatBase_}
    , dimensions_{rhs.dimensions_}
    , imageFormat_{rhs.imageFormat_}
    , swizzleMask_{rhs.swizzleMask_}
    , interpolation_{rhs.interpolation_}
    , wrapping_{rhs.wrapping_} {

    initialize(nullptr);
    OpenCL::getPtr()->getQueue().enqueueCopyImage(rhs.get(), *clImage_, glm::size3_t(0),
                                                  glm::size3_t(0), glm::size3_t(dimensions_));
}

VolumeCL::~VolumeCL() = default;

void VolumeCL::initialize(const void* voxels) {
    clImage_ =
        std::make_unique<cl::Image3D>(OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE,
                                      getFormat(), dimensions_.x, dimensions_.y, dimensions_.z);

    if (voxels) {
        OpenCL::getPtr()->getQueue().enqueueWriteImage(*clImage_, true, glm::size3_t(0),
                                                       glm::size3_t(dimensions_), 0, 0,
                                                       const_cast<void*>(voxels));
    }
}

const size3_t& VolumeCL::getDimensions() const { return dimensions_; }

void VolumeCL::setDimensions(size3_t dimensions) {
    dimensions_ = dimensions;
    initialize(nullptr);
}

VolumeCL* VolumeCL::clone() const { return new VolumeCL(*this); }

void VolumeCL::upload(const void* data) {
    OpenCL::getPtr()->getQueue().enqueueWriteImage(
        *clImage_, true, glm::size3_t(0), glm::size3_t(dimensions_), 0, 0, const_cast<void*>(data));
}

void VolumeCL::download(void* data) const {
    OpenCL::getPtr()->getQueue().enqueueReadImage(*clImage_, true, glm::size3_t(0),
                                                  glm::size3_t(dimensions_), 0, 0, data);
}

cl::ImageFormat VolumeCL::getFormat() const { return imageFormat_; }

cl::Image3D& VolumeCL::getEditable() { return *clImage_; }

const cl::Image3D& VolumeCL::get() const { return *clImage_; }

std::type_index VolumeCL::getTypeIndex() const { return std::type_index(typeid(VolumeCL)); }

void VolumeCL::setSwizzleMask(const SwizzleMask& mask) { swizzleMask_ = mask; }

SwizzleMask VolumeCL::getSwizzleMask() const { return swizzleMask_; }

void VolumeCL::setInterpolation(InterpolationType interpolation) { interpolation_ = interpolation; }

InterpolationType VolumeCL::getInterpolation() const { return interpolation_; }

void VolumeCL::setWrapping(const Wrapping3D& wrapping) { wrapping_ = wrapping; }

Wrapping3D VolumeCL::getWrapping() const { return wrapping_; }

}  // namespace inviwo

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::VolumeCL& value) {
    return setArg(index, value.get());
}

}  // namespace cl
