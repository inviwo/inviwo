/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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
    : ImageGLProcessor("img_gradient.frag")
    , channel_{"channel", "Channel", "Selected channel used for gradient calculations"_help,
               util::enumeratedOptions("Channel", 4)} {

    dataFormat_ = DataVec2Float32::get();
    swizzleMask_ = swizzlemasks::defaultData(2);

    addProperty(channel_);
}

void ImageGradient::preProcess(TextureUnitContainer&, Shader& shader) {
    const auto* layer = inport_.getData()->getColorLayer();
    shader.setUniform("inverseMetricTensor",
                      layer->getCoordinateTransformer().getInverseMetricTensor());
    shader.setUniform("channel", channel_.getSelectedValue());

    const double gradientEstimate = glm::compMax(glm::abs(layer->dataMap.dataRange)) /
                                    glm::compMax(layer->getWorldSpaceGradientSpacing());
    auto outputLayer = outport_.getEditableData()->getColorLayer();
    outputLayer->dataMap.dataRange = dvec2{-gradientEstimate, gradientEstimate};
    outputLayer->dataMap.valueRange = dvec2{-gradientEstimate, gradientEstimate};
}

}  // namespace inviwo
