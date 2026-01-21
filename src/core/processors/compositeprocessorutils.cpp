/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

#include <inviwo/core/processors/compositeprocessorutils.h>

#include <inviwo/core/processors/compositeprocessor.h>
#include <inviwo/core/processors/compositesource.h>
#include <inviwo/core/processors/compositesink.h>

#include <inviwo/core/processors/sequenceprocessor.h>
#include <inviwo/core/processors/sequencecompositesource.h>
#include <inviwo/core/processors/sequencecompositesink.h>

#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

namespace {

constexpr std::string_view missingMessage =
    R"(Unable to find a {1} Processor for outport: "{0}"
Probably need to register a {1} Processor for the port:
registerProcessor<{2}<InportType, OutportType>>();
or use the convenience function:
registerDefaultsForDataType<DataType>();
)";

auto partitionConnections(const std::vector<PortConnection> connections,
                          const std::vector<Processor*> selected)
    -> std::tuple<std::vector<PortConnection>, std::unordered_map<Outport*, std::vector<Inport*>>,
                  std::unordered_map<Outport*, std::vector<Inport*>>> {

    std::vector<PortConnection> internal;
    std::unordered_map<Outport*, std::vector<Inport*>> inConnections;
    std::unordered_map<Outport*, std::vector<Inport*>> outConnections;

    for (const auto& c : connections) {
        auto out = util::contains(selected, c.getOutport()->getProcessor());
        auto in = util::contains(selected, c.getInport()->getProcessor());
        if (out && in) {
            internal.push_back(c);
        } else if (!out && in) {
            inConnections[c.getOutport()].push_back(c.getInport());
        } else if (out && !in) {
            outConnections[c.getOutport()].push_back(c.getInport());
        }
    }

    return {internal, inConnections, outConnections};
}

auto partitionLinks(const std::vector<PropertyLink> links, const std::vector<Processor*> selected)
    -> std::tuple<std::vector<PropertyLink>, std::unordered_map<Property*, std::vector<Property*>>,
                  std::unordered_map<Property*, std::vector<Property*>>> {

    std::vector<PropertyLink> internal;
    std::unordered_map<Property*, std::vector<Property*>> inLinks;
    std::unordered_map<Property*, std::vector<Property*>> outLinks;

    for (const auto& l : links) {
        auto out = util::contains(selected, l.getSource()->getOwner()->getProcessor());
        auto in = util::contains(selected, l.getDestination()->getOwner()->getProcessor());
        if (out && in) {
            internal.push_back(l);
        } else if (!out && in) {
            inLinks[l.getSource()].push_back(l.getDestination());
        } else if (out && !in) {
            outLinks[l.getSource()].push_back(l.getDestination());
        }
    }

    return {internal, inLinks, outLinks};
}

template <typename Source>
std::shared_ptr<Source> addMetaSource(const std::vector<Inport*>& inports,
                                      ProcessorNetwork& subNetwork, std::string_view portId,
                                      const ProcessorFactory* pf) {
    if (auto metaSource = std::dynamic_pointer_cast<Source>(
            pf->createShared(fmt::format("{}{}", portId, Source::identifierSuffix())))) {
        subNetwork.addProcessor(metaSource);
        bool optional = true;
        for (auto* inport : inports) {
            optional &= inport->isOptional();
            subNetwork.addConnection(metaSource->getOutports().front(), inport);
        }
        metaSource->getSuperInport().setOptional(optional);
        auto portIdentifier = inports.front()->getIdentifier();
        // Make first letter uppercase for readability when combined with processor
        // display name
        if (!portIdentifier.empty()) {
            portIdentifier.front() = static_cast<char>(std::toupper(portIdentifier.front()));
        }

        auto id = util::stripIdentifier(inports.front()->getProcessor()->getDisplayName() +
                                        portIdentifier);
        metaSource->getSuperInport().setIdentifier(id);

        return metaSource;
    } else {
        return nullptr;
    }
}

