/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerInformation::processorInfo_{
    "org.inviwo.LayerInformation",                  // Class identifier
    "Layer Information",                            // Display name
    "Information",                                  // Category
    CodeState::Stable,                              // Code state
    Tags::CPU | Tag{"Layer"} | Tag{"Information"},  // Tags
    R"(Shows available information provided by the input layer.)"_unindentHelp};

const ProcessorInfo LayerInformation::getProcessorInfo() const { return processorInfo_; }

LayerInformation::LayerInformation()
    : Processor{}
    , layer_("layer", "Input layer"_help)
    , layerInfo_("dataInformation", "Data Information") {

    addPort(layer_);
    addProperties(layerInfo_);

    layerInfo_.setSerializationMode(PropertySerializationMode::None);

    setAllPropertiesCurrentStateAsDefault();
}

void LayerInformation::process() {
    auto layer = layer_.getData();

    layerInfo_.updateFromLayer(*layer);
}

}  // namespace inviwo
