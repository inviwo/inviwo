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

#include <modules/basegl/processors/volumeprocessing/vectormagnitudeprocessor.h>

#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/base/algorithm/algorithmoptions.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>
#include <modules/opengl/shader/shader.h>

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <glm/vec4.hpp>

namespace inviwo {
class TextureUnitContainer;

const ProcessorInfo VectorMagnitudeProcessor::processorInfo_{
    "org.inviwo.VectorMagnitude",  // Class identifier
    "Vector Magnitude",            // Display name
    "Volume Operation",            // Category
    CodeState::Stable,             // Code state
    Tags::GL,                      // Tags
    "Calculates the magnitude of the input volume."_help,
};
const ProcessorInfo& VectorMagnitudeProcessor::getProcessorInfo() const { return processorInfo_; }

VectorMagnitudeProcessor::VectorMagnitudeProcessor()
    : VolumeGLProcessor("vectormagnitudeprocessor.frag",
                        VolumeConfig{.format = DataFloat32::get()}) {

    addProperties(calculateDataRange_, dataRange_);
    calculateDataRange_.set(true);
    calculateDataRange_.setCurrentStateAsDefault();

    outport_.setHelp("Gradient magnitude of the input volume"_help);
}

VectorMagnitudeProcessor::~VectorMagnitudeProcessor() = default;

void VectorMagnitudeProcessor::preProcess([[maybe_unused]] TextureUnitContainer& cont,
                                          Shader& shader, [[maybe_unused]] VolumeConfig& config) {
    int numChannels = 3;
    if (inport_ && inport_->hasData()) {
        numChannels = static_cast<int>(inport_->getData()->getDataFormat()->getComponents());
    }

    shader.setUniform("numInputChannels_", numChannels);
}

void VectorMagnitudeProcessor::postProcess(Volume& volume) {
    if (calculateDataRange_) {
        const auto range = volume.dataMap.dataRange;
        volume.dataMap.dataRange = dvec2(0.0, range.y);
        volume.dataMap.valueRange = dvec2(0.0, range.y);
    }
}

}  // namespace inviwo