template <typename Sink>
std::shared_ptr<Sink> addMetaSink(Outport* outport, const std::vector<Inport*>& inports,
                                  ProcessorNetwork& subNetwork, std::string_view portId,
                                  const ProcessorFactory* pf) {
    if (auto metasink = std::dynamic_pointer_cast<Sink>(std::shared_ptr<Processor>(
            pf->createShared(fmt::format("{}{}", portId, Sink::identifierSuffix()))))) {
        subNetwork.addProcessor(metasink);
        subNetwork.addConnection(outport, metasink->getInports().front());

        auto portIdentifier = outport->getIdentifier();
        // Make first letter uppercase for readability when combined with processor
        // display name
        if (!portIdentifier.empty()) {
            portIdentifier.front() = static_cast<char>(std::toupper(portIdentifier.front()));
        }
        auto id = util::stripIdentifier(outport->getProcessor()->getDisplayName() + portIdentifier);
        metasink->getSuperOutport().setIdentifier(id);
        return metasink;
    } else {
        return nullptr;
    }
}

}  // namespace

void util::replaceSelectionWithCompositeProcessor(ProcessorNetwork& network) {
    try {
        const NetworkLock lock(&network);

        const auto selected = util::getSelected(&network);
        const auto connections = network.getConnections();
        const auto links = network.getLinks();

        auto* app = network.getApplication();
        auto* pf = app->getProcessorFactory();

        auto comp = std::make_shared<CompositeProcessor>("composite", "Composite", app);
        auto* meta = comp->createMetaData<ProcessorMetaData>(ProcessorMetaData::classIdentifier);
        auto center = util::getCenterPosition(selected);
        meta->setPosition(center);

        auto& subNetwork = comp->getSubNetwork();
        const NetworkLock subLock(&subNetwork);
        for (auto* p : selected) {
            subNetwork.addProcessor(network.removeProcessor(p));
        }
        network.addProcessor(comp);
        util::offsetPosition(selected, -center);
        util::setSelected(selected, false);

        // Connections
        const auto [internalConnections, inConnections, outConnections] =
            partitionConnections(connections, selected);

        for (auto c : internalConnections) {
            subNetwork.addConnection(c);
        }

        for (auto& [outport, inports] : inConnections) {
            const auto portId = outport->getClassIdentifier();
            if (auto metaSource =
                    addMetaSource<CompositeSourceBase>(inports, subNetwork, portId, pf)) {
                network.addConnection(outport, &metaSource->getSuperInport());
            } else {
                log::error(missingMessage, portId, "Composite Source", "CompositeSource");
            }
        }

        for (auto& [outport, inports] : outConnections) {
            const auto portId = outport->getClassIdentifier();
            if (auto metasink =
                    addMetaSink<CompositeSinkBase>(outport, inports, subNetwork, portId, pf)) {
                for (auto* inport : inports) {
                    network.addConnection(&metasink->getSuperOutport(), inport);
                }
            } else {
                log::error(missingMessage, portId, "Composite Sink", "CompositeSink");
            }
        }

        // Links
        const auto [internalLinks, inLinks, outLinks] = partitionLinks(links, selected);

        for (auto l : internalLinks) {
            subNetwork.addLink(l);
        }
        for (auto& [src, dests] : inLinks) {
            for (auto* dst : dests) {
                network.addLink(src, comp->addSuperProperty(dst));
            }
        }
        for (auto& [src, dests] : outLinks) {
            auto* superSrc = comp->addSuperProperty(src);
            for (auto* dst : dests) {
                network.addLink(superSrc, dst);
            }
        }

    } catch (const Exception& e) {
        log::exception(e);
    }
}

