/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/base/processors/volumesequenceelementselectorprocessor.h>

#include <inviwo/core/processors/processorinfo.h>                    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                   // for CodeState, CodeState...
#include <inviwo/core/processors/processortags.h>                    // for Tags, Tags::CPU
#include <inviwo/core/properties/ordinalproperty.h>                  // for IntSizeTProperty
#include <inviwo/core/util/glmvec.h>                                 // for uvec3
#include <modules/base/processors/vectorelementselectorprocessor.h>  // for VectorElementSelecto...
#include <modules/base/properties/sequencetimerproperty.h>           // for SequenceTimerProperty

#include <functional>  // for __base

namespace inviwo {
class Volume;

const ProcessorInfo VolumeSequenceElementSelectorProcessor::processorInfo_{
    "org.inviwo.TimeStepSelector",       // Class identifier
    "Volume Sequence Element Selector",  // Display name
    "Data Selector",                     // Category
    CodeState::Stable,                   // Code state
    Tags::CPU,                           // Tags
    "Select a specific volume out of a sequence of volumes"_help,
};
const ProcessorInfo& VolumeSequenceElementSelectorProcessor::getProcessorInfo() const {
    return processorInfo_;
}
VolumeSequenceElementSelectorProcessor::VolumeSequenceElementSelectorProcessor()
    : VectorElementSelectorProcessor<Volume>() {
    timeStep_.index_.autoLinkToProperty<VolumeSequenceElementSelectorProcessor>(
        "timeStep.selectedSequenceIndex");
}

}  // namespace inviwo
