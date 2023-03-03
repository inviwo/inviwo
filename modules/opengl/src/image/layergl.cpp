/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <modules/opengl/image/layergl.h>

#include <inviwo/core/datastructures/image/imagetypes.h>           // for InterpolationType, Lay...
#include <inviwo/core/datastructures/image/layerrepresentation.h>  // for LayerRepresentation
#include <inviwo/core/util/assertion.h>                            // for IVW_ASSERT
#include <inviwo/core/util/glmvec.h>                               // for size2_t
#include <modules/opengl/glformats.h>                              // for GLFormat, GLFormats
#include <modules/opengl/openglutils.h>                            // for convertWrappingToGL
#include <modules/opengl/texture/texture2d.h>                      // for Texture2D
#include <modules/opengl/texture/textureunit.h>                    // for TextureUnit
#include <modules/opengl/texture/textureutils.h>                   // for bindTexture

#include <type_traits>  // for remove_extent_t

namespace inviwo {
class DataFormatBase;

LayerGL::LayerGL(std::shared_ptr<Texture2D> tex, LayerType type)
    : LayerRepresentation{type, tex->getDataFormat()}, texture_{tex} {
    IVW_ASSERT(texture_, "Texture should never be nullptr");
}

LayerGL::LayerGL(size2_t dimensions, LayerType type, const DataFormatBase* format,
                 const SwizzleMask& swizzleMask, InterpolationType interpolation,
                 const Wrapping2D& wrap)
    : LayerRepresentation{type, format}, texture_(nullptr) {

    const auto& glFormat = GLFormats::get(getDataFormatId());
    if (getLayerType() == LayerType::Depth) {
        texture_ = std::make_shared<Texture2D>(dimensions, GL_DEPTH_COMPONENT,
                                               GL_DEPTH_COMPONENT32F, glFormat.type, GL_NEAREST,
                                               swizzleMask, utilgl::convertWrappingToGL(wrap));
    } else {
        utilgl::convertInterpolationToGL(interpolation);
        texture_ = std::make_shared<Texture2D>(dimensions, glFormat,
                                               utilgl::convertInterpolationToGL(interpolation),
                                               swizzleMask, utilgl::convertWrappingToGL(wrap));
    }
}

LayerGL::LayerGL(const LayerGL& rhs) : LayerRepresentation(rhs), texture_(rhs.texture_->clone()) {}

LayerGL& LayerGL::operator=(const LayerGL& rhs) {
    if (this != &rhs) {
        LayerRepresentation::operator=(rhs);
        texture_ = std::shared_ptr<Texture2D>(rhs.texture_->clone());
    }

    return *this;
}

LayerGL::~LayerGL() = default;

LayerGL* LayerGL::clone() const { return new LayerGL(*this); }

void LayerGL::bindTexture(GLenum texUnit) const {
    texUnit_ = texUnit;
    utilgl::bindTexture(*texture_, texUnit);
}

void LayerGL::bindTexture(const TextureUnit& texUnit) const {
    texUnit_ = texUnit.getEnum();
    utilgl::bindTexture(*texture_, texUnit_);
}

void LayerGL::unbindTexture() const {
    glActiveTexture(texUnit_);
    texture_->unbind();
    glActiveTexture(GL_TEXTURE0);
}

bool LayerGL::copyRepresentationsTo(LayerRepresentation* /*targetLayerGL*/) const {
    /*const LayerGL* source = this;
    LayerGL* target = dynamic_cast<LayerGL*>(targetLayerGL);
    if(!target){
        return false;
        LogError("Target representation missing.");
    }

    const Texture2D* sTex = source->getTexture().get();
    Texture2D* tTex = target->getTexture().get();
    tTex->uploadFromPBO(sTex);
    LGL_ERROR;*/
    return false;
}

std::type_index LayerGL::getTypeIndex() const { return std::type_index(typeid(LayerGL)); }

void LayerGL::setDimensions(size2_t dimensions) {
    texture_->unbind();
    texture_->resize(dimensions);
    texture_->bind();
}

const size2_t& LayerGL::getDimensions() const { return texture_->getDimensions(); }

void LayerGL::setSwizzleMask(const SwizzleMask& mask) { texture_->setSwizzleMask(mask); }

SwizzleMask LayerGL::getSwizzleMask() const { return texture_->getSwizzleMask(); }

void LayerGL::setInterpolation(InterpolationType interpolation) {
    texture_->setInterpolation(interpolation);
}

InterpolationType LayerGL::getInterpolation() const { return texture_->getInterpolation(); }

void LayerGL::setWrapping(const Wrapping2D& wrapping) {
    texture_->setWrapping(utilgl::convertWrappingToGL(wrapping));
}

Wrapping2D LayerGL::getWrapping() const {
    return utilgl::convertWrappingFromGL(texture_->getWrapping());
}

}  // namespace inviwo