void util::expandCompositeProcessorIntoNetwork(CompositeProcessor& composite) {
    try {
        // Make sure we delete the composite as the last thing we do, after any locks
        auto holder = composite.shared_from_this();
        auto& network = *composite.getNetwork();
        auto& subNetwork = composite.getSubNetwork();
        const NetworkLock lock(&network);
        const NetworkLock subLock(&subNetwork);

        const auto subProcessors = subNetwork.getProcessors();
        const auto subConnections = subNetwork.getConnections();
        const auto connections = network.getConnections();

        // Gather Links
        auto subLinks = subNetwork.getLinks();
        for (auto& l : network.getLinks()) {
            if (l.getSource()->getOwner()->getProcessor() == &composite) {
                if (auto* orgSource = composite.getSubProperty(l.getSource())) {
                    network.removeLink(l);
                    subLinks.emplace_back(orgSource, l.getDestination());
                }
            } else if (l.getDestination()->getOwner()->getProcessor() == &composite) {
                if (auto* orgDest = composite.getSubProperty(l.getDestination())) {
                    network.removeLink(l);
                    subLinks.emplace_back(l.getSource(), orgDest);
                }
            }
        }

        // Move Processors
        for (auto* p : subProcessors) {
            auto* sink = dynamic_cast<CompositeSinkBase*>(p);
            auto* source = dynamic_cast<CompositeSourceBase*>(p);
            if (!sink && !source) {
                network.addProcessor(subNetwork.removeProcessor(p));
            }
        }
        auto* meta =
            composite.createMetaData<ProcessorMetaData>(ProcessorMetaData::classIdentifier);
        util::offsetPosition(subProcessors, meta->getPosition());
        util::setSelected(subProcessors, true);

        // Connections
        for (auto& c : subConnections) {
            if (auto* sink = dynamic_cast<CompositeSinkBase*>(c.getInport()->getProcessor())) {
                for (auto& con : connections) {
                    if (con.getOutport() == &sink->getSuperOutport()) {
                        network.removeConnection(con);
                        network.addConnection(c.getOutport(), con.getInport());
                    }
                }
            } else if (auto* source =
                           dynamic_cast<CompositeSourceBase*>(c.getOutport()->getProcessor())) {
                for (auto& con : connections) {
                    if (con.getInport() == &source->getSuperInport()) {
                        network.removeConnection(con);
                        network.addConnection(con.getOutport(), c.getInport());
                    }
                }
            } else {
                network.addConnection(c);
            }
        }

        // Links
        for (auto& l : subLinks) {
            network.addLink(l);
        }

        network.removeProcessor(&composite);
    } catch (const Exception& e) {
        log::exception(e);
    }
}

void util::replaceSelectionWithSequenceProcessor(ProcessorNetwork& network) {
    try {
        const NetworkLock lock(&network);

        const auto selected = util::getSelected(&network);
        const auto connections = network.getConnections();
        const auto links = network.getLinks();

        auto* app = network.getApplication();
        auto* pf = app->getProcessorFactory();

        auto comp = std::make_shared<SequenceProcessor>("sequence", "Sequence", app);
        auto* meta = comp->createMetaData<ProcessorMetaData>(ProcessorMetaData::classIdentifier);
        auto center = util::getCenterPosition(selected);
        meta->setPosition(center);

        auto& subNetwork = comp->getSubNetwork();
        const NetworkLock subLock(&subNetwork);
        for (auto* p : selected) {
            subNetwork.addProcessor(network.removeProcessor(p));
        }
        network.addProcessor(comp);
        util::offsetPosition(selected, -center);
        util::setSelected(selected, false);

        // Connections
        const auto [internalConnections, inConnections, outConnections] =
            partitionConnections(connections, selected);

        for (auto c : internalConnections) {
            subNetwork.addConnection(c);
        }

        for (auto& [outport, inports] : inConnections) {
            const auto portId = outport->getClassIdentifier();
            if (auto metaSource =
                    addMetaSource<SequenceCompositeSourceBase>(inports, subNetwork, portId, pf)) {
                // network.addConnection(outport, &metaSource->getSuperInport());
            } else {
                log::error(missingMessage, portId, "Sequence Source", "SequenceCompositeSource");
            }
        }

        for (auto& [outport, inports] : outConnections) {
            const auto portId = inports.front()->getClassIdentifier();
            if (auto metasink = addMetaSink<SequenceCompositeSinkBase>(outport, inports, subNetwork,
                                                                       portId, pf)) {
                // for (auto* inport : inports) {
                //     network.addConnection(&metasink->getSuperOutport(), inport);
                // }
            } else {
                log::error(missingMessage, portId, "Sequence Sink", "SequenceCompositeSink");
            }
        }

        // Links
        const auto [internalLinks, inLinks, outLinks] = partitionLinks(links, selected);

        for (auto l : internalLinks) {
            subNetwork.addLink(l);
        }
        for (auto& [src, dests] : inLinks) {
            for (auto* dst : dests) {
                network.addLink(src, comp->addSuperProperty(dst));
            }
        }
        for (auto& [src, dests] : outLinks) {
            auto* superSrc = comp->addSuperProperty(src);
            for (auto* dst : dests) {
                network.addLink(superSrc, dst);
            }
        }

    } catch (const Exception& e) {
        log::exception(e);
    }
}

}  // namespace inviwo
