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

ProcessorNetwork::ProcessorNetwork(InviwoApplication* application)
    : ProcessorNetworkObservable()
    , ProcessorObserver()
    , PropertyOwnerObserver()
    , application_(application)
    , linkEvaluator_(this) {}

ProcessorNetwork::~ProcessorNetwork() {
    lock();
    clear();
}

bool ProcessorNetwork::addProcessor(Processor* processor) {
    NetworkLock lock(this);

    notifyObserversProcessorNetworkWillAddProcessor(processor);
    processors_[processor->getIdentifier()] = processor;
    processor->setNetwork(this);
    processor->ProcessorObservable::addObserver(this);
    addPropertyOwnerObservation(processor);
    
    if (auto widget = application_->getProcessorWidgetFactory()->create(processor)) {
        processor->setProcessorWidget(std::move(widget));
    }
    
    processor->invalidate(InvalidationLevel::InvalidResources);
    modified();
    notifyObserversProcessorNetworkDidAddProcessor(processor);
    return true;
}

void ProcessorNetwork::removeProcessor(Processor* processor) {
    if (!processor) return;
    NetworkLock lock(this);

    // Remove all connections for this processor 
    for (auto outport : processor->getOutports()) {
        std::vector<Inport*> inports = outport->getConnectedInports();
        for (auto inport : inports) {
            removeConnection(outport, inport);
        }
    }
    for (auto inport : processor->getInports()) {
        std::vector<Outport*> outports = inport->getConnectedOutports();
        for (auto outport : outports) {
            removeConnection(outport, inport);
        }
    }

    // Remove all links for this processor
    auto toDelete = util::copy_if(links_, [&](const PropertyLink& link) {
        return link.involves(processor);
    });
    for (auto& link : toDelete) {
        removeLink(link.getSource(), link.getDestination());
    }

    // remove processor itself
    notifyObserversProcessorNetworkWillRemoveProcessor(processor);
    processors_.erase(processor->getIdentifier());
    processor->ProcessorObservable::removeObserver(this);
    removePropertyOwnerObservation(processor);
    processor->setNetwork(nullptr);
    processor->setProcessorWidget(nullptr);
    modified();
    notifyObserversProcessorNetworkDidRemoveProcessor(processor);
}

void ProcessorNetwork::removeAndDeleteProcessor(Processor* processor) {
    if (!processor) return;
    removeProcessor(processor);
    delete processor;
}

Processor* ProcessorNetwork::getProcessorByIdentifier(std::string identifier) const {
    return util::map_find_or_null(processors_, identifier);
}

std::vector<Processor*> ProcessorNetwork::getProcessors() const {
    return util::transform(processors_,
                           [](ProcessorMap::const_reference elem) { return elem.second; });
}

void ProcessorNetwork::addConnection(const PortConnection& connection) {
    addConnection(connection.getOutport(), connection.getInport());
}
void ProcessorNetwork::addConnection(Outport* src, Inport* dst) {
    if (!isPortInNetwork(src)) throw Exception("Outport not found in network");
    if (!isPortInNetwork(dst)) throw Exception("Inport not found in network");
    
    if (src && dst && !isConnected(src, dst) && dst->canConnectTo(src)) {
        NetworkLock lock(this);

        PortConnection connection(src, dst);
        notifyObserversProcessorNetworkWillAddConnection(connection);

        connections_.emplace(src, dst);
        connectionsVec_.emplace_back(src, dst);
        modified();
        
        dst->connectTo(src);

        notifyObserversProcessorNetworkDidAddConnection(connection);
    }
}

void ProcessorNetwork::removeConnection(const PortConnection& connection){
    auto it = connections_.find(connection);
    if (it != connections_.end()) {
        NetworkLock lock(this);

        notifyObserversProcessorNetworkWillRemoveConnection(connection);

        modified();
        connection.getInport()->disconnectFrom(connection.getOutport());
        connections_.erase(it);
        util::erase_remove(connectionsVec_, connection);

        notifyObserversProcessorNetworkDidRemoveConnection(connection);
    }
}
void ProcessorNetwork::removeConnection(Outport* src, Inport* dst) {
    PortConnection connection(src, dst);
    removeConnection(connection);
}

bool ProcessorNetwork::isConnected(const PortConnection& connection) const {
     return connections_.find(connection) != connections_.end();
}
bool ProcessorNetwork::isConnected(Outport* src, Inport* dst) const {
    return isConnected(PortConnection(src, dst));
}

const std::vector<PortConnection>& ProcessorNetwork::getConnections() const {
    return connectionsVec_;
}

bool ProcessorNetwork::isPortInNetwork(Port* port) const {
    if (auto processor = port->getProcessor()) {
        if (processor == getProcessorByIdentifier(processor->getIdentifier())) {
            return true;
        }
    }
    return false;
}

