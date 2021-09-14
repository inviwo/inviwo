/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/discretedata/processors/channeloperations.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ChannelOperations::processorInfo_{
    "org.inviwo.ChannelOperations",  // Class identifier
    "Channel Operations",            // Display name
    "Undefined",                     // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
};
const ProcessorInfo ChannelOperations::getProcessorInfo() const { return processorInfo_; }

ChannelOperations::ChannelOperations()
    : Processor()
    , dataIn_("dataIn")
    , dataOut_("dataOut")
    , channelOperations_("channelOps", "Channel Operations", [this]() {
        std::vector<std::unique_ptr<Property>> v;
        v.emplace_back(std::make_unique<ChannelOpProperty<NormalizeChannelOperation>>(
            "normalizeChannel", "Normalize Channel", dataIn_));
        v.emplace_back(std::make_unique<ChannelOpProperty<MagnitudeOperation>>(
            "magnitude", "Magnitude", dataIn_));
        return v;
    }()) {

    addPort(dataIn_);
    addPort(dataOut_);
    addProperty(channelOperations_);
    dataIn_.onChange([this]() {
        for (auto prop : channelOperations_.getPropertiesByType<ChannelOpPropertyBase>()) {
            prop->baseChannel_.updateChannelList();
        }
    });
}

void ChannelOperations::process() {
    if (!dataIn_.hasData()) {
        dataOut_.clear();
        return;
    }
    auto outData = std::make_shared<DataSet>(*dataIn_.getData());

    for (auto prop : channelOperations_.getPropertiesByType<ChannelOpPropertyBase>()) {
        auto channel = prop->applyOperation();
        if (channel) outData->addChannel(channel);
    }

    dataOut_.setData(outData);
}

ChannelOpPropertyBase::ChannelOpPropertyBase(const std::string& identifier,
                                             const std::string& displayName,
                                             DataSetInport& dataInport,
                                             DataChannelProperty::ChannelFilter filter)
    : CompositeProperty(identifier, displayName)
    , baseChannel_(dataInport, "baseChannel", "Base Channel", filter)
    , channelName_("channelName", "Name") {
    addProperties(baseChannel_, channelName_);
    std::cout << "========= Added props" << std::endl;
}

ChannelOpPropertyBase::ChannelOpPropertyBase(const ChannelOpPropertyBase& prop)
    : CompositeProperty(prop), baseChannel_(prop.baseChannel_), channelName_(prop.channelName_) {
    addProperties(baseChannel_, channelName_);
    std::cout << "========= Added props FIRST" << std::endl;
}

}  // namespace discretedata
}  // namespace inviwo
