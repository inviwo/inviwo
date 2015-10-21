/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/settings/linksettings.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/network/processornetworkconverter.h>
#include <algorithm>

namespace inviwo {

ProcessorNetwork::ProcessorNetwork()
    : ProcessorNetworkObservable()
    , ProcessorObserver()
    , modified_(true)
    , locked_(0)
    , deserializing_(false)
    , linkEvaluator_(this) {}

ProcessorNetwork::~ProcessorNetwork() {
    lock();
    clear();
}

bool ProcessorNetwork::addProcessor(Processor* processor) {
    NetworkLock lock(this);

    if (!InviwoApplication::getPtr()->checkIfAllTagsAreSupported(processor->getTags())) {
        LogNetworkWarn("Processor '" << processor->getDisplayName()
                                     << "' was considered as not supported by the application.");
        return false;
    }

    notifyObserversProcessorNetworkWillAddProcessor(processor);
    processors_[processor->getIdentifier()] = processor;
    processor->ProcessorObservable::addObserver(this);
    processor->invalidate(INVALID_RESOURCES);
    modified();
    notifyObserversProcessorNetworkDidAddProcessor(processor);
    return true;
}

void ProcessorNetwork::removeProcessor(Processor* processor) {
    if (!processor) return;
    NetworkLock lock(this);

    // Remove all connections for this processor
    PortConnectionMap connections = connections_;
    for (auto& connection : connections)
        if (connection.second->involvesProcessor(processor))
            removeConnection(connection.second->getOutport(), connection.second->getInport());

    for (auto outport : processor->getOutports()) {
        for (auto inport : outport->getConnectedInports()) {
            removeConnection(outport, inport);
        }
    }
    for (auto inport : processor->getInports()) {
        for (auto outport : inport->getConnectedOutports()) {
            removeConnection(outport, inport);
        }
    }

    // Remove all links for this processor
    PropertyLinkMap propertyLinks = links_;
    for (auto& propertyLink : propertyLinks) {
        if (propertyLink.second->getSourceProperty()->getOwner()->getProcessor() == processor ||
            propertyLink.second->getDestinationProperty()->getOwner()->getProcessor() ==
                processor) {
            removeLink(propertyLink.second->getSourceProperty(),
                       propertyLink.second->getDestinationProperty());
        }
    }

    // remove processor itself
    notifyObserversProcessorNetworkWillRemoveProcessor(processor);
    processors_.erase(processor->getIdentifier());
    processor->ProcessorObservable::removeObserver(this);
    modified();
    notifyObserversProcessorNetworkDidRemoveProcessor(processor);
}

void ProcessorNetwork::removeAndDeleteProcessor(Processor* processor) {
    if (!processor) return;

    removeProcessor(processor);
    if (processor->isInitialized()) processor->deinitialize();
    
    delete processor;
}

Processor* ProcessorNetwork::getProcessorByIdentifier(std::string identifier) const {
    return util::map_find_or_null(processors_, identifier);
}

std::vector<Processor*> ProcessorNetwork::getProcessors() const {
    return util::transform(processors_,
                           [](ProcessorMap::const_reference elem) { return elem.second; });
}

PortConnection* ProcessorNetwork::addConnection(Outport* sourcePort, Inport* destPort) {
    PortConnection* connection = getConnection(sourcePort, destPort);

    if (!connection && sourcePort && destPort && destPort->canConnectTo(sourcePort)) {
        NetworkLock lock(this);
        connection = new PortConnection(sourcePort, destPort);
        notifyObserversProcessorNetworkWillAddConnection(connection);
        
        connections_[std::make_pair(sourcePort, destPort)] = connection;
        modified();
        destPort->connectTo(sourcePort);
        
        notifyObserversProcessorNetworkDidAddConnection(connection);
    }

    return connection;
}

void ProcessorNetwork::removeConnection(Outport* sourcePort, Inport* destPort) {
    auto itm = connections_.find(std::make_pair(sourcePort, destPort));
    if (itm != connections_.end()) {
        NetworkLock lock(this);
        PortConnection* connection = itm->second;
        notifyObserversProcessorNetworkWillRemoveConnection(connection);

        modified();
        destPort->disconnectFrom(sourcePort);
        connections_.erase(itm);

        notifyObserversProcessorNetworkDidRemoveConnection(connection);
        delete connection;
    }
}

bool ProcessorNetwork::isConnected(Outport* sourcePort, Inport* destPort) {
    return getConnection(sourcePort, destPort) != nullptr;
}

PortConnection* ProcessorNetwork::getConnection(Outport* sourcePort, Inport* destPort) {
    return util::map_find_or_null(connections_, std::make_pair(sourcePort, destPort));
}

std::vector<PortConnection*> ProcessorNetwork::getConnections() const {
    return util::transform(
        connections_, [](PortConnectionMap::const_reference elem) { return elem.second; });
}

PropertyLink* ProcessorNetwork::addLink(Property* sourceProperty, Property* destinationProperty) {
    auto link = getLink(sourceProperty, destinationProperty);
    if (!link) {
        NetworkLock lock(this);
        link = new PropertyLink(sourceProperty, destinationProperty);
        notifyObserversProcessorNetworkWillAddLink(link);
        links_[std::make_pair(sourceProperty, destinationProperty)] = link;
        linkEvaluator_.addLink(link);  // add to cache
        modified();
        notifyObserversProcessorNetworkDidAddLink(link);
    }
    return link;
}

void ProcessorNetwork::removeLink(Property* sourceProperty, Property* destinationProperty) {
    auto it = links_.find(std::make_pair(sourceProperty, destinationProperty));
    if (it != links_.end()) {
        NetworkLock lock(this);
        PropertyLink* link = it->second;
        notifyObserversProcessorNetworkWillRemoveLink(link);
        linkEvaluator_.removeLink(link);
        links_.erase(it);
        modified();
        notifyObserversProcessorNetworkDidRemoveLink(link);
        delete link;
    }
}

bool ProcessorNetwork::isLinked(Property* sourceProperty, Property* destinationProperty) {
    return getLink(sourceProperty, destinationProperty) != nullptr;
}

PropertyLink* ProcessorNetwork::getLink(Property* src,
                                        Property* dst) const {
    return util::map_find_or_null(links_, std::make_pair(src, dst));
}

std::vector<PropertyLink*> ProcessorNetwork::getLinks() const {
    return util::transform(
        links_, [](PropertyLinkMap::const_reference elem) { return elem.second; });
}

bool ProcessorNetwork::isLinkedBidirectional(Property* startProperty, Property* endProperty) {
    return isLinked(startProperty, endProperty) && isLinked(endProperty, startProperty);
}

std::vector<Property*> ProcessorNetwork::getPropertiesLinkedTo(Property* property) {
    return linkEvaluator_.getPropertiesLinkedTo(property);
}

std::vector<PropertyLink*> ProcessorNetwork::getLinksBetweenProcessors(Processor* p1,
                                                                       Processor* p2) {
    return linkEvaluator_.getLinksBetweenProcessors(p1, p2);
}

struct LinkCheck {
    LinkCheck() : linkSettings_(*InviwoApplication::getPtr()->getSettingsByType<LinkSettings>()) {}
    bool operator()(const Property* p) const { return !linkSettings_.isLinkable(p); }

private:
    const LinkSettings& linkSettings_;
};

struct AutoLinkCheck {
    AutoLinkCheck(const Property* p, LinkingConditions linkCondition)
        : property_(p), linkCondition_(linkCondition) {}
    bool operator()(const Property* p) const {
        return !AutoLinker::canLink(p, property_, linkCondition_);
    }

private:
    const Property* property_;
    LinkingConditions linkCondition_;
};

struct AutoLinkSort {
    AutoLinkSort(const Property* p) { pos_ = getPosition(p); }

