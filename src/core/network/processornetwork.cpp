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
#include <algorithm>

namespace inviwo {

ProcessorPair::ProcessorPair(Processor* p1, Processor* p2) 
: processor1_(p1<p2?p1:p2), processor2_(p1<p2?p2:p1) {}

bool operator==(const ProcessorPair& p1, const ProcessorPair& p2) {
    return p1.processor1_ == p2.processor1_ && p1.processor2_ == p2.processor2_;
}

bool operator<(const ProcessorPair& p1, const ProcessorPair& p2) {
    if (p1.processor1_ != p2.processor1_) {
        return p1.processor1_ < p2.processor1_;
    } else {
        return p1.processor2_ < p2.processor2_;
    }
}

ProcessorNetwork::ProcessorNetwork()
    : ProcessorNetworkObservable()
    , ProcessorObserver()
    , modified_(true)
    , locked_(0)
    , deserializing_(false)
    , invalidating_(false)
    , linkEvaluator_(nullptr)
    , linking_(false) {
    linkEvaluator_ = new LinkEvaluator();
}

ProcessorNetwork::~ProcessorNetwork() {
    lock();
    clear();

    delete linkEvaluator_;
}

bool ProcessorNetwork::addProcessor(Processor* processor) {
    lock();

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
    unlock();
    return true;
}

void ProcessorNetwork::removeProcessor(Processor* processor) {
    if (!processor) return;

    // Remove all connections for this processor
    std::vector<PortConnection*> portConnections = getConnections();
    for (auto& portConnection : portConnections)
        if (portConnection->involvesProcessor(processor))
            removeConnection(portConnection->getOutport(), portConnection->getInport());

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
    PropertyLinkMap propertyLinks = propertyLinks_;
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
    auto it = processors_.find(processor->getIdentifier());
    if (it != processors_.end()) processors_.erase(it);
    processor->ProcessorObservable::removeObserver(this);
    modified();
    notifyObserversProcessorNetworkDidRemoveProcessor(processor);
}

void ProcessorNetwork::removeAndDeleteProcessor(Processor* processor) {
    if (!processor) return;
    
    removeProcessor(processor);
    if (processor->isInitialized()) {
        processor->deinitialize();
    }
    delete processor;
}

Processor* ProcessorNetwork::getProcessorByIdentifier(std::string identifier) const {
    ProcessorMap::const_iterator it = processors_.find(identifier);
    if(it!= processors_.end()) return it->second;
    return nullptr;
}

std::vector<Processor*> ProcessorNetwork::getProcessors() const {
    ProcessorVector processors;
    for (const auto& elem : processors_) {
        processors.push_back(elem.second);
    }
    return processors;
}


PortConnection* ProcessorNetwork::addConnection(Outport* sourcePort, Inport* destPort) {
    PortConnection* connection = getConnection(sourcePort, destPort);

    if (!connection && sourcePort && destPort && destPort->canConnectTo(sourcePort)) {
        notifyObserversProcessorNetworkWillAddConnection(connection);
        lock();
        connection = new PortConnection(sourcePort, destPort);
        portConnectionsMap_[std::make_pair(sourcePort, destPort)] = connection;
        portConnectionsVec_.push_back(connection);
        modified();
        destPort->connectTo(sourcePort);
        unlock();
        notifyObserversProcessorNetworkDidAddConnection(connection);
    }

    return connection;
}

void ProcessorNetwork::removeConnection(Outport* sourcePort, Inport* destPort) {
    auto itm = portConnectionsMap_.find(std::make_pair(sourcePort, destPort));
    if (itm != portConnectionsMap_.end()) {
        PortConnection* connection = itm->second;
        notifyObserversProcessorNetworkWillRemoveConnection(connection);
        lock();
        modified();
        destPort->disconnectFrom(sourcePort);

        portConnectionsMap_.erase(itm);

        PortConnectionVector::iterator itv = std::find(portConnectionsVec_.begin(), portConnectionsVec_.end(), connection);
        if (itv != portConnectionsVec_.end()) {
            portConnectionsVec_.erase(itv);
        }
        unlock();
        notifyObserversProcessorNetworkDidRemoveConnection(connection);
        delete connection;
    }
}

bool ProcessorNetwork::isConnected(Outport* sourcePort, Inport* destPort) {
    if (getConnection(sourcePort, destPort))
        return true;

    return false;
}

PortConnection* ProcessorNetwork::getConnection(Outport* sourcePort, Inport* destPort) {
    PortConnectionMap::iterator it = portConnectionsMap_.find(std::make_pair(sourcePort, destPort));
    if (it != portConnectionsMap_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<PortConnection*> ProcessorNetwork::getConnections() const {
    return portConnectionsVec_;
}

/////////////////////////////////////////////////////////////////////////////////////////
//Property Links

PropertyLink* ProcessorNetwork::addLink(Property* sourceProperty, Property* destinationProperty) {
    PropertyLink* link = getLink(sourceProperty, destinationProperty);
    if (!link) {
        link = new PropertyLink(sourceProperty, destinationProperty);
        notifyObserversProcessorNetworkWillAddLink(link);
        propertyLinks_[std::make_pair(sourceProperty, destinationProperty)] = link;
        addToPrimaryCache(link); //add to cache
        modified();
        notifyObserversProcessorNetworkDidAddLink(link);
    }
    return link;
}

void ProcessorNetwork::removeLink(Property* sourceProperty, Property* destinationProperty) {
    PropertyLinkMap::iterator it =
        propertyLinks_.find(std::make_pair(sourceProperty, destinationProperty));
    if (it != propertyLinks_.end()) {
        PropertyLink* link = it->second;
        notifyObserversProcessorNetworkWillRemoveLink(link);
        removeFromPrimaryCache(link);
        propertyLinks_.erase(it);
        modified();
        notifyObserversProcessorNetworkDidRemoveLink(link);
        delete link;
    }
}

bool ProcessorNetwork::isLinked(Property* sourceProperty, Property* destinationProperty) {
    if (getLink(sourceProperty, sourceProperty))
        return true;
    return false;
}

PropertyLink* ProcessorNetwork::getLink(Property* sourceProperty,
                                        Property* destinationProperty) const {
    PropertyLinkMap::const_iterator it =
        propertyLinks_.find(std::make_pair(sourceProperty, destinationProperty));
    if (it != propertyLinks_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<PropertyLink*> ProcessorNetwork::getLinks() const {
    PropertyLinkVector links;
    for (const auto& elem : propertyLinks_) {
        links.push_back(elem.second);
    }
    return links;
}

bool ProcessorNetwork::isLinkedBidirectional(Property* startProperty, Property* endProperty) {
    return getLink(startProperty, endProperty) != nullptr &&
           getLink(endProperty, startProperty) != nullptr;
}

ProcessorNetwork::PropertyLinkVector ProcessorNetwork::getLinksBetweenProcessors(
    Processor* p1, Processor* p2) {
    
    ProcessorLinkMap::iterator it = processorLinksCache_.find(ProcessorPair(p1,p2));
    if (it != processorLinksCache_.end()) {
        return it->second;
    }
    return PropertyLinkVector();
}

void ProcessorNetwork::addToPrimaryCache(PropertyLink* propertyLink) {
    Property* srcProperty = propertyLink->getSourceProperty();
    Property* dstProperty = propertyLink->getDestinationProperty();

    // Update ProcessorLink cache
    Processor* p1 = srcProperty->getOwner()->getProcessor();
    Processor* p2 = dstProperty->getOwner()->getProcessor();
    processorLinksCache_[ProcessorPair(p1, p2)].push_back(propertyLink);

    // Update primary cache
    std::vector<Property*>& cachelist = propertyLinkPrimaryCache_[srcProperty];
    if (std::find(cachelist.begin(), cachelist.end(), dstProperty) == cachelist.end()) {
        cachelist.push_back(dstProperty);
    }

    if (cachelist.empty()) propertyLinkPrimaryCache_.erase(srcProperty);

    clearSecondaryCache();
}

void ProcessorNetwork::removeFromPrimaryCache(PropertyLink* propertyLink) {
    Property* srcProperty = propertyLink->getSourceProperty();
    Property* dstProperty = propertyLink->getDestinationProperty();

    // Update ProcessorLink cache
    Processor* p1 = srcProperty->getOwner()->getProcessor();
    Processor* p2 = dstProperty->getOwner()->getProcessor();

    ProcessorLinkMap::iterator it = processorLinksCache_.find(ProcessorPair(p1, p2));
    if (it != processorLinksCache_.end()) {
        it->second.erase(std::remove(it->second.begin(), it->second.end(), propertyLink),
                         it->second.end());
        if (it->second.empty()) processorLinksCache_.erase(it);
    }

    // Update primary cache
    std::vector<Property*>& cachelist = propertyLinkPrimaryCache_[srcProperty];
    std::vector<Property*>::iterator sIt =
        std::find(cachelist.begin(), cachelist.end(), dstProperty);

    if (sIt != cachelist.end()) cachelist.erase(sIt);

    if (cachelist.empty()) propertyLinkPrimaryCache_.erase(srcProperty);

    clearSecondaryCache();
}

void ProcessorNetwork::updatePropertyLinkCaches() {
    clearSecondaryCache();
    propertyLinkPrimaryCache_.clear();
    for (auto& elem : propertyLinks_) {
        addToPrimaryCache(elem.second);
    }
}

void ProcessorNetwork::clearSecondaryCache() {
    propertyLinkSecondaryCache_.clear();
}

std::vector<PropertyLink>& ProcessorNetwork::getTriggerdLinksForProperty(Property* property) {
    if (propertyLinkSecondaryCache_.find(property) != propertyLinkSecondaryCache_.end()) {
        return propertyLinkSecondaryCache_[property];
    } else {
        return addToSecondaryCache(property);
    }
}

std::vector<Property*> ProcessorNetwork::getLinkedProperties(Property* property) {
    // check if link connectivity has been computed and cached already
    if (propertyLinkSecondaryCache_.find(property) != propertyLinkSecondaryCache_.end()) {
        const std::vector<PropertyLink>& list = propertyLinkSecondaryCache_[property];
        std::vector<Property*> pvec;
        for (const auto& elem : list) {
            pvec.push_back(elem.getDestinationProperty());
        }
        return pvec;
    } else {
        const std::vector<PropertyLink>& list = addToSecondaryCache(property);
        std::vector<Property*> pvec;
        for (const auto& elem : list) {
            pvec.push_back(elem.getDestinationProperty());
        }
        return pvec;
    }
}

std::vector<PropertyLink>& ProcessorNetwork::addToSecondaryCache(Property* src) {
    std::vector<PropertyLink> links;
    std::vector<Property*> dest = propertyLinkPrimaryCache_[src];
    for (auto& elem : dest) {
        if (src != elem) secondaryCacheHelper(links, src, elem);
    }

    propertyLinkSecondaryCache_[src] = links;
    return propertyLinkSecondaryCache_[src];
}

void ProcessorNetwork::secondaryCacheHelper(std::vector<PropertyLink>& links, Property* src,
                                            Property* dst) {
    // Check that we don't use a previous source or destination as the new destination.
    if (std::find_if(links.begin(), links.end(), PropertyLinkContainsTest(dst)) == links.end()) {
        links.push_back(PropertyLink(src, dst));

        // Follow the links of destination all links of all owners (CompositeProperties).
        for (Property* newSrc = dst; newSrc != nullptr;
             newSrc = dynamic_cast<Property*>(newSrc->getOwner())) {
            // Recurse over outgoing links.
            std::vector<Property*> dest = propertyLinkPrimaryCache_[newSrc];
            for (auto& elem : dest) {
                if (newSrc != elem) secondaryCacheHelper(links, newSrc, elem);
            }
        }

        // If we link to a CompositeProperty, make sure to evaluate sublinks.
        CompositeProperty* cp = dynamic_cast<CompositeProperty*>(dst);
        if (cp) {
            std::vector<Property*> srcProps = cp->getProperties();
            for (auto& srcProp : srcProps) {
                // Recurse over outgoing links.
                std::vector<Property*> dest = propertyLinkPrimaryCache_[srcProp];
                for (auto& elem : dest) {
                    if (srcProp != elem) secondaryCacheHelper(links, srcProp, elem);
                }
            }
        }
    }
}

struct LinkCheck {
    LinkCheck() : linkSettings_(*InviwoApplication::getPtr()->getSettingsByType<LinkSettings>()) {}
    bool operator()(const Property *p) const { return !linkSettings_.isLinkable(p); }

private:
    const LinkSettings& linkSettings_;
};

struct AutoLinkCheck {
    AutoLinkCheck(const Property *p, LinkingConditions linkCondition)
        : property_(p), linkCondition_(linkCondition) {}
    bool operator()(const Property *p) const {
        return !AutoLinker::canLink(p, property_, linkCondition_);
    }

private:
    const Property *property_;
    LinkingConditions linkCondition_;
};

struct AutoLinkSort {
    AutoLinkSort(const Property *p) { pos_ = getPosition(p); }

    bool operator()(const Property *a, const Property *b) {
        // TODO Figure out which candidate is best.
        // using distance now
        float da = glm::distance(pos_, getPosition(a));
        float db = glm::distance(pos_, getPosition(b));
        return da < db;
    }

private:
    vec2 pos_;
    std::map<const Property *, vec2> cache_;

    vec2 getPosition(const Property *p) {
        std::map<const Property *, vec2>::const_iterator it = cache_.find(p);
        if (it != cache_.end()) return it->second;
        return cache_[p] = getPosition(p->getOwner()->getProcessor());
    }

    vec2 getPosition(const Processor *processor) {
        ProcessorMetaData *meta = processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
        if (meta) {
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

    std::vector<Property*> destprops = getPropertiesRecursive(processor);
    destprops.erase(std::remove_if(destprops.begin(), destprops.end(), linkChecker),
                    destprops.end());  // remove properties for which autolinking is disalbed

    if (destprops.size() == 0) {  // no candidates for autolinking in the new processor
        return;
    }

    std::vector<Property*> properties;
    for (auto& elem : processors_) {
        if (elem.second != processor) {
            std::vector<Property*> p = getPropertiesRecursive(elem.second);
            properties.insert(properties.end(), p.begin(), p.end());
        }
    }
    properties.erase(std::remove_if(properties.begin(), properties.end(), linkChecker),
                     properties.end());  // remove properties for which autolinking is disalbed
    if (properties.size() == 0) {  // no candidates for autolinking in the new processor
        return;
    }

    for (auto& destprop : destprops) {
        std::vector<Property*> candidates = properties;
        AutoLinkCheck autoLinkChecker(destprop, LinkMatchingTypeAndId);
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(), autoLinkChecker),
                         candidates.end());
        AutoLinkSort sorter(destprop);
        std::sort(candidates.begin(), candidates.end(), sorter);

        if (candidates.size() > 0) {
            addLink(candidates[0], destprop);
            evaluatePropertyLinks(candidates[0]);  // Propagate the link to the new Processor.
            addLink(destprop, candidates[0]);
        }
    }
}

std::vector<Property*> ProcessorNetwork::getPropertiesRecursive(PropertyOwner* owner) {
    std::vector<Property*> properties;
    std::vector<Property*> props = owner->getProperties();
    for (auto& prop : props) {
        properties.push_back(prop);
        PropertyOwner* o = dynamic_cast<PropertyOwner*>(prop);
        if(o){
            std::vector<Property*> p = getPropertiesRecursive(o);
            properties.insert(properties.end(),p.begin(), p.end());
        }
    }
    return properties;
}

/////////////////////////////////////////////////////////////////////////////////////////

void ProcessorNetwork::clear() {
    NetworkLock lock(this);
    
    std::vector<Processor*> processors = getProcessors();
    //Invalidate inports to alert processors that they should stop their calculations.
    for (auto processor : processors) {
        for (auto inport : processor->getInports()) inport->invalidate(INVALID_OUTPUT);
    }

    for (auto processor : processors) {
        RenderContext::getPtr()->activateDefaultRenderContext();
        removeAndDeleteProcessor(processor);
    }
}

void ProcessorNetwork::modified() {
    modified_ = true;
}


void ProcessorNetwork::setModified(bool modified) { 
    modified_ = modified; 
}

bool ProcessorNetwork::isModified() const { 
    return modified_; 
}

bool ProcessorNetwork::isLinking() const{
    return linking_;
}

bool ProcessorNetwork::isInvalidating() const { 
    return invalidating_; 
}

void ProcessorNetwork::onProcessorInvalidationBegin(Processor* p) {
    std::vector<Processor*>::iterator it =
        std::find_if(processorsInvalidating_.begin(), processorsInvalidating_.end(),
                     ComparePointers<Processor>(p));

    if (it != processorsInvalidating_.end()) return;

    processorsInvalidating_.push_back(p);

    if (!isInvalidating()) {
        invalidating_ = true;
    }
}

void ProcessorNetwork::onProcessorInvalidationEnd(Processor* p) {
    std::vector<Processor*>::iterator it =
        std::find_if(processorsInvalidating_.begin(), processorsInvalidating_.end(),
        ComparePointers<Processor>(p));

    if (it != processorsInvalidating_.end()) processorsInvalidating_.erase(it);

    if (processorsInvalidating_.empty()) {
        invalidating_ = false;

        notifyObserversProcessorNetworkEvaluateRequest();
    }
}

void ProcessorNetwork::onProcessorRequestEvaluate(Processor*) {
    notifyObserversProcessorNetworkEvaluateRequest();
}

void ProcessorNetwork::onProcessorIdentifierChange(Processor* processor) {
    for (ProcessorMap::iterator it = processors_.begin(); it != processors_.end(); ++it) {
        if(it->second == processor) {
            processors_.erase(it);
            processors_[processor->getIdentifier()] = processor;
            break;
        }
    }
}

Processor* ProcessorNetwork::getInvalidationInitiator() {
    if(processorsInvalidating_.empty())
        return nullptr;
    else
        return processorsInvalidating_[0]; 
}

//linking helpers

void ProcessorNetwork::onAboutPropertyChange(Property* modifiedProperty) {
    performLinkingOnPropertyChange(modifiedProperty);
    notifyObserversProcessorNetworkChanged();
}

void ProcessorNetwork::performLinkingOnPropertyChange(Property* modifiedProperty) {
    if (modifiedProperty)
        evaluatePropertyLinks(modifiedProperty);
}


void ProcessorNetwork::evaluatePropertyLinks(Property* modifiedProperty) {
    if (isLinking()) return;

    lock();
    linking_ = true;

    const std::vector<PropertyLink>& links = getTriggerdLinksForProperty(modifiedProperty);
    for (auto& link : links) {
        linkEvaluator_->evaluate(link.getSourceProperty(), link.getDestinationProperty());
    }
   
    if (isLinking()) linking_ = false; 
    unlock();
}

void ProcessorNetwork::serialize(IvwSerializer& s) const {
    s.serialize("ProcessorNetworkVersion", processorNetworkVersion_);
    s.serialize("Processors", getProcessors(), "Processor");
    s.serialize("Connections", getConnections(), "Connection");
    s.serialize("PropertyLinks", getLinks(), "PropertyLink");
    
    InviwoSetupInfo info(InviwoApplication::getPtr());
    s.serialize("InviwoSetup", info);
}

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
        NetworkConverter nv(version);
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

        ProcessorVector processors;
        d.deserialize("Processors", processors, "Processor");
        for (size_t i = 0; i < processors.size(); ++i) {
            if (processors[i]) {
                addProcessor(processors[i]);
            } else {
                LogNetworkWarn("Failed deserialization: Processor Nr." << i);
            }
        }
    } catch (const SerializationException& exception) {
        clear();
        throw AbortException("DeSerialization exception " + exception.getMessage(),
                             exception.getContext());
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception.", IvwContext);
    }

    // Connections
    try {
        std::vector<PortConnection*> portConnections;
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

                delete portConnections[i];
            } else {
                LogNetworkWarn("Failed deserialization: Port Connection Nr." << i);
            }
        }
    } catch (const SerializationException& exception) {
        throw IgnoreException("DeSerialization Exception " + exception.getMessage(), exception.getContext());
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception.", IvwContext);
    }

    // Links
    try {
        std::vector<PropertyLink*> propertyLinks;
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

                delete propertyLinks[j];

            } else {
                LogNetworkWarn("Unable to establish property link Nr: " << j);
            }
        }

