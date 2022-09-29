/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <modules/base/processors/volumesequencesingletimestepsampler.h>

#include <inviwo/core/ports/dataoutport.h>           // for DataOutport
#include <inviwo/core/ports/outportiterable.h>       // for OutportIterableImpl<>::const_iterator
#include <inviwo/core/ports/volumeport.h>            // for VolumeSequenceInport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>   // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>    // for Tags, Tags::None
#include <inviwo/core/properties/ordinalproperty.h>  // for DoubleProperty
#include <inviwo/core/properties/valuewrapper.h>     // for PropertySerializationMode, PropertyS...
#include <inviwo/core/util/logcentral.h>             // for LogCentral, LogWarn
#include <inviwo/core/util/spatialsampler.h>         // for SpatialSampler
#include <inviwo/core/util/volumesequenceutils.h>    // for getTimestamp, hasTimestamp, getTimes...

#include <functional>                                // for __base
#include <string_view>                               // for string_view
#include <utility>                                   // for pair

#include <fmt/core.h>                                // for format
#include <glm/fwd.hpp>                               // for uvec3

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeSequenceSingleTimestepSamplerProcessor::processorInfo_{
    "org.inviwo.VolumeSequenceSingleTimestepSampler",  // Class identifier
    "Volume Sequence Single Timestep Sampler",         // Display name
    "Spatial Sampler",                                 // Category
    CodeState::Experimental,                           // Code state
    Tags::None,                                        // Tags
};

const ProcessorInfo VolumeSequenceSingleTimestepSamplerProcessor::getProcessorInfo() const {
    return processorInfo_;
}

VolumeSequenceSingleTimestepSamplerProcessor::VolumeSequenceSingleTimestepSamplerProcessor()
    : Processor()
    , volumeSequence_("volumeSequence")
    , sampler_("sampler")
    , timestamp_("timestamp", "Timestamp") {

    addPort(volumeSequence_);
    addPort(sampler_);
    addProperty(timestamp_);

    timestamp_.setSerializationMode(PropertySerializationMode::All);

    volumeSequence_.onChange([&]() {
        if (!sampler_.hasData()) return;
        auto seq = volumeSequence_.getData();
        if (!util::hasTimestamps(*seq, false)) {
            LogWarn("Input volume Sequence does not have timestamps, behaviour is undefined");
        }

        auto newrange = util::getTimestampRange(*seq);
        float t = static_cast<float>((timestamp_.get() - timestamp_.getMinValue()) /
                                     (timestamp_.getMaxValue() - timestamp_.getMinValue()));

        timestamp_.setMinValue(newrange.first);
        timestamp_.setMaxValue(newrange.second);
        timestamp_.setCurrentStateAsDefault();

        timestamp_.set(newrange.first + t * (newrange.second - newrange.first));
    });
}

void VolumeSequenceSingleTimestepSamplerProcessor::process() {

    float t = 0;
    auto vols = util::getVolumesForTimestep(*volumeSequence_.getData(), timestamp_.get());

    if (util::hasTimestamp(vols.first) && util::hasTimestamp(vols.second)) {
        auto t0 = util::getTimestamp(vols.first);
        auto t1 = util::getTimestamp(vols.second);
        if (t0 != t1) {
            t = static_cast<float>((timestamp_.get() - t0) / (t1 - t0));
        }
    }

    auto sampler =
        std::make_shared<VolumeSequenceSingleTimestepSampler>(t, vols.first, vols.second);
    sampler_.setData(sampler);
}

}  // namespace inviwo