    bool operator()(const Property* a, const Property* b) {
        // TODO Figure out which candidate is best.
        // using distance now
        float da = glm::distance(pos_, getPosition(a));
        float db = glm::distance(pos_, getPosition(b));
        return da < db;
    }

private:
    vec2 pos_;
    std::map<const Property*, vec2> cache_;

    vec2 getPosition(const Property* p) {
        std::map<const Property*, vec2>::const_iterator it = cache_.find(p);
        if (it != cache_.end()) return it->second;
        return cache_[p] = getPosition(p->getOwner()->getProcessor());
    }

    vec2 getPosition(const Processor* processor) {
        if (auto meta =
                processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER)) {
            return static_cast<vec2>(meta->getPosition());
        } else {
            LogWarnCustom("getProcessorPosition",
                          "No ProcessorMetaData for added processor found while auto linking");
            return vec2(0, 0);
        }
        return vec2(0, 0);
    }
};

void ProcessorNetwork::autoLinkProcessor(Processor* processor) {
    LinkCheck linkChecker;

    std::vector<Property*> destprops = processor->getPropertiesRecursive();

    // remove properties for which auto linking is disabled
    util::erase_remove_if(destprops, linkChecker);
    // no candidates for auto linking in the new processor
    if (destprops.size() == 0) return;

    std::vector<Property*> properties;
    for (auto& elem : processors_) {
        if (elem.second != processor) {
            std::vector<Property*> p = elem.second->getPropertiesRecursive();
            properties.insert(properties.end(), p.begin(), p.end());
        }
    }

    // remove properties for which auto linking is disabled
    util::erase_remove_if(properties, linkChecker);
    // no candidates for auto linking in the new processor
    if (properties.size() == 0) return;

    for (auto& destprop : destprops) {
        std::vector<Property*> candidates = properties;
        AutoLinkCheck autoLinkChecker(destprop, LinkMatchingTypeAndId);

        util::erase_remove_if(candidates, autoLinkChecker);

        AutoLinkSort sorter(destprop);
        std::sort(candidates.begin(), candidates.end(), sorter);

        if (candidates.size() > 0) {
            addLink(candidates.front(), destprop);
            // Propagate the link to the new Processor.
            linkEvaluator_.evaluateLinksFromProperty(candidates.front());
            addLink(destprop, candidates.front());
        }
    }
}

