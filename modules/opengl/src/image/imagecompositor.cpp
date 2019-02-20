/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

ImageCompositor::ImageCompositor(std::string programFileName) : shader_(programFileName) {}

void ImageCompositor::composite(const Image& source, Image& target, ImageType type) {
    utilgl::activateTarget(target, type);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, source, "tex0", ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, target, "tex1", ImageType::ColorDepthPicking);
    utilgl::setShaderUniforms(shader_, target, "outportParameters");
    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void ImageCompositor::composite(ImageInport& source, ImageOutport& target, ImageType type) {
    if (source.isReady() && target.hasData()) {
        utilgl::activateTarget(target, type);
        shader_.activate();

        TextureUnitContainer units;
        utilgl::bindAndSetUniforms(shader_, units, *source.getData(), "tex0",
                                   ImageType::ColorDepthPicking);
        utilgl::bindAndSetUniforms(shader_, units, *target.getData(), "tex1",
                                   ImageType::ColorDepthPicking);
        utilgl::setShaderUniforms(shader_, target, "outportParameters");
        utilgl::singleDrawImagePlaneRect();

        shader_.deactivate();
        utilgl::deactivateCurrentTarget();
    }
}

}  // namespace inviwo
