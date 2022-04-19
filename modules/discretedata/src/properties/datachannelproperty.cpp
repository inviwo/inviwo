/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <modules/discretedata/properties/datachannelproperty.h>
#include <set>

namespace inviwo {
namespace discretedata {
const std::string DataChannelProperty::classIdentifier = "inviwo.discretedata.datachannelproperty";

DataChannelProperty::DataChannelProperty(const std::string& identifier,
                                         const std::string& displayName, DataSetInport* dataInport,
                                         ChannelFilter filter, InvalidationLevel invalidationLevel,
                                         PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , datasetInput_(dataInport)
    , channelFilter_(filter)
    , channelName_("channelName", "Channel name", {{"NONE", "NONE"}})
    , gridPrimitive_("gridPrimitive", "Primitive with data")
    , ongoingChange_(false) {
    auto updateCallback = [&]() { updateChannelList(); };
    if (datasetInput_) datasetInput_->onChange(updateCallback);
    gridPrimitive_.onChange(updateCallback);

    addProperty(gridPrimitive_);
    addProperty(channelName_);
}

void DataChannelProperty::setDatasetInput(DataSetInport* port) {
    datasetInput_ = port;
    if (datasetInput_) datasetInput_->onChange([&]() { updateChannelList(); });
}

// DataChannelProperty::DataChannelProperty(const std::string& identifier,
//                                          const std::string& displayName, ChannelFilter filter,
//                                          InvalidationLevel invalidationLevel,
//                                          PropertySemantics semantics)
//     : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
//     , datasetInput_(nullptr)
//     , channelFilter_(filter)
//     , channelName_("channelName", "Channel name", {{"NONE", "NONE"}})
//     , gridPrimitive_("gridPrimitive", "Primitive with data")
//     , ongoingChange_(false) {
//     auto updateCallback = [&]() { updateChannelList(); };
//     gridPrimitive_.onChange(updateCallback);

//     addProperty(gridPrimitive_);
//     addProperty(channelName_);
// }

DataChannelProperty::DataChannelProperty(const DataChannelProperty& prop)
    : CompositeProperty(prop)
    , datasetInput_(prop.datasetInput_)
    , channelFilter_(prop.channelFilter_)
    , channelName_("channelName", "Channel name", {{"NONE", "NONE"}})
    , gridPrimitive_("gridPrimitive", "Primitive with data")
    , ongoingChange_(false) {
    auto updateCallback = [&]() { updateChannelList(); };
    if (datasetInput_) datasetInput_->onChange(updateCallback);
    gridPrimitive_.onChange(updateCallback);

    addProperty(gridPrimitive_);
    addProperty(channelName_);
    updateChannelList();
}

DataChannelProperty& DataChannelProperty::operator=(const DataChannelProperty& prop) {
    // CompositeProperty::operator=(prop);
    setIdentifier(prop.getIdentifier());
    setDisplayName(prop.getDisplayName());
    if (prop.datasetInput_) datasetInput_ = prop.datasetInput_;
    channelFilter_ = {prop.channelFilter_};
    channelName_.replaceOptions({{"NONE", "NONE"}});
    // gridPrimitive_ = {"gridPrimitive", "Primitive with data"};
    ongoingChange_ = false;

    auto updateCallback = [&]() { updateChannelList(); };
    if (datasetInput_) datasetInput_->onChange(updateCallback);
    gridPrimitive_.onChange(updateCallback);

    // addProperty(gridPrimitive_);
    // addProperty(channelName_);
    updateChannelList();
}

std::shared_ptr<const Channel> DataChannelProperty::getCurrentChannel() const {
    if (!datasetInput_) return nullptr;

    auto pInDataSet = datasetInput_->getData();
    if (!pInDataSet || channelName_.size() == 0 || gridPrimitive_.size() == 0 ||
        channelName_.get().compare("NONE") == 0)
        return nullptr;
    return pInDataSet->getChannel(channelName_.get(), gridPrimitive_.get());
}

void DataChannelProperty::updateChannelList() {
    if (ongoingChange_) return;

    convertedChannels_.clear();
    // If no dataset is given, keep names for now.
    if (!datasetInput_ || !datasetInput_->hasData()) return;

    // Get the current name to select same name if possible.
    std::string lastName = channelName_.get();  // channelName_.size() ? channelName_.get() : "";
    GridPrimitive lastPrimitive =
        gridPrimitive_.size() ? gridPrimitive_.get() : GridPrimitive::Vertex;

    channelName_.clearOptions();
    gridPrimitive_.clearOptions();
    channelName_.addOption("NONE", "NONE");

    if (!datasetInput_) return;
    auto dataset = datasetInput_->getData();
    if (!dataset) return;

    // Update channel list with filter.
    auto dataSetNames = dataset->getChannelNames();
    if (dataSetNames.size() == 0) return;

    ongoingChange_ = true;
    // Remove this function as callback from itself.
    // std::weak_ptr<std::function<void()>> sanityCheck = gridPrimitiveOnChange_;
    // gridPrimitiveOnChange_ = nullptr;  // gridPrimitive_.onChangeScoped([]() {});

    // Assemble all present primitives.
    std::set<GridPrimitive> usedPrimitives;
    for (auto& channel : dataSetNames) {
        if (usedPrimitives.insert(channel.second).second)
            gridPrimitive_.addOption(primitiveName(channel.second), primitiveName(channel.second),
                                     channel.second);
    }
    // if (!usedPrimitives.count(lastPrimitive)) lastPrimitive = *usedPrimitives.begin();
    gridPrimitive_.setSelectedValue(lastPrimitive);

    for (auto& channel : dataSetNames) {
        if (channel.second == gridPrimitive_.get() && channelFilter_(dataset->getChannel(channel)))
            channelName_.addOption(channel.first, channel.first);
    }

    channelName_.setSelectedIdentifier(lastName);

    ongoingChange_ = false;
    // Set this function as callback again.
    // gridPrimitive_.
    // gridPrimitiveOnChange_ = gridPrimitive_.onChangeScoped([&]() { updateChannelList(); });
}

// void DataChannelProperty::deserialize(Deserializer& d) {
//     CompositeProperty::deserialize(d);
//     updateChannelList();
// }
// void DataChannelProperty::serialize(Serializer& s) {
//     CompositeProperty::serialize(s);
//     updateChannelList();
// }

}  // namespace discretedata
}  // namespace inviwo