/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/network/processornetworkconverter.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/metadata/processormetadata.h>

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

Processor* ProcessorNetwork::addProcessor(std::unique_ptr<Processor> processor) {
    auto p = processor.get();
    addProcessor(processor.release());
    return p;
}

bool ProcessorNetwork::addProcessor(Processor* processor) {
    NetworkLock lock(this);

    processor->setIdentifier(util::findUniqueIdentifier(
        processor->getIdentifier(),
        [&](const std::string& id) { return getProcessorByIdentifier(id) == nullptr; }, ""));

    notifyObserversProcessorNetworkWillAddProcessor(processor);
    processors_[util::stripIdentifier(processor->getIdentifier())] = processor;
    processor->setNetwork(this);
    processor->ProcessorObservable::addObserver(this);
    addPropertyOwnerObservation(processor);

    auto meta = processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
    meta->addObserver(this);

    if (auto widget = application_->getProcessorWidgetFactory()->create(processor)) {
        processor->setProcessorWidget(std::move(widget));
    }

    processor->invalidate(InvalidationLevel::InvalidResources);
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
    auto toDelete =
        util::copy_if(links_, [&](const PropertyLink& link) { return link.involves(processor); });
    for (auto& link : toDelete) {
        removeLink(link.getSource(), link.getDestination());
    }

    // remove processor itself
    notifyObserversProcessorNetworkWillRemoveProcessor(processor);
    processors_.erase(util::stripIdentifier(processor->getIdentifier()));
    processor->ProcessorObservable::removeObserver(this);
    removePropertyOwnerObservation(processor);
    processor->setNetwork(nullptr);
    processor->setProcessorWidget(nullptr);
    notifyObserversProcessorNetworkDidRemoveProcessor(processor);
}

void ProcessorNetwork::removeAndDeleteProcessor(Processor* processor) {
    if (!processor) return;
    RenderContext::getPtr()->activateDefaultRenderContext();
    removeProcessor(processor);
    delete processor;
}

Processor* ProcessorNetwork::getProcessorByIdentifier(std::string identifier) const {
    return util::map_find_or_null(processors_, util::stripIdentifier(identifier));
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

    if (canConnect(src, dst) && !isConnected(src, dst)) {
        NetworkLock lock(this);

        PortConnection connection(src, dst);
        notifyObserversProcessorNetworkWillAddConnection(connection);

        dst->connectTo(src);
        connections_.emplace(src, dst);
        connectionsVec_.emplace_back(src, dst);

        notifyObserversProcessorNetworkDidAddConnection(connection);
    }
}

bool ProcessorNetwork::canConnect(const Outport* src, const Inport* dst) const {
    return src != nullptr && dst != nullptr && dst->canConnectTo(src);
}

