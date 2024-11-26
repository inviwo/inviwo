/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/basegl/processors/layerprocessing/layergradient.h>

#include <modules/opengl/shader/shaderutils.h>

#include <glm/gtx/component_wise.hpp>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerGradient::processorInfo_{
    "org.inviwo.LayerGradient",  // Class identifier
    "Layer Gradient",            // Display name
    "Layer Operation",           // Category
    CodeState::Stable,           // Code state
    Tags::GL | Tag{"Layer"},     // Tags
    R"(Computes the gradient of one channel of the input Layer.)"_unindentHelp,
};

const ProcessorInfo& LayerGradient::getProcessorInfo() const { return processorInfo_; }

LayerGradient::LayerGradient()
    : LayerGLProcessor{utilgl::findShaderResource("img_gradient.frag")}
    , channel_{"channel", "Channel", "Selected channel used for gradient calculations"_help,
               util::enumeratedOptions("Channel", 4)} {

    addProperties(channel_);
}

void LayerGradient::preProcess(TextureUnitContainer&, const Layer& input, Layer&) {
    const auto gradientSpacing{input.getWorldSpaceGradientSpacing()};
    shader_.setUniform("worldSpaceGradientSpacing", gradientSpacing);
    shader_.setUniform("textureSpaceGradientSpacing",
                       mat2{glm::scale(input.getCoordinateTransformer().getWorldToTextureMatrix(),
                                       vec3{gradientSpacing, 1.0f})});

    shader_.setUniform("channel", channel_.getSelectedValue());
}

LayerConfig LayerGradient::outputConfig(const Layer& input) const {
    const double gradientEstimate = glm::compMax(glm::abs(input.dataMap.dataRange)) /
                                    glm::compMax(input.getWorldSpaceGradientSpacing());
    return input.config().updateFrom({.format = DataFormatBase::get(DataFormatId::Vec2Float32),
                                      .swizzleMask = swizzlemasks::defaultData(2),
                                      .dataRange = dvec2{-gradientEstimate, gradientEstimate},
                                      .valueRange = dvec2{-gradientEstimate, gradientEstimate}});
}

}  // namespace inviwo
