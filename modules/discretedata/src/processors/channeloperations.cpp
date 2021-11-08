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
    , channelOperations_(
          "channelOps", "Channel Operations",
          [this]() {
              std::vector<std::unique_ptr<Property>> v;
              v.emplace_back(std::make_unique<ChannelOpProperty<NormalizeChannelOperation>>(
                  "normalizeChannel", "Normalize Channel", &dataIn_, "Normalized"));
              v.emplace_back(std::make_unique<ChannelOpProperty<MagnitudeOperation>>(
                  "magnitude", "Magnitude", &dataIn_, "Magnitude"));
              v.emplace_back(std::make_unique<ChannelOpProperty<NormalizedMagnitudeOperation>>(
                  "normalizedMagnitude", "Normalized Magnitude", &dataIn_, "NormMag"));
              v.emplace_back(std::make_unique<ChannelOpProperty<AppendOperation>>(
                  "append", "Append Value", &dataIn_, "Appended"));
              return v;
          }())
// , testProp_("normalizeChannel", "Normalize Channel", &dataIn_, "Normalized")
{

    addPort(dataIn_);
    addPort(dataOut_);
    addProperty(channelOperations_);

    // dataIn_.onChange([this]() {
    //     for (auto prop : channelOperations_.getPropertiesByType<ChannelOpPropertyBase>()) {
    //         prop->baseChannel_.updateChannelList();
    //     }
    // });
}

void ChannelOperations::process() {
    if (!dataIn_.hasData()) {
        dataOut_.clear();
        return;
    }
    auto outData = std::make_shared<DataSet>(*dataIn_.getData());
    // std::cout << "CHannelOp process:" << std::endl;
    // for (auto* prop : channelOperations_) {
    //     std::cout << "  " << prop->getIdentifier() << std::endl;
    // }

    for (auto prop : channelOperations_.getPropertiesByType<ChannelOpPropertyBase>()) {
        auto channel = prop->applyOperation();
        LogWarn("Made channel " << channel.get());
        if (channel) {
            outData->addChannel(channel);
            LogWarn("  and added it");
        }
    }

    dataOut_.setData(outData);
}

ChannelOpPropertyBase::ChannelOpPropertyBase(const std::string& identifier,
                                             const std::string& displayName,
                                             DataSetInport* dataInport,
                                             const std::string& defaultName,
                                             DataChannelProperty::ChannelFilter filter)
    : CompositeProperty(identifier, displayName)
    , baseChannel_("baseChannel", "Base Channel", dataInport, filter)
    , channelName_("channelName", "Name", defaultName) {
    addProperties(baseChannel_, channelName_);
    std::cout << "========= Added props" << std::endl;
}

ChannelOpPropertyBase::ChannelOpPropertyBase(const ChannelOpPropertyBase& prop)
    : CompositeProperty(prop), baseChannel_(prop.baseChannel_), channelName_(prop.channelName_) {
    addProperties(baseChannel_, channelName_);
    std::cout << "========= Added props FIRST" << std::endl;
}

void ChannelOperations::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    // clearProperties();
    // channelOperations_.clear();
    // // addProperty(channelOperations_);

    // std::vector<std::string> identifiers;
    // std::vector<std::string> selectedChannel;
    // std::vector<int> gridPrimitive;
    // std::vector<std::string> channelName;
    // d.deserialize("CO_identifiers", identifiers);
    // d.deserialize("CO_selectedChannel", selectedChannel);
    // d.deserialize("CO_gridPrimitive;", gridPrimitive);
    // d.deserialize("CO_channelName", channelName);
    // size_t numProps = std::min(std::min(identifiers.size(), selectedChannel.size()),
    //                            std::min(gridPrimitive.size(), channelName.size()));
    // std::cout << "NumProps: " << numProps << std::endl;

    // for (ind p = 0; p < numProps; ++p) {
    //     std::cout << fmt::format("Prop| id: {}, chann: {}", identifiers[p], selectedChannel[p])
    //               << std::endl;
    //     std::cout << fmt::format("      prim: {}, channName: {}", gridPrimitive[p],
    //     channelName[p])
    //               << std::endl;

    //     if (identifiers[p].rfind("normalize", 0) == 0) {
    //         auto prop = new ChannelOpProperty<NormalizeChannelOperation>(
    //             identifiers[p], "Normalize Channel", dataIn_, "Normalized");
    //         prop->baseChannel_.gridPrimitive_.setSelectedValue(GridPrimitive(gridPrimitive[p]));
    //         prop->baseChannel_.channelName_.setSelectedValue(selectedChannel[p]);
    //         prop->channelName_.set(channelName[p]);
    //         std::cout << "    yeah." << std::endl;
    //         channelOperations_.addProperty(prop);
    //     }

    //     else if (identifiers[p].rfind("magnitude", 0) == 0) {
    //         auto prop = new ChannelOpProperty<MagnitudeOperation>(
    //             identifiers[0], "Magnitude Channel", dataIn_, "Magnitude");
    //         std::cout << "    made prop." << std::endl;
    //         prop->baseChannel_.gridPrimitive_.setSelectedValue(GridPrimitive(gridPrimitive[p]));
    //         std::cout << "    selected prim." << std::endl;
    //         prop->baseChannel_.channelName_.setSelectedValue(selectedChannel[p]);
    //         std::cout << "    selected channel name." << std::endl;
    //         prop->channelName_.set(channelName[p]);
    //         std::cout << "    yeah." << std::endl;
    //         channelOperations_.addProperty(prop);
    //     }
    // }
}

void ChannelOperations::serialize(Serializer& s) const {
    Processor::serialize(s);

    // std::vector<std::string> identifiers;
    // std::vector<std::string> selectedChannel;
    // std::vector<int> gridPrimitive;
    // std::vector<std::string> channelName;
    // std::cout << "Serializing!" << std::endl;

    // // for (auto* prop : getProperties()) {
    // //     std::cout << "  Any prop| " << prop->getIdentifier() << std::endl;
    // // }

    // for (auto* prop : channelOperations_.getPropertiesByType<ChannelOpPropertyBase>()) {
    //     identifiers.push_back(prop->getIdentifier());
    //     selectedChannel.push_back(prop->baseChannel_.channelName_.get());
    //     gridPrimitive.push_back((int)prop->baseChannel_.gridPrimitive_.get());
    //     channelName.push_back(prop->channelName_.get());
    //     std::cout << fmt::format("Prop| id: {}, chann: {}", identifiers.back(),
    //                              selectedChannel.back())
    //               << std::endl;
    //     std::cout << fmt::format("      prim: {}, channName: {}", gridPrimitive.back(),
    //                              channelName.back())
    //               << std::endl;
    // }
    // size_t numProps = std::min(std::min(identifiers.size(), selectedChannel.size()),
    //                            std::min(gridPrimitive.size(), channelName.size()));
    // std::cout << "NumProps: " << numProps << std::endl;

    // s.serialize("CO_identifiers", identifiers);
    // s.serialize("CO_selectedChannel", selectedChannel);
    // s.serialize("CO_gridPrimitive;", gridPrimitive);
    // s.serialize("CO_channelName", channelName);
}

}  // namespace discretedata
}  // namespace inviwo
