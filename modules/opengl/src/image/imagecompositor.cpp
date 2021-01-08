/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <modules/opengl/image/imagecompositor.h>

#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/openglutils.h>
#include <inviwo/core/util/assertion.h>

namespace inviwo {

ImageCompositor::ImageCompositor(std::string programFileName)
    : shader_(programFileName)
    , colorTex_({1u, 1u}, GLFormat(), GL_LINEAR)
    , depthTex_({1u, 1u}, GLFormat(), GL_LINEAR)
    , pickingTex_({1u, 1u}, GLFormat(), GL_LINEAR) {}

void ImageCompositor::composite(const Image& source, Image& target, ImageType type) {
    copyTextures(target);

    utilgl::activateTarget(target, type);
    shader_.activate();
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, source, "tex0", ImageType::ColorDepthPicking);
    bindTextures(units, "tex1");
    utilgl::setShaderUniforms(shader_, target, "outportParameters");
    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void ImageCompositor::composite(ImageInport& source, ImageOutport& target, ImageType type) {
    if (source.isReady() && target.hasData()) {
        copyTextures(*target.getData());

        utilgl::activateTarget(target, type);
        shader_.activate();
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);

        TextureUnitContainer units;
        utilgl::bindAndSetUniforms(shader_, units, *source.getData(), "tex0",
                                   ImageType::ColorDepthPicking);
        bindTextures(units, "tex1");
        utilgl::setShaderUniforms(shader_, target, "outportParameters");
        utilgl::singleDrawImagePlaneRect();

        shader_.deactivate();
        utilgl::deactivateCurrentTarget();
    }
}

void ImageCompositor::copyTextures(const Image& target) {
    auto glrep = target.getRepresentation<ImageGL>();
    {
        auto layer = glrep->getColorLayerGL();
        IVW_ASSERT(layer, "no color layer in target image");
        colorTex_ = *layer->getTexture();
    }
    {
        auto layer = glrep->getDepthLayerGL();
        IVW_ASSERT(layer, "no depth layer in target image");
        depthTex_ = *layer->getTexture();
    }
    {
        auto layer = glrep->getPickingLayerGL();
        IVW_ASSERT(layer, "no picking layer in target image");
        pickingTex_ = *layer->getTexture();
    }
}

void ImageCompositor::bindTextures(TextureUnitContainer& cont, const std::string& id) {
    TextureUnit unit1, unit2, unit3;
    utilgl::bindTexture(colorTex_, unit1);
    utilgl::bindTexture(depthTex_, unit2);
    utilgl::bindTexture(pickingTex_, unit3);
    shader_.setUniform(id + "Color", unit1);
    shader_.setUniform(id + "Depth", unit2);
    shader_.setUniform(id + "Picking", unit3);
    cont.push_back(std::move(unit1));
    cont.push_back(std::move(unit2));
    cont.push_back(std::move(unit3));
}

}  // namespace inviwo
