/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/discretedata/processors/vectorfromdataset.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VectorFromDataset::processorInfo_{
    "org.inviwo.VectorFromDataset",  // Class identifier
    "Vector From Dataset",           // Display name
    "Undefined",                     // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
};
const ProcessorInfo VectorFromDataset::getProcessorInfo() const { return processorInfo_; }

VectorFromDataset::VectorFromDataset()
    : Processor()
    , portInDataSet_("outport")
    , channelsToExport_("channelsToExport", "Channels to Export") {

    addPort(portInDataSet_);
    addProperty(channelsToExport_);
    isSink_.setUpdate([]() { return true; });  // Always update network up to here.
}

void VectorFromDataset::process() {
    // Use this function only if the data input is changed, to update the properties and outports.
    // Switching on and off channels for export is handled by the properties via callbacks.
    if (!portInDataSet_.isChanged()) return;

    auto data = portInDataSet_.getData();

    auto channels = data->getChannelNames();
    std::vector<Property*> allProps = channelsToExport_.getProperties();
    std::vector<Outport*> allPorts = getOutports();

    // Remove properties and ports we no longer need.
    for (auto propIt = allProps.begin(); propIt != allProps.end(); ++propIt) {
        if (std::find_if(channels.begin(), channels.end(), [&](auto& ch) {
                return ch.first.compare((*propIt)->getIdentifier()) == 0;
            }) == channels.end()) {
            removeProperty(*propIt);
        }
    }

    for (auto portIt = allPorts.begin(); portIt != allPorts.end(); ++portIt) {
        if (std::find_if(channels.begin(), channels.end(), [&](auto& ch) {
                return ch.first.compare((*portIt)->getIdentifier()) == 0;
            }) == channels.end()) {
            removePort(*portIt);
        }
    }

    // Add proeprties and outports where none exist.
    for (auto& channelId : channels) {
        auto prop =
            dynamic_cast<BoolProperty*>(channelsToExport_.getPropertyByIdentifier(channelId.first));
        if (!prop) {
            auto* boolProp = new BoolProperty(channelId.first, channelId.first, false);
            channelsToExport_.addProperty(boolProp);
            prop = boolProp;

            // Remove or add outport when the checkbox is changed.
            boolProp->onChange([thisProp = boolProp, processor = this,
                                channel = data->getChannel(channelId.first, channelId.second)]() {
                if (!thisProp->get()) {
                    processor->removePort(thisProp->getIdentifier());
                } else {
                    Outport* port = processor->getOutport(thisProp->getIdentifier());
                    channel->dispatch<void, dispatching::filter::Scalars, 1,
                                      4>([&](auto datachannel) {
                        using ChannelVec =
                            typename std::remove_pointer<decltype(datachannel)>::type::DefaultVec;
                        using OutputType = DataOutport<std::vector<ChannelVec>>;
                        OutputType* outport = dynamic_cast<OutputType*>(port);
                        if (!outport) {
                            if (port)
                                processor->removePort(port);  // Outport exists, but wrong type.
                            auto uniqueOutport =
                                std::make_unique<OutputType>(datachannel->getName());
                            processor->addPort(std::move(uniqueOutport));
                            port = processor->getOutport(thisProp->getIdentifier());
                            outport = dynamic_cast<OutputType*>(port);
                        }
                        auto dataVector =
                            std::make_shared<std::vector<ChannelVec>>(datachannel->size());
                        datachannel->fill(*dataVector->data(), 0, datachannel->size());
                        outport->setData(dataVector);
                    });
                }
            });
        }

        prop->propertyModified();  // Create the outport if needed, update data.
    }
}

}  // namespace discretedata
}  // namespace inviwo
