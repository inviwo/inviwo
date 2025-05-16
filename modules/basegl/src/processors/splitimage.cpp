/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/camera/camera.h>        // for mat4
#include <inviwo/core/ports/imageport.h>                     // for ImageInport, BaseImageInport
#include <inviwo/core/processors/processor.h>                // for Processor
#include <inviwo/core/processors/processorinfo.h>            // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>           // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>            // for Tags, Tags::GL
#include <inviwo/core/properties/invalidationlevel.h>        // for InvalidationLevel, Invalidat...
#include <inviwo/core/properties/optionproperty.h>           // for OptionPropertyOption, Option...
#include <inviwo/core/properties/ordinalproperty.h>          // for FloatProperty
#include <inviwo/core/util/glmvec.h>                         // for ivec2, ivec4
#include <inviwo/core/util/staticstring.h>                   // for operator+
#include <inviwo/core/util/stringconversion.h>               // for toString
#include <modules/basegl/datastructures/splittersettings.h>  // for Direction, Direction::Vertical
#include <modules/basegl/properties/splitterproperty.h>      // for SplitterProperty
#include <modules/basegl/rendering/splitterrenderer.h>       // for SplitterRenderer
#include <modules/opengl/image/imagegl.h>                    // for ImageGL
#include <modules/opengl/image/layergl.h>                    // for LayerGL
#include <modules/opengl/inviwoopengl.h>                     // for glDisable, glEnable, GL_SCIS...
#include <modules/opengl/openglutils.h>                      // for ScissorState
#include <modules/opengl/shader/shader.h>                    // for Shader
#include <modules/opengl/shader/shaderutils.h>               // for ImageInport
#include <modules/opengl/sharedopenglresources.h>            // for SharedOpenGLResources
#include <modules/opengl/texture/textureunit.h>              // for TextureUnit, TextureUnitCont...
#include <modules/opengl/texture/textureutils.h>             // for singleDrawImagePlaneRect

#include <cstddef>      // for size_t
#include <memory>       // for shared_ptr, shared_ptr<>::el...
#include <type_traits>  // for remove_extent_t
#include <utility>      // for move

#include <glm/vec2.hpp>  // for vec<>::(anonymous)

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SplitImage::processorInfo_{
    "org.inviwo.SplitImage",  // Class identifier
    "Split Image",            // Display name
    "Image Operation",        // Category
    CodeState::Stable,        // Code state
    Tags::GL,                 // Tags
    "Split screen of two input images. "
    "The images are split in the middle either horizontally or vertically."_help,
};
const ProcessorInfo& SplitImage::getProcessorInfo() const { return processorInfo_; }

SplitImage::SplitImage()
    : Processor()
    , inport0_("inputA", "first image (left/top)"_help)
    , inport1_("inputB", "second image (right/bottom)"_help)
    , outport_("outport", "resulting image where the two input images are split in the middle"_help)
    , splitDirection_("splitDirection", "Split Direction",
                      "split direction, i.e. either vertical or horizontal"_help,
                      {{"vertical", "Vertical", splitter::Direction::Vertical},
                       {"horizontal", "Horizontal", splitter::Direction::Horizontal}},
                      0)
    , splitPosition_("splitPosition", "Split Position", "normalized split position [0,1]"_help,
                     0.5f, {0.0f, ConstraintBehavior::Immutable},
                     {1.0f, ConstraintBehavior::Immutable})
    , splitterSettings_("handlebarWidget", "Handle Bar", true, splitter::Style::Handle)
    , renderer_(this) {

    inport0_.setOptional(true);
    inport1_.setOptional(true);
    addPort(inport0_);
    addPort(inport1_);
    addPort(outport_);

    addProperties(splitDirection_, splitPosition_, splitterSettings_);

    renderer_.setInvalidateAction([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    renderer_.setDragAction([this](float pos, int) { splitPosition_.set(pos); });
}

void SplitImage::process() {
    utilgl::activateAndClearTarget(outport_);

    if (!inport0_.isReady() && !inport1_.isReady()) {
        // no inputs, render noise

        auto shader = SharedOpenGLResources::getPtr()->getNoiseShader();
        shader->activate();
        utilgl::singleDrawImagePlaneRect();
        shader->deactivate();
    } else {
        const ivec2 dims(outport_.getDimensions());

        ivec4 viewport0;
        ivec4 viewport1;
        if (splitDirection_ == splitter::Direction::Vertical) {
            int width = static_cast<int>(splitPosition_ * dims.x);
            viewport0 = ivec4(0, 0, width, dims.y);
            viewport1 = ivec4(width, 0, dims.x - width, dims.y);
        } else {
            int height = static_cast<int>(splitPosition_ * dims.y);
            viewport0 = ivec4(0, dims.y - height, dims.x, height);
            viewport1 = ivec4(0, 0, dims.x, dims.y - height);
        }

        auto renderPort = [&](const ImageInport& inport, const auto& viewport) {
            TextureUnit colorUnit, depthUnit, pickingUnit;
            TextureUnitContainer additionalColorUnits;
            Shader* shader = nullptr;
            if (inport.isReady()) {
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

            glEnable(GL_SCISSOR_TEST);
            utilgl::ScissorState scissorviewport(viewport);

            shader->activate();
            utilgl::singleDrawImagePlaneRect();
            shader->deactivate();

            glDisable(GL_SCISSOR_TEST);
        };

        renderPort(inport0_, viewport0);
        renderPort(inport1_, viewport1);
    }

    renderer_.render(splitterSettings_, splitDirection_, {splitPosition_},
                     outport_.getDimensions());

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
