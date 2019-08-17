/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwo/core/util/formats.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/glformats.h>
#include <modules/opengl/texture/texture.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

LayerGL::LayerGL(size2_t dimensions, LayerType type, const DataFormatBase* format,
                 std::shared_ptr<Texture2D> tex, const SwizzleMask& swizzleMask)
    : LayerRepresentation(type, format)
    , dimensions_(dimensions)
    , texture_(tex)
    , swizzleMask_(swizzleMask) {
    if (!texture_) {
        auto glFormat = GLFormats::get(getDataFormatId());

        if (getLayerType() == LayerType::Depth) {
            texture_ = std::make_shared<Texture2D>(getDimensions(), GL_DEPTH_COMPONENT,
                                                   GL_DEPTH_COMPONENT24, glFormat.type, GL_NEAREST);
        } else {
            texture_ = std::make_shared<Texture2D>(getDimensions(), glFormat, GL_LINEAR);
        }
    }
    texture_->setSwizzleMask(swizzleMask_);
}

LayerGL::LayerGL(const LayerGL& rhs)
    : LayerRepresentation(rhs)
    , dimensions_(rhs.dimensions_)
    , texture_(rhs.texture_->clone())
    , swizzleMask_(rhs.swizzleMask_) {
    texture_->setSwizzleMask(swizzleMask_);
}

LayerGL& LayerGL::operator=(const LayerGL& rhs) {
    if (this != &rhs) {
        LayerRepresentation::operator=(rhs);
        dimensions_ = rhs.dimensions_;
        texture_ = std::shared_ptr<Texture2D>(rhs.texture_->clone());
        swizzleMask_ = rhs.swizzleMask_;
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
    dimensions_ = dimensions;

    if (texture_) {
        texture_->unbind();
        texture_->resize(dimensions_);
        texture_->bind();
    }
}

const size2_t& LayerGL::getDimensions() const { return dimensions_; }

void LayerGL::setSwizzleMask(const SwizzleMask& mask) {
    swizzleMask_ = mask;
    if (texture_) {
        texture_->setSwizzleMask(mask);
    }
}

SwizzleMask LayerGL::getSwizzleMask() const { return swizzleMask_; }

}  // namespace inviwo
