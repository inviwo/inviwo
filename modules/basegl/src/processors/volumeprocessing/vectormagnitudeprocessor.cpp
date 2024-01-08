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

#include <modules/basegl/processors/volumeprocessing/vectormagnitudeprocessor.h>

#include <inviwo/core/datastructures/datamapper.h>                         // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>            // for Representation...
#include <inviwo/core/datastructures/representationconverterfactory.h>     // for Representation...
#include <inviwo/core/datastructures/volume/volumeram.h>                   // for VolumeRAM
#include <inviwo/core/ports/volumeport.h>                                  // for VolumeInport
#include <inviwo/core/processors/processorinfo.h>                          // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                         // for CodeState, Cod...
#include <inviwo/core/processors/processortags.h>                          // for Tags, Tags::GL
#include <inviwo/core/util/formats.h>                                      // for DataFloat32
#include <inviwo/core/util/glmvec.h>                                       // for dvec2
#include <modules/base/algorithm/algorithmoptions.h>                       // for IgnoreSpecialV...
#include <modules/base/algorithm/dataminmax.h>                             // for volumeMinMax
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>  // for VolumeGLProcessor
#include <modules/opengl/shader/shader.h>                                  // for Shader

#include <algorithm>      // for max, min
#include <cstdlib>        // for abs
#include <memory>         // for shared_ptr
#include <string>         // for string
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair

#include <glm/vec4.hpp>  // for vec, vec<>::(a...

namespace inviwo {
class TextureUnitContainer;

const ProcessorInfo VectorMagnitudeProcessor::processorInfo_{
    "org.inviwo.VectorMagnitude",  // Class identifier
    "Vector Magnitude",            // Display name
    "Volume Operation",            // Category
    CodeState::Stable,             // Code state
    Tags::GL,                      // Tags
};
const ProcessorInfo VectorMagnitudeProcessor::getProcessorInfo() const { return processorInfo_; }

VectorMagnitudeProcessor::VectorMagnitudeProcessor()
    : VolumeGLProcessor("vectormagnitudeprocessor.frag") {
    this->dataFormat_ = DataFloat32::get();
}

VectorMagnitudeProcessor::~VectorMagnitudeProcessor() = default;

void VectorMagnitudeProcessor::preProcess(TextureUnitContainer&) {
    int numChannels = 3;
    if (inport_.hasData()) {
        numChannels = static_cast<int>(inport_.getData()->getDataFormat()->getComponents());
    }

    shader_.setUniform("numInputChannels_", numChannels);
}

void VectorMagnitudeProcessor::postProcess() {

    auto minMax =
        util::volumeMinMax(volume_->getRepresentation<VolumeRAM>(), IgnoreSpecialValues::Yes);
    double minV, maxV;
    minV = minMax.first.x;
    maxV = minMax.second.x;
    for (int i = 0; i < 3; i++) {
        minV = std::min(minV, minMax.first[i]);
        maxV = std::max(maxV, minMax.second[i]);
    }

    volume_->dataMap_.dataRange = dvec2(0, std::abs(maxV));
    volume_->dataMap_.valueRange = dvec2(minV, maxV);
}

}  // namespace inviwo