void ProcessorNetwork::removeConnection(const PortConnection& connection) {
    auto it = connections_.find(connection);
    if (it != connections_.end()) {
        NetworkLock lock(this);

        notifyObserversProcessorNetworkWillRemoveConnection(connection);

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

    if (!isLinked(src, dst) && canLink(src, dst)) {
        NetworkLock lock(this);
        PropertyLink link(src, dst);
        notifyObserversProcessorNetworkWillAddLink(link);
        links_.insert(link);
        linkEvaluator_.addLink(link);  // add to cache
        notifyObserversProcessorNetworkDidAddLink(link);
    }
}

bool ProcessorNetwork::canLink(const Property* src, const Property* dst) const {
    return linkEvaluator_.canLink(src, dst);
}

void ProcessorNetwork::removeLink(const PropertyLink& link) {
    auto it = links_.find(link);
    if (it != links_.end()) {
        NetworkLock lock(this);
        notifyObserversProcessorNetworkWillRemoveLink(link);
        linkEvaluator_.removeLink(link);
        links_.erase(it);
        notifyObserversProcessorNetworkDidRemoveLink(link);
    }
}
void ProcessorNetwork::removeLink(Property* src, Property* dst) {
    PropertyLink link(src, dst);
    removeLink(link);
}

void ProcessorNetwork::onWillRemoveProperty(Property* property, size_t /*index*/) {
    if (auto comp = dynamic_cast<PropertyOwner*>(property)) {
        size_t i = 0;
        for (auto p : comp->getProperties()) {
            onWillRemoveProperty(p, i);
            i++;
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
    return util::transform(links_, [](PropertyLink elem) { return elem; });
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
        removeAndDeleteProcessor(processor);
    }
}

bool ProcessorNetwork::isEmpty() const { return processors_.empty(); }

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

void ProcessorNetwork::onProcessorIdentifierChanged(Processor* processor,
                                                    const std::string& oldIdentifier) {
    processors_.erase(util::stripIdentifier(oldIdentifier));
    processors_[util::stripIdentifier(processor->getIdentifier())] = processor;
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

void ProcessorNetwork::onProcessorMetaDataPositionChange() {
    notifyObserversProcessorNetworkChanged();
}
void ProcessorNetwork::onProcessorMetaDataVisibilityChange() {
    notifyObserversProcessorNetworkChanged();
}
void ProcessorNetwork::onProcessorMetaDataSelectionChange() {
    notifyObserversProcessorNetworkChanged();
}

void ProcessorNetwork::serialize(Serializer& s) const {
    s.serialize("ProcessorNetworkVersion", processorNetworkVersion_);
    s.serialize("Processors", getProcessors(), "Processor");
    s.serialize("Connections", getConnections(), "Connection");
    s.serialize("PropertyLinks", getLinks(), "PropertyLink");
}

void ProcessorNetwork::addPropertyOwnerObservation(PropertyOwner* po) {
    po->addObserver(this);
    for (auto child : po->getCompositeProperties()) {
        addPropertyOwnerObservation(child);
    }
}

void ProcessorNetwork::removePropertyOwnerObservation(PropertyOwner* po) {
    po->removeObserver(this);
    for (auto child : po->getCompositeProperties()) {
        removePropertyOwnerObservation(child);
    }
}

int ProcessorNetwork::getVersion() const { return processorNetworkVersion_; }

const int ProcessorNetwork::processorNetworkVersion_ = 16;

void ProcessorNetwork::deserialize(Deserializer& d) {
    NetworkLock lock(this);

    // This will set deserializing_ to true while keepTrueWillAlive is in scope
    // and set it to false no matter how we leave the scope
    util::KeepTrueWhileInScope keepTrueWillAlive(&deserializing_);

    int version = 0;
    d.deserialize("ProcessorNetworkVersion", version);

    if (version != processorNetworkVersion_) {
        LogNetworkSpecial((&d), LogLevel::Warn,
                          "Loading old workspace ("
                              << d.getFileName() << ") Processor Network version: " << version
                              << ". Updating to version: " << processorNetworkVersion_ << ".");
        ProcessorNetworkConverter nv(version);
        d.convertVersion(&nv);
    }

    // Processors
    try {
        RenderContext::getPtr()->activateDefaultRenderContext();

        auto des =
            util::MapDeserializer<std::string, Processor*>("Processors", "Processor", "identifier")
                .setIdentifierTransform(
                    [](const std::string& id) { return util::stripIdentifier(id); })
                .setMakeNew([]() {
                    RenderContext::getPtr()->activateDefaultRenderContext();
                    return nullptr;
                })
                .onNew([&](const std::string& /*id*/, Processor*& p) { addProcessor(p); })
                .onRemove([&](const std::string& id) {
                    removeAndDeleteProcessor(getProcessorByIdentifier(id));
                });
        des(d, processors_);

    } catch (const Exception& exception) {
        clear();
        throw AbortException("Deserialization error: " + exception.getMessage(),
                             exception.getContext());
    } catch (const std::exception& exception) {
        clear();
        throw AbortException("Deserialization error: " + std::string(exception.what()),
                             IVW_CONTEXT);
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception during deserialization.", IVW_CONTEXT);
    }

    // Connections
    try {
        std::vector<PortConnection> connections;
        d.deserialize("Connections", connections, "Connection");

        // remove any already existing connections.
        PortConnections save;
        util::erase_remove_if(connections, [&](auto& c) {
            if (connections_.count(c) != 0) {
                save.insert(c);
                return true;
            } else {
                return false;
            }
        });

        // remove any no longer used connections
        auto remove = util::copy_if(connections_, [&](auto& c) { return save.count(c) == 0; });
        for (auto& c : remove) removeConnection(c);

        // Add the new connections
        for (auto& c : connections) {
            try {
                addConnection(c);
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }

    } catch (const Exception& exception) {
        clear();
        throw IgnoreException("Deserialization error: " + exception.getMessage(),
                              exception.getContext());
    } catch (const std::exception& exception) {
        clear();
        throw AbortException("Deserialization error: " + std::string(exception.what()),
                             IVW_CONTEXT);
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception during deserialization.", IVW_CONTEXT);
    }

    // Links
    try {
        std::vector<PropertyLink> links;
        d.deserialize("PropertyLinks", links, "PropertyLink");

        // remove any already existing links.
        PropertyLinks save;
        util::erase_remove_if(links, [&](auto& l) {
            if (links_.count(l) != 0) {
                save.insert(l);
                return true;
            } else {
                return false;
            }
        });

        // remove any no longer used links
        auto remove = util::copy_if(links_, [&](auto& l) { return save.count(l) == 0; });
        for (auto& l : remove) removeLink(l);

        // Add the new links
        for (auto& link : links) {
            try {
                addLink(link);
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }

    } catch (const Exception& exception) {
        clear();
        throw IgnoreException("Deserialization error: " + exception.getMessage(),
                              exception.getContext());
    } catch (const std::exception& exception) {
        clear();
        throw AbortException("Deserialization error: " + std::string(exception.what()),
                             IVW_CONTEXT);
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception during deserialization.", IVW_CONTEXT);
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

InviwoApplication* ProcessorNetwork::getApplication() const { return application_; }

}  // namespace inviwo
