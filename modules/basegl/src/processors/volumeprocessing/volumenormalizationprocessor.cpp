/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumenormalizationprocessor.h>

#include <inviwo/core/ports/volumeport.h>                  // for VolumeInport, VolumeOutport
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/processors/processorinfo.h>          // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>         // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>          // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>           // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>      // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>      // for InvalidationLevel, Invalidatio...
#include <inviwo/core/properties/property.h>               // for Property
#include <inviwo/core/util/formats.h>                      // for DataFormatBase, NumericType
#include <inviwo/core/util/exception.h>                    // for Exception
#include <modules/basegl/algorithm/volumenormalization.h>  // for VolumeNormalization

#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <vector>       // for vector

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeNormalizationProcessor::processorInfo_{
    "org.inviwo.VolumeNormalizationProcessor",  // Class identifier
    "Volume Normalization",                     // Display name
    "Volume Operation",                         // Category
    CodeState::Stable,                          // Code state
    Tags::GL,                                   // Tags
    R"(Normalizes the selected channels of the input volume based on its data range to range [0, 1].
The format of the resulting volume is 32bit floating point matching the number of channels in the input volume.
The data range and value range are set to [0, 1]. If no channel is selected for normalization, the input volume
will be forwarded unchanged.

Note that this algorithm normalizes channels independently, it does not normalize a multi-channel
volume in terms of vector norms.
)"_unindentHelp,
};
const ProcessorInfo VolumeNormalizationProcessor::getProcessorInfo() const {
    return processorInfo_;
}

VolumeNormalizationProcessor::VolumeNormalizationProcessor()
    : Processor()
    , volumeInport_("volumeInport", "Input Volume"_help)
    , volumeOutport_("volumeOutport",
                     "Resulting, normalized Volume with floating point precision"_help)
    , inputChannels_(
          "inputChannels", "Input Channels",
          util::ordinalCount(4).set(PropertySemantics::Text).set(InvalidationLevel::Valid))
    , channels_("channels", "Channels", "Toggles the normalization of individual channels"_help)
    , normalizeChannels_{{{"normalizeChannel0", "Channel 1", true},
                          {"normalizeChannel1", "Channel 2", true},
                          {"normalizeChannel2", "Channel 3", true},
                          {"normalizeChannel3", "Channel 4", true}}}
    , volumeNormalization_([&]() { this->invalidate(InvalidationLevel::InvalidOutput); }) {

    addPorts(volumeInport_, volumeOutport_);

    for (auto& p : normalizeChannels_) {
        channels_.addProperty(p);
    }

    inputChannels_.setReadOnly(true);
    addProperties(inputChannels_, channels_);

    volumeInport_.onChange([this]() {
        if (volumeInport_.hasData()) {
            const auto channels =
                static_cast<int>(volumeInport_.getData()->getDataFormat()->getComponents());
            inputChannels_.set(channels);
        }
    });
}

void VolumeNormalizationProcessor::process() {
    auto inputVolume = volumeInport_.getData();

    const bvec4 normalize{normalizeChannels_[0], normalizeChannels_[1], normalizeChannels_[2],
                          normalizeChannels_[3]};
    if (glm::any(normalize)) {
        volumeNormalization_.setNormalizeChannels(normalize);
        volumeOutport_.setData(volumeNormalization_.normalize(*inputVolume));
    } else {
        volumeOutport_.setData(inputVolume);
    }
}

}  // namespace inviwo
