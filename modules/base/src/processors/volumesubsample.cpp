/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/base/processors/volumesubsample.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo {

const ProcessorInfo VolumeSubsample::processorInfo_{
    "org.inviwo.VolumeSubsample",  // Class identifier
    "Volume Subsample",            // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,       // Code state
    Tags::CPU,                     // Tags
};
const ProcessorInfo VolumeSubsample::getProcessorInfo() const { return processorInfo_; }

VolumeSubsample::VolumeSubsample()
    : PoolProcessor()
    , inport_("inputVolume")
    , outport_("outputVolume")
    , enabled_("enabled", "Enable Operation", true)
    , subSampleFactors_("subSampleFactors", "Factors", ivec3(1), ivec3(1), ivec3(8)) {

    addPort(inport_);
    addPort(outport_);

    addProperty(enabled_);
    addProperty(subSampleFactors_);
}

void VolumeSubsample::process() {
    const size3_t factors =
        glm::min(static_cast<size3_t>(glm::max(subSampleFactors_.get(), ivec3(1))),
                 inport_.getData()->getDimensions());

    if (enabled_ && factors != size3_t(1, 1, 1)) {
        outport_.clear();
        dispatchOne([volume = inport_.getData(),
                     f = subSampleFactors_.get()]() { return subsample(volume, f); },
                    [this](std::shared_ptr<Volume> result) {
                        outport_.setData(result);
                        newResults();
                    });
    } else {
        outport_.setData(inport_.getData());
    }
}

std::shared_ptr<Volume> VolumeSubsample::subsample(std::shared_ptr<const Volume> volume,
                                                   size3_t f) {
    auto vol = volume->getRepresentation<VolumeRAM>();
    auto sample = std::make_shared<Volume>(util::volumeSubSample(vol, f));
    sample->copyMetaDataFrom(*volume);
    sample->dataMap_ = volume->dataMap_;
    sample->setModelMatrix(volume->getModelMatrix());
    sample->setWorldMatrix(volume->getWorldMatrix());
    return sample;
}

}  // namespace inviwo
