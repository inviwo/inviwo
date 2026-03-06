/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagenormalizationprocessor.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>
#include <modules/base/algorithm/algorithmoptions.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <modules/opengl/shader/shader.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <fmt/core.h>
#include <fmt/format.h>
#include <glm/common.hpp>
#include <glm/gtx/io.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace inviwo {
class TextureUnitContainer;

const ProcessorInfo ImageNormalizationProcessor::processorInfo_{
    "org.inviwo.ImageNormalization",  // Class identifier
    "Image Normalization",            // Display name
    "Image Operation",                // Category
    CodeState::Stable,                // Code state
    Tags::GL,                         // Tags
    R"(
Normalizes the RGB channels of the input image given a specific range.
)"_unindentHelp};
const ProcessorInfo& ImageNormalizationProcessor::getProcessorInfo() const {
    return processorInfo_;
}

ImageNormalizationProcessor::ImageNormalizationProcessor()
    : ImageGLProcessor("img_normalize.frag")
    , normalizeSeparately_("normalizeSeparately", "Normalize Channels Separately",
                           "If true, each channel will be normalized on its own. "
                           "Otherwise the global min/max values are used for all channels."_help)
    , signNormalized_("signNormalized", "Sign Normalized",
                      "Toggles normalization centered at zero to range [-max, max]"_help, false)
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

    addProperties(normalizeSeparately_, signNormalized_, dataMin_, dataMax_);
    dataMin_.setReadOnly(true);
    dataMax_.setReadOnly(true);
}

void ImageNormalizationProcessor::preProcess(TextureUnitContainer&, Shader& shader) {
    if (inport_.isChanged()) {
        auto minMax =
            util::layerMinMax(inport_.getData()->getColorLayer(), IgnoreSpecialValues::Yes);
        dataMin_.set(minMax.first);
        dataMax_.set(minMax.second);
    }

    // never normalize alpha
    dvec4 minValue{dvec3{dataMin_.get()}, 0.0};
    dvec4 maxValue{dvec3{dataMax_.get()}, 1.0};
    dvec2 valueRange{0.0, 1.0};

    if (signNormalized_) {
        maxValue = glm::max(glm::abs(minValue), glm::abs(maxValue));
        minValue = -maxValue;
        valueRange = dvec2{-1.0, 1.0};
    }
    if (!normalizeSeparately_) {
        maxValue = dvec4{glm::compMax(maxValue)};
        minValue = dvec4{glm::compMin(minValue)};
    }

    float minDataType = 0.0f;
    float maxDataType = 1.0f;
    if (auto dataformat = inport_.getData()->getColorLayer()->getDataFormat();
        dataformat->getNumericType() != NumericType::Float) {
        minDataType = static_cast<float>(dataformat->getMin());
        maxDataType = static_cast<float>(dataformat->getMax());
    }

    shader.setUniform("minValue", vec4{minValue});
    shader.setUniform("maxValue", vec4{maxValue});
    shader.setUniform("minDataType", minDataType);
    shader.setUniform("maxDataType", maxDataType);

    auto layer = outport_.getEditableData()->getColorLayer();
    dvec2 dataRange = DataMapper::defaultDataRangeFor(layer->getDataFormat());
    if (layer->getDataFormat()->getNumericType() == NumericType::Float && signNormalized_) {
        dataRange = dvec2{-1.0, 1.0};
    }
    layer->dataMap.dataRange = dataRange;
    layer->dataMap.valueRange = valueRange;
}

}  // namespace inviwo
