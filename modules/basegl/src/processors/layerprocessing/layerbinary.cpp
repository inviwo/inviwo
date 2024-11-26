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

#include <modules/basegl/processors/layerprocessing/layerbinary.h>

#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerBinary::processorInfo_{
    "org.inviwo.LayerBinary",  // Class identifier
    "Layer Binary",            // Display name
    "Layer Operation",         // Category
    CodeState::Stable,         // Code state
    Tags::GL | Tag{"Layer"},   // Tags
    R"(Computes a binary image of the input layer using a threshold. The output will
       contain "0" for all values below the threshold and "1" otherwise.)"_unindentHelp};

const ProcessorInfo& LayerBinary::getProcessorInfo() const { return processorInfo_; }

LayerBinary::LayerBinary()
    : LayerGLProcessor{utilgl::findShaderResource("img_binary.frag")}
    , channel_{"channel", "Channel", "Selected channel used for binarization"_help,
               util::enumeratedOptions("Channel", 4)}
    , threshold_{"threshold", "Threshold",
                 util::ordinalSymmetricVector(0.5f, 1.0f)
                     .set("Threshold used for binarization of the input layer"_help)}
    , comparison_{"comparison",
                  "Comparison",
                  "Comparison function for threshold"_help,
                  {{"greater", "Greater", ">"},
                   {"greaterEqual", "Greater Equal", ">="},
                   {"less", "Less", "<"},
                   {"lessEqual", "Less Equal", "<="}},
                  0,
                  InvalidationLevel::InvalidResources} {
    addProperties(channel_, threshold_, comparison_);
}

void LayerBinary::initializeResources() {
    shader_.getFragmentShaderObject()->addShaderDefine("COMPARE", comparison_.getSelectedValue());
    LayerGLProcessor::initializeResources();
}

void LayerBinary::preProcess(TextureUnitContainer&, const Layer&, Layer&) {
    utilgl::setUniforms(shader_, channel_, threshold_);
}

LayerConfig LayerBinary::outputConfig(const Layer& input) const {
    auto format = input.getDataFormat();
    return input.config().updateFrom(
        {.format = DataFormatBase::get(format->getNumericType(), 1, format->getPrecision()),
         .swizzleMask = swizzlemasks::defaultData(1),
         .dataRange = DataMapper::defaultDataRangeFor(input.getDataFormat()),
         .valueRange = dvec2{0.0, 1.0}});
}

}  // namespace inviwo