        if (!errorHandle.messages.empty()) {
            LogNetworkWarn("There were errors while loading workspace: " + d.getFileName() + "\n" +
                           joinString(errorHandle.messages, "\n"));
        }

    } catch (const SerializationException& exception) {
        throw IgnoreException("DeSerialization Exception " + exception.getMessage(), exception.getContext());
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception.", IvwContext);
    }

    notifyObserversProcessorNetworkChanged();
}

bool ProcessorNetwork::isDeserializing()const {
    return deserializing_;
}

Property* ProcessorNetwork::getProperty(std::vector<std::string> path) const {
    if (path.size() >= 2){
        Processor* processor = getProcessorByIdentifier(path[0]);
        if (processor) {
            std::vector<std::string> propPath(path.begin()+1, path.end());
            return processor->getPropertyByPath(propPath);
        }
    }
    return nullptr;
}

const int ProcessorNetwork::processorNetworkVersion_ = 10;


ProcessorNetwork::NetworkConverter::NetworkConverter(int from)
    : VersionConverter(), from_(from) {}

bool ProcessorNetwork::NetworkConverter::convert(TxElement* root) {
    switch (from_) {
        case 0:
            traverseNodes(root, &ProcessorNetwork::NetworkConverter::updateProcessorType);
        case 1:
            traverseNodes(root, &ProcessorNetwork::NetworkConverter::updateMetaDataTree);
        case 2:
            traverseNodes(root, &ProcessorNetwork::NetworkConverter::updatePropertType);
        case 3:
            traverseNodes(root, &ProcessorNetwork::NetworkConverter::updateShadingMode);
        case 4:
            traverseNodes(root, &ProcessorNetwork::NetworkConverter::updateCameraToComposite);
        case 5:
            traverseNodes(root, &ProcessorNetwork::NetworkConverter::updateMetaDataType);
        case 6:
            traverseNodes(root, &ProcessorNetwork::NetworkConverter::updateMetaDataKeys);
        case 7:
            traverseNodes(root, &ProcessorNetwork::NetworkConverter::updateDimensionTag);
        case 8:
            traverseNodes(root, &ProcessorNetwork::NetworkConverter::updatePropertyLinks);
        case 9:
            ProcessorNetwork::NetworkConverter::updatePortsInProcessors(root);
        default:
            break;
    }
    
    return true;
}

