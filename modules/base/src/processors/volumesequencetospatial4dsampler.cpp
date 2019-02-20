/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/base/processors/volumesequencetospatial4dsampler.h>
#include <inviwo/core/util/volumesequencesampler.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeSequenceToSpatial4DSampler::processorInfo_{
    "org.inviwo.VolumeSequenceToSpatial4DSampler",  // Class identifier
    "Volume Sequence To Spatial 4D Sampler",        // Display name
    "Spatial Sampler",                              // Category
    CodeState::Experimental,                        // Code state
    Tags::None,                                     // Tags
};
const ProcessorInfo VolumeSequenceToSpatial4DSampler::getProcessorInfo() const {
    return processorInfo_;
}

VolumeSequenceToSpatial4DSampler::VolumeSequenceToSpatial4DSampler()
    : Processor()
    , volumeSequence_("volumeSequence")
    , sampler_("sampler")
    , allowLooping_("allowLooping", "Allow Looping", true) {
    addPort(volumeSequence_);
    addPort(sampler_);

    addProperty(allowLooping_);
}

void VolumeSequenceToSpatial4DSampler::process() {
    auto sampler =
        std::make_shared<VolumeSequenceSampler>(volumeSequence_.getData(), allowLooping_.get());
    sampler_.setData(sampler);
}

}  // namespace inviwo
