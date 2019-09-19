/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/linksettings.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/network/autolinker.h>

#include <iterator>
#include <unordered_set>

namespace inviwo {

namespace util {

std::unordered_set<Processor*> getDirectPredecessors(Processor* processor) {
    std::unordered_set<Processor*> predecessors;
    for (auto port : processor->getInports()) {
        for (auto connectedPort : port->getConnectedOutports()) {
            predecessors.insert(connectedPort->getProcessor());
        }
    }
    return predecessors;
}

std::unordered_set<Processor*> getDirectSuccessors(Processor* processor) {
    std::unordered_set<Processor*> successors;
    for (auto port : processor->getOutports()) {
        for (auto connectedPort : port->getConnectedInports()) {
            successors.insert(connectedPort->getProcessor());
        }
    }
    return successors;
}

std::unordered_set<Processor*> getPredecessors(Processor* processor) {
    std::unordered_set<Processor*> predecessors;
    std::unordered_set<Processor*> state;
    traverseNetwork<TraversalDirection::Up, VisitPattern::Post>(
        state, processor, [&predecessors](Processor* p) { predecessors.insert(p); });

    return predecessors;
}

std::unordered_set<Processor*> getSuccessors(Processor* processor) {
    std::unordered_set<Processor*> successors;
    std::unordered_set<Processor*> state;
    traverseNetwork<TraversalDirection::Down, VisitPattern::Post>(
        state, processor, [&successors](Processor* p) { successors.insert(p); });

    return successors;
}

std::vector<Processor*> topologicalSort(ProcessorNetwork* network) {
    // perform topological sorting and store processor order in sorted

    std::vector<Processor*> sinkProcessors;
    util::copy_if(network->getProcessors(), std::back_inserter(sinkProcessors),
                  [](Processor* p) { return p->isSink(); });

    std::unordered_set<Processor*> state;
    std::vector<Processor*> sorted;
    for (auto processor : sinkProcessors) {
        traverseNetwork<TraversalDirection::Up, VisitPattern::Post>(
            state, processor, [&sorted](Processor* p) { sorted.push_back(p); });
    }
    return sorted;
}

std::vector<Processor*> topologicalSortFiltered(ProcessorNetwork* network) {
    // perform topological sorting and store processor order in sorted

    std::vector<Processor*> sinkProcessors;
    util::copy_if(network->getProcessors(), std::back_inserter(sinkProcessors),
                  [](Processor* p) { return p->isSink(); });

    std::unordered_set<Processor*> state;
    std::vector<Processor*> sorted;
    for (auto processor : sinkProcessors) {
        traverseNetwork<TraversalDirection::Up, VisitPattern::Post>(
            state, processor, [&sorted](Processor* p) { sorted.push_back(p); },
            [](Processor* p, Inport* from, Outport* to) {
                return p->isConnectionActive(from, to);
            });
    }
    return sorted;
}

std::vector<ivec2> getPositions(const std::vector<Processor*>& processors) {
    return util::transform(processors, [](Processor* p) { return getPosition(p); });
}

std::vector<ivec2> getPositions(ProcessorNetwork* network) {
    std::vector<ivec2> res;
    network->forEachProcessor([&](Processor* p) { res.push_back(getPosition(p)); });
    return res;
}

ivec2 getCenterPosition(const std::vector<Processor*>& processors) {
    ivec2 center{0};
    if (processors.empty()) return center;

    for (auto p : processors) {
        center += getPosition(p);
    }
    return center / static_cast<int>(processors.size());
}

ivec2 getCenterPosition(ProcessorNetwork* network) {
    ivec2 center{0};
    int count = 0;
    network->forEachProcessor([&](Processor* p) {
        center += getPosition(p);
        ++count;
    });
    if (count == 0) {
        return center;
    } else {
        return center / count;
    }
}

std::pair<ivec2, ivec2> getBoundingBox(const std::vector<Processor*>& processors) {
    if (processors.empty()) return {ivec2{0}, ivec2{0}};
    ivec2 minPos{std::numeric_limits<int>::max()};
    ivec2 maxPos{std::numeric_limits<int>::lowest()};

    for (auto p : processors) {
        auto pos = getPosition(p);
        minPos = glm::min(minPos, pos);
        maxPos = glm::max(maxPos, pos);
    }
    return {minPos, maxPos};
}

std::pair<ivec2, ivec2> getBoundingBox(ProcessorNetwork* network) {
    ivec2 minPos{std::numeric_limits<int>::max()};
    ivec2 maxPos{std::numeric_limits<int>::lowest()};
    bool empty = true;
    network->forEachProcessor([&](Processor* p) {
        auto pos = getPosition(p);
        minPos = glm::min(minPos, pos);
        maxPos = glm::max(maxPos, pos);
        empty = false;
    });
    if (empty) {
        return {ivec2{0}, ivec2{0}};
    } else {
        return {minPos, maxPos};
    }
}

void offsetPosition(const std::vector<Processor*>& processors, ivec2 offset) {
    for (auto p : processors) {
        if (auto meta = getMetaData(p)) {
            meta->setPosition(meta->getPosition() + offset);
        }
    }
}

void setSelected(const std::vector<Processor*>& processors, bool selected) {
    for (auto p : processors) {
        setSelected(p, selected);
    }
}

PropertyDistanceSorter::PropertyDistanceSorter() {}

void PropertyDistanceSorter::setTarget(vec2 pos) { pos_ = pos; }
void PropertyDistanceSorter::setTarget(const Property* target) { pos_ = getPosition(target); }

bool PropertyDistanceSorter::operator()(const Property* a, const Property* b) {
    float da = glm::distance(pos_, getPosition(a));
    float db = glm::distance(pos_, getPosition(b));
    return da < db;
}

vec2 PropertyDistanceSorter::getPosition(const Property* p) {
    auto it = cache_.find(p);
    if (it != cache_.end()) return it->second;
    return cache_[p] = getPosition(p->getOwner()->getProcessor());
}

vec2 PropertyDistanceSorter::getPosition(const Processor* processor) {
    return util::getPosition(processor);
}

void serializeSelected(ProcessorNetwork* network, std::ostream& os, const std::string& refPath) {
    Serializer serializer(refPath);

    detail::PartialProcessorNetwork ppc(network);
    serializer.serialize("ProcessorNetwork", ppc);
    serializer.writeFile(os);
}

std::vector<Processor*> appendDeserialized(ProcessorNetwork* network, std::istream& is,
                                           const std::string& refPath, InviwoApplication* app) {
    NetworkLock lock(network);
    auto deserializer = app->getWorkspaceManager()->createWorkspaceDeserializer(is, refPath);

    detail::PartialProcessorNetwork ppc(network);
    deserializer.deserialize("ProcessorNetwork", ppc);

    return ppc.getAddedProcessors();
}

detail::PartialProcessorNetwork::PartialProcessorNetwork(ProcessorNetwork* network)
    : network_(network) {}

std::vector<Processor*> detail::PartialProcessorNetwork::getAddedProcessors() const {
    return addedProcessors_;
}

void detail::PartialProcessorNetwork::serialize(Serializer& s) const {
    std::vector<Processor*> selected;
    util::copy_if(network_->getProcessors(), std::back_inserter(selected), [](const Processor* p) {
        auto m = p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
        return m->isSelected();
    });

    std::vector<PortConnection> connections;
    util::copy_if(network_->getConnections(), std::back_inserter(connections),
                  [&selected](const PortConnection& c) {
                      auto in = c.getInport()->getProcessor();
                      auto out = c.getOutport()->getProcessor();
                      return util::contains(selected, in) && util::contains(selected, out);
                  });

    std::vector<PortConnection> partialInConnections;
    util::copy_if(network_->getConnections(), std::back_inserter(partialInConnections),
                  [&selected](const PortConnection& c) {
                      auto in = c.getInport()->getProcessor();
                      auto out = c.getOutport()->getProcessor();
                      return util::contains(selected, in) && !util::contains(selected, out);
                  });

    auto partialIn = util::transform(partialInConnections, [](const PortConnection& c) {
        return detail::PartialConnection{
            c.getOutport()->getProcessor()->getIdentifier() + "/" + c.getOutport()->getIdentifier(),
            c.getInport()};
    });

    std::vector<PropertyLink> links;
    util::copy_if(network_->getLinks(), std::back_inserter(links),
                  [&selected](const PropertyLink& c) {
                      auto src = c.getSource()->getOwner()->getProcessor();
                      auto dst = c.getDestination()->getOwner()->getProcessor();
                      return util::contains(selected, src) && util::contains(selected, dst);
                  });

    std::vector<PropertyLink> srcLinks;
    util::copy_if(network_->getLinks(), std::back_inserter(srcLinks),
                  [&selected](const PropertyLink& c) {
                      auto src = c.getSource()->getOwner()->getProcessor();
                      auto dst = c.getDestination()->getOwner()->getProcessor();
                      return util::contains(selected, src) && !util::contains(selected, dst);
                  });
    auto partialSrcLinks = util::transform(srcLinks, [](const PropertyLink& c) {
        return detail::PartialSrcLink{c.getSource(),
                                      joinString(c.getDestination()->getPath(), ".")};
    });

    std::vector<PropertyLink> dstLinks;
    util::copy_if(network_->getLinks(), std::back_inserter(dstLinks),
                  [&selected](const PropertyLink& c) {
                      auto src = c.getSource()->getOwner()->getProcessor();
                      auto dst = c.getDestination()->getOwner()->getProcessor();
                      return !util::contains(selected, src) && util::contains(selected, dst);
                  });
    auto partialDstLinks = util::transform(dstLinks, [](const PropertyLink& c) {
        return detail::PartialDstLink{joinString(c.getSource()->getPath(), "."),
                                      c.getDestination()};
    });

    s.serialize("ProcessorNetworkVersion", network_->getVersion());
    s.serialize("Processors", selected, "Processor");
    s.serialize("Connections", connections, "Connection");
    s.serialize("PartialInConnections", partialIn, "Connection");
    s.serialize("PropertyLinks", links, "PropertyLink");
    s.serialize("PartialSrcLinks", partialSrcLinks, "PropertyLink");
    s.serialize("PartialDstLinks", partialDstLinks, "PropertyLink");
}

void detail::PartialProcessorNetwork::deserialize(Deserializer& d) {
    try {
        std::vector<std::unique_ptr<Processor>> processors;
        std::vector<std::unique_ptr<PortConnection>> connections;
        std::vector<std::unique_ptr<detail::PartialConnection>> partialIn;
        std::vector<std::unique_ptr<PropertyLink>> links;
        std::vector<std::unique_ptr<detail::PartialSrcLink>> partialSrcLinks;
        std::vector<std::unique_ptr<detail::PartialDstLink>> partialDstLinks;
        d.deserialize("Processors", processors, "Processor");
        d.deserialize("Connections", connections, "Connection");
        d.deserialize("PartialInConnections", partialIn, "Connection");
        d.deserialize("PropertyLinks", links, "PropertyLink");
        d.deserialize("PartialSrcLinks", partialSrcLinks, "PropertyLink");
        d.deserialize("PartialDstLinks", partialDstLinks, "PropertyLink");

        for (auto p : network_->getProcessors()) {
            auto m = p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
            m->setSelected(false);
        }

        for (auto& p : processors) {
            network_->addProcessor(p.get());

            AutoLinker::addLinks(network_, p.get());

            addedProcessors_.push_back(p.get());
            p.release();
        }
        for (auto& c : connections) {
            network_->addConnection(c->getOutport(), c->getInport());
        }
        for (auto& c : partialIn) {
            auto parts = splitString(c->outportPath_, '/');
            if (parts.size() != 2) continue;
            if (auto p = network_->getProcessorByIdentifier(parts[0])) {
                if (auto outport = p->getOutport(parts[1])) {
                    network_->addConnection(outport, c->inport_);
                }
            }
        }

        for (auto& l : links) {
            network_->addLink(l->getSource(), l->getDestination());
        }
        for (auto& l : partialSrcLinks) {
            auto path = splitString(l->dstPath_, '.');
            if (auto dst = network_->getProperty(path)) {
                network_->addLink(l->src_, dst);
            }
        }
        for (auto& l : partialDstLinks) {
            auto path = splitString(l->srcPath_, '.');
            if (auto src = network_->getProperty(path)) {
                network_->addLink(src, l->dst_);
            }
        }

    } catch (Exception& e) {
        util::log(IVW_CONTEXT_CUSTOM("Paste"), e.getMessage(), LogLevel::Warn, LogAudience::User);
    }
}

detail::PartialConnection::PartialConnection(std::string path, Inport* inport)
    : outportPath_(path), inport_(inport) {}

detail::PartialConnection::PartialConnection() {}

void detail::PartialConnection::serialize(Serializer& s) const {
    s.serialize("OutPortPath", outportPath_);
    s.serialize("InPort", inport_);
}

void detail::PartialConnection::deserialize(Deserializer& d) {
    d.deserialize("OutPortPath", outportPath_);
    d.deserialize("InPort", inport_);
}

detail::PartialSrcLink::PartialSrcLink(Property* src, std::string path)
    : src_(src), dstPath_(path) {}

detail::PartialSrcLink::PartialSrcLink() {}

void detail::PartialSrcLink::serialize(Serializer& s) const {
    s.serialize("SourceProperty", src_);
    s.serialize("DestinationPropertyPath", dstPath_);
}

void detail::PartialSrcLink::deserialize(Deserializer& d) {
    d.deserialize("SourceProperty", src_);
    d.deserialize("DestinationPropertyPath", dstPath_);
}

detail::PartialDstLink::PartialDstLink(std::string path, Property* dst)
    : srcPath_(path), dst_(dst) {}

detail::PartialDstLink::PartialDstLink() {}

void detail::PartialDstLink::serialize(Serializer& s) const {
    s.serialize("SourcePropertyPath", srcPath_);
    s.serialize("DestinationProperty", dst_);
}

void detail::PartialDstLink::deserialize(Deserializer& d) {
    d.deserialize("SourcePropertyPath", srcPath_);
    d.deserialize("DestinationProperty", dst_);
}

bool addProcessorOnConnection(ProcessorNetwork* network, std::unique_ptr<Processor> processor,
                              PortConnection connection) {

    Inport* connectionInport = connection.getInport();
    Outport* connectionOutport = connection.getOutport();

    Inport* inport = util::find_if_or_null(
        processor->getInports(),
        [connectionOutport](Inport* port) { return port->canConnectTo(connectionOutport); });

    Outport* outport = util::find_if_or_null(
        processor->getOutports(),
        [connectionInport](Outport* port) { return connectionInport->canConnectTo(port); });

    if (inport && outport) {
        NetworkLock lock(network);
        network->addProcessor(std::move(processor));

        // Remove old connection
        network->removeConnection(connection);

        // Add new Connections
        network->addConnection(connectionOutport, inport);
        network->addConnection(outport, connectionInport);
        return true;
    } else {
        return false;
    }
}

void replaceProcessor(ProcessorNetwork* network, std::unique_ptr<Processor> aNewProcessor,
                      Processor* oldProcessor) {

    auto newProcessor = network->addProcessor(std::move(aNewProcessor));

    util::setPosition(newProcessor, util::getPosition(oldProcessor));

    const std::vector<Inport*>& inports = newProcessor->getInports();
    const std::vector<Outport*>& outports = newProcessor->getOutports();
    const std::vector<Inport*>& oldInports = oldProcessor->getInports();
    const std::vector<Outport*>& oldOutports = oldProcessor->getOutports();

    NetworkLock lock(network);

    std::vector<PortConnection> newConnections;

    for (size_t i = 0; i < std::min(inports.size(), oldInports.size()); ++i) {
        for (auto outport : oldInports[i]->getConnectedOutports()) {
            if (inports[i]->canConnectTo(outport)) {
                // save new connection connectionOutportoldInport-processorInport
                newConnections.emplace_back(outport, inports[i]);
            }
        }
    }

    for (size_t i = 0; i < std::min(outports.size(), oldOutports.size()); ++i) {
        for (auto inport : oldOutports[i]->getConnectedInports()) {
            if (inport->canConnectTo(outports[i])) {
                // save new connection processorOutport-connectionInport
                newConnections.emplace_back(outports[i], inport);
            }
        }
    }

    // Copy over the value of old props to new ones if id and class name are equal.
    auto newProps = newProcessor->getProperties();
    auto oldProps = oldProcessor->getProperties();

    std::map<Property*, Property*> propertymap;

    for (auto newProp : newProps) {
        for (auto oldProp : oldProps) {
            if (newProp->getIdentifier() == oldProp->getIdentifier() &&
                newProp->getClassIdentifier() == oldProp->getClassIdentifier()) {
                newProp->set(oldProp);
                propertymap[oldProp] = newProp;
            }
        }
    }

    // Move property links to the new processor
    auto links = network->getLinks();
    for (auto oldProp : oldProps) {
        for (auto link : links) {
            if (link.getDestination() == oldProp) {
                auto match = propertymap.find(oldProp);
                if (match != propertymap.end()) {
                    // add link from
                    Property* start = link.getSource();
                    // to
                    Property* end = match->second;

                    network->addLink(start, end);
                }
            }
            if (link.getSource() == oldProp) {
                auto match = propertymap.find(oldProp);
                if (match != propertymap.end()) {
                    // add link from
                    Property* start = match->second;
                    // to
                    Property* end = link.getDestination();

                    network->addLink(start, end);
                }
            }
        }
    }

    // remove old processor
    network->removeProcessor(oldProcessor);
    delete oldProcessor;

    // create all new connections
    for (auto& con : newConnections) network->addConnection(con);
}

}  // namespace util
}  // namespace inviwo
