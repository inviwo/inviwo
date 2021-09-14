/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2021 Inviwo Foundation
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

#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/linksettings.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/network/autolinker.h>
#include <inviwo/core/network/networkedge.h>

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

namespace {
struct PathPair : Serializable {
    PathPair() = default;
    PathPair(std::string asrc, std::string adst) : src{std::move(asrc)}, dst{std::move(adst)} {}
    virtual ~PathPair() = default;
    std::string src;
    std::string dst;

    virtual void serialize(Serializer& s) const override {
        s.serialize("src", src, SerializationTarget::Attribute);
        s.serialize("dst", dst, SerializationTarget::Attribute);
    }
    virtual void deserialize(Deserializer& d) override {
        d.deserialize("src", src, SerializationTarget::Attribute);
        d.deserialize("dst", dst, SerializationTarget::Attribute);
    }
};
}  // namespace

void detail::PartialProcessorNetwork::serialize(Serializer& s) const {
    std::vector<Processor*> selected;
    util::copy_if(network_->getProcessors(), std::back_inserter(selected), [](const Processor* p) {
        auto m = p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
        return m->isSelected();
    });

    std::vector<NetworkEdge> internalConnections;
    std::vector<NetworkEdge> externalConnections;
    for (const auto& connection : network_->getConnections()) {
        const auto in = connection.getInport()->getProcessor();
        const auto out = connection.getOutport()->getProcessor();
        if (util::contains(selected, in)) {
            if (util::contains(selected, out)) {
                internalConnections.emplace_back(connection);
            } else {
                externalConnections.emplace_back(connection);
            }
        }
    }

    std::vector<NetworkEdge> internalLinks;
    std::vector<NetworkEdge> outLinks;
    std::vector<NetworkEdge> inLinks;
    for (const auto& link : network_->getLinks()) {
        const auto src = link.getSource()->getOwner()->getProcessor();
        const auto dst = link.getDestination()->getOwner()->getProcessor();
        const auto srcInt = util::contains(selected, src);
        const auto dstInt = util::contains(selected, dst);

        if (srcInt && dstInt) {
            internalLinks.emplace_back(link);
        } else if (srcInt) {
            outLinks.emplace_back(link);
        } else if (dstInt) {
            inLinks.emplace_back(link);
        }
    }

    s.serialize("ProcessorNetworkVersion", network_->getVersion());
    s.serialize("Processors", selected, "Processor");
    s.serialize("InternalConnections", internalConnections, "Connection");
    s.serialize("ExternalConnections", externalConnections, "Connection");
    s.serialize("InternalPropertyLinks", internalLinks, "PropertyLink");
    s.serialize("OutPropertyLinks", outLinks, "PropertyLink");
    s.serialize("InPropertyLinks", inLinks, "PropertyLink");
}

void detail::PartialProcessorNetwork::deserialize(Deserializer& d) {
    try {
        std::vector<std::unique_ptr<Processor>> processors;
        std::vector<NetworkEdge> internalConnections;
        std::vector<NetworkEdge> externalConnections;
        std::vector<NetworkEdge> internalLinks;
        std::vector<NetworkEdge> outLinks;
        std::vector<NetworkEdge> inLinks;
        d.deserialize("Processors", processors, "Processor");
        d.deserialize("InternalConnections", internalConnections, "Connection");
        d.deserialize("ExternalConnections", externalConnections, "Connection");
        d.deserialize("InternalPropertyLinks", internalLinks, "PropertyLink");
        d.deserialize("OutPropertyLinks", outLinks, "PropertyLink");
        d.deserialize("InPropertyLinks", inLinks, "PropertyLink");

        for (auto p : network_->getProcessors()) {
            auto m = p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
            m->setSelected(false);
        }

        std::map<std::string, std::string, std::less<>> processorIds;
        for (auto& p : processors) {
            auto orgId = p->getIdentifier();
            network_->addProcessor(p.get());
            processorIds[orgId] = p->getIdentifier();
            AutoLinker::addLinks(network_, p.get());

            addedProcessors_.push_back(p.get());
            p.release();
        }

        for (auto& c : internalConnections) {
            try {
                c.updateProcessorID(processorIds);
                auto connection = c.toConnection(*network_);
                network_->addConnection(connection);
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }
        for (auto& c : externalConnections) {
            try {
                c.updateDstProcessorID(processorIds);
                auto connection = c.toConnection(*network_);
                network_->addConnection(connection);
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }

        for (auto& l : internalLinks) {
            try {
                l.updateProcessorID(processorIds);
                auto link = l.toLink(*network_);
                network_->addLink(link.getSource(), link.getDestination());
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }
        for (auto& l : outLinks) {
            try {
                l.updateSrcProcessorID(processorIds);
                auto link = l.toLink(*network_);
                network_->addLink(link.getSource(), link.getDestination());
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }
        for (auto& l : inLinks) {
            try {
                l.updateDstProcessorID(processorIds);
                auto link = l.toLink(*network_);
                network_->addLink(link.getSource(), link.getDestination());
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }

    } catch (Exception& e) {
        util::log(IVW_CONTEXT_CUSTOM("Paste"), e.getMessage(), LogLevel::Warn, LogAudience::User);
    }
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

    NetworkLock lock(network);

    std::vector<PortConnection> newConnections;
    {
        std::vector<Inport*> oldInports = oldProcessor->getInports();
        for (auto* newInport : newProcessor->getInports()) {
            auto it = std::find_if(oldInports.begin(), oldInports.end(), [&](Inport* oldInport) {
                return std::all_of(oldInport->getConnectedOutports().begin(),
                                   oldInport->getConnectedOutports().end(), [&](Outport* outport) {
                                       return newInport->canConnectTo(outport);
                                   });
            });

            if (it != oldInports.end()) {
                for (auto* outport : (*it)->getConnectedOutports()) {
                    newConnections.emplace_back(outport, newInport);
                }
                oldInports.erase(it);
            }
        }
    }

    {
        std::vector<Outport*> oldOutports = oldProcessor->getOutports();
        for (auto* newOutport : newProcessor->getOutports()) {
            auto it =
                std::find_if(oldOutports.begin(), oldOutports.end(), [&](Outport* oldOutport) {
                    return std::all_of(
                        oldOutport->getConnectedInports().begin(),
                        oldOutport->getConnectedInports().end(),
                        [&](Inport* inport) { return inport->canConnectTo(newOutport); });
                });

            if (it != oldOutports.end()) {
                for (auto* inport : (*it)->getConnectedInports()) {
                    newConnections.emplace_back(newOutport, inport);
                }
                oldOutports.erase(it);
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
