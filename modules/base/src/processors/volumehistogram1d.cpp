/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/base/processors/volumehistogram1d.h>

#include <utility>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeHistogram1D::processorInfo_{
    "org.inviwo.VolumeHistogram1D",  // Class identifier
    "Volume Histogram1D",            // Display name
    "Volume",                        // Category
    CodeState::Experimental,         // Code state
    Tags::CPU | Tag{"Histogram"},    // Tags
    R"(Calculate the 1D histogram of a volume.)"_unindentHelp,
};

const ProcessorInfo& VolumeHistogram1D::getProcessorInfo() const { return processorInfo_; }

VolumeHistogram1D::VolumeHistogram1D()
    : PoolProcessor{}
    , inport_{"inport", "Volume data to compute the histogram for."_help}
    , outport_{"outport", "Computed volume histogram"_help} {

    addPorts(inport_, outport_);
}

void VolumeHistogram1D::process() {
    const auto calc = [data = inport_.getData()]() { return data->calculateHistograms(); };

    dispatchOne(calc, [this](std::shared_ptr<const std::vector<Histogram1D>> histograms) {
        outport_.setData(std::move(histograms));
        newResults();
    });
}

}  // namespace inviwo
