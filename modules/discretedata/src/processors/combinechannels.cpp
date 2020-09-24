/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/discretedata/processors/combinechannels.h>
#include <inviwo/core/properties/boolproperty.h>

#include <string>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CombineChannels::processorInfo_{
    "org.inviwo.CombineChannels",  // Class identifier
    "Combine Channels",            // Display name
    "Undefined",                   // Category
    CodeState::Stable,             // Code state
    Tags::None,                    // Tags
};
const ProcessorInfo CombineChannels::getProcessorInfo() const { return processorInfo_; }

CombineChannels::CombineChannels()
    : Processor()
    , dataIn_("inDataSet")
    , dataOut_("outDataSet")
    , channelName_("channelName", "Combined Channel Name", "Combined")
    , channelList_("channelList", "Combined Channels") {

    addPort(dataIn_);
    addPort(dataOut_);
    addProperties(channelName_, channelList_);
    channelList_.setSerializationMode(PropertySerializationMode::All);
}

void CombineChannels::addAllChannelProperties() {
    channelList_.clearProperties();
    // LogWarn("Cleared properties, left " << channelList_.size());
    if (!dataIn_.hasData()) return;

    for (auto channel : dataIn_.getData()->getChannels()) {
        channelList_.addProperty(new BoolProperty(combineString(*channel.second),
                                                  fmt::format("{} ({})", channel.first.first,
                                                              primitiveName(channel.first.second)),
                                                  false),
                                 true);
    }
}

void CombineChannels::process() {
    LogWarn("~*~<=========================================>~*~");
    // if (firstProcess_ && channelList_.size() > 0) return;

    // Refresh channel list.
    if (dataIn_.isChanged() && !firstProcess_) {

        if (!dataIn_.hasData() || dataIn_.getData()->size() == 0) {
            LogWarn("Aborting - No channels!");
            channelList_.clearProperties();
            dataOut_.detachData();
            return;
        }
        LogWarn("Adding all them channels!");
        addAllChannelProperties();
    }

    // Filter out channels that do not match the ones selected.
    if (channelList_.isModified() && !firstProcess_) {
        // std::string format = "";
        // GridPrimitive gridPrim = GridPrimitive::Undef;
        // for (auto prop : channelList_.getProperties()) {
        //     BoolProperty* boolProp = dynamic_cast<BoolProperty*>(prop);
        //     if (!boolProp || !boolProp->get()) continue;

        //     const std::string& identifier = boolProp->getIdentifier();
        //     gridPrim = uncombineString(identifier).second;
        //     format = uncombineFormat(identifier);
        //     LogWarn("Selected format: " << format);
        //     break;
        // }

        // // None selected? Allow any channel.
        // if (format.length() == 0) {
        //     if (dataIn_.getData()->size() != static_cast<ind>(channelList_.size())) {
        //         addAllChannelProperties();
        //     }
        //     dataOut_.setData(dataIn_.getData());
        //     return;
        // }

        // // Remove channels with different types.
        // for (ind p = ind(channelList_.size() - 1); p >= 0; --p) {
        //     auto* prop = channelList_[p];
        //     const std::string& identifier = prop->getIdentifier();
        //     // std::cout << "Prop " << identifier << std::endl;

        //     if (!identifier.rfind(format, 0) || uncombineString(identifier).second != gridPrim) {
        //         std::cout << "Bye bye! " << identifier << std::endl;
        //         channelList_.removeProperty(p);
        //         return;
        //     }
        // }
    }

    firstProcess_ = false;

    // // Create new channel.
    std::vector<std::shared_ptr<const Channel>> selectedChannels;

    for (auto prop : channelList_) {

        BoolProperty* boolProp = dynamic_cast<BoolProperty*>(prop);
        if (!boolProp || !boolProp->get()) {
            continue;
        }

        const std::string& identifier = boolProp->getIdentifier();
        auto key = uncombineString(identifier);

        auto channel = dataIn_.getData()->getChannel(key);
        selectedChannels.push_back(channel);
    }

    if (selectedChannels.size() >= 2) {
        auto combinedChannel = createCombinedChannel(selectedChannels, channelName_.get());

        std::shared_ptr<DataSet> dataSet = std::make_shared<DataSet>(*dataIn_.getData());
        dataSet->addChannel(combinedChannel);
        dataOut_.setData(dataSet);
    } else {
        dataOut_.setData(dataIn_.getData());
    }
}

std::string CombineChannels::combineString(const Channel& channel) {

    auto scalarType = DataFormatBase::get(channel.getDataFormatId());
    return fmt::format("{}---{}---{}", scalarType->getString(), channel.getName(),
                       int(channel.getGridPrimitiveType()));
}
std::pair<std::string, GridPrimitive> CombineChannels::uncombineString(
    const std::string& combinedString) {

    size_t idxFirst = combinedString.find("---");
    size_t idxSecond = combinedString.find("---", idxFirst + 1);
    GridPrimitive gridPrim =
        static_cast<GridPrimitive>(std::stoi(combinedString.substr(idxSecond + 3)));
    std::string name = combinedString.substr(idxFirst + 3, idxSecond - idxFirst - 3);
    return std::make_pair(name, gridPrim);
}

std::string CombineChannels::uncombineFormat(const std::string& combinedString) {
    return combinedString.substr(0, combinedString.find("---"));
}

}  // namespace discretedata
}  // namespace inviwo
