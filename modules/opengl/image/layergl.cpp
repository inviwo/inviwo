/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/texture.h>
#include <modules/opengl/texture/texture2d.h>

namespace inviwo {

LayerGL::LayerGL(size2_t dimensions, LayerType type, const DataFormatBase* format, std::shared_ptr<Texture2D> tex)
    : LayerRepresentation(dimensions, type, format), texture_(tex) {
    if (!texture_) {
        GLFormats::GLFormat glFormat = getGLFormats()->getGLFormat(getDataFormatId());

        if (getLayerType() == DEPTH_LAYER) {
            texture_ = std::make_shared<Texture2D>(getDimensions(), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24,
                glFormat.type, GL_NEAREST);
        } else {
            texture_ = std::make_shared<Texture2D>(getDimensions(), glFormat, GL_LINEAR);
        }
    }
}

LayerGL::LayerGL(const LayerGL& rhs) : LayerRepresentation(rhs), texture_(rhs.texture_->clone()) {
    //texture_ = rhs.texture_->clone();
}

LayerGL& LayerGL::operator=(const LayerGL& rhs) {
    if (this != &rhs) {
        LayerRepresentation::operator=(rhs);
        texture_ = std::shared_ptr<Texture2D>(rhs.texture_->clone());
    }

    return *this;
}

LayerGL::~LayerGL() { }

LayerGL* LayerGL::clone() const { return new LayerGL(*this); }


void LayerGL::bindTexture(GLenum texUnit) const {
    texUnit_ = texUnit;
    utilgl::bindTexture(texture_.get(), texUnit);
}

void LayerGL::unbindTexture() const {
    glActiveTexture(texUnit_);
    texture_->unbind();
    glActiveTexture(GL_TEXTURE0);
}

bool LayerGL::copyRepresentationsTo(DataRepresentation* targetLayerGL) const {
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

void LayerGL::setDimensions(size2_t dimensions) {
    dimensions_ = dimensions;

    if (texture_) {
        texture_->unbind();
        texture_->resize(dimensions_);
        texture_->bind();
    }
}

}  // namespace
