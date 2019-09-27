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

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/metadata/metadatafactory.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/interactionevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/ports/imageport.h>

namespace inviwo {

Processor::Processor(const std::string& identifier, const std::string& displayName)
    : PropertyOwner()
    , ProcessorObservable()
    , processorWidget_(nullptr)
    , isReady_{true, [this](const bool&) { notifyObserversReadyChange(this); },
               [this]() { return allInportsAreReady(); }}
    , isSink_{true, [this](const bool&) { notifyObserversSinkChange(this); },
              [this]() { return outports_.empty(); }}
    , isSource_{true, [this](const bool&) { notifyObserversSourceChange(this); },
                [this]() { return inports_.empty(); }}
    , identifier_(identifier)
    , displayName_{displayName}
    , network_(nullptr) {
    createMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
}

Processor::~Processor() = default;

void Processor::addPortInternal(Inport* port, const std::string& portGroup) {
    if (getPort(port->getIdentifier()) != nullptr) {
        throw Exception("Processor \"" + getIdentifier() + "\" Can't add inport, identifier \"" +
                            port->getIdentifier() + "\" already exist.",
                        IVW_CONTEXT);
    }
    if (port->getIdentifier().empty()) {
        throw Exception("Adding port with empty identifier", IVW_CONTEXT);
    }
    util::validateIdentifier(port->getIdentifier(), "Port", IVW_CONTEXT);

    port->setProcessor(this);
    inports_.push_back(port);
    addPortToGroup(port, portGroup);

    notifyObserversProcessorPortAdded(this, port);
    isSource_.update();
    port->isReady_.setNotify([this](const bool&) { isReady_.update(); });
    port->isOptional_.setNotify([this](const bool&) { isReady_.update(); });
    isReady_.update();
}

void Processor::addPortInternal(Outport* port, const std::string& portGroup) {
    if (getPort(port->getIdentifier()) != nullptr) {
        throw Exception("Processor \"" + getIdentifier() + "\" Can't add outport, identifier \"" +
                            port->getIdentifier() + "\" already exist.",
                        IVW_CONTEXT);
    }
    if (port->getIdentifier().empty()) {
        throw Exception("Adding port with empty identifier", IVW_CONTEXT);
    }
    util::validateIdentifier(port->getIdentifier(), "Port", IVW_CONTEXT);

    port->setProcessor(this);
    outports_.push_back(port);
    addPortToGroup(port, portGroup);

    notifyObserversProcessorPortAdded(this, port);
    isSink_.update();
    isReady_.update();
}

Port* Processor::removePort(const std::string& identifier) {
    if (auto port = getPort(identifier)) {
        if (auto inport = dynamic_cast<Inport*>(port)) return removePort(inport);
        if (auto outport = dynamic_cast<Outport*>(port)) return removePort(outport);
    }
    return nullptr;
}

Inport* Processor::removePort(Inport* port) {
    notifyObserversProcessorPortRemoved(this, port);
    port->isReady_.setNotify([](const bool&) {});
    port->isOptional_.setNotify([](const bool&) {});
    port->setProcessor(nullptr);
    util::erase_remove(inports_, port);
    removePortFromGroups(port);

    // This will delete the port if owned
    util::erase_remove_if(ownedInports_, [&port](const std::unique_ptr<Inport>& p) {
        if (p.get() == port) {
            port = nullptr;
            return true;
        } else {
            return false;
        }
    });
    isSource_.update();
    isReady_.update();
    return port;
}

Outport* Processor::removePort(Outport* port) {
    notifyObserversProcessorPortRemoved(this, port);
    port->setProcessor(nullptr);
    util::erase_remove(outports_, port);
    removePortFromGroups(port);

    // This will delete the port if owned
    util::erase_remove_if(ownedOutports_, [&port](const std::unique_ptr<Outport>& p) {
        if (p.get() == port) {
            port = nullptr;
            return true;
        } else {
            return false;
        }
    });
    isSink_.update();
    isReady_.update();
    return port;
}

void Processor::addPortToGroup(Port* port, const std::string& portGroup) {
    portGroups_[port] = portGroup;
    groupPorts_[portGroup].push_back(port);
}

void Processor::removePortFromGroups(Port* port) {
    auto group = portGroups_[port];
    util::erase_remove(groupPorts_[group], port);
    if (groupPorts_[group].empty()) groupPorts_.erase(group);
    portGroups_.erase(port);
}

std::string Processor::getClassIdentifier() const { return getProcessorInfo().classIdentifier; }

std::string Processor::getCategory() const { return getProcessorInfo().category; }

CodeState Processor::getCodeState() const { return getProcessorInfo().codeState; }

Tags Processor::getTags() const { return getProcessorInfo().tags; }

void Processor::setIdentifier(const std::string& identifier) {
    if (identifier != identifier_) {
        util::validateIdentifier(identifier, "Processor", IVW_CONTEXT, " ()=&");
        if (network_ && network_->getProcessorByIdentifier(identifier) != nullptr) {
            throw Exception("Processor identifier \"" + identifier + "\" already in use.",
                            IVW_CONTEXT);
        }
        auto old = identifier_;
        identifier_ = identifier;
        notifyObserversIdentifierChanged(this, old);
    }
}

const std::string& Processor::getIdentifier() const { return identifier_; }

const std::string& Processor::getDisplayName() const { return displayName_; }
void Processor::setDisplayName(const std::string& displayName) {
    if (displayName_ != displayName) {
        auto old = displayName_;
        displayName_ = displayName;
        notifyObserversDisplayNameChanged(this, old);
    }
}

void Processor::setProcessorWidget(std::unique_ptr<ProcessorWidget> processorWidget) {
    processorWidget_ = std::move(processorWidget);
}

ProcessorWidget* Processor::getProcessorWidget() const { return processorWidget_.get(); }

bool Processor::hasProcessorWidget() const { return (processorWidget_ != nullptr); }

void Processor::setNetwork(ProcessorNetwork* network) { network_ = network; }

Port* Processor::getPort(const std::string& identifier) const {
    for (auto port : inports_)
        if (port->getIdentifier() == identifier) return port;
    for (auto port : outports_)
        if (port->getIdentifier() == identifier) return port;
    return nullptr;
}

Inport* Processor::getInport(const std::string& identifier) const {
    for (auto port : inports_)
        if (port->getIdentifier() == identifier) return port;
    return nullptr;
}

Outport* Processor::getOutport(const std::string& identifier) const {
    for (auto port : outports_)
        if (port->getIdentifier() == identifier) return port;
    return nullptr;
}

const std::vector<Inport*>& Processor::getInports() const { return inports_; }

const std::vector<Outport*>& Processor::getOutports() const { return outports_; }

const std::string& Processor::getPortGroup(Port* port) const {
    auto it = portGroups_.find(port);
    if (it != portGroups_.end()) {
        return it->second;
    } else {
        throw Exception("Can't find group for port: \"" + port->getIdentifier() + "\".",
                        IVW_CONTEXT);
    }
}

std::vector<std::string> Processor::getPortGroups() const {
    std::vector<std::string> groups;
    for (const auto& item : groupPorts_) {
        groups.push_back(item.first);
    }
    return groups;
}

const std::vector<Port*>& Processor::getPortsInGroup(const std::string& portGroup) const {
    auto it = groupPorts_.find(portGroup);
    if (it != groupPorts_.end()) {
        return it->second;
    } else {
        throw Exception("Can't find port group: \"" + portGroup + "\".", IVW_CONTEXT);
    }
}

const std::vector<Port*>& Processor::getPortsInSameGroup(Port* port) const {
    return getPortsInGroup(getPortGroup(port));
}

void Processor::invalidate(InvalidationLevel invalidationLevel, Property* modifiedProperty) {
    notifyObserversInvalidationBegin(this);
    PropertyOwner::invalidate(invalidationLevel, modifiedProperty);
    if (!isValid()) {
        for (auto& port : outports_) port->invalidate(InvalidationLevel::InvalidOutput);
    }
    notifyObserversInvalidationEnd(this);
}

bool Processor::isSource() const { return isSource_; }

bool Processor::isSink() const { return isSink_; }

bool Processor::isReady() const { return isReady_; }

bool Processor::allInportsAreReady() const {
    return util::all_of(inports_, [](Inport* p) { return p->isReady() || p->isOptional(); });
}

bool Processor::allInportsConnected() const {
    return util::all_of(inports_, [](Inport* p) { return p->isConnected(); });
}

void Processor::addInteractionHandler(InteractionHandler* interactionHandler) {
    util::push_back_unique(interactionHandlers_, interactionHandler);
}

void Processor::removeInteractionHandler(InteractionHandler* interactionHandler) {
    util::erase_remove(interactionHandlers_, interactionHandler);
}

bool Processor::hasInteractionHandler() const { return !interactionHandlers_.empty(); }

const std::vector<InteractionHandler*>& Processor::getInteractionHandlers() const {
    return interactionHandlers_;
}

void Processor::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("identifier", identifier_, SerializationTarget::Attribute);
    s.serialize("displayName", displayName_, SerializationTarget::Attribute);