void ProcessorNetwork::addLink(const PropertyLink& link) {
    addLink(link.getSource(), link.getDestination());
}
void ProcessorNetwork::addLink(Property* src, Property* dst) {
    if (!isPropertyInNetwork(src)) throw Exception("Source property not found in network");
    if (!isPropertyInNetwork(dst)) throw Exception("Destination property not found in network");

    if (!isLinked(src, dst)) {
        NetworkLock lock(this);
        PropertyLink link(src, dst);
        notifyObserversProcessorNetworkWillAddLink(link);
        links_.insert(link);
        linkEvaluator_.addLink(link);  // add to cache
        modified();
        notifyObserversProcessorNetworkDidAddLink(link);
    }
}

void ProcessorNetwork::removeLink(const PropertyLink& link) {
    auto it = links_.find(link);
    if (it != links_.end()) {
        NetworkLock lock(this);
        notifyObserversProcessorNetworkWillRemoveLink(link);
        linkEvaluator_.removeLink(link);
        links_.erase(it);
        modified();
        notifyObserversProcessorNetworkDidRemoveLink(link);
    }
}
void ProcessorNetwork::removeLink(Property* src, Property* dst) {
    PropertyLink link(src, dst);
    removeLink(link);
}

void ProcessorNetwork::onWillRemoveProperty(Property* property, size_t index) {
    if (auto comp = dynamic_cast<PropertyOwner*>(property)) {
        size_t index = 0;
        for (auto p : comp->getProperties()) {
            onWillRemoveProperty(p, index);
            index++;
        }
    }

    auto toDelete =
        util::copy_if(links_, [&](const PropertyLink& link) { return link.involves(property); });
    for (auto& link : toDelete) removeLink(link);
}

bool ProcessorNetwork::isLinked(const PropertyLink& link) const {
    return links_.find(link) != links_.end();
}
bool ProcessorNetwork::isLinked(Property* src, Property* dst) const {
    return isLinked(PropertyLink(src, dst));
}

std::vector<PropertyLink> ProcessorNetwork::getLinks() const {
    return util::transform(links_, [](PropertyLinks::const_reference elem) { return elem; });
}

bool ProcessorNetwork::isLinkedBidirectional(Property* src, Property* dst) {
    return isLinked(src, dst) && isLinked(dst, src);
}

std::vector<Property*> ProcessorNetwork::getPropertiesLinkedTo(Property* property) {
    return linkEvaluator_.getPropertiesLinkedTo(property);
}

std::vector<PropertyLink> ProcessorNetwork::getLinksBetweenProcessors(Processor* p1,
                                                                      Processor* p2) {
    return linkEvaluator_.getLinksBetweenProcessors(p1, p2);
}

struct LinkCheck {
    LinkCheck(const LinkSettings& settings) : linkSettings_(settings) {}
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
    LinkCheck linkChecker(*(application_->getSettingsByType<LinkSettings>()));

    std::vector<Property*> allNewPropertes = processor->getPropertiesRecursive();

    std::vector<Property*> properties;
    for (auto& elem : processors_) {
        if (elem.second != processor) {
            std::vector<Property*> p = elem.second->getPropertiesRecursive();
            properties.insert(properties.end(), p.begin(), p.end());
        }
    }

    auto destprops = allNewPropertes;
    // remove properties for which auto linking is disabled
    util::erase_remove_if(properties, linkChecker);
    util::erase_remove_if(destprops, linkChecker);

    // auto link based on global settings
    for (auto& destprop : destprops) {
        std::vector<Property*> candidates = properties;
        AutoLinkCheck autoLinkChecker(destprop, LinkMatchingTypeAndId);

        util::erase_remove_if(candidates, autoLinkChecker);

        if (candidates.size() > 0) {
            AutoLinkSort sorter(destprop);
            std::sort(candidates.begin(), candidates.end(), sorter);

            addLink(candidates.front(), destprop);
            // Propagate the link to the new Processor.
            linkEvaluator_.evaluateLinksFromProperty(candidates.front());
            addLink(destprop, candidates.front());
        }
    }

