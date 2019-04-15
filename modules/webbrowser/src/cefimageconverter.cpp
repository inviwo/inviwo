/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/webbrowser/cefimageconverter.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {
CefImageConverter::CefImageConverter(vec3 pickingColor) {
    shader_.activate();
    shader_.setUniform("pickingColor", pickingColor);
    shader_.deactivate();
}

void CefImageConverter::convert(const Texture2D& fromCefOutput, ImageOutport& toInviwOutput,
                                const ImageInport* background) {
    if (background && background->isConnected()) {
        utilgl::activateTargetAndCopySource(toInviwOutput, *background);
    } else {
        utilgl::activateAndClearTarget(toInviwOutput, ImageType::ColorPicking);
    }
    utilgl::BlendModeState blendModeStateGL(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    shader_.activate();

    utilgl::setShaderUniforms(shader_, toInviwOutput, "outportParameters_");

    // bind input image
    TextureUnit texUnit;
    utilgl::bindTexture(fromCefOutput, texUnit);
    shader_.setUniform("inport_", texUnit);

    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