void ProcessorNetwork::NetworkConverter::traverseNodes(TxElement* node, updateType update) {
    (this->*update)(node);
    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); child++) {
        traverseNodes(child.Get(), update);
    }
}

void ProcessorNetwork::NetworkConverter::updateProcessorType(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "Processor") {
        std::string type = node->GetAttributeOrDefault("type", "");
        if (splitString(type, '.').size() < 3){
            node->SetAttribute("type", "org.inviwo." + type);
        }
    }
}

void ProcessorNetwork::NetworkConverter::updateMetaDataTree(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "MetaDataList") {
        node->SetValue("MetaDataMap");
    }
    if (key == "MetaData") {
        node->SetValue("MetaDataItem");
        node->SetAttribute("key", node->GetAttribute("type"));
    }
}

void ProcessorNetwork::NetworkConverter::updatePropertType(TxElement* node) {
    std::string renamed[] = {
        "undefined",
        "BoolProperty",
        "AdvancedMaterialProperty",
        "BaseOptionProperty",
        "OptionPropertyFloat",
        "OptionPropertyDouble",
        "OptionPropertyInt",
        "OptionPropertyInt64",
        "OptionPropertyString",
        "OptionPropertyFloatVec2",
        "OptionPropertyFloatVec3",
        "OptionPropertyFloatVec4",
        "OptionPropertyDoubleVec2",
        "OptionPropertyDoubleVec3",
        "OptionPropertyDoubleVec4",
        "OptionPropertyIntVec2",
        "OptionPropertyIntVec3",
        "OptionPropertyIntVec4",
        "OptionPropertyFloatMat2",
        "OptionPropertyFloatMat3",
        "OptionPropertyFloatMat4",
        "OptionPropertyDoubleMat2",
        "OptionPropertyDoubleMat3",
        "OptionPropertyDoubleMat4",
        "ButtonProperty",
        "CameraProperty",
        "CompositeProperty",
        "DirectoryProperty",
        "EventProperty",
        "FileProperty",
        "ImageEditorProperty",
        "FloatMinMaxProperty",
        "DoubleMinMaxProperty",
        "IntMinMaxProperty",
        "FloatProperty",
        "DoubleProperty",
        "IntProperty",
        "Int64Property",
        "FloatVec2Property",
        "FloatVec3Property",
        "FloatVec4Property",
        "DoubleVec2Property",
        "DoubleVec3Property",
        "DoubleVec4Property",
        "IntVec2Property",
        "IntVec3Property",
        "IntVec4Property",
        "FloatMat2Property",
        "FloatMat3Property",
        "FloatMat4Property",
        "DoubleMat2Property",
        "DoubleMat3Property",
        "DoubleMat4Property",
        "SimpleLightingProperty",
        "SimpleRaycastingProperty",
        "StringProperty",
        "TransferFunctionProperty" };

    std::string key;
    node->GetValue(&key);

    if (key == "Property") {
        std::string type = node->GetAttributeOrDefault("type", "");
        int size = sizeof(renamed)/sizeof(std::string);
        if(std::find(renamed, renamed+size, type) != renamed+size) {
            node->SetAttribute("type", "org.inviwo." + type);
        }
    }
}

