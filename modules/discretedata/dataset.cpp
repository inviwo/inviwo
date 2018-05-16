/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2012-2018 Inviwo Foundation
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

#include "dataset.h"
#include "discretedata/channels/bufferchannel.h"

namespace inviwo {
namespace dd {

SharedChannel DataChannelMap::addChannel(Channel* const channel) {
    SharedChannel sharedChannel(channel);
    addChannel(sharedChannel);

    return sharedChannel;
}

void DataChannelMap::addChannel(SharedConstChannel sharedChannel) {
    ChannelSet.insert(std::make_pair(
        std::make_pair(sharedChannel->getName(), sharedChannel->getGridPrimitiveType()),
        sharedChannel));
}

bool DataChannelMap::removeChannel(SharedConstChannel channel) {
    return ChannelSet.erase(std::make_pair(channel->getName(), channel->getGridPrimitiveType()));
}

SharedConstChannel DataChannelMap::getFirstChannel() const {
    auto it = ChannelSet.begin();

    if (it == ChannelSet.end()) return SharedConstChannel();

    return it->second;
}

SharedConstChannel DataChannelMap::getChannel(const std::string& name,
                                              const GridPrimitive definedOn) const {
    auto key = std::make_pair(name, definedOn);
    return getChannel(key);
}

SharedConstChannel DataChannelMap::getChannel(std::pair<std::string, GridPrimitive>& key) const {
    auto it = ChannelSet.find(key);

    if (it == ChannelSet.end()) return SharedConstChannel();

    return it->second;
}

std::vector<std::pair<std::string, GridPrimitive>> DataChannelMap::getChannelNames() const {
    ind numChannels = getNumChannels();

    std::vector<std::pair<std::string, GridPrimitive>> channelNames;
    channelNames.reserve(numChannels);
    for (auto& key : ChannelSet) {
        channelNames.push_back(key.first);
    }

    return channelNames;
}

}  // namespace
}
