/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "volumegl.h"
#include <inviwo/core/datastructures/volume/volume.h>
#include <modules/opengl/shader/shader.h>
#include <algorithm>

namespace inviwo {

VolumeGL::VolumeGL(size3_t dimensions, const DataFormatBase* format, bool initializeTexture)
    : VolumeRepresentation(format), dimensions_(dimensions)
    , volumeTexture_(std::make_shared<Texture3D>(dimensions_, getGLFormats()->getGLFormat(format->getId()), GL_LINEAR)) {
    if (initializeTexture) {
        volumeTexture_->initialize(nullptr);
    }
}

VolumeGL::VolumeGL(std::shared_ptr<Texture3D> tex, const DataFormatBase* format)
    : VolumeRepresentation(format), dimensions_(tex->getDimensions()), volumeTexture_(tex) {}

VolumeGL::VolumeGL(const VolumeGL& rhs)
    : VolumeRepresentation(rhs)
    , dimensions_(rhs.dimensions_)
    , volumeTexture_(rhs.volumeTexture_->clone()) {}

VolumeGL& VolumeGL::operator=(const VolumeGL& rhs) {
    if (this != &rhs) {
        VolumeRepresentation::operator=(rhs);
        dimensions_ = rhs.dimensions_;
        volumeTexture_ = std::shared_ptr<Texture3D>(rhs.volumeTexture_->clone());
    }
    return *this;
}

VolumeGL::~VolumeGL() {
}

VolumeGL* VolumeGL::clone() const { return new VolumeGL(*this); }

void VolumeGL::bindTexture(GLenum texUnit) const {
    glActiveTexture(texUnit);
    volumeTexture_->bind();
    glActiveTexture(GL_TEXTURE0);
}

void VolumeGL::unbindTexture() const { volumeTexture_->unbind(); }

const size3_t& VolumeGL::getDimensions() const { return dimensions_; }

void VolumeGL::setDimensions(size3_t dimensions) {
    dimensions_ = dimensions;
    volumeTexture_->uploadAndResize(nullptr, dimensions_);
}

}  // namespace