void ProcessorNetwork::NetworkConverter::updateMetaDataType(TxElement* node) {
    std::string renamed[] = {
        "BoolMetaData",
        "IntMetaData",
        "FloatMetaData",
        "DoubleMetaData",
        "StringMetaData",
        "FloatVec2MetaData",
        "FloatVec3MetaData",
        "FloatVec4MetaData",
        "DoubleVec2MetaData",
        "DoubleVec3MetaData",
        "DoubleVec4MetaData",
        "IntVec2MetaData",
        "IntVec3MetaData",
        "IntVec4MetaData",
        "UIntVec2MetaData",
        "UIntVec3MetaData",
        "UIntVec4MetaData",
        "FloatMat2MetaData",
        "FloatMat3MetaData",
        "FloatMat4MetaData",
        "DoubleMat2MetaData",
        "DoubleMat4MetaData",
        "DoubleMat3MetaData",
        "VectorMetaData<2, Float>",
        "VectorMetaData<3, Float>",
        "VectorMetaData<4, Float>",
        "VectorMetaData<2, Int>",
        "VectorMetaData<3, Int>",
        "VectorMetaData<4, Int>",
        "VectorMetaData<2, Uint",
        "VectorMetaData<3, UInt>",
        "VectorMetaData<4, UInt>",
        "MatrixMetaData<2, Float>",
        "MatrixMetaData<3, Float>",
        "MatrixMetaData<4, Float>",
        "PositionMetaData",
        "ProcessorMetaData",
        "ProcessorWidgetMetaData",
        "PropertyEditorWidgetMetaData"
        
    };
    std::string key;
    node->GetValue(&key);

    if (key == "MetaDataItem") {
        std::string type = node->GetAttributeOrDefault("type", "");
        int size = sizeof(renamed)/sizeof(std::string);
        if (std::find(renamed, renamed+size, type) != renamed+size) {
            node->SetAttribute("type", "org.inviwo." + type);
        }
    }
}

