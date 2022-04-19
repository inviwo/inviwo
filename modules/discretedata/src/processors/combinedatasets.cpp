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
    CodeState::Stable,             // Code state
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
}

void CombineDataSets::process() {

    auto dataSets = additionalDataSetsIn_.getVectorData();
    size_t numDataSets = dataSets.size();

    if (!baseDataSetIn_.hasData() || numDataSets < 1) {
        dataOut_.clear();
        clearProperties();
        return;
    }

    const Connectivity& grid = *baseDataSetIn_.getData()->getGrid();
    std::vector<bool> isRepresented(numDataSets, false);
    std::vector<Property*> removeProps;

    for (auto* dataSetProp : getPropertiesByType<DataSetChannelsProperty>()) {

        bool matched = false;

        // Test for unchanged DataSets.
        for (size_t i = 0; i < numDataSets; ++i) {
            if (!isRepresented[i] && dataSetProp->dataSet_ == dataSets[i]) {
                isRepresented[i] = true;
                matched = true;
                break;
            }
        }
        if (matched) continue;

        // Test for DataSets with same name.
        for (size_t i = 0; i < numDataSets; ++i) {
            if (!isRepresented[i] &&
                dataSetProp->dataSet_->getName().compare(dataSets[i]->getName()) == 0) {
                dataSetProp->updateDataSet(dataSets[i], grid);
                isRepresented[i] = true;
                matched = true;
                break;
            }
        }
        if (matched) continue;

        // Throw out.
        removeProps.push_back(dataSetProp);
    }
    for (auto* prop : removeProps) removeProperty(prop);

    for (auto&& [data, represented] : util::zip(dataSets, isRepresented)) {
        if (represented) continue;

        auto ident = fmt::format("dataset{}_{}", data->getName(), rand());

        while (getPropertyByIdentifier(ident)) {
            ident = fmt::format("dataset{}_{}", data->getName(), rand());
        }
        addProperty(new DataSetChannelsProperty(ident, data, grid));
    }

    // Is this the first process after deserializing?
    // Then set the channels we deserialized.
    if (deserializedChannels_.size()) {
        deserializeSelection();
        deserializedChannels_.clear();
    }

    // Actually process!
    LogWarn("@ Processing! Like, for real!");
    auto dataSetOut = std::make_shared<DataSet>(*baseDataSetIn_.getData());
    for (auto* dataSetProp : getPropertiesByType<DataSetChannelsProperty>()) {

        for (auto* channProp : dataSetProp->getPropertiesByType<ChannelPickingListProperty>()) {
            if (channProp->get()) dataSetOut->addChannel(channProp->channel_);
        }
    }

    dataOut_.setData(dataSetOut);
}

void CombineDataSets::deserializeSelection() {
    auto strIt = deserializedChannels_.begin();

    do {
        auto isValid = [&]() { return strIt != deserializedChannels_.end(); };
        auto isDataSet = [&]() { return strIt->rfind("_dataset_", 0) == 0; };

        // Check if this string denotes a dataset, not channel.
        while (isValid() && isDataSet()) {
            std::string dataSetStr = strIt->substr(9, strIt->length() - 9);

            DataSetChannelsProperty* currentDataSetProp = nullptr;
            for (auto* dataSetProp : getPropertiesByType<DataSetChannelsProperty>()) {
                if (dataSetProp->getDisplayName().compare(dataSetStr) == 0) {
                    currentDataSetProp = dataSetProp;
                    break;
                }
            }
            if (!currentDataSetProp) {
                LogError("Error deserialising");
                return;
            }

            strIt++;
            // Read the next strings denoting channels that should be set.
            while (isValid() && !isDataSet()) {
                std::cout << "\nNow,\n* All Properties" << std::endl;
                for (auto* prop : currentDataSetProp->getProperties()) {
                    std::cout << "*   Prop: " << prop->getDisplayName() << std::endl;
                }

                // Check all datasets.
                if (!isDataSet()) {
                    std::cout << "Strign to pick up property from: " << *strIt << std::endl;
                    auto* channProp = dynamic_cast<ChannelPickingListProperty*>(
                        currentDataSetProp->getPropertyByIdentifier(*strIt));
                    if (channProp) channProp->set(true);
                }
                strIt++;
            }
        }
    } while (isValid());
}

void CombineDataSets::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    clearProperties();

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

    for (auto& str : channelString) {
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

DataSetChannelsProperty::DataSetChannelsProperty(
    const std::string& identifier,  // const std::string& name,
    std::shared_ptr<const DataSet>& dataSet, const Connectivity& grid)
    : CompositeProperty(identifier, dataSet->getName()), dataSet_(dataSet) {

    for (const auto& it : dataSet->getChannels()) {
        const Channel& channel = *it.second;
        auto* channelProp = new ChannelPickingListProperty(it.second, grid);
        addProperty(channelProp);
        std::cout << fmt::format("* Channel {} -> {} ({})", channel.getName(),
                                 channelProp->getDisplayName(), channelProp->getIdentifier())
                  << std::endl;
        std::cout << "* All Properties" << std::endl;
        for (auto* prop : getProperties()) {
            std::cout << "*    Prop: " << prop->getDisplayName() << std::endl;
        }
    }
}

void DataSetChannelsProperty::updateDataSet(std::shared_ptr<const DataSet>& dataSet,
                                            const Connectivity& grid) {
    const auto newChannelNames = dataSet->getChannelNames();
    std::vector<bool> channelHasProperty(newChannelNames.size(), false);

    std::vector<ChannelPickingListProperty*> removeProps;
    for (auto* channProp : getPropertiesByType<ChannelPickingListProperty>()) {

        bool channelInNewDataSetToo = false;

        for (size_t i = 0; i < newChannelNames.size(); ++i) {
            auto& name = newChannelNames[i];
            if (name.first.compare(channProp->getIdentifier()) == 0 &&
                channProp->channel_->size() == dataSet->getChannel(name)->size()) {

                channProp->channel_ = dataSet->getChannel(name);
                channelInNewDataSetToo = true;
                channelHasProperty[i] = true;
                break;
            }
        }

        if (!channelInNewDataSetToo) {
            removeProps.push_back(channProp);
        }
    }
    for (auto* prop : removeProps) {
        removeProperty(prop);
    }

    for (auto&& [name, hasProperty] : util::zip(newChannelNames, channelHasProperty)) {
        if (!hasProperty) {
            addProperty(new ChannelPickingListProperty(dataSet->getChannel(name), grid));
        }
    }
}

}  // namespace discretedata
}  // namespace inviwo
