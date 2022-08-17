/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/openglutils.h>

namespace inviwo {

ImageCompositor::ImageCompositor(std::string programFileName) : shader(programFileName) {}

void ImageCompositor::composite(const Image& source0, const Image& source1, Image& destination,
                                ImageType type) {

    IVW_ASSERT(&source0 != &destination, "source0 can not be same as destination");
    IVW_ASSERT(&source1 != &destination, "source1 can not be same as destination");

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
    utilgl::activateTarget(destination, type);
    shader.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader, units, source0, "tex0", ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader, units, source1, "tex1", ImageType::ColorDepthPicking);

    utilgl::setShaderUniforms(shader, destination, "outportParameters");
    utilgl::singleDrawImagePlaneRect();

    shader.deactivate();
    utilgl::deactivateCurrentTarget();
}

void ImageCompositor::composite(const ImageInport& source0, const ImageInport& source1,
                                ImageOutport& destination, ImageType type) {
    if (source0.isReady() && source1.isReady()) {
        if (!destination.hasEditableData()) {
            destination.setData(
                std::make_shared<Image>(destination.getDimensions(), destination.getDataFormat()));
        }
        composite(*source0.getData(), *source1.getData(), *destination.getEditableData(), type);
    }
}

}  // namespace inviwo
