/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/base/processors/volumechannelcombiner.h>

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <modules/base/algorithm/combinechannels.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeChannelCombiner::processorInfo_{
    "org.inviwo.VolumeChannelCombiner",  // Class identifier
    "Volume Channel Combiner",           // Display name
    "Volume Operation",                  // Category
    CodeState::Experimental,             // Code state
    Tags::CPU | Tag{"Volume"},           // Tags
    R"(
Combines multiple volume channels into a single volume with multiple channels. All volumes must 
share the same dimensions. The resulting data format depends on the common data type
and precision of the inputs.
)"_unindentHelp,
};

const ProcessorInfo VolumeChannelCombiner::getProcessorInfo() const { return processorInfo_; }

namespace {
const std::vector<OptionPropertyIntOption> channelsList = {{"channel1", "Channel 1", 0},
                                                           {"channel2", "Channel 2", 1},
                                                           {"channel3", "Channel 3", 2},
                                                           {"channel4", "Channel 4", 3}};
}

VolumeChannelCombiner::VolumeChannelCombiner()
    : Processor{}
    , source_{VolumeInport{"source1", "Input for the first channel (red)"_help},
              VolumeInport{"source2", "Input for the second channel (green, optional)"_help},
              VolumeInport{"source3", "Input for the third channel (blue, optional)"_help},
              VolumeInport{"source4", "Input for the fourth channel (alpha, optional)"_help}}
    , outport_{"outport", "Resulting Volume with combined channels"_help}

    , channel_{OptionPropertyInt{"dest1", "Channel 1 Out",
                                 "Selected channel of the first input"_help, channelsList},
               OptionPropertyInt{"dest2", "Channel 2 Out",
                                 "Selected channel of the second input"_help, channelsList},
               OptionPropertyInt{"dest3", "Channel 3 Out",
                                 "Selected channel of the third input"_help, channelsList},
               OptionPropertyInt{"dest4", "Channel 4 Out",
                                 "Selected channel of the fourth input"_help, channelsList}}
    , dataRange_{"dataRange", "Data Range", source_[0], true} {

    addPorts(source_[0], source_[1], source_[2], source_[3], outport_);
    for (auto& port : std::span(source_.begin() + 1, 3)) {
        port.setOptional(true);
    }

    for (auto& prop : channel_) {
        addProperty(prop);
    }
    addProperty(dataRange_);
}

void VolumeChannelCombiner::process() {
    auto volume = util::combineChannels<Volume, VolumeRAM>(
        source_, {{channel_[0], channel_[1], channel_[2], channel_[3]}});
    volume->dataMap_.dataRange = dataRange_.getDataRange();
    volume->dataMap_.valueRange = dataRange_.getValueRange();

    outport_.setData(volume);
}

}  // namespace inviwo