void ProcessorNetwork::evaluateLinksFromProperty(Property* source) {
    linkEvaluator_.evaluateLinksFromProperty(source);
}


void ProcessorNetwork::clear() {
    NetworkLock lock(this);

    // make sure the pool is not doing any work.
    InviwoApplication::getPtr()->waitForPool();

    std::vector<Processor*> processors = getProcessors();
    // Invalidate inports to alert processors that they should stop their calculations.
    for (auto processor : processors) {
        for (auto inport : processor->getInports()) inport->invalidate(INVALID_OUTPUT);
    }

    for (auto processor : processors) {
        RenderContext::getPtr()->activateDefaultRenderContext();
        removeAndDeleteProcessor(processor);
    }
}

void ProcessorNetwork::modified() { modified_ = true; }

void ProcessorNetwork::setModified(bool modified) { modified_ = modified; }

bool ProcessorNetwork::isModified() const { return modified_; }

bool ProcessorNetwork::isInvalidating() const { return !processorsInvalidating_.empty(); }

bool ProcessorNetwork::isLinking() const { return linkEvaluator_.isLinking(); }

void ProcessorNetwork::onProcessorInvalidationBegin(Processor* p) {
    util::push_back_unique(processorsInvalidating_, p);
}

void ProcessorNetwork::onProcessorInvalidationEnd(Processor* p) {
    util::erase_remove(processorsInvalidating_, p);

    if (processorsInvalidating_.empty()) {
        notifyObserversProcessorNetworkEvaluateRequest();
    }
}

void ProcessorNetwork::onProcessorRequestEvaluate(Processor*) {
    notifyObserversProcessorNetworkEvaluateRequest();
}

void ProcessorNetwork::onProcessorIdentifierChange(Processor* processor) {
    util::map_erase_remove_if(processors_, [processor](ProcessorMap::const_reference elem) {
        return elem.second == processor;
    });

    processors_[processor->getIdentifier()] = processor;
}

Processor* ProcessorNetwork::getInvalidationInitiator() {
    if (processorsInvalidating_.empty())
        return nullptr;
    else
        return processorsInvalidating_[0];
}


void ProcessorNetwork::onAboutPropertyChange(Property* modifiedProperty) {
    if (modifiedProperty) linkEvaluator_.evaluateLinksFromProperty(modifiedProperty);
    notifyObserversProcessorNetworkChanged();
}

void ProcessorNetwork::serialize(IvwSerializer& s) const {
    s.serialize("ProcessorNetworkVersion", processorNetworkVersion_);
    s.serialize("Processors", getProcessors(), "Processor");
    s.serialize("Connections", getConnections(), "Connection");
    s.serialize("PropertyLinks", getLinks(), "PropertyLink");

    InviwoSetupInfo info(InviwoApplication::getPtr());
    s.serialize("InviwoSetup", info);
}

const int ProcessorNetwork::processorNetworkVersion_ = 11;

