/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

void util::replaceSelectionWithCompositeProcessor(ProcessorNetwork& network) {
    try {
        NetworkLock lock(&network);

        std::vector<Processor*> selected;
        util::copy_if(
            network.getProcessors(), std::back_inserter(selected), [](const Processor* p) {
                auto m = p->getMetaData<ProcessorMetaData>(ProcessorMetaData::classIdentifier);
                return m->isSelected();
            });

        std::vector<PortConnection> connections = network.getConnections();
        std::vector<PropertyLink> links = network.getLinks();

        auto app = network.getApplication();
        auto comp = std::make_shared<CompositeProcessor>("composite", "Composite", app);
        auto* meta = comp->createMetaData<ProcessorMetaData>(ProcessorMetaData::classIdentifier);
        auto center = util::getCenterPosition(selected);
        meta->setPosition(center);

        auto& subNetwork = comp->getSubNetwork();
        NetworkLock subLock(&subNetwork);
        for (auto p : selected) {
            subNetwork.addProcessor(network.removeProcessor(p));
        }
        network.addProcessor(comp);
        util::offsetPosition(selected, -center);
        util::setSelected(selected, false);

        // Connections
        std::unordered_map<Outport*, std::vector<Inport*>> inConnections;
        std::unordered_map<Outport*, std::vector<Inport*>> outConnections;

        auto pf = app->getProcessorFactory();
        for (auto& c : connections) {
            auto out = util::contains(selected, c.getOutport()->getProcessor());
            auto in = util::contains(selected, c.getInport()->getProcessor());
            if (out && in) {
                subNetwork.addConnection(c);
            } else if (!out && in) {
                inConnections[c.getOutport()].push_back(c.getInport());
            } else if (out && !in) {
                outConnections[c.getOutport()].push_back(c.getInport());
            }
        }

        for (auto& [outport, inports] : inConnections) {
            auto portId = outport->getClassIdentifier();

            if (auto source = pf->createShared(fmt::format("{}.metasource", portId))) {
                if (auto* metasouce = dynamic_cast<CompositeSourceBase*>(source.get())) {
                    subNetwork.addProcessor(source);
                    bool optional = true;
                    for (auto inport : inports) {
                        optional &= inport->isOptional();
                        subNetwork.addConnection(metasouce->getOutports().front(), inport);
                    }
                    metasouce->getSuperInport().setOptional(optional);
                    auto portIdentifier = inports.front()->getIdentifier();
                    // Make first letter uppercase for readability when combined with processor
                    // display name
                    if (!portIdentifier.empty()) {
                        portIdentifier.front() =
                            static_cast<char>(std::toupper(portIdentifier.front()));
                    }

                    auto id = util::stripIdentifier(
                        inports.front()->getProcessor()->getDisplayName() + portIdentifier);
                    metasouce->getSuperInport().setIdentifier(id);
                    network.addConnection(outport, &metasouce->getSuperInport());
                } else {
                    LogErrorCustom(
                        "CompositeProcessor",
                        "Unable to find a Composite Source Processor for outport: \""
                            << portId
                            << "\"\n"
                               "Probably need to register a Composite Source Processor for the "
                               "port:\n"
                               "registerProcessor<CompositeSource<InportType, OutportType>>();\n"
                               "or use the convenience function:\n"
                               "registerDefaultsForDataType<DataType>();");
                }
            } else {
                LogErrorCustom(
                    "CompositeProcessor",
                    "Unable to find a Composite Source Processor for outport: \""
                        << portId
                        << "\"\n"
                           "Probably need to register a Composite Source Processor for the port:\n"
                           "registerProcessor<CompositeSource<InportType, OutportType>>();\n"
                           "or use the convenience function:\n"
                           "registerDefaultsForDataType<DataType>();");
            }
        }

        for (auto& [outport, inports] : outConnections) {
            auto portId = outport->getClassIdentifier();

            if (auto sink = std::shared_ptr<Processor>(
                    pf->createShared(fmt::format("{}.metasink", portId)))) {
                if (auto* metasink = dynamic_cast<CompositeSinkBase*>(sink.get())) {
                    subNetwork.addProcessor(sink);
                    subNetwork.addConnection(outport, metasink->getInports().front());
                    for (auto inport : inports) {
                        network.addConnection(&metasink->getSuperOutport(), inport);
                    }
                    auto portIdentifier = outport->getIdentifier();
                    // Make first letter uppercase for readability when combined with processor
                    // display name
                    if (!portIdentifier.empty()) {
                        portIdentifier.front() =
                            static_cast<char>(std::toupper(portIdentifier.front()));
                    }
                    auto id = util::stripIdentifier(outport->getProcessor()->getDisplayName() +
                                                    portIdentifier);
                    metasink->getSuperOutport().setIdentifier(id);
                } else {
                    LogErrorCustom(
                        "CompositeProcessor",
                        "Unable to find a Composite Sink Processor for outport: \""
                            << portId
                            << "\"\n"
                               "Probably need to register a Composite Sink Processor for the "
                               "port:\n"
                               "registerProcessor<CompositeSink<InportType, OutportType>>();\n"
                               "or use the convenience function:\n"
                               "registerDefaultsForDataType<DataType>();");
                }
            } else {
                LogErrorCustom(
                    "CompositeProcessor",
                    "Unable to find a Composite Sink Processor for outport: \""
                        << portId
                        << "\"\n"
                           "Probably need to register a Composite Sink Processor for the port:\n"
                           "registerProcessor<CompositeSink<InportType, OutportType>>();\n"
                           "or use the convenience function:\n"
                           "registerDefaultsForDataType<DataType>();");
            }
        }

        // Links
        std::unordered_map<Property*, std::vector<Property*>> inLinks;
        std::unordered_map<Property*, std::vector<Property*>> outLinks;
        for (auto& l : links) {
            auto out = util::contains(selected, l.getSource()->getOwner()->getProcessor());
            auto in = util::contains(selected, l.getDestination()->getOwner()->getProcessor());
            if (out && in) {
                subNetwork.addLink(l);
            } else if (!out && in) {
                inLinks[l.getSource()].push_back(l.getDestination());
            } else if (out && !in) {
                outLinks[l.getSource()].push_back(l.getDestination());
            }
        }

        for (auto& item : inLinks) {
            for (auto p : item.second) {
                auto prop = comp->addSuperProperty(p);
                network.addLink(item.first, prop);
            }
        }

        for (auto& item : outLinks) {
            auto prop = comp->addSuperProperty(item.first);
            for (auto p : item.second) {
                network.addLink(prop, p);
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
        NetworkLock lock(&network);
        NetworkLock subLock(&subNetwork);

        std::vector<Processor*> subProcessors = subNetwork.getProcessors();
        std::vector<PortConnection> subConnections = subNetwork.getConnections();
        std::vector<PropertyLink> subLinks = subNetwork.getLinks();

        std::vector<PortConnection> connections = network.getConnections();
        // Gather Links
        for (auto& l : network.getLinks()) {
            if (l.getSource()->getOwner()->getProcessor() == &composite) {
                if (auto orgSource = composite.getSubProperty(l.getSource())) {
                    network.removeLink(l);
                    subLinks.push_back(PropertyLink(orgSource, l.getDestination()));
                }
            } else if (l.getDestination()->getOwner()->getProcessor() == &composite) {
                if (auto orgDest = composite.getSubProperty(l.getDestination())) {
                    network.removeLink(l);
                    subLinks.push_back(PropertyLink(l.getSource(), orgDest));
                }
            }
        }

        // Move Processors
        for (auto p : subProcessors) {
            auto sink = dynamic_cast<CompositeSinkBase*>(p);
            auto source = dynamic_cast<CompositeSourceBase*>(p);
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
            if (auto sink = dynamic_cast<CompositeSinkBase*>(c.getInport()->getProcessor())) {
                for (auto& con : connections) {
                    if (con.getOutport() == &sink->getSuperOutport()) {
                        network.removeConnection(con);
                        network.addConnection(c.getOutport(), con.getInport());
                    }
                }
            } else if (auto source =
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
        NetworkLock lock(&network);

        std::vector<Processor*> selected;
        util::copy_if(
            network.getProcessors(), std::back_inserter(selected), [](const Processor* p) {
                auto m = p->getMetaData<ProcessorMetaData>(ProcessorMetaData::classIdentifier);
                return m->isSelected();
            });

        std::vector<PortConnection> connections = network.getConnections();
        std::vector<PropertyLink> links = network.getLinks();

        auto app = network.getApplication();
        auto comp = std::make_shared<SequenceProcessor>("sequence", "Sequence", app);
        auto meta = comp->createMetaData<ProcessorMetaData>(ProcessorMetaData::classIdentifier);
        auto center = util::getCenterPosition(selected);
        meta->setPosition(center);

        auto& subNetwork = comp->getSubNetwork();
        NetworkLock subLock(&subNetwork);
        for (auto p : selected) {
            subNetwork.addProcessor(network.removeProcessor(p));
        }
        network.addProcessor(comp);
        util::offsetPosition(selected, -center);
        util::setSelected(selected, false);

        // Connections
        std::unordered_map<Outport*, std::vector<Inport*>> inConnections;
        std::unordered_map<Outport*, std::vector<Inport*>> outConnections;

        auto pf = app->getProcessorFactory();
        for (auto& c : connections) {
            auto out = util::contains(selected, c.getOutport()->getProcessor());
            auto in = util::contains(selected, c.getInport()->getProcessor());
            if (out && in) {
                subNetwork.addConnection(c);
            } else if (!out && in) {
                inConnections[c.getOutport()].push_back(c.getInport());
            } else if (out && !in) {
                outConnections[c.getOutport()].push_back(c.getInport());
            }
        }

        for (auto& [outport, inports] : inConnections) {
            auto portId = outport->getClassIdentifier();

            if (auto source = pf->createShared(
                    fmt::format("{}{}", portId, SequenceCompositeSourceBase::identifierSuffix()))) {
                if (auto metasouce = dynamic_cast<SequenceCompositeSourceBase*>(source.get())) {
                    subNetwork.addProcessor(source);
                    bool optional = true;
                    for (auto inport : inports) {
                        optional &= inport->isOptional();
                        subNetwork.addConnection(metasouce->getOutports().front(), inport);
                    }
                    metasouce->getSuperInport().setOptional(optional);
                    auto portIdentifier = inports.front()->getIdentifier();
                    // Make first letter uppercase for readability when combined with processor
                    // display name
                    if (!portIdentifier.empty()) {
                        portIdentifier.front() =
                            static_cast<char>(std::toupper(portIdentifier.front()));
                    }

                    auto id = util::stripIdentifier(
                        inports.front()->getProcessor()->getDisplayName() + portIdentifier);
                    metasouce->getSuperInport().setIdentifier(id);
                    // network.addConnection(c.first, &metasouce->getSuperInport());
                } else {
                    LogErrorCustom(
                        "SequenceProcessor",
                        "Unable to find a Sequence Source Processor for outport: \""
                            << portId
                            << "\"\n"
                               "Probably need to register a Sequence Source Processor for the "
                               "port:\n"
                               "registerProcessor<SequenceCompositeSource<InportType, "
                               "OutportType>>();\n"
                               "or use the convenience function:\n"
                               "registerDefaultsForDataType<DataType>();");
                }
            } else {
                LogErrorCustom(
                    "SequenceProcessor",
                    "Unable to find a Sequence Source Processor for outport: \""
                        << portId
                        << "\"\n"
                           "Probably need to register a Sequence Source Processor for the port:\n"
                           "registerProcessor<SequenceCompositeSource<InportType, "
                           "OutportType>>();\n"
                           "or use the convenience function:\n"
                           "registerDefaultsForDataType<DataType>();");
            }
        }

        for (auto& [outport, inports] : outConnections) {
            auto portId = inports.front()->getClassIdentifier();

            if (auto sink = pf->createShared(
                    fmt::format("{}{}", portId, SequenceCompositeSinkBase::identifierSuffix()))) {
                if (auto metasink = dynamic_cast<SequenceCompositeSinkBase*>(sink.get())) {
                    subNetwork.addProcessor(sink);
                    subNetwork.addConnection(outport, metasink->getInports().front());
                    // for (auto inport : inports) {
                    //  network.addConnection(&metasink->getSuperOutport(), inport);
                    //}
                    auto portIdentifier = outport->getIdentifier();
                    // Make first letter uppercase for readability when combined with processor
                    // display name
                    if (!portIdentifier.empty()) {
                        portIdentifier.front() =
                            static_cast<char>(std::toupper(portIdentifier.front()));
                    }
                    auto id = util::stripIdentifier(outport->getProcessor()->getDisplayName() +
                                                    portIdentifier);
                    metasink->getSuperOutport().setIdentifier(id);
                } else {
                    LogErrorCustom(
                        "SequenceProcessor",
                        "Unable to find a Sequence Sink Processor for outport: \""
                            << portId
                            << "\"\n"
                               "Probably need to register a Sequence Sink Processor for the "
                               "port:\n"
                               "registerProcessor<SequenceCompositeSink<InportType, "
                               "OutportType>>();\n"
                               "or use the convenience function:\n"
                               "registerDefaultsForDataType<DataType>();");
                }
            } else {
                LogErrorCustom(
                    "SequenceProcessor",
                    "Unable to find a Sequence Sink Processor for outport: \""
                        << portId
                        << "\"\n"
                           "Probably need to register a Sequence Sink Processor for the port:\n"
                           "registerProcessor<SequenceCompositeSink<InportType, OutportType>>();\n"
                           "or use the convenience function:\n"
                           "registerDefaultsForDataType<DataType>();");
            }
        }

        // Links
        std::unordered_map<Property*, std::vector<Property*>> inLinks;
        std::unordered_map<Property*, std::vector<Property*>> outLinks;
        for (auto& l : links) {
            auto out = util::contains(selected, l.getSource()->getOwner()->getProcessor());
            auto in = util::contains(selected, l.getDestination()->getOwner()->getProcessor());
            if (out && in) {
                subNetwork.addLink(l);
            } else if (!out && in) {
                inLinks[l.getSource()].push_back(l.getDestination());
            } else if (out && !in) {
                outLinks[l.getSource()].push_back(l.getDestination());
            }
        }

        for (auto& item : inLinks) {
            for (auto p : item.second) {
                auto prop = comp->addSuperProperty(p);
                network.addLink(item.first, prop);
            }
        }

        for (auto& item : outLinks) {
            auto prop = comp->addSuperProperty(item.first);
            for (auto p : item.second) {
                network.addLink(prop, p);
            }
        }

    } catch (const Exception& e) {
        log::exception(e);
    }
}

}  // namespace inviwo