    s.serialize("InteractonHandlers", interactionHandlers_, "InteractionHandler");

    std::map<std::string, std::string> portGroups;
    for (auto& item : portGroups_) portGroups[item.first->getIdentifier()] = item.second;
    s.serialize("PortGroups", portGroups, "PortGroup");

    auto ownedInportIds = util::transform(
        ownedInports_, [](const std::unique_ptr<Inport>& p) { return p->getIdentifier(); });
    s.serialize("OwnedInportIdentifiers", ownedInportIds, "InportIdentifier");

    auto ownedOutportIds = util::transform(
        ownedOutports_, [](const std::unique_ptr<Outport>& p) { return p->getIdentifier(); });
    s.serialize("OwnedOutportIdentifiers", ownedOutportIds, "OutportIdentifier");

    s.serialize("InPorts", inports_, "InPort");
    s.serialize("OutPorts", outports_, "OutPort");

    PropertyOwner::serialize(s);
    MetaDataOwner::serialize(s);
}

void Processor::deserialize(Deserializer& d) {
    d.deserialize("identifier", identifier_, SerializationTarget::Attribute);
    d.deserialize("displayName", displayName_, SerializationTarget::Attribute);

    d.deserialize("InteractonHandlers", interactionHandlers_, "InteractionHandler");

    std::map<std::string, std::string> portGroups;
    d.deserialize("PortGroups", portGroups, "PortGroup");

    {
        std::vector<std::string> ownedInportIds;
        d.deserialize("OwnedInportIdentifiers", ownedInportIds, "InportIdentifier");

        auto desInports =
            util::IdentifiedDeserializer<std::string, Inport*>("InPorts", "InPort")
                .setGetId([](Inport* const& port) { return port->getIdentifier(); })
                .setMakeNew([]() { return nullptr; })
                .setNewFilter([&](const std::string& id, size_t /*ind*/) {
                    return util::contains(ownedInportIds, id);
                })
                .onNew([&](Inport*& port) {
                    addPort(std::unique_ptr<Inport>(port), portGroups[port->getIdentifier()]);
                })
                .onRemove([&](const std::string& id) {
                    if (util::contains_if(ownedInports_, [&](std::unique_ptr<Inport>& op) {
                            return op->getIdentifier() == id;
                        })) {
                        delete removePort(id);
                    }
                });

        desInports(d, inports_);
    }
    {
        std::vector<std::string> ownedOutportIds;
        d.deserialize("OwnedOutportIdentifiers", ownedOutportIds, "OutportIdentifier");

        auto desOutports =
            util::IdentifiedDeserializer<std::string, Outport*>("OutPorts", "OutPort")
                .setGetId([](Outport* const& port) { return port->getIdentifier(); })
                .setMakeNew([]() { return nullptr; })
                .setNewFilter([&](const std::string& id, size_t /*ind*/) {
                    return util::contains(ownedOutportIds, id);
                })
                .onNew([&](Outport*& port) {
                    addPort(std::unique_ptr<Outport>(port), portGroups[port->getIdentifier()]);
                })
                .onRemove([&](const std::string& id) {
                    if (util::contains_if(ownedOutports_, [&](std::unique_ptr<Outport>& op) {
                            return op->getIdentifier() == id;
                        })) {
                        delete removePort(id);
                    }
                });

        desOutports(d, outports_);
    }

    PropertyOwner::deserialize(d);
    MetaDataOwner::deserialize(d);
}

void Processor::setValid() {
    PropertyOwner::setValid();
    for (auto inport : inports_) inport->setChanged(false);
    for (auto outport : outports_) outport->setValid();
}

void Processor::invokeEvent(Event* event) {
    if (event->hash() == PickingEvent::chash()) {
        static_cast<PickingEvent*>(event)->invoke(this);
    }
    if (event->hasBeenUsed()) return;

    PropertyOwner::invokeEvent(event);
    if (event->hasBeenUsed()) return;

    for (auto elem : interactionHandlers_) elem->invokeEvent(event);
}

void Processor::propagateEvent(Event* event, Outport* source) {
    if (!event->markAsVisited(this)) return;

    invokeEvent(event);
    bool used = event->hasBeenUsed();
    if (used) return;

    for (auto inport : getInports()) {
        if (event->shouldPropagateTo(inport, this, source)) {
            inport->propagateEvent(event);
            used |= event->markAsUnused();
        }
    }
    event->setUsed(used);
}

std::vector<std::string> Processor::getPath() const {
    std::vector<std::string> path;
    path.push_back(util::stripIdentifier(identifier_));
    return path;
}

}  // namespace inviwo
