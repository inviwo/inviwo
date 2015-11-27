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

#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/metadata/processormetadata.h>
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
                  [](Processor* p) { return p->isEndProcessor(); });

    ProcessorStates state;
    std::vector<Processor*> sorted;
    for (auto processor : endProcessors) {
        traverseNetwork<TraversalDirection::Up, VisitPattern::Post>(
            state, processor, [&sorted](Processor* p) { sorted.push_back(p); });
    }
    return sorted;
}

IVW_CORE_API void util::serializeSelected(ProcessorNetwork* network, std::ostream& os,
                                          const std::string& refPath) {
    std::vector<Processor*> selected;
    util::copy_if(network->getProcessors(), std::back_inserter(selected), [](const Processor* p) {
        auto m = p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
        return m->isSelected();
    });

    std::vector<PortConnection*> connections;
    util::copy_if(network->getConnections(), std::back_inserter(connections),
                  [&selected](PortConnection* c) {
                      auto in = c->getInport()->getProcessor();
                      auto out = c->getOutport()->getProcessor();
                      return util::contains(selected, in) && util::contains(selected, out);
                  });

    std::vector<PortConnection*> partialInConnections;
    util::copy_if(network->getConnections(), std::back_inserter(partialInConnections),
                  [&selected](PortConnection* c) {
                      auto in = c->getInport()->getProcessor();
                      auto out = c->getOutport()->getProcessor();
                      return util::contains(selected, in) && !util::contains(selected, out);
                  });

    auto partialIn = util::transform(partialInConnections, [](PortConnection* c) {
        return detail::PartialConnection{c->getOutport()->getProcessor()->getIdentifier() + "/" +
                                             c->getOutport()->getIdentifier(),
                                         c->getInport()};
    });

    std::vector<PropertyLink*> links;
    util::copy_if(network->getLinks(), std::back_inserter(links), [&selected](PropertyLink* c) {
        auto src = c->getSourceProperty()->getOwner()->getProcessor();
        auto dst = c->getDestinationProperty()->getOwner()->getProcessor();
        return util::contains(selected, src) && util::contains(selected, dst);
    });

    std::vector<PropertyLink*> srcLinks;
    util::copy_if(network->getLinks(), std::back_inserter(srcLinks), [&selected](PropertyLink* c) {
        auto src = c->getSourceProperty()->getOwner()->getProcessor();
        auto dst = c->getDestinationProperty()->getOwner()->getProcessor();
        return util::contains(selected, src) && !util::contains(selected, dst);
    });
    auto partialSrcLinks = util::transform(srcLinks, [](PropertyLink* c) {
        return detail::PartialSrcLink{c->getSourceProperty(),
                                      joinString(c->getDestinationProperty()->getPath(), ".")};
    });

    std::vector<PropertyLink*> dstLinks;
    util::copy_if(network->getLinks(), std::back_inserter(dstLinks), [&selected](PropertyLink* c) {
        auto src = c->getSourceProperty()->getOwner()->getProcessor();
        auto dst = c->getDestinationProperty()->getOwner()->getProcessor();
        return !util::contains(selected, src) && util::contains(selected, dst);
    });
    auto partialDstLinks = util::transform(dstLinks, [](PropertyLink* c) {
        return detail::PartialDstLink{joinString(c->getSourceProperty()->getPath(), "."),
                                      c->getDestinationProperty()};
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

IVW_CORE_API std::vector<Processor*> util::appendDeserialized(ProcessorNetwork* network, std::istream& is,
                                           const std::string& refPath) {
    std::vector<Processor*> addedProcessors;
    try {
        Deserializer xmlDeserializer(is, refPath);
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
            network->autoLinkProcessor(p.get());
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
            network->addLink(l->getSourceProperty(), l->getDestinationProperty());
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
