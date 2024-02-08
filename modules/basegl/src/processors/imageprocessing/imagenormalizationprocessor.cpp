/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/layer.h>                      // for Layer
#include <inviwo/core/datastructures/image/layerram.h>                   // for LayerRAM
#include <inviwo/core/datastructures/representationconverter.h>          // for RepresentationCo...
#include <inviwo/core/datastructures/representationconverterfactory.h>   // for RepresentationCo...
#include <inviwo/core/ports/imageport.h>                                 // for ImageInport, Ima...
#include <inviwo/core/processors/processorinfo.h>                        // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                       // for CodeState, CodeS...
#include <inviwo/core/processors/processortags.h>                        // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>                         // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>                    // for InvalidationLevel
#include <inviwo/core/properties/stringproperty.h>                       // for StringProperty
#include <inviwo/core/util/formats.h>                                    // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                     // for dvec3, dvec4, vec4
#include <inviwo/core/util/logcentral.h>                                 // for LogCentral, LogWarn
#include <modules/base/algorithm/algorithmoptions.h>                     // for IgnoreSpecialValues
#include <modules/base/algorithm/dataminmax.h>                           // for layerMinMax
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>  // for ImageGLProcessor
#include <modules/opengl/shader/shader.h>                                // for Shader

#include <algorithm>      // for max, min
#include <memory>         // for shared_ptr, uniq...
#include <string>         // for string
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair

#include <fmt/core.h>      // for basic_string_view
#include <fmt/format.h>    // for to_string, forma...
#include <glm/common.hpp>  // for abs, max
#include <glm/gtx/io.hpp>  // for operator<<
#include <glm/vec3.hpp>    // for vec<>::(anonymous)
#include <glm/vec4.hpp>    // for vec, vec<>::(ano...

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
const ProcessorInfo ImageNormalizationProcessor::getProcessorInfo() const { return processorInfo_; }

ImageNormalizationProcessor::ImageNormalizationProcessor()
    : ImageGLProcessor("img_normalize.frag")
    , normalizeSeparately_("normalizeSeparately", "Normalize Channels Separately",
                           "If true, each channel will be normalized on its own. "
                           "Otherwise the global min/max values are used for all channels."_help)
    , zeroCentered_("zeroCentered", "Centered at Zero",
                    "Toggles normalization centered at zero to range [-max, max]"_help, false)
    , minS_("min", "Min Value", "Min value of the input image (read-only)"_help, "")
    , maxS_("max", "Max Value", "Max value of the input image (read-only)"_help, "")
    , min_(0.0)
    , max_(1.0) {
    minS_.setInvalidationLevel(InvalidationLevel::Valid);
    maxS_.setInvalidationLevel(InvalidationLevel::Valid);
    minS_.setReadOnly(true);
    maxS_.setReadOnly(true);

    addProperty(normalizeSeparately_);
    addProperty(zeroCentered_);
    addProperty(minS_);
    addProperty(maxS_);

    setAllPropertiesCurrentStateAsDefault();
}

ImageNormalizationProcessor::~ImageNormalizationProcessor() = default;

void ImageNormalizationProcessor::preProcess(TextureUnitContainer&) {
    if (inport_.isChanged() || normalizeSeparately_.isModified()) {
        updateMinMax();
    }

    dvec3 min{min_};
    dvec3 max{max_};

    if (zeroCentered_) {
        max = glm::max(glm::abs(min), glm::abs(max));
        min = -max;
    }

    auto df = inport_.getData()->getColorLayer()->getRepresentation<LayerRAM>()->getDataFormat();
    double typeMax = 1.0;
    double typeMin = 0.0;
    if (df->getNumericType() != NumericType::Float) {
        typeMax = df->getMax();
        typeMin = df->getMin();
    }

    shader_.setUniform("typeMax_", static_cast<float>(typeMax));
    shader_.setUniform("typeMin_", static_cast<float>(typeMin));

    if (normalizeSeparately_.get()) {
        shader_.setUniform("min_", static_cast<vec4>(dvec4(min, 0.0)));
        shader_.setUniform("max_", static_cast<vec4>(dvec4(max, 1.0)));
    } else {
        double minV = std::min(std::min(min.x, min.y), min.z);
        double maxV = std::max(std::max(max.x, max.y), max.z);
        shader_.setUniform("min_", vec4(minV, minV, minV, 0.0f));
        shader_.setUniform("max_", vec4(maxV, maxV, maxV, 1.0f));
    }
}

void ImageNormalizationProcessor::updateMinMax() {
    if (!inport_.hasData()) return;

    const LayerRAM* img = inport_.getData()->getColorLayer()->getRepresentation<LayerRAM>();

    auto df = img->getDataFormat();
    if (df->getNumericType() == NumericType::SignedInteger ||
        df->getNumericType() == NumericType::UnsignedInteger) {
        if (df->getSizeInBytes() > 8) {
            LogWarn(
                "Image Normalization only works on float data or Integer data with 8 or 16 bit "
                "precision, got:  "
                << df->getSizeInBytes() * 8);
        }
    }

    auto minMax = util::layerMinMax(img, IgnoreSpecialValues::Yes);

    min_ = minMax.first;
    max_ = minMax.second;

    minS_.set(fmt::to_string(minMax.first));
    maxS_.set(fmt::to_string(minMax.second));

    min_.a = 0.0;  // never normalize alpha
    max_.a = 1.0;
}

}  // namespace inviwo
