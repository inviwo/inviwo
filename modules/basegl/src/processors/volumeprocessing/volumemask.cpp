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

#include <modules/basegl/processors/volumeprocessing/volumemask.h>
#include <modules/basegl/algorithm/masking.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeMask::processorInfo_{
    "org.inviwo.VolumeMask",  // Class identifier
    "Volume Mask",            // Display name
    "Volume Operation",       // Category
    CodeState::Experimental,  // Code state
    Tags::GL,                 // Tags
    R"(Construct a binary mask volume base on a input volume)"_unindentHelp,
};

const ProcessorInfo& VolumeMask::getProcessorInfo() const { return processorInfo_; }

VolumeMask::VolumeMask()
    : Processor{}
    , inport_{"inport", ""_help}
    , outport_{"outport", ""_help}

    , maskMissingValue_{"maskMissingValue", "Mask missing values", false}
    , maskZero_{"maskZero", "Mask zero values", false}
    , maskNaN_{"maskNaN", "Mask NaN values", false}
    , maskInf_{"maskInf", "Mask Inf values", false}
    , maskTool_{} {

    addPorts(inport_, outport_);
    addProperties(maskMissingValue_, maskZero_, maskNaN_, maskInf_);
}

void VolumeMask::process() {
    auto data = inport_.getData();

    auto mask = maskTool_.process(
        mask::Config{
            .maskMissingValue = maskMissingValue_.get(),
            .maskZero = maskZero_.get(),
            .maskNaN = maskNaN_.get(),
            .maskInf = maskInf_.get(),
        },
        *data);

    outport_.setData(mask);
}

}  // namespace inviwo
