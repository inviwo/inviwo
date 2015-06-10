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

#include <modules/opencl/volume/volumecl.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

VolumeCL::VolumeCL(const DataFormatBase* format, const void* data)
    : VolumeRepresentation(format)
    , dimensions_(size3_t(128, 128, 128))
    , imageFormat_(dataFormatToCLImageFormat(format->getId())) {
    initialize(data);
}

VolumeCL::VolumeCL(size3_t dimensions, const DataFormatBase* format, const void* data)
    : VolumeRepresentation(format)
    , dimensions_(dimensions)
    , imageFormat_(dataFormatToCLImageFormat(format->getId())) {
    initialize(data);
}

VolumeCL::VolumeCL(const VolumeCL& rhs)
    : VolumeRepresentation(rhs), dimensions_(rhs.dimensions_), imageFormat_(rhs.imageFormat_) {
    initialize(nullptr);
    OpenCL::getPtr()->getQueue().enqueueCopyImage(rhs.get(), *clImage_, glm::size3_t(0),
                                                  glm::size3_t(0), glm::size3_t(dimensions_));
}

VolumeCL::~VolumeCL() { deinitialize(); }

void VolumeCL::initialize(const void* voxels) {
    clImage_ = new cl::Image3D(OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE, getFormat(),
                               dimensions_.x, dimensions_.y, dimensions_.z);

    if (voxels) {
        OpenCL::getPtr()->getQueue().enqueueWriteImage(*clImage_, true, glm::size3_t(0),
                                                       glm::size3_t(dimensions_), 0, 0,
                                                       const_cast<void*>(voxels));
    }

    VolumeCL::initialize();
}

const size3_t& VolumeCL::getDimensions() const { return dimensions_; }

void VolumeCL::setDimensions(size3_t dimensions) {
    dimensions_ = dimensions;
    deinitialize();
    initialize();
}

VolumeCL* VolumeCL::clone() const { return new VolumeCL(*this); }

void VolumeCL::deinitialize() { delete clImage_; }

void VolumeCL::upload(const void* data) {
    OpenCL::getPtr()->getQueue().enqueueWriteImage(
        *clImage_, true, glm::size3_t(0), glm::size3_t(dimensions_), 0, 0, const_cast<void*>(data));
}

void VolumeCL::download(void* data) const {
    OpenCL::getPtr()->getQueue().enqueueReadImage(*clImage_, true, glm::size3_t(0),
                                                  glm::size3_t(dimensions_), 0, 0, data);
}

cl::ImageFormat VolumeCL::getFormat() const { return imageFormat_; }

cl::Image3D& VolumeCL::getEditable() { return *static_cast<cl::Image3D*>(clImage_); }

const cl::Image3D& VolumeCL::get() const {
    return *const_cast<const cl::Image3D*>(static_cast<const cl::Image3D*>(clImage_));
}

}  // namespace

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::VolumeCL& value) {
    return setArg(index, value.get());
}

}  // namespace cl
