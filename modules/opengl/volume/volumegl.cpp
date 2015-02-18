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
#include <modules/opengl/glwrap/shader.h>
#include <algorithm>

namespace inviwo {

VolumeGL::VolumeGL(uvec3 dimensions, const DataFormatBase* format, bool initializeTexture)
    : VolumeRepresentation(dimensions, format), volumeTexture_(NULL) {
    GLFormats::GLFormat glFormat = getGLFormats()->getGLFormat(getDataFormatId());
    volumeTexture_ = new Texture3D(dimensions_, glFormat, GL_LINEAR);
    if(initializeTexture){
        volumeTexture_->initialize(NULL);
    }
}

VolumeGL::VolumeGL(Texture3D* tex, const DataFormatBase* format)
    : VolumeRepresentation(tex->getDimensions(), format), volumeTexture_(tex) {
}

VolumeGL::VolumeGL(const VolumeGL& rhs) : VolumeRepresentation(rhs) {
    volumeTexture_ = rhs.volumeTexture_->clone();
}

VolumeGL& VolumeGL::operator=(const VolumeGL& rhs) {
    if (this != &rhs) {
        VolumeRepresentation::operator=(rhs);
        volumeTexture_ = rhs.volumeTexture_->clone();
    }

    return *this;
}

VolumeGL::~VolumeGL() { 
    if (volumeTexture_ && volumeTexture_->decreaseRefCount() <= 0) {
        delete volumeTexture_;
        volumeTexture_ = NULL;
    }
}

VolumeGL* VolumeGL::clone() const { return new VolumeGL(*this); }

void VolumeGL::bindTexture(GLenum texUnit) const {
    glActiveTexture(texUnit);
    volumeTexture_->bind();
    glActiveTexture(GL_TEXTURE0);
}

void VolumeGL::unbindTexture() const { volumeTexture_->unbind(); }

void VolumeGL::setDimensions(uvec3 dimensions) {
    dimensions_ = dimensions;
    volumeTexture_->uploadAndResize(NULL, dimensions_);
}

Texture3D* VolumeGL::getTexture() { return volumeTexture_; }

const Texture3D* VolumeGL::getTexture() const { return volumeTexture_; }

}  // namespace
