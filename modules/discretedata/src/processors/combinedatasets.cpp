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

#include <modules/discretedata/processors/combinedatasets.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CombineDataSets::processorInfo_{
    "org.inviwo.CombineDataSets",  // Class identifier
    "Combine Data Sets",           // Display name
    "Undefined",                   // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
};
const ProcessorInfo CombineDataSets::getProcessorInfo() const { return processorInfo_; }

CombineDataSets::CombineDataSets()
    : Processor()
    , baseDataSetIn_("baseDataSetIn")
    , additionalDataSetsIn_("additionalDataSetsIn")
    , dataOut_("dataSetOut") {

    addPort(baseDataSetIn_);
    addPort(additionalDataSetsIn_);
    addPort(dataOut_);
    // addProperty(position_);

    std::cout << fmt::format("=> Num Props at init: {} ({} DataSet Props)", getProperties().size(),
                             getPropertiesByType<DataSetChannelsProperty>().size())
              << std::endl;
}

void CombineDataSets::process() {
    std::cout << fmt::format("=> Num Props at process: {} ({} DataSet Props)",
                             getProperties().size(),
                             getPropertiesByType<DataSetChannelsProperty>().size())
              << std::endl;
    auto dataSets = additionalDataSetsIn_.getVectorData();
    size_t numDataSets = dataSets.size();

    if (!baseDataSetIn_.hasData() || numDataSets < 1) {
        dataOut_.clear();
        clearProperties();
        std::cout << "Clearing up" << std::endl;
        return;
    }

    const Connectivity& grid = *baseDataSetIn_.getData()->getGrid();
    std::vector<bool> isRepresented(numDataSets, false);
    std::vector<Property*> removeProps;

    for (auto* dataSetProp : getPropertiesByType<DataSetChannelsProperty>()) {
        // for (auto& prop : getProperties()) {
        // auto* dataSetProp = dynamic_cast<DataSetChannelsProperty*>(prop);
        // // At startup, we might retain some properties.
        // if (!dataSetProp) {
        //     // This was loaded. Is it at least a composite property?
        //     auto* compositeProp = dynamic_cast<CompositeProperty*>(prop);
        //     if (!compositeProp) {
        //         std::cout << fmt::format("Okay, what the fuck is this thing? {} called \'{}\'",
        //                                  prop->getIdentifier(), prop->getDisplayName())
        //                   << std::endl;
        //         removeProps.push_back(prop);
        //         continue;
        //     }
        //     continue;
        // }

        bool matched = false;
        // if (DataSetChannelsProperty) {
        //      bool matched = true;
        // // Test for unchanged DataSets.
        // for (size_t i = 0; i < numDataSets; ++i) {
        //     if (prop->dataSet_ == dataSets[i]) {
        //         isRepresented[i] = true;
        //         matched = true;
        //         std::cout << "  Which we recognize!"
        //         break;
        //     }
        // }
        // if (matched) continue;
        // // Test for DataSets with same name.
        // for (size_t i = 0; i < numDataSets; ++i) {
        //     if (prop->dataSet_->getName().compare(dataSets[i]->getName()) == 0) {
        //         prop->updateDataSet(dataSets[i], grid);
        //         matched = true;
        //         break;
        //     }
        // }
        // std::cout << "Has prop " << prop->getIdentifier() << std::endl;

        // Test for unchanged DataSets.
        for (size_t i = 0; i < numDataSets; ++i) {
            if (dataSetProp->dataSet_ == dataSets[i]) {
                isRepresented[i] = true;
                matched = true;
                std::cout << "  Which we recognize!" << std::endl;
                break;
            }
        }
        if (matched) continue;

        // Test for DataSets with same name.
        for (size_t i = 0; i < numDataSets; ++i) {
            if (dataSetProp->dataSet_->getName().compare(dataSets[i]->getName()) == 0) {
                dataSetProp->updateDataSet(dataSets[i], grid);
                matched = true;
                std::cout << "  Which we recognize, by name at least!" << std::endl;
                break;
            }
        }
        if (matched) continue;

        // Throw out.
        removeProps.push_back(dataSetProp);
    }
    std::cout << fmt::format("Removing {} properties.", removeProps.size()) << std::endl;
    for (auto* prop : removeProps) removeProperty(prop);

    for (auto&& [data, represented] : util::zip(dataSets, isRepresented)) {
        // std::cout << fmt::format("{} is {}", data->getName(),
        //                          represented ? "represented" : "not represented")
        // << std::endl;
        if (represented) continue;

        auto ident = fmt::format("dataset{}_{}", data->getName(), rand());

        while (getPropertyByIdentifier(ident)) {
            ident = fmt::format("dataset{}_{}", data->getName(), rand());
        }
        std::cout << "  ident: " << ident << std::endl;
        addProperty(new DataSetChannelsProperty(ident, data, grid));

        // addProperty(new IntProperty(fmt::format("s{}", rand()), ident, 42));
        std::cout << "Added " << data->getName() << std::endl;
    }

    // Is this the first process after deserializing?
    // Then set the channels we deserialized.
    if (deserializedChannels_.size()) {
        auto strIt = deserializedChannels_.begin();
        DataSetChannelsProperty* dataSetProp = nullptr;

        do {
            auto isDataSet = [&]() { return strIt->rfind("_dataset_", 0) == 0; };

            // Check if this string denotes a dataset, not channel.
            while (isDataSet()) {
                std::string dataSetStr = strIt->substr(9, strIt->length() - 9);

                bool foundChannel = false;
                strIt++;

                // Read the next strings denoting channels that should be set.
                while (strIt != deserializedChannels_.end() && !isDataSet()) {

                    // Check all datasets.
                    for (auto* dataSetProp : getPropertiesByType<DataSetChannelsProperty>()) {
                        if (dataSetProp->getDisplayName().compare(dataSetStr) == 0) {
                            while (!isDataSet()) {
                                auto* channProp = dynamic_cast<ChannelPickingListProperty*>(
                                    dataSetProp->getPropertyByIdentifier(*strIt));
                                if (channProp) {
                                    channProp->set(true);
                                    std::cout << "===> Did set " << *strIt << " - Hallelujah!"
                                              << std::endl;
                                    foundChannel = true;
                                    break;
                                }
                            }
                        }
                        if (foundChannel) break;
                    }
                    strIt++;
                }
            }

            strIt++;
        } while (strIt != deserializedChannels_.end());

        // return;
        deserializedChannels_.clear();
    }
}

void CombineDataSets::deserialize(Deserializer& d) {
    Processor::deserialize(d);

    d.deserialize("selectedChannels", deserializedChannels_);
}

void CombineDataSets::serialize(Serializer& s) const {
    Processor::serialize(s);
    std::vector<std::string> channelString;
    for (auto* prop : getPropertiesByType<DataSetChannelsProperty>()) {
        channelString.emplace_back(fmt::format("_dataset_{}", prop->dataSet_->getName()));

        for (auto* channProp : prop->getPropertiesByType<ChannelPickingListProperty>()) {
            if (channProp->get()) channelString.emplace_back(channProp->channel_->getName());
        }
    }

    s.serialize("selectedChannels", channelString);
}

ChannelPickingListProperty::ChannelPickingListProperty(
    const std::shared_ptr<const Channel>& channel, const Connectivity& grid)
    : BoolProperty(channel->getName(), channel->getName()), channel_(channel) {

    if (channel->size() != grid.getNumElements(channel->getGridPrimitiveType())) {
        this->setReadOnly(true);
        return;
    }
}

}  // namespace discretedata
}  // namespace inviwo
