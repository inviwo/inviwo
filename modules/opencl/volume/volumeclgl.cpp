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
#include <modules/opencl/openclsharing.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

VolumeCLGL::VolumeCLGL(const DataFormatBase* format, Texture3D* data)
    : VolumeRepresentation(format)
    , dimensions_(data != NULL ? data->getDimensions() : uvec3(64))
    , texture_(data) {
    if (data) {
        initialize(data);
    }
}

VolumeCLGL::VolumeCLGL(const uvec3& dimensions, const DataFormatBase* format, Texture3D* data)
    : VolumeRepresentation(format), dimensions_(dimensions), texture_(data) {
    initialize(data);
}

VolumeCLGL::VolumeCLGL(const VolumeCLGL& rhs)
    : VolumeRepresentation(rhs), dimensions_(rhs.dimensions_) {
    initialize(rhs.texture_);
}

VolumeCLGL::~VolumeCLGL() { deinitialize(); }

void VolumeCLGL::initialize(Texture3D* texture) {
    ivwAssert(texture != 0, "Cannot initialize with null OpenGL texture");
    // Indicate that the texture should not be deleted.
    texture->increaseRefCount();
    CLTextureSharingMap::iterator it = OpenCLImageSharing::clImageSharingMap_.find(texture);

    if (it == OpenCLImageSharing::clImageSharingMap_.end()) {
        clImage_ = new cl::Image3DGL(OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE,
                                     GL_TEXTURE_3D, 0, texture->getID());
        OpenCLImageSharing::clImageSharingMap_.insert(
            TextureCLImageSharingPair(texture_, new OpenCLImageSharing(clImage_)));
    } else {
        clImage_ = it->second->sharedMemory_;
        it->second->increaseRefCount();
    }

    texture->addObserver(this);
    VolumeCLGL::initialize();
}

const uvec3& VolumeCLGL::getDimensions() const { return dimensions_; }

VolumeCLGL* VolumeCLGL::clone() const { return new VolumeCLGL(*this); }

void VolumeCLGL::deinitialize() {
    // Delete OpenCL image before texture
    CLTextureSharingMap::iterator it = OpenCLImageSharing::clImageSharingMap_.find(texture_);

    if (it != OpenCLImageSharing::clImageSharingMap_.end()) {
        if (it->second->decreaseRefCount() == 0) {
            delete it->second->sharedMemory_;
            it->second->sharedMemory_ = 0;
            delete it->second;
            OpenCLImageSharing::clImageSharingMap_.erase(it);
        }
    }

    if (texture_ && texture_->decreaseRefCount() <= 0) {
        delete texture_;
        texture_ = NULL;
    }
}

void VolumeCLGL::notifyBeforeTextureInitialization() {
    CLTextureSharingMap::iterator it = OpenCLImageSharing::clImageSharingMap_.find(texture_);

    if (it != OpenCLImageSharing::clImageSharingMap_.end()) {
        if (it->second->decreaseRefCount() == 0) {
            delete it->second->sharedMemory_;
            it->second->sharedMemory_ = 0;
        }
    }

    clImage_ = 0;
}

void VolumeCLGL::notifyAfterTextureInitialization() {
    CLTextureSharingMap::iterator it = OpenCLImageSharing::clImageSharingMap_.find(texture_);

    if (it != OpenCLImageSharing::clImageSharingMap_.end()) {
        if (it->second->getRefCount() == 0) {
            it->second->sharedMemory_ =
                new cl::Image3DGL(OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE, GL_TEXTURE_3D,
                                  0, texture_->getID());
        }

        clImage_ = it->second->sharedMemory_;
        it->second->increaseRefCount();
    }
}

void VolumeCLGL::aquireGLObject(
    std::vector<cl::Event>* syncEvents /*= NULL*/,
    const cl::CommandQueue& queue /*= OpenCL::getPtr()->getQueue()*/) const {
    std::vector<cl::Memory> syncImages(1, *clImage_);
    queue.enqueueAcquireGLObjects(&syncImages, syncEvents);
}

void VolumeCLGL::releaseGLObject(
    std::vector<cl::Event>* syncEvents /*= NULL*/, cl::Event* event /*= NULL*/,
    const cl::CommandQueue& queue /*= OpenCL::getPtr()->getQueue()*/) const {
    std::vector<cl::Memory> syncImages(1, *clImage_);
    queue.enqueueReleaseGLObjects(&syncImages, syncEvents, event);
}

void VolumeCLGL::setDimensions(uvec3 dimensions) {
    if (dimensions == dimensions_) {
        return;
    }
    dimensions_ = dimensions;

    // Make sure that the OpenCL layer is deleted before resizing the texture
    // By observing the texture we will make sure that the OpenCL layer is
    // deleted and reattached after resizing is done.
    const_cast<Texture3D*>(texture_)->uploadAndResize(NULL, dimensions);
}

cl::Image3D& VolumeCLGL::getEditable() { return *static_cast<cl::Image3D*>(clImage_); }

const cl::Image3D& VolumeCLGL::get() const {
    return *const_cast<const cl::Image3D*>(static_cast<const cl::Image3D*>(clImage_));
}

const Texture3D* VolumeCLGL::getTexture() const { return texture_; }

}  // namespace

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::VolumeCLGL& value) {
    return setArg(index, value.get());
}

}  // namespace cl
