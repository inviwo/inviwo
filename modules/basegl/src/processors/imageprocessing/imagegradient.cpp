/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagegradient.h>

#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/opengl/shader/shader.h>

#include <glm/gtx/component_wise.hpp>

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace inviwo {
class TextureUnitContainer;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageGradient::processorInfo_{
    "org.inviwo.ImageGradient",  // Class identifier
    "Image Gradient",            // Display name
    "Image Operation",           // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
    R"(Computes the gradient of one channel of the input image.)"_unindentHelp,
};
const ProcessorInfo& ImageGradient::getProcessorInfo() const { return processorInfo_; }

ImageGradient::ImageGradient()
    : ImageGLProcessor("img_gradient.frag"), channel_("channel", "Channel") {

    dataFormat_ = DataVec2Float32::get();
    swizzleMask_ = {
        {ImageChannel::Red, ImageChannel::Green, ImageChannel::Zero, ImageChannel::One}};

    channel_.addOption("Channel 1", "Channel 1", 0);
    channel_.setCurrentStateAsDefault();

    inport_.onChange([&]() {
        if (inport_.hasData()) {
            int channels = static_cast<int>(inport_.getData()->getDataFormat()->getComponents());
            if (channels == static_cast<int>(channel_.size())) return;
            channel_.clearOptions();
            for (int i = 0; i < channels; i++) {
                std::stringstream ss;
                ss << "Channel " << i;
                channel_.addOption(ss.str(), ss.str(), i);
            }
            channel_.setCurrentStateAsDefault();
        }
    });

    addProperty(channel_);
}

void ImageGradient::preProcess(TextureUnitContainer&) {
    const auto layer = inport_.getData()->getColorLayer();
    const auto gradientSpacing{layer->getWorldSpaceGradientSpacing()};
    shader_.setUniform("worldSpaceGradientSpacing", gradientSpacing);
    shader_.setUniform("textureSpaceGradientSpacing",
                       mat2{glm::scale(layer->getCoordinateTransformer().getWorldToTextureMatrix(),
                                       vec3{gradientSpacing, 1.0f})});

    shader_.setUniform("channel", channel_.getSelectedValue());

    const double gradientEstimate = glm::compMax(glm::abs(layer->dataMap.dataRange)) /
                                    glm::compMax(layer->getWorldSpaceGradientSpacing());
    auto outputLayer = outport_.getEditableData()->getColorLayer();
    outputLayer->dataMap.dataRange = dvec2{-gradientEstimate, gradientEstimate};
    outputLayer->dataMap.valueRange = dvec2{-gradientEstimate, gradientEstimate};
}

}  // namespace inviwo
