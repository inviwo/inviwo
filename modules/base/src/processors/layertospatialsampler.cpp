/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <modules/base/processors/layertospatialsampler.h>
#include <inviwo/core/util/imagesampler.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerToSpatialSampler::processorInfo_{
    "org.inviwo.LayerToSpatialSampler",  // Class identifier
    "Layer To Spatial Sampler",          // Display name
    "Spatial Sampler",                   // Category
    CodeState::Experimental,             // Code state
    Tags::CPU,                           // Tags
    R"(Generates a spatial sampler for the given input layer.)"_unindentHelp};

const ProcessorInfo& LayerToSpatialSampler::getProcessorInfo() const { return processorInfo_; }

LayerToSpatialSampler::LayerToSpatialSampler()
    : Processor{}
    , layer_{"layer", "Input layer"_help}
    , sampler_{"sampler", "Sampler for the input layer"_help} {

    addPorts(layer_, sampler_);
}

void LayerToSpatialSampler::process() {
    auto sampler = std::make_shared<ImageSampler<dvec2>>(layer_.getData());
    sampler_.setData(sampler);
}

}  // namespace inviwo
