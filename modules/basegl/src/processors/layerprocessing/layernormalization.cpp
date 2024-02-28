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

#include <modules/basegl/processors/layerprocessing/layernormalization.h>

#include <inviwo/core/util/formats.h>
#include <modules/base/algorithm/dataminmax.h>
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

const ProcessorInfo LayerNormalization::getProcessorInfo() const { return processorInfo_; }

namespace {

constexpr std::string_view fragmentShader = util::trim(R"(
#include "utils/structs.glsl"
#include "utils/sampler2d.glsl"

uniform sampler2D inport;
uniform ImageParameters inportParameters;
uniform ImageParameters outportParameters;

uniform vec4 minValue = vec4(0);
uniform vec4 maxValue = vec4(1);

uniform float minDataType = 0.0;
uniform float maxDataType = 1.0;

void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;
    vec4 value = texture(inport, texCoords) * (maxDataType - minDataType) + minDataType;

    FragData0 = (value - minValue) / (maxValue - minValue);
}
)");

}  // namespace

LayerNormalization::LayerNormalization()
    : LayerGLProcessor{std::make_shared<StringShaderResource>("LayerNormalization.frag",
                                                              fragmentShader)}
    , normalizeIndividually_{"normalizeIndividually", "Normalize Channels Individually",
                             "If true, each channel will be normalized on its own. "
                             "Otherwise the global min/max values are used for all channels."_help}
    , zeroCentered_{"zeroCentered", "Centered at Zero",
                    "Toggles normalization centered at zero to range [-max, max]"_help, false}
    , dataMin_{"dataMin", "Min Value",
               util::ordinalSymmetricVector(dvec4{0}, dvec4{100.0})
                   .set(PropertySemantics::Text)
                   .set(InvalidationLevel::Valid)
                   .set("Minimum value of the input layer (read-only)"_help)}
    , dataMax_{"dataMax", "Max Value",
               util::ordinalSymmetricVector(dvec4{0}, dvec4{100.0})
                   .set(PropertySemantics::Text)
                   .set(InvalidationLevel::Valid)
                   .set("Maximum value of the input layer (read-only)"_help)} {

    addProperties(normalizeIndividually_, zeroCentered_, dataMin_, dataMax_);
    dataMin_.setReadOnly(true);
    dataMax_.setReadOnly(true);
}

void LayerNormalization::preProcess(TextureUnitContainer&) {
    if (inport_.isChanged()) {
        auto minMax = util::layerMinMax(inport_.getData().get(), IgnoreSpecialValues::Yes);
        dataMin_.set(minMax.first);
        dataMax_.set(minMax.second);
    }

    dvec4 minValue{dataMin_.get()};
    dvec4 maxValue{dataMax_.get()};
    dvec2 valueRange{0.0, 1.0};

    if (zeroCentered_) {
        maxValue = glm::max(glm::abs(minValue), glm::abs(maxValue));
        minValue = -maxValue;
        valueRange = dvec2{-1.0, 1.0};
    }
    if (!normalizeIndividually_) {
        maxValue = dvec4{glm::compMax(maxValue)};
        minValue = dvec4{glm::compMin(minValue)};
    }

    shader_.setUniform("minValue", vec4{minValue});
    shader_.setUniform("maxValue", vec4{maxValue});

    auto dataformat = inport_.getData()->getDataFormat();

    float minDataType = 0.0f;
    float maxDataType = 1.0f;
    if (dataformat->getNumericType() != NumericType::Float) {
        minDataType = static_cast<float>(dataformat->getMin());
        maxDataType = static_cast<float>(dataformat->getMax());
    }

    if (dataformat->getNumericType() == NumericType::Float) {
        layer_->dataMap.dataRange = dvec2{0.0, 1.0};
    }
    layer_->dataMap.valueRange = valueRange;
}

LayerConfig LayerNormalization::outputConfig(const Layer& input) const {
    return input.config().updateFrom(
        {.dataRange = DataMapper::defaultDataRangeFor(input.getDataFormat()),
         .valueRange = dvec2(0.0, 1.0)});
}

void LayerNormalization::postProcess() {}

}  // namespace inviwo
