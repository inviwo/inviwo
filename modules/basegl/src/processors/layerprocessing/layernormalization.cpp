/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2026 Inviwo Foundation
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

#include <modules/basegl/processors/layerprocessing/layernormalization.h>

#include <inviwo/core/util/formats.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerNormalization::processorInfo_{
    "org.inviwo.LayerNormalization",  // Class identifier
    "Layer Normalization",            // Display name
    "Layer Operation",                // Category
    CodeState::Stable,                // Code state
    Tags::GL | Tag{"Layer"},          // Tags
    R"(Normalize the input layer either per channel or globally.)"_unindentHelp};

const ProcessorInfo& LayerNormalization::getProcessorInfo() const { return processorInfo_; }

LayerNormalization::LayerNormalization()
    : LayerGLProcessor{utilgl::findShaderResource("img_normalize.frag")}
    , normalizeIndividually_{"normalizeIndividually", "Normalize Channels Individually",
                             "If true, each channel will be normalized on its own. "
                             "Otherwise the global min/max values are used for all channels."_help}
    , signNormalized_{"signNormalized", "Sign Normalized",
                      "Toggles normalization centered at zero to range [-max, max]"_help, false}
    , dataMin_{"dataMin", "Min Value",
               util::ordinalSymmetricVector(dvec4{0}, dvec4{100.0})
                   .setInc(dvec4{0.0001})
                   .set(PropertySemantics::Text)
                   .set(InvalidationLevel::Valid)
                   .set("Minimum value of the input layer (read-only)"_help)}
    , dataMax_{"dataMax", "Max Value",
               util::ordinalSymmetricVector(dvec4{0}, dvec4{100.0})
                   .setInc(dvec4{0.0001})
                   .set(PropertySemantics::Text)
                   .set(InvalidationLevel::Valid)
                   .set("Maximum value of the input layer (read-only)"_help)} {

    addProperties(enabled_, normalizeIndividually_, signNormalized_, dataMin_, dataMax_);
    dataMin_.setReadOnly(true);
    dataMax_.setReadOnly(true);
}

void LayerNormalization::preProcess(TextureUnitContainer&, Shader& shader, const Layer& input,
                                    Layer&) {
    if (inport_.isChanged()) {
        const auto [min, max] = dataMinMaxGL_.minMax(input);
        dataMin_.set(min);
        dataMax_.set(max);
        // reactivate shader since DataMinMaxGL uses its own shaders
        shader.activate();
    }

    dvec4 minValue{dataMin_.get()};
    dvec4 maxValue{dataMax_.get()};

    if (signNormalized_) {
        maxValue = glm::max(glm::abs(minValue), glm::abs(maxValue));
        minValue = -maxValue;
    }
    if (!normalizeIndividually_) {
        maxValue = dvec4{glm::compMax(maxValue)};
        minValue = dvec4{glm::compMin(minValue)};
    }

    float minDataType = 0.0f;
    float maxDataType = 1.0f;
    const auto* dataformat = input.getDataFormat();
    if (dataformat->getNumericType() != NumericType::Float) {
        minDataType = static_cast<float>(dataformat->getMin());
        maxDataType = static_cast<float>(dataformat->getMax());
    }
    float resultOffset = 0.0f;
    float resultScaling = 1.0f;
    if (signNormalized_ && (dataformat->getNumericType() == NumericType::SignedInteger ||
                            dataformat->getNumericType() == NumericType::Float)) {
        resultOffset = -1.0f;
        resultScaling = 2.0f;
    }

    shader.setUniform("minValue", vec4{minValue});
    shader.setUniform("maxValue", vec4{maxValue});
    shader.setUniform("minDataType", minDataType);
    shader.setUniform("maxDataType", maxDataType);
    shader.setUniform("resultOffset", resultOffset);
    shader.setUniform("resultScaling", resultScaling);
}

LayerConfig LayerNormalization::outputConfig(const Layer& input) const {
    dvec2 dataRange = DataMapper::defaultDataRangeFor(input.getDataFormat());
    dvec2 valueRange = signNormalized_ ? dvec2{-1.0, 1.0} : dvec2{0.0, 1.0};
    if (input.getDataFormat()->getNumericType() == NumericType::Float && signNormalized_) {
        dataRange = dvec2{-1.0, 1.0};
    }

    return input.config().updateFrom({
        .dataRange = dataRange,
        .valueRange = valueRange,
    });
}

}  // namespace inviwo
