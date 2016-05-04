/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/linksettings.h>
#include <inviwo/core/links/linkconditions.h>

#include <iterator>

namespace inviwo {

bool util::ProcessorStates::hasBeenVisited(Processor* processor) const {
    return visited_.find(processor) != visited_.end();
}

void util::ProcessorStates::setProcessorVisited(Processor* processor) {
    visited_.insert(processor);
}

void util::ProcessorStates::clear() { visited_.clear(); }

std::unordered_set<Processor*> util::getDirectPredecessors(Processor* processor) {
    std::unordered_set<Processor*> predecessors;
    for (auto port : processor->getInports()) {
        for (auto connectedPort : port->getConnectedOutports()) {
            predecessors.insert(connectedPort->getProcessor());
        }
    }
    return predecessors;
}

std::unordered_set<Processor*> util::getDirectSuccessors(Processor* processor) {
    std::unordered_set<Processor*> successors;
    for (auto port : processor->getOutports()) {
        for (auto connectedPort : port->getConnectedInports()) {
            successors.insert(connectedPort->getProcessor());
        }
    }
    return successors;
}

std::unordered_set<Processor*> util::getPredecessors(Processor* processor) {
    std::unordered_set<Processor*> predecessors;
    ProcessorStates state;
    traverseNetwork<TraversalDirection::Up, VisitPattern::Post>(
        state, processor, [&predecessors](Processor* p) { predecessors.insert(p); });

    return predecessors;
}

std::unordered_set<Processor*> util::getSuccessors(Processor* processor) {
    std::unordered_set<Processor*> successors;
    ProcessorStates state;
    traverseNetwork<TraversalDirection::Down, VisitPattern::Post>(
        state, processor, [&successors](Processor* p) { successors.insert(p); });

    return successors;
}

std::vector<Processor*> util::topologicalSort(ProcessorNetwork* network) {
    // perform topological sorting and store processor order in sorted

    std::vector<Processor*> endProcessors;
    util::copy_if(network->getProcessors(), std::back_inserter(endProcessors),
                  [](Processor* p) { return p->isSink(); });

    ProcessorStates state;
    std::vector<Processor*> sorted;
    for (auto processor : endProcessors) {
        traverseNetwork<TraversalDirection::Up, VisitPattern::Post>(
            state, processor, [&sorted](Processor* p) { sorted.push_back(p); });
    }
    return sorted;
}


util::PropertyDistanceSorter::PropertyDistanceSorter() {}
void util::PropertyDistanceSorter::setTarget(const Property* target) { pos_ = getPosition(target); }

bool util::PropertyDistanceSorter::operator()(const Property* a, const Property* b) {
    float da = glm::distance(pos_, getPosition(a));
    float db = glm::distance(pos_, getPosition(b));
    return da < db;
}

vec2 util::PropertyDistanceSorter::getPosition(const Property* p) {
    auto it = cache_.find(p);
    if (it != cache_.end()) return it->second;
    return cache_[p] = getPosition(p->getOwner()->getProcessor());
}

vec2 util::PropertyDistanceSorter::getPosition(const Processor* processor) {
    if (auto meta =
            processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)) {
        return static_cast<vec2>(meta->getPosition());
    }
    return vec2(0, 0);
}

void util::autoLinkProcessor(ProcessorNetwork* network, Processor* processor) {
    auto app = network->getApplication();
    auto linkSettings = app->getSettingsByType<LinkSettings>();

    auto linkChecker = [&](const Property* p) { return !linkSettings->isLinkable(p); };

    auto allNewPropertes = processor->getPropertiesRecursive();

    std::vector<Property*> properties;
    network->forEachProcessor([&](Processor* p) {
        if (p != processor) {
            std::vector<Property*> props = p->getPropertiesRecursive();
            properties.insert(properties.end(), props.begin(), props.end());
        }
    });

    auto destprops = allNewPropertes;
    // remove properties for which auto linking is disabled
    util::erase_remove_if(properties, linkChecker);
    util::erase_remove_if(destprops, linkChecker);

    util::PropertyDistanceSorter distSorter;

    // auto link based on global settings
    for (auto& destprop : destprops) {
        std::vector<Property*> candidates = properties;

        auto isNotAutoLinkAble = [&](const Property* p) {
            return !AutoLinker::canLink(p, destprop, LinkMatchingTypeAndId);
        };
        util::erase_remove_if(candidates, isNotAutoLinkAble);

        if (candidates.size() > 0) {
            distSorter.setTarget(destprop);
            std::sort(candidates.begin(), candidates.end(), distSorter);

            network->addLink(candidates.front(), destprop);
            // Propagate the link to the new Processor.
            network->evaluateLinksFromProperty(candidates.front());
            network->addLink(destprop, candidates.front());
        }
    }

    // Auto link based property
    for (auto& destprop : allNewPropertes) {
        std::vector<Property*> candidates;
        for (auto& srcPropertyIdentifier : destprop->getAutoLinkToProperty()) {
            auto& classID = srcPropertyIdentifier.first;
            auto& path = srcPropertyIdentifier.second;

            network->forEachProcessor([&](Processor* srcProcessor) {
                if (srcProcessor != processor && srcProcessor->getClassIdentifier() == classID) {
                    if (auto src = srcProcessor->getPropertyByPath(splitString(path, '.'))) {
                        candidates.push_back(src);
                    }
                }
            });
        }

        if (candidates.size() > 0) {
            distSorter.setTarget(destprop);
            std::sort(candidates.begin(), candidates.end(), distSorter);

            network->addLink(candidates.front(), destprop);
            // Propagate the link to the new Processor.
            network->evaluateLinksFromProperty(candidates.front());
            network->addLink(destprop, candidates.front());
        }
    }
}