void ProcessorNetwork::deserialize(IvwDeserializer& d) {
    // This will set deserializing_ to true while keepTrueWillAlive is in scope
    // and set it to false no matter how we leave the scope
    util::KeepTrueWhileInScope keepTrueWillAlive(&deserializing_);

    int version = 0;
    d.deserialize("ProcessorNetworkVersion", version);

    if (version != processorNetworkVersion_) {
        LogNetworkWarn("Loading old workspace ("
                       << d.getFileName() << ") version: " << version
                       << ". Updating to version: " << processorNetworkVersion_);
        ProcessorNetworkConverter nv(version);
        d.convertVersion(&nv);
    }

    InviwoSetupInfo info;
    d.deserialize("InviwoSetup", info);

    ErrorHandle errorHandle(info);

    // Processors
    try {
        DeserializationErrorHandle<ErrorHandle> processor_err(d, "Processor", &errorHandle,
                                                              &ErrorHandle::handleProcessorError);
        DeserializationErrorHandle<ErrorHandle> inport_err(d, "InPort", &errorHandle,
                                                           &ErrorHandle::handlePortError);
        DeserializationErrorHandle<ErrorHandle> outport_err(d, "OutPort", &errorHandle,
                                                            &ErrorHandle::handlePortError);

        std::vector<std::unique_ptr<Processor>> processors;
        d.deserialize("Processors", processors, "Processor");
        for (size_t i = 0; i < processors.size(); ++i) {
            if (processors[i]) {
                addProcessor(processors[i].release());
            } else {
                LogNetworkWarn("Failed deserialization: Processor Nr." << i);
            }
        }
    } catch (const SerializationException& exception) {
        clear();
        throw AbortException("DeSerialization exception " + exception.getMessage(),
                             exception.getContext());
    } catch (Exception& exception) {
        clear();
        throw AbortException("Error deserializing network " + exception.getMessage(),
                             exception.getContext());
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception.", IvwContext);
    }

    // Connections
    try {
        std::vector<std::unique_ptr<PortConnection>> portConnections;
        DeserializationErrorHandle<ErrorHandle> connection_err(d, "Connection", &errorHandle,
                                                               &ErrorHandle::handleConnectionError);
        d.deserialize("Connections", portConnections, "Connection");

        for (size_t i = 0; i < portConnections.size(); i++) {
            if (portConnections[i]) {
                Outport* outPort = portConnections[i]->getOutport();
                Inport* inPort = portConnections[i]->getInport();

                if (!(outPort && inPort && addConnection(outPort, inPort))) {
                    LogNetworkWarn("Unable to establish port connection Nr." << i);
                }
            } else {
                LogNetworkWarn("Failed deserialization: Port Connection Nr." << i);
            }
        }
    } catch (const SerializationException& exception) {
        throw IgnoreException("DeSerialization Exception " + exception.getMessage(),
                              exception.getContext());
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception.", IvwContext);
    }

    // Links
    try {
        std::vector<std::unique_ptr<PropertyLink>> propertyLinks;
        DeserializationErrorHandle<ErrorHandle> connection_err(d, "PropertyLink", &errorHandle,
                                                               &ErrorHandle::handleLinkError);
        d.deserialize("PropertyLinks", propertyLinks, "PropertyLink");

        for (size_t j = 0; j < propertyLinks.size(); j++) {
            if (propertyLinks[j]) {
                Property* srcProperty = propertyLinks[j]->getSourceProperty();
                Property* destProperty = propertyLinks[j]->getDestinationProperty();

                if (!(srcProperty && destProperty && addLink(srcProperty, destProperty))) {
                    LogNetworkWarn("Unable to establish property link Nr: " << j);
                }
            } else {
                LogNetworkWarn("Unable to establish property link Nr: " << j);
            }
        }

        if (!errorHandle.messages.empty()) {
            LogNetworkWarn("There were errors while loading workspace: " + d.getFileName() + "\n" +
                           joinString(errorHandle.messages, "\n"));
        }

    } catch (const SerializationException& exception) {
        throw IgnoreException("DeSerialization Exception " + exception.getMessage(),
                              exception.getContext());
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception.", IvwContext);
    }

    notifyObserversProcessorNetworkChanged();
}

bool ProcessorNetwork::isDeserializing() const { return deserializing_; }

Property* ProcessorNetwork::getProperty(std::vector<std::string> path) const {
    if (path.size() >= 2) {
        Processor* processor = getProcessorByIdentifier(path[0]);
        if (processor) {
            std::vector<std::string> propPath(path.begin() + 1, path.end());
            return processor->getPropertyByPath(propPath);
        }
    }
    return nullptr;
}

}  // namespace
