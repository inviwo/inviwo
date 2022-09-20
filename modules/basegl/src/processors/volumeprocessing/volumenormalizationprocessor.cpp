/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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
#include <inviwo/core/util/logcentral.h>                   // for LogCentral, LogWarn
#include <modules/basegl/algorithm/volumenormalization.h>  // for VolumeNormalization

#include <cstddef>                                         // for size_t
#include <functional>                                      // for __base
#include <memory>                                          // for shared_ptr
#include <string>                                          // for string
#include <string_view>                                     // for string_view
#include <type_traits>                                     // for remove_extent_t
#include <vector>                                          // for vector

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeNormalizationProcessor::processorInfo_{
    "org.inviwo.VolumeNormalizationProcessor",  // Class identifier
    "Volume Normalization",                     // Display name
    "Volume Operation",                         // Category
    CodeState::Stable,                          // Code state
    Tags::GL,                                   // Tags
};
const ProcessorInfo VolumeNormalizationProcessor::getProcessorInfo() const {
    return processorInfo_;
}

VolumeNormalizationProcessor::VolumeNormalizationProcessor()
    : Processor()
    , volumeInport_("volumeInport")
    , volumeOutport_("volumeOutport")
    , channels_("channels", "Channels")
    , normalizeChannel0_("normalizeChannel0", "Channel 1", true)
    , normalizeChannel1_("normalizeChannel1", "Channel 2", false)
    , normalizeChannel2_("normalizeChannel2", "Channel 3", false)
    , normalizeChannel3_("normalizeChannel3", "Channel 4", false)
    , volumeNormalization_([&]() { this->invalidate(InvalidationLevel::InvalidOutput); }) {

    addPorts(volumeInport_, volumeOutport_);

    normalizeChannel1_.setVisible(false);
    normalizeChannel2_.setVisible(false);
    normalizeChannel3_.setVisible(false);

    channels_.addProperties(normalizeChannel0_, normalizeChannel1_, normalizeChannel2_,
                            normalizeChannel3_);

    addProperties(channels_);

    volumeInport_.onChange([this]() {
        if (volumeInport_.hasData()) {
            auto volume = volumeInport_.getData();

            const auto channels = static_cast<int>(volume->getDataFormat()->getComponents());
            if (channels == static_cast<int>(channels_.getProperties().size())) return;

            auto properties = channels_.getProperties();

            dynamic_cast<BoolProperty*>(properties[0])->set(true);

            for (int i = 1; i < 4; i++) {
                auto boolProp = dynamic_cast<BoolProperty*>(properties[i]);
                boolProp->set(i < channels);
                boolProp->setVisible(i < channels);
            }

            volumeNormalization_.reset();
        }
    });
}

void VolumeNormalizationProcessor::process() {
    auto inputVolume = volumeInport_.getData();
    auto channelProperties = channels_.getProperties();

    bool apply = false;
    for (size_t i{0}; i < channelProperties.size(); ++i) {
        apply = apply || dynamic_cast<BoolProperty*>(channelProperties[i])->get();
    }
    if (inputVolume->getDataFormat()->getNumericType() != NumericType::Float) {
        LogWarn("Numeric type of input volume is not floating point, omitting normalization.");
        apply = false;
    }

    if (!apply) {
        volumeOutport_.setData(inputVolume);
    } else {
        for (size_t i{0}; i < channelProperties.size(); ++i) {
            volumeNormalization_.setNormalizeChannel(
                i, dynamic_cast<BoolProperty*>(channelProperties[i])->get());
        }

        volumeOutport_.setData(volumeNormalization_.normalize(*inputVolume));
    }
}

}  // namespace inviwo
