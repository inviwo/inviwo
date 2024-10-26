/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2024 Inviwo Foundation
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

#include <modules/base/processors/layerinformation.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/util/glm.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerInformation::processorInfo_{
    "org.inviwo.LayerInformation",                  // Class identifier
    "Layer Information",                            // Display name
    "Information",                                  // Category
    CodeState::Stable,                              // Code state
    Tags::CPU | Tag{"Layer"} | Tag{"Information"},  // Tags
    R"(Shows available information provided by the input layer.)"_unindentHelp};

const ProcessorInfo& LayerInformation::getProcessorInfo() const { return processorInfo_; }

LayerInformation::LayerInformation()
    : Processor{}
    , layer_("layer", "Input layer"_help)
    , layerInfo_("dataInformation", "Data Information")
    , transformations_("transformations", "Transformations")
    , modelTransform_("modelTransform_", "Model Transform", mat4(1.0f),
                      util::filled<mat3>(std::numeric_limits<float>::lowest()),
                      util::filled<mat3>(std::numeric_limits<float>::max()),
                      util::filled<mat3>(0.001f), InvalidationLevel::Valid)
    , worldTransform_("worldTransform_", "World Transform", mat4(1.0f),
                      util::filled<mat3>(std::numeric_limits<float>::lowest()),
                      util::filled<mat3>(std::numeric_limits<float>::max()),
                      util::filled<mat3>(0.001f), InvalidationLevel::Valid)
    , basis_("basis", "Basis", mat3(1.0f), util::filled<mat3>(std::numeric_limits<float>::lowest()),
             util::filled<mat3>(std::numeric_limits<float>::max()), util::filled<mat3>(0.001f),
             InvalidationLevel::Valid)
    , offset_("offset", "Offset", vec3(0.0f), vec3(std::numeric_limits<float>::lowest()),
              vec3(std::numeric_limits<float>::max()), vec3(0.001f), InvalidationLevel::Valid,
              PropertySemantics::Text) {

    addPort(layer_);

    transformations_.addProperties(modelTransform_, worldTransform_, basis_, offset_);
    transformations_.setCollapsed(true);
    transformations_.setReadOnly(true);

    addProperties(layerInfo_, transformations_);

    layerInfo_.setReadOnly(true);
    layerInfo_.setSerializationMode(PropertySerializationMode::None);

    setAllPropertiesCurrentStateAsDefault();
}

void LayerInformation::process() {
    auto layer = layer_.getData();

    layerInfo_.updateForNewLayer(*layer, util::OverwriteState::Yes);

    util::updateDefaultState(modelTransform_, layer->getModelMatrix(), util::OverwriteState::Yes);
    util::updateDefaultState(worldTransform_, layer->getWorldMatrix(), util::OverwriteState::Yes);
    util::updateDefaultState(basis_, layer->getBasis(), util::OverwriteState::Yes);
    util::updateDefaultState(offset_, layer->getOffset(), util::OverwriteState::Yes);
}

}  // namespace inviwo
