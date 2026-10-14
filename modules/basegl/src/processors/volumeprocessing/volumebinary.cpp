/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2026 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumebinary.h>

#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/staticstring.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/shader/shaderutils.h>

#include <memory>
#include <type_traits>

namespace inviwo {
class TextureUnitContainer;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeBinary::processorInfo_{
    "org.inviwo.VolumeBinary",  // Class identifier
    "Volume Binary",            // Display name
    "Volume Operation",         // Category
    CodeState::Stable,          // Code state
    Tags::GL,                   // Tags
    R"(Computes a binary volume of the input volume using a threshold. The output
    will contain "0" for all values below the threshold and "1" otherwise.)"_unindentHelp,
};
const ProcessorInfo& VolumeBinary::getProcessorInfo() const { return processorInfo_; }

VolumeBinary::VolumeBinary()
    : VolumeGLProcessor("volume_binary.frag",
                        VolumeConfig{.formatId = DataFormatId::UInt8,
                                     .valueAxis = Axis{.name = "mask", .unit = Unit{}},
                                     .dataRange = vec2(0, 255),
                                     .valueRange = vec2(0, 255)})
    , dataRange_{"dataRange", "Input Range", *inport_, false, InvalidationLevel::Valid}
    , dataSpace_{"dataSpace",
                 "Data Space",
                 "Defines the data space of the threshold value."_help,
                 {{"data", "Data Range", DataMapper::Space::Data},
                  {"normalized", "Normalized Range", DataMapper::Space::Normalized},
                  {"value", "Value Range", DataMapper::Space::Value}},
                 1}
    , threshold_{"threshold", "Threshold",
                 util::ordinalSymmetricVector(0.5f, 1.0f)
                     .setInc(0.0001f)
                     .set("Threshold used for the binarization of the input volume"_help)}
    , op_{"operator",
          "Operator",
          {{"greaterthen", ">", Operator::GreaterThen},
           {"greaterthenorequal", ">=", Operator::GreaterThenOrEqual},
           {"lessthen", "<", Operator::LessThen},
           {"lessthenorequal", "<=", Operator::LessThenOrEqual},
           {"equal", "==", Operator::Equal},
           {"notequal", "!=", Operator::NotEqual}},
          0,
          InvalidationLevel::InvalidResources} {

    outport_.setHelp("Binary mask volume"_help);

    addProperties(dataRange_, dataSpace_, threshold_, op_);
}

void VolumeBinary::preProcess([[maybe_unused]] TextureUnitContainer& cont, Shader& shader,
                              [[maybe_unused]] VolumeConfig& config) {
    const auto threshold = [&]() {
        if (inport_.has_value() && inport_->hasData()) {
            const DataMapper& dm = inport_->getData()->dataMap;
            switch (dataSpace_.get()) {
                using enum DataMapper::Space;
                case Data: return static_cast<float>(dm.mapFromDataToNormalized(threshold_.get()));
                case Value:
                    return static_cast<float>(dm.mapFromValueToNormalized(threshold_.get()));
                case Normalized:
                default:         return threshold_.get();
            }
        }
        return threshold_.get();
    }();

    shader.setUniform(threshold_.getIdentifier(), threshold);
}

void VolumeBinary::initializeShader(Shader& shader) {
    shader.getFragmentShaderObject()->addShaderDefine("OP", op_.getSelectedDisplayName());
}

}  // namespace inviwo