void util::serializeSelected(ProcessorNetwork* network, std::ostream& os,
                                          const std::string& refPath) {
    std::vector<Processor*> selected;
    util::copy_if(network->getProcessors(), std::back_inserter(selected), [](const Processor* p) {
        auto m = p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
        return m->isSelected();
    });

    std::vector<PortConnection> connections;
    util::copy_if(network->getConnections(), std::back_inserter(connections),
                  [&selected](const PortConnection& c) {
                      auto in = c.getInport()->getProcessor();
                      auto out = c.getOutport()->getProcessor();
                      return util::contains(selected, in) && util::contains(selected, out);
                  });

    std::vector<PortConnection> partialInConnections;
    util::copy_if(network->getConnections(), std::back_inserter(partialInConnections),
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
    util::copy_if(network->getLinks(), std::back_inserter(links),
                  [&selected](const PropertyLink& c) {
                      auto src = c.getSource()->getOwner()->getProcessor();
                      auto dst = c.getDestination()->getOwner()->getProcessor();
                      return util::contains(selected, src) && util::contains(selected, dst);
                  });

    std::vector<PropertyLink> srcLinks;
    util::copy_if(network->getLinks(), std::back_inserter(srcLinks),
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
    util::copy_if(network->getLinks(), std::back_inserter(dstLinks),
                  [&selected](const PropertyLink& c) {
                      auto src = c.getSource()->getOwner()->getProcessor();
                      auto dst = c.getDestination()->getOwner()->getProcessor();
                      return !util::contains(selected, src) && util::contains(selected, dst);
                  });
    auto partialDstLinks = util::transform(dstLinks, [](const PropertyLink& c) {
        return detail::PartialDstLink{joinString(c.getSource()->getPath(), "."),
                                      c.getDestination()};
    });

    Serializer xmlSerializer(refPath);
    xmlSerializer.serialize("Processors", selected, "Processor");
    xmlSerializer.serialize("Connections", connections, "Connection");
    xmlSerializer.serialize("PartialInConnections", partialIn, "Connection");
    xmlSerializer.serialize("PropertyLinks", links, "PropertyLink");
    xmlSerializer.serialize("PartialSrcLinks", partialSrcLinks, "PropertyLink");
    xmlSerializer.serialize("PartialDstLinks", partialDstLinks, "PropertyLink");

    xmlSerializer.writeFile(os);
}

std::vector<Processor*> util::appendDeserialized(ProcessorNetwork* network,
                                                              std::istream& is,
                                                              const std::string& refPath,
                                                              InviwoApplication* app) {
    NetworkLock lock(network);

    std::vector<Processor*> addedProcessors;
    try {
        Deserializer xmlDeserializer(app, is, refPath);
        std::vector<std::unique_ptr<Processor>> processors;
        std::vector<std::unique_ptr<PortConnection>> connections;
        std::vector<std::unique_ptr<detail::PartialConnection>> partialIn;
        std::vector<std::unique_ptr<PropertyLink>> links;
        std::vector<std::unique_ptr<detail::PartialSrcLink>> partialSrcLinks;
        std::vector<std::unique_ptr<detail::PartialDstLink>> partialDstLinks;
        xmlDeserializer.deserialize("Processors", processors, "Processor");
        xmlDeserializer.deserialize("Connections", connections, "Connection");
        xmlDeserializer.deserialize("PartialInConnections", partialIn, "Connection");
        xmlDeserializer.deserialize("PropertyLinks", links, "PropertyLink");
        xmlDeserializer.deserialize("PartialSrcLinks", partialSrcLinks, "PropertyLink");
        xmlDeserializer.deserialize("PartialDstLinks", partialDstLinks, "PropertyLink");

        for (auto p : network->getProcessors()) {
            auto m = p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
            m->setSelected(false);
        }

        for (auto& p : processors) {
            network->addProcessor(p.get());
            util::autoLinkProcessor(network, p.get());
            addedProcessors.push_back(p.get());
            p.release();
        }
        for (auto& c : connections) {
            network->addConnection(c->getOutport(), c->getInport());
        }
        for (auto& c : partialIn) {
            auto parts = splitString(c->outportPath_, '/');
            if (parts.size() != 2) continue;
            if (auto p = network->getProcessorByIdentifier(parts[0])) {
                if (auto outport = p->getOutport(parts[1])) {
                    network->addConnection(outport, c->inport_);
                }
            }
        }

        for (auto& l : links) {
            network->addLink(l->getSource(), l->getDestination());
        }
        for (auto& l : partialSrcLinks) {
            auto path = splitString(l->dstPath_, '.');
            if (auto dst = network->getProperty(path)) {
                network->addLink(l->src_, dst);
            }
        }
        for (auto& l : partialDstLinks) {
            auto path = splitString(l->srcPath_, '.');
            if (auto src = network->getProperty(path)) {
                network->addLink(src, l->dst_);
            }
        }

    } catch (Exception& e) {
        util::log(IvwContextCustom("Paste"), e.getMessage(), LogLevel::Warn, LogAudience::User);
    }
    return addedProcessors;
}


}  // namespace
