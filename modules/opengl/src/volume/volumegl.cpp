/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <modules/opengl/volume/volumegl.h>

#include <inviwo/core/datastructures/image/imagetypes.h>             // for InterpolationType
#include <inviwo/core/datastructures/volume/volumerepresentation.h>  // for VolumeRepresentation
#include <inviwo/core/util/assertion.h>                              // for IVW_ASSERT
#include <inviwo/core/util/formats.h>                                // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                 // for size3_t
#include <modules/opengl/glformats.h>                                // for GLFormats
#include <modules/opengl/openglutils.h>                              // for convertWrappingToGL
#include <modules/opengl/texture/texture3d.h>                        // for Texture3D
#include <inviwo/core/resourcemanager/resource.h>

#include <type_traits>  // for remove_extent_t

namespace inviwo {

VolumeGL::VolumeGL(size3_t dimensions, const DataFormatBase* format, const SwizzleMask& swizzleMask,
                   InterpolationType interpolation, const Wrapping3D& wrapping,
                   bool initializeTexture)
    : VolumeRepresentation{}
    , texture_{std::make_shared<Texture3D>(dimensions, GLFormats::get(format->getId()),
                                           utilgl::convertInterpolationToGL(interpolation),
                                           swizzleMask, utilgl::convertWrappingToGL(wrapping))} {
    if (initializeTexture) {
        texture_->initialize(nullptr);
    }
}

VolumeGL::VolumeGL(const VolumeReprConfig& config, bool initializeTexture)
    : VolumeGL{config.dimensions.value_or(VolumeConfig::defaultDimensions),
               config.format ? config.format : VolumeConfig::defaultFormat,
               config.swizzleMask.value_or(VolumeConfig::defaultSwizzleMask),
               config.interpolation.value_or(VolumeConfig::defaultInterpolation),
               config.wrapping.value_or(VolumeConfig::defaultWrapping),
               initializeTexture} {}

VolumeGL::VolumeGL(std::shared_ptr<Texture3D> tex) : VolumeRepresentation{}, texture_(tex) {
    IVW_ASSERT(texture_, "The texture should never be nullptr.");
}

VolumeGL::VolumeGL(const VolumeGL& rhs)
    : VolumeRepresentation(rhs), texture_(rhs.texture_->clone()) {}

VolumeGL& VolumeGL::operator=(const VolumeGL& rhs) {
    if (this != &rhs) {
        VolumeRepresentation::operator=(rhs);
        texture_ = std::shared_ptr<Texture3D>(rhs.texture_->clone());
    }
    return *this;
}

VolumeGL::~VolumeGL() = default;

VolumeGL* VolumeGL::clone() const { return new VolumeGL(*this); }

const DataFormatBase* VolumeGL::getDataFormat() const { return texture_->getDataFormat(); }

void VolumeGL::bindTexture(GLenum texUnit) const {
    glActiveTexture(texUnit);
    texture_->bind();
    glActiveTexture(GL_TEXTURE0);
}

void VolumeGL::unbindTexture() const { texture_->unbind(); }

void VolumeGL::setDimensions(size3_t dimensions) { texture_->uploadAndResize(nullptr, dimensions); }

const size3_t& VolumeGL::getDimensions() const { return texture_->getDimensions(); }

std::type_index VolumeGL::getTypeIndex() const { return std::type_index(typeid(VolumeGL)); }

void VolumeGL::setSwizzleMask(const SwizzleMask& mask) { texture_->setSwizzleMask(mask); }

SwizzleMask VolumeGL::getSwizzleMask() const { return texture_->getSwizzleMask(); }

void VolumeGL::setInterpolation(InterpolationType interpolation) {
    texture_->setInterpolation(interpolation);
}

InterpolationType VolumeGL::getInterpolation() const { return texture_->getInterpolation(); }

void VolumeGL::setWrapping(const Wrapping3D& wrapping) {
    texture_->setWrapping(utilgl::convertWrappingToGL(wrapping));
}

Wrapping3D VolumeGL::getWrapping() const {
    return utilgl::convertWrappingFromGL(texture_->getWrapping());
}

void VolumeGL::updateResource(const ResourceMeta& meta) const {
    if (texture_) {
        resource::meta(resource::GL{texture_->getID()}, meta);
    }
}

}  // namespace inviwo
