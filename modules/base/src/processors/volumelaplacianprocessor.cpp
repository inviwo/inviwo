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

#include <modules/base/processors/volumelaplacianprocessor.h>

namespace inviwo {

const ProcessorInfo VolumeLaplacianProcessor::processorInfo_{
    "org.inviwo.VolumeLaplacianProcessor",  // Class identifier
    "Volume Laplacian",                     // Display name
    "Volume Operation",                     // Category
    CodeState::Experimental,                // Code state
    Tags::CPU,                              // Tags
};
const ProcessorInfo VolumeLaplacianProcessor::getProcessorInfo() const { return processorInfo_; }

VolumeLaplacianProcessor::VolumeLaplacianProcessor()
    : PoolProcessor()
    , inport_("inport")
    , outport_("outport")
    , postProcessing_(
          "postProcessing", "Post Processing",
          {{"none", "None", util::VolumeLaplacianPostProcessing::None},
           {"normalized", "Normalized", util::VolumeLaplacianPostProcessing::Normalized},
           {"signNormalized", "Sign Normalized",
            util::VolumeLaplacianPostProcessing::SignNormalized},
           {"scaled", "Scaled", util::VolumeLaplacianPostProcessing::Scaled}},
          0)
    , scale_("scale", "Scale", 0.0, 0.0, 1000.0, 0.0001, InvalidationLevel::InvalidOutput,
             PropertySemantics::Text)
    , inVolume_("inputVolume", "Input Volume")
    , outVolume_("outputVolume", "Output Volume") {

    addPort(inport_);
    addPort(outport_);

    addProperties(postProcessing_, scale_, inVolume_, outVolume_);
    scale_.visibilityDependsOn(postProcessing_, [](const auto& p) {
        return p.get() == util::VolumeLaplacianPostProcessing::Scaled;
    });
}

void VolumeLaplacianProcessor::process() {
    auto invol = inport_.getData();
    inVolume_.updateForNewVolume(*invol.get());

    const auto calc = [volume = invol, postProcessing = postProcessing_.get(),
                       scale = scale_.get()]() -> std::shared_ptr<Volume> {
        return util::volumeLaplacian(volume, postProcessing, scale);
    };

    outport_.clear();
    dispatchOne(calc, [this](std::shared_ptr<Volume> result) {
        outVolume_.updateForNewVolume(*result);
        outport_.setData(result);
        newResults();
    });
}

}  // namespace inviwo
