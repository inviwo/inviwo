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

#include <modules/opencl/volume/volumecl.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

VolumeCL::VolumeCL(const DataFormatBase* format, const void* data)
    : VolumeRepresentation(uvec3(128, 128, 128), format)
    , imageFormat_(dataFormatToCLImageFormat(format->getId())) {
    initialize(data);
}

VolumeCL::VolumeCL(uvec3 dimensions, const DataFormatBase* format, const void* data)
    : VolumeRepresentation(dimensions, format)
    , imageFormat_(dataFormatToCLImageFormat(format->getId())) {
    initialize(data);
}

VolumeCL::VolumeCL(const VolumeCL& rhs)
    : VolumeRepresentation(rhs), imageFormat_(rhs.imageFormat_) {
    initialize(NULL);
    OpenCL::getPtr()->getQueue().enqueueCopyImage(rhs.get(), *clImage_, glm::svec3(0),
                                                  glm::svec3(0), glm::svec3(dimensions_));
}

VolumeCL::~VolumeCL() { deinitialize(); }

void VolumeCL::initialize(const void* voxels) {
    clImage_ = new cl::Image3D(OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE, getFormat(),
                               dimensions_.x, dimensions_.y, dimensions_.z);

    if (voxels != NULL) {
        OpenCL::getPtr()->getQueue().enqueueWriteImage(*clImage_, true, glm::svec3(0),
                                                       glm::svec3(dimensions_), 0, 0,
                                                       const_cast<void*>(voxels));
    }

    VolumeCL::initialize();
}

VolumeCL* VolumeCL::clone() const {
    return new VolumeCL(*this);
}

void VolumeCL::deinitialize() {
    delete clImage_;
}

void VolumeCL::upload(const void* data) {
    OpenCL::getPtr()->getQueue().enqueueWriteImage(
        *clImage_, true, glm::svec3(0), glm::svec3(dimensions_), 0, 0, const_cast<void*>(data));
}

void VolumeCL::download(void* data) const {
    OpenCL::getPtr()->getQueue().enqueueReadImage(*clImage_, true, glm::svec3(0),
                                                  glm::svec3(dimensions_), 0, 0, data);
}

} // namespace

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::VolumeCL& value) {
    return setArg(index, value.get());
}


} // namespace cl