void ProcessorNetwork::NetworkConverter::updateShadingMode(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "Property") {
        std::string type = node->GetAttributeOrDefault("type", "");
        std::string identifier = node->GetAttributeOrDefault("identifier", "");
        if (type == "org.inviwo.OptionPropertyString" && identifier == "shadingMode") {
            node->SetAttribute("type", "org.inviwo.OptionPropertyInt");
        }
    }
}

void ProcessorNetwork::NetworkConverter::updateCameraToComposite(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "Property") {
        std::string type = node->GetAttributeOrDefault("type", "");
        std::string identifier = node->GetAttributeOrDefault("identifier", "");
        if (type == "org.inviwo.CameraProperty") {
            std::vector<TxElement> subNodeVector;

            // create
            TxElement newNode;
            newNode.SetValue("Properties");

            // temp list
            std::vector<TxElement*> toBeDeleted;

            // copy and remove children
            ticpp::Iterator<TxElement> child;
            for (child = child.begin(node); child != child.end(); child++) {
                std::string propKey;
                TxElement* subNode = child.Get();
                subNode->GetValue(&propKey);
                if (propKey == "lookFrom" || propKey == "lookTo" || propKey == "lookUp" ||
                    propKey == "fovy" || propKey == "aspectRatio" || propKey == "nearPlane" ||
                    propKey == "farPlane") {
                    subNode->SetValue("Property");
                    newNode.InsertEndChild(*subNode->Clone());
                    toBeDeleted.push_back(subNode);
                }
            }

            for (auto& elem : toBeDeleted) {
                node->RemoveChild(elem);
            }

            // insert new node
            node->InsertEndChild(newNode);

            LogNetworkWarn("Camera property updated to composite property. Workspace requires resave")
        }
    }
}