    // Auto link based property
    for (auto& destprop : allNewPropertes) {
        std::vector<Property*> candidates;
        for (auto& srcPropertyIdentifier : destprop->getAutoLinkToProperty()) {
            for (auto& srcProcessor : processors_) {
                if (srcProcessor.second != processor &&
                    srcProcessor.second->getClassIdentifier() == srcPropertyIdentifier.first) {
                    auto srcProperty = srcProcessor.second->getPropertyByPath(
                        splitString(srcPropertyIdentifier.second, '.'));
                    if (srcProperty) {
                        candidates.push_back(srcProperty);
                    }
                }
            }
        }

        if (candidates.size() > 0) {
            AutoLinkSort sorter(destprop);
            std::sort(candidates.begin(), candidates.end(), sorter);

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
    application_->waitForPool();

    std::vector<Processor*> processors = getProcessors();
    // Invalidate inports to alert processors that they should stop their calculations.
    for (auto processor : processors) {
        for (auto inport : processor->getInports())
            inport->invalidate(InvalidationLevel::InvalidOutput);
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

void ProcessorNetwork::onProcessorPortRemoved(Processor*, Port* port) {
    auto toDelete = util::copy_if(connectionsVec_, [&](const PortConnection& item) {
        return item.getInport() == port || item.getOutport() == port;
    });
    for (auto& item : toDelete) {
        removeConnection(item.getOutport(), item.getInport());
    }
}

void ProcessorNetwork::onAboutPropertyChange(Property* modifiedProperty) {
    if (modifiedProperty) linkEvaluator_.evaluateLinksFromProperty(modifiedProperty);
    notifyObserversProcessorNetworkChanged();
}

void ProcessorNetwork::serialize(Serializer& s) const {
    s.serialize("ProcessorNetworkVersion", processorNetworkVersion_);
    s.serialize("Processors", getProcessors(), "Processor");
    s.serialize("Connections", getConnections(), "Connection");
    s.serialize("PropertyLinks", getLinks(), "PropertyLink");

    InviwoSetupInfo info(application_);
    s.serialize("InviwoSetup", info);
}

void ProcessorNetwork::addPropertyOwnerObservation(PropertyOwner* po) {
    po->addObserver(this);
    for(auto child : po->getCompositeProperties()){
        addPropertyOwnerObservation(child);
    }
}

void ProcessorNetwork::removePropertyOwnerObservation(PropertyOwner* po) {
    po->removeObserver(this);
    for (auto child : po->getCompositeProperties()) {
        removePropertyOwnerObservation(child);
    }
}

const int ProcessorNetwork::processorNetworkVersion_ = 12;

void ProcessorNetwork::deserialize(Deserializer& d) {
    NetworkLock lock(this);
    
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

    DeserializationErrorHandle<ErrorHandle> errorHandle(d, info, d);

    // Processors
    try {
        RenderContext::getPtr()->activateDefaultRenderContext();

        auto des =
            util::MapDeserializer<std::string, Processor*>("Processors", "Processor", "identifier")
                .setMakeNew([]() { return nullptr; })
                .onNew([&](const std::string& id, Processor*& p) { addProcessor(p); })
                .onRemove([&](const std::string& id) {
                    removeAndDeleteProcessor(getProcessorByIdentifier(id));
                });
        des(d, processors_);

    } catch (const SerializationException& exception) {
        clear();
        throw AbortException("Deserialization error: " + exception.getMessage(),
                             exception.getContext());
    } catch (Exception& exception) {
        clear();
        throw AbortException("Deserialization error: " + exception.getMessage(),
                             exception.getContext());
    } catch (...) {
        clear();
        throw AbortException("Deserialization error", IvwContext);
    }

    // Connections
    try {
        auto toDelete = connections_;
        std::vector<PortConnection> connections;
        d.deserialize("Connections", connections, "Connection");

        for (auto& con : connections) {
            if (!isConnected(con)) addConnection(con);
            toDelete.erase(con);
        }
        for (auto& con : toDelete) removeConnection(con);

    } catch (const SerializationException& exception) {
        throw IgnoreException("Deserialization error: " + exception.getMessage(),
                              exception.getContext());
    } catch (...) {
        clear();
        throw AbortException("Deserialization error:", IvwContext);
    }

    // Links
    try {
        auto toDelete = links_;
        std::vector<PropertyLink> links;
        d.deserialize("PropertyLinks", links, "PropertyLink");

        for (auto& link : links) {
            if (!isLinked(link)) addLink(link);
            toDelete.erase(link);
        }
        for (auto& link : toDelete) removeLink(link);

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
        if (auto processor = getProcessorByIdentifier(path[0])) {
            std::vector<std::string> propPath(path.begin() + 1, path.end());
            return processor->getPropertyByPath(propPath);
        }
    }
    return nullptr;
}

bool ProcessorNetwork::isPropertyInNetwork(Property* prop) const {
    if (auto owner = prop->getOwner()) {
        if (auto processor = owner->getProcessor()) {
            if (processor == getProcessorByIdentifier(processor->getIdentifier())) {
                return true;
            }
        }
    }
    return false;
}

InviwoApplication* ProcessorNetwork::getApplication() const {
    return application_;
}

}  // namespace
