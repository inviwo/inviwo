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

#include <inviwo/sgct/networksyncmanager.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/networkedge.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/sgct/io/communication.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/util/rendercontext.h>

namespace inviwo {

NetworkSyncServer::NetworkSyncServer(ProcessorNetwork& net) : net_{net} { net_.addObserver(this); }

const std::vector<SgctCommand>& NetworkSyncServer::getCommands() {
    std::scoped_lock lock{commandsMutex_};
    collectPropertyChanges();
    return commands_;
}
void NetworkSyncServer::clearCommands() {
    std::scoped_lock lock{commandsMutex_};
    commands_.clear();
}

std::vector<std::byte> NetworkSyncServer::getEncodedCommandsAndClear() {
    std::scoped_lock lock{commandsMutex_};
    collectPropertyChanges();
    auto bytes = inviwo::util::encode(commands_);
    commands_.clear();
    return bytes;
}

template <typename Command, typename Item>
SgctCommand create(const Item& item) {
    Serializer s{""};
    s.serialize("edge", NetworkEdge{item});
    std::stringstream ss;
    s.writeFile(ss, false);
    return Command{std::move(ss).str()};
}

void NetworkSyncServer::onProcessorNetworkDidAddProcessor(Processor* processor) {
    processor->ProcessorObservable::addObserver(this);

    Serializer s{""};
    s.serialize("processor", processor);
    std::stringstream ss;
    s.writeFile(ss, false);

    std::scoped_lock lock{commandsMutex_};
    commands_.push_back(command::AddProcessor{std::move(ss).str()});
}
void NetworkSyncServer::onProcessorNetworkDidRemoveProcessor(Processor* processor) {
    processor->ProcessorObservable::removeObserver(this);
    collectPropertyChanges();

    std::scoped_lock lock{commandsMutex_};
    commands_.push_back(command::RemoveProcessor{processor->getIdentifier()});
}
void NetworkSyncServer::onProcessorNetworkDidAddConnection(const PortConnection& connection) {
    collectPropertyChanges();
    std::scoped_lock lock{commandsMutex_};
    commands_.push_back(create<command::AddConnection>(connection));
}
void NetworkSyncServer::onProcessorNetworkDidRemoveConnection(const PortConnection& connection) {
    collectPropertyChanges();
    std::scoped_lock lock{commandsMutex_};
    commands_.push_back(create<command::RemoveConnection>(connection));
}
void NetworkSyncServer::onProcessorNetworkDidAddLink(const PropertyLink& link) {
    collectPropertyChanges();
    std::scoped_lock lock{commandsMutex_};
    commands_.push_back(create<command::AddLink>(link));
}
void NetworkSyncServer::onProcessorNetworkDidRemoveLink(const PropertyLink& link) {
    collectPropertyChanges();
    std::scoped_lock lock{commandsMutex_};
    commands_.push_back(create<command::RemoveLink>(link));
}

void NetworkSyncServer::showStats(bool show) {
    std::scoped_lock lock{commandsMutex_};
    commands_.push_back(command::Stats{show});
}

void NetworkSyncServer::onAboutPropertyChange(Property* property) {
    if (property) {
        std::scoped_lock lock{modifiedMutex_};
        modified_.push_back(property);
    } else {
        1 + 1;
    }
}

void NetworkSyncServer::collectPropertyChanges() {
    const auto getOwners = [](Property* p) {
        std::vector<PropertyOwner*> owners;
        PropertyOwner* owner = p->getOwner();
        while (owner) {
            owners.push_back(owner);
            owner = owner->getOwner();
        }
        std::reverse(owners.begin(), owners.end());
        return owners;
    };

    using PathAndProperty = std::pair<std::vector<PropertyOwner*>, Property*>;
    std::vector<PathAndProperty> pathAndProperties;
    {
        std::scoped_lock lock{modifiedMutex_};
        if (modified_.empty()) return;

        std::sort(modified_.begin(), modified_.end());
        modified_.erase(std::unique(modified_.begin(), modified_.end()), modified_.end());

        std::transform(modified_.begin(), modified_.end(), std::back_inserter(pathAndProperties),
                       [&](auto* p) {
                           return PathAndProperty{getOwners(p), p};
                       });
        modified_.clear();
    }

    std::sort(pathAndProperties.begin(), pathAndProperties.end(),
              [](const PathAndProperty& a, const PathAndProperty& b) {
                  if (a.first == b.first) {
                      return a.second < b.second;
                  } else {
                      return a.first < b.first;
                  }
              });

    auto newEnd = std::unique(pathAndProperties.begin(), pathAndProperties.end(),
                              [](const PathAndProperty& a, const PathAndProperty& b) {
                                  const auto& pathA = a.first;
                                  const auto& pathB = b.first;
                                  const auto& propA = a.second;
                                  const auto& propB = b.second;
                                  if (pathA.size() > pathB.size()) return false;
                                  if (pathA.size() == pathB.size())
                                      return pathA == pathB && propA == propB;
                                  return std::equal(pathA.begin(), pathA.end(), pathB.begin(),
                                                    pathB.begin() + pathA.size());
                              });

    std::vector<Property*> unique;
    std::transform(pathAndProperties.begin(), newEnd, std::back_inserter(unique),
                   [](const PathAndProperty& a) { return a.second; });

    // Only send one of the properties in a set of linked properties
    auto linked = [&](Property* a, Property* b) {
        auto alinks = net_.getPropertiesLinkedTo(a);
        if (!util::contains(alinks, b)) return false;
        auto blinks = net_.getPropertiesLinkedTo(b);
        if (!util::contains(blinks, a)) return false;

        return true;
    };

    std::vector<Property*> uniqueAfterLinks;
    for (auto* p : unique) {
        if (!util::contains_if(uniqueAfterLinks, [&](Property* b) { return linked(p, b); })) {
            uniqueAfterLinks.push_back(p);
        }
    }

    std::vector<std::string> paths;
    std::transform(uniqueAfterLinks.begin(), uniqueAfterLinks.end(), std::back_inserter(paths),
                   [](auto* p) { return p->getPath(); });

    Serializer s{""};
    s.serialize("paths", paths);
    s.serialize("modified", uniqueAfterLinks);
    std::stringstream ss;
    s.writeFile(ss, false);
    commands_.push_back(command::Update{std::move(ss).str()});
}

NetworkSyncClient::NetworkSyncClient(ProcessorNetwork& net)
    : net_{net}, wm_{*net_.getApplication()->getWorkspaceManager()} {}

void NetworkSyncClient::applyCommands(const std::vector<SgctCommand>& commands) {
    RenderContext::getPtr()->activateDefaultRenderContext();

    for (const auto& command : commands) {
        try {
            std::visit(
                util::overloaded{
                    [](const command::Nop&) {},
                    [&](const command::AddProcessor& update) {
                        std::stringstream is{update.data};
                        auto d = wm_.createWorkspaceDeserializer(is, "");
                        std::unique_ptr<Processor> processor;
                        d.deserialize("processor", processor);
                        net_.addProcessor(std::move(processor));
                    },
                    [&](const command::RemoveProcessor& update) {
                        net_.removeProcessor(update.data);
                    },
                    [&](const command::AddConnection& update) {
                        std::stringstream is{update.data};
                        auto d = wm_.createWorkspaceDeserializer(is, "");
                        NetworkEdge edge;
                        d.deserialize("edge", edge);
                        net_.addConnection(edge.toConnection(net_));
                    },
                    [&](const command::RemoveConnection& update) {
                        std::stringstream is{update.data};
                        auto d = wm_.createWorkspaceDeserializer(is, "");
                        NetworkEdge edge;
                        d.deserialize("edge", edge);
                        net_.removeConnection(edge.toConnection(net_));
                    },
                    [&](const command::AddLink& update) {
                        std::stringstream is{update.data};
                        auto d = wm_.createWorkspaceDeserializer(is, "");
                        NetworkEdge edge;
                        d.deserialize("edge", edge);
                        net_.addLink(edge.toLink(net_));
                    },
                    [&](const command::RemoveLink& update) {
                        std::stringstream is{update.data};
                        auto d = wm_.createWorkspaceDeserializer(is, "");
                        NetworkEdge edge;
                        d.deserialize("edge", edge);
                        net_.removeLink(edge.toLink(net_));
                    },

                    [&](const command::Update& update) {
                        std::stringstream is{update.data};
                        auto d = wm_.createWorkspaceDeserializer(is, "");
                        std::vector<std::string> paths;
                        std::vector<Property*> modifiedProps;

                        d.deserialize("paths", paths);
                        for (auto& path : paths) {
                            modifiedProps.push_back(net_.getProperty(path));
                        }
                        d.deserialize("modified", modifiedProps);
                    },
                    [&](const command::Stats& stats) { onStats(stats.show); }},

                command);
        } catch (const Exception& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error);
        }
    }
}

}  // namespace inviwo