void ProcessorNetwork::NetworkConverter::updateMetaDataKeys(TxElement* node) {
    std::string renamed[] = {
        "PositionMetaData",
        "ProcessorMetaData",
        "ProcessorWidgetMetaData",
        "PropertyEditorWidgetMetaData"
    };

    std::string key;
    node->GetValue(&key);

    if (key == "MetaDataItem") {
        std::string keyname = node->GetAttributeOrDefault("key", "");
        int size = sizeof(renamed) / sizeof(std::string);
        if (std::find(renamed, renamed + size, keyname) != renamed + size) {
            node->SetAttribute("key", "org.inviwo." + keyname);
        }
    }
}

void ProcessorNetwork::NetworkConverter::updateDimensionTag(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "dimension") {
        node->SetValue("dimensions");
    }
}

void ProcessorNetwork::NetworkConverter::updatePropertyLinks(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "PropertyLink") {
        TxElement* properties = node->FirstChildElement(false);
        if (properties) {
            TxElement* src = properties->FirstChild()->ToElement();
            TxElement* dest = properties->LastChild()->ToElement();
            src->SetValue("SourceProperty");
            dest->SetValue("DestinationProperty");

            node->InsertEndChild(*src);
            node->InsertEndChild(*dest);

            node->RemoveChild(properties);

            std::stringstream ss;
            ss << *node;
            std::string txt = ss.str();
        }
    }
}

