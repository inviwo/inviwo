/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/basegl/processors/splitimage.h>

#include <modules/opengl/openglutils.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/image/imagegl.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SplitImage::processorInfo_{
    "org.inviwo.SplitImage",  // Class identifier
    "Split Image",            // Display name
    "Image Operation",        // Category
    CodeState::Stable,        // Code state
    Tags::GL,                 // Tags
};
const ProcessorInfo SplitImage::getProcessorInfo() const { return processorInfo_; }

SplitImage::SplitImage()
    : Processor()
    , inport0_("inputA")
    , inport1_("inputB")
    , outport_("outport")
    , splitDirection_("splitDirection", "Split Direction",
                      {{"vertical", "Vertical", SplitDirection::Vertical},
                       {"horizontal", "Horizontal", SplitDirection::Horizontal}},
                      0)
    , splitPosition_("splitPosition", "Split Position", 0.5f, 0.0f, 1.0f) {

    inport0_.setOptional(true);
    inport1_.setOptional(true);
    addPort(inport0_);
    addPort(inport1_);
    addPort(outport_);

    addProperty(splitDirection_);
    addProperty(splitPosition_);
}

void SplitImage::process() {
    utilgl::activateAndClearTarget(outport_);

    if (!inport0_.hasData() && !inport1_.hasData()) {
        // no inputs, render noise

        auto shader = SharedOpenGLResources::getPtr()->getNoiseShader();
        shader->activate();
        utilgl::singleDrawImagePlaneRect();
        shader->deactivate();
    } else {
        utilgl::GlBoolState scissor(GL_SCISSOR_TEST, GL_TRUE);

        const ivec2 dims(outport_.getDimensions());

        ivec4 viewport0;
        ivec4 viewport1;
        if (splitDirection_.get() == SplitDirection::Vertical) {
            int width = static_cast<int>(splitPosition_.get() * dims.x);
            viewport0 = ivec4(0, 0, width, dims.y);
            viewport1 = ivec4(width, 0, dims.x - width, dims.y);
        } else {
            int height = static_cast<int>(splitPosition_.get() * dims.y);
            viewport0 = ivec4(0, dims.y - height, dims.x, height);
            viewport1 = ivec4(0, 0, dims.x, dims.y - height);
        }

        auto renderPort = [&](const ImageInport &inport, const auto &viewport) {
            utilgl::ScissorState scissor(viewport);

            TextureUnit colorUnit, depthUnit, pickingUnit;
            TextureUnitContainer additionalColorUnits;
            Shader *shader = nullptr;
            if (inport.hasData()) {
                shader = SharedOpenGLResources::getPtr()->getImageCopyShader(
                    inport.getData()->getNumberOfColorLayers());
                shader->activate();
                shader->setUniform("dataToClip", mat4(1.0f));

                auto imageGL = inport.getData()->getRepresentation<ImageGL>();
                imageGL->getColorLayerGL()->bindTexture(colorUnit.getEnum());
                shader->setUniform("color_", colorUnit.getUnitNumber());
                if (imageGL->getDepthLayerGL()) {
                    imageGL->getDepthLayerGL()->bindTexture(depthUnit.getEnum());
                    shader->setUniform("depth_", depthUnit.getUnitNumber());
                }
                if (imageGL->getPickingLayerGL()) {
                    imageGL->getPickingLayerGL()->bindTexture(pickingUnit.getEnum());
                    shader->setUniform("picking_", pickingUnit.getUnitNumber());
                }
                for (size_t i = 1; i < imageGL->getNumberOfColorLayers(); ++i) {
                    TextureUnit unit;
                    imageGL->getColorLayerGL(i)->bindTexture(unit.getEnum());
                    shader->setUniform("color" + toString<size_t>(i), unit.getUnitNumber());
                    additionalColorUnits.push_back(std::move(unit));
                }
            } else {
                shader = SharedOpenGLResources::getPtr()->getNoiseShader();
            }

            shader->activate();
            utilgl::singleDrawImagePlaneRect();
            shader->deactivate();
        };

        renderPort(inport0_, viewport0);
        renderPort(inport1_, viewport1);
    }
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
