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

#include <modules/opencl/volume/volumeclgl.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {
CLTexture3DSharingMap VolumeCLGL::clVolumeSharingMap_;

VolumeCLGL::VolumeCLGL(const DataFormatBase* format, Texture3D* data)
    : VolumeRepresentation(format)
    , dimensions_(data != nullptr ? data->getDimensions() : size3_t(64))
    , texture_(data) {
    initialize();
}

VolumeCLGL::VolumeCLGL(const size3_t& dimensions, const DataFormatBase* format, std::shared_ptr<Texture3D> data)
    : VolumeRepresentation(format), dimensions_(dimensions), texture_(data) {
    initialize();
}

VolumeCLGL::VolumeCLGL(const VolumeCLGL& rhs)
    : VolumeRepresentation(rhs), dimensions_(rhs.dimensions_), texture_(rhs.texture_->clone()) {
    initialize();
}

VolumeCLGL::~VolumeCLGL() { deinitialize(); }

void VolumeCLGL::initialize() {
    if (texture_) {
        const auto it = VolumeCLGL::clVolumeSharingMap_.find(texture_);

        if (it == VolumeCLGL::clVolumeSharingMap_.end()) {
            clImage_ = std::make_shared<cl::Image3DGL>(OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE,
                GL_TEXTURE_3D, 0, texture_->getID());
            VolumeCLGL::clVolumeSharingMap_.insert(
                Texture3DCLImageSharingPair(texture_, clImage_));
        } else {
            clImage_ = it->second;
        }

        texture_->addObserver(this);
    }
}

const size3_t& VolumeCLGL::getDimensions() const { return dimensions_; }

VolumeCLGL* VolumeCLGL::clone() const { return new VolumeCLGL(*this); }

void VolumeCLGL::deinitialize() {
    // Delete OpenCL image before texture
    const auto it = VolumeCLGL::clVolumeSharingMap_.find(texture_);
    clImage_.reset();
    if (it != VolumeCLGL::clVolumeSharingMap_.end()) {
        if (it->second.use_count() == 1) {
            VolumeCLGL::clVolumeSharingMap_.erase(it);
        }
    }
}

void VolumeCLGL::notifyBeforeTextureInitialization() {
    const auto it = VolumeCLGL::clVolumeSharingMap_.find(texture_);
    // Release reference
    clImage_.reset();
    if (it != VolumeCLGL::clVolumeSharingMap_.end()) {
        if (it->second.use_count() == 1) {
            VolumeCLGL::clVolumeSharingMap_.erase(it);
        }
    }
}

void VolumeCLGL::notifyAfterTextureInitialization() {
    const auto it = VolumeCLGL::clVolumeSharingMap_.find(texture_);

    if (it != VolumeCLGL::clVolumeSharingMap_.end()) {
        clImage_ = it->second;
    } else {
        try {
            clImage_ = std::make_shared<cl::Image3DGL>(OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE, GL_TEXTURE_3D,
                0, texture_->getID());
            VolumeCLGL::clVolumeSharingMap_.insert(
                Texture3DCLImageSharingPair(texture_, clImage_));
        } catch (cl::Error& err) {
            LogOpenCLError(err.err());
            throw err;
        }
    }
}

void VolumeCLGL::aquireGLObject(
    std::vector<cl::Event>* syncEvents /*= nullptr*/,
    const cl::CommandQueue& queue /*= OpenCL::getPtr()->getQueue()*/) const {
    std::vector<cl::Memory> syncImages(1, *clImage_);
    queue.enqueueAcquireGLObjects(&syncImages, syncEvents);
}

void VolumeCLGL::releaseGLObject(
    std::vector<cl::Event>* syncEvents /*= nullptr*/, cl::Event* event /*= nullptr*/,
    const cl::CommandQueue& queue /*= OpenCL::getPtr()->getQueue()*/) const {
    std::vector<cl::Memory> syncImages(1, *clImage_);
    queue.enqueueReleaseGLObjects(&syncImages, syncEvents, event);
}

void VolumeCLGL::setDimensions(size3_t dimensions) {
    if (dimensions == dimensions_) {
        return;
    }
    dimensions_ = dimensions;

    // Make sure that the OpenCL layer is deleted before resizing the texture
    // By observing the texture we will make sure that the OpenCL layer is
    // deleted and reattached after resizing is done.
    texture_->uploadAndResize(nullptr, dimensions);
}

cl::Image3DGL& VolumeCLGL::getEditable() { return *clImage_; }

const cl::Image3DGL& VolumeCLGL::get() const {
    return *clImage_;
}

std::shared_ptr<Texture3D> VolumeCLGL::getTexture() const {
    return texture_;
}

}  // namespace

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::VolumeCLGL& value) {
    return setArg(index, value.get());
}

}  // namespace cl