void ProcessorNetwork::NetworkConverter::updatePortsInProcessors(TxElement* root) {
    struct RefManager : public ticpp::Visitor {
        virtual bool VisitEnter(const TxElement& node, const TxAttribute*) {
            std::string id = node.GetAttributeOrDefault("id", "");
            if (!id.empty()) {
                ids_.push_back(id);
                std::sort(ids_.begin(), ids_.end());
            }
            return true;
        };

        std::string getNewRef() {
            std::string ref("ref0");
            for (int i = 1; std::find(ids_.begin(), ids_.end(), ref) != ids_.end(); ++i) {
                ref = "ref" + toString(i);
            }
            ids_.push_back(ref);
            return ref;
        };

        std::vector<std::string> ids_;
    };

    RefManager refs;

    root->Accept(&refs);

    TxNode* processorlist = root->FirstChild("Processors");
    std::map<std::string, TxElement*> processorsOutports;
    std::map<std::string, TxElement*> processorsInports;

    ticpp::Iterator<TxElement> child;
    for (child = child.begin(processorlist); child != child.end(); child++) {
        // create

        TxElement* outports = new TxElement("OutPorts");
        child->LinkEndChild(outports);
        processorsOutports[child->GetAttributeOrDefault("identifier", "")] = outports;

        TxElement* inports = new TxElement("InPorts");
        child->LinkEndChild(inports);
        processorsInports[child->GetAttributeOrDefault("identifier", "")] = inports;
    }

    TxNode* connectionlist = root->FirstChild("Connections");
    for (child = child.begin(connectionlist); child != child.end(); child++) {
        TxElement* outport = child->FirstChild("OutPort")->ToElement();
        if (outport->GetAttributeOrDefault("reference", "").empty()) {
            std::string pid = outport->FirstChild("Processor")->ToElement()->GetAttributeOrDefault(
                "identifier", "");
            outport->RemoveChild(outport->FirstChild());

            TxElement* outclone = outport->Clone()->ToElement();

            std::string id = outport->GetAttributeOrDefault("id", "");
            if (id.empty()) id = refs.getNewRef();

            outport->SetAttribute("reference", id);
            outport->RemoveAttribute("id");

            outclone->SetAttribute("id", id);
            processorsOutports[pid]->LinkEndChild(outclone);
        }

        TxElement* inport = child->FirstChild("InPort")->ToElement();
        if (inport->GetAttributeOrDefault("reference", "").empty()) {
            std::string pid = inport->FirstChild("Processor")->ToElement()->GetAttributeOrDefault(
                "identifier", "");
            inport->RemoveChild(inport->FirstChild());

            TxElement* inclone = inport->Clone()->ToElement();

            std::string id = inport->GetAttributeOrDefault("id", "");
            if (id.empty()) id = refs.getNewRef();

            inport->SetAttribute("reference", id);
            inport->RemoveAttribute("id");

            inclone->SetAttribute("id", id);
            processorsInports[pid]->LinkEndChild(inclone);
        }
    }

    for (auto& processorsOutport : processorsOutports) {
        delete processorsOutport.second;
    }

    for (auto& processorsInport : processorsInports) {
        delete processorsInport.second;
    }
}

NetworkLock::NetworkLock() : network_(InviwoApplication::getPtr()->getProcessorNetwork()) {
    network_->lock();
}

NetworkLock::NetworkLock(ProcessorNetwork* network) : network_(network) {
    network_->lock();
}

NetworkLock::~NetworkLock() {
    network_->unlock();
}

} // namespace
