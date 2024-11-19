/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
#include <inviwo/core/network/networkvisitor.h>
#include <inviwo/core/network/processornetwork.h>

#include <fmt/format.h>

namespace inviwo {

std::function<ProcessorStatus()> Processor::getDefaultIsReadyUpdater(Processor* p) {
    return [p]() -> ProcessorStatus {
        if (p->allInportsAreReady()) {
            return ProcessorStatus::Ready;
        } else {
            static constexpr std::string_view reason{"Inports are not ready"};
            return {ProcessorStatus::NotReady, reason};
        }
    };
}

Processor::Processor(std::string_view identifier, std::string_view displayName)
    : PropertyOwner()
    , ProcessorObservable()
    , processorWidget_(nullptr)
    , isReady_{true, [this](const bool&) { notifyObserversReadyChange(this); },
               getDefaultIsReadyUpdater(this)}
    , isSink_{true, [this](const bool&) { notifyObserversSinkChange(this); },
              [this]() { return outports_.empty(); }}
    , isSource_{true, [this](const bool&) { notifyObserversSourceChange(this); },
                [this]() { return inports_.empty(); }}
    , identifier_(identifier)
    , displayName_{displayName}
    , network_(nullptr) {

    if (!identifier_.empty()) {
        util::validateIdentifier(identifier_, "Processor", IVW_CONTEXT);
    }

    createMetaData<ProcessorMetaData>(ProcessorMetaData::classIdentifier);
}

Processor::~Processor() = default;

void Processor::addPortInternal(Inport* port, std::string_view portGroup) {
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

void Processor::addPortInternal(Outport* port, std::string_view portGroup) {
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

Port* Processor::removePort(std::string_view identifier) {
    if (auto inport = getInport(identifier)) {
        return removePort(inport);
    }
    if (auto outport = getOutport(identifier)) {
        return removePort(outport);
    }
    return nullptr;
}

Inport* Processor::removePort(Inport* port) {
    notifyObserversProcessorPortRemoved(this, port);
    port->isReady_.setNotify([](const bool&) {});
    port->isOptional_.setNotify([](const bool&) {});
    port->setProcessor(nullptr);
    std::erase(inports_, port);
    removePortFromGroups(port);

    // This will delete the port if owned
    std::erase_if(ownedInports_, [&port](const std::unique_ptr<Inport>& p) {
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
    std::erase(outports_, port);
    removePortFromGroups(port);

    // This will delete the port if owned
    std::erase_if(ownedOutports_, [&port](const std::unique_ptr<Outport>& p) {
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

void Processor::accept(NetworkVisitor& visitor) {
    if (visitor.enter(*this)) {
        for (auto* elem : properties_) {
            elem->accept(visitor);
        }
        visitor.exit(*this);
    }
}

void Processor::addPortToGroup(Port* port, std::string_view portGroup) {
    portGroups_[port] = portGroup;
    groupPorts_[std::string(portGroup)].push_back(port);
}

void Processor::removePortFromGroups(Port* port) {
    auto group = portGroups_[port];
    std::erase(groupPorts_[group], port);
    if (groupPorts_[group].empty()) groupPorts_.erase(group);
    portGroups_.erase(port);
}

const std::string& Processor::getClassIdentifier() const {
    return getProcessorInfo().classIdentifier;
}

const std::string& Processor::getCategory() const { return getProcessorInfo().category; }

CodeState Processor::getCodeState() const { return getProcessorInfo().codeState; }

const Tags& Processor::getTags() const { return getProcessorInfo().tags; }

void Processor::setIdentifier(std::string_view identifier) {
    if (identifier != identifier_) {
        util::validateIdentifier(identifier, "Processor", IVW_CONTEXT);
        if (network_ && network_->getProcessorByIdentifier(identifier) != nullptr) {
            throw Exception(IVW_CONTEXT, "Processor identifier \"{}\" already in use.", identifier);
        }
        auto old = identifier_;
        identifier_ = identifier;
        identifierDispatcher_.invoke(identifier_, old);
    }
}

const std::string& Processor::getIdentifier() const { return identifier_; }

auto Processor::onIdentifierChange(std::function<void(std::string_view, std::string_view)> callback)
    -> NameDispatcherHandle {
    return identifierDispatcher_.add(callback);
}

const std::string& Processor::getDisplayName() const { return displayName_; }
void Processor::setDisplayName(std::string_view displayName) {
    if (displayName_ != displayName) {
        auto old = displayName_;
        displayName_ = displayName;
        displayNameDispatcher_.invoke(displayName_, old);
    }
}
auto Processor::onDisplayNameChange(
    std::function<void(std::string_view, std::string_view)> callback) -> NameDispatcherHandle {
    return displayNameDispatcher_.add(callback);
}

void Processor::setProcessorWidget(std::unique_ptr<ProcessorWidget> processorWidget) {
    processorWidget_ = std::move(processorWidget);
}

ProcessorWidget* Processor::getProcessorWidget() const { return processorWidget_.get(); }

bool Processor::hasProcessorWidget() const { return (processorWidget_ != nullptr); }

void Processor::setNetwork(ProcessorNetwork* network) { network_ = network; }

Port* Processor::getPort(std::string_view identifier) const {
    for (auto port : inports_)
        if (port->getIdentifier() == identifier) return port;
    for (auto port : outports_)
        if (port->getIdentifier() == identifier) return port;
    return nullptr;
}

Inport* Processor::getInport(std::string_view identifier) const {
    for (auto port : inports_)
        if (port->getIdentifier() == identifier) return port;
    return nullptr;
}

Outport* Processor::getOutport(std::string_view identifier) const {
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
        throw Exception(IVW_CONTEXT, "Can't find group for port: \"{}\".", port->getIdentifier());
    }
}

std::vector<std::string> Processor::getPortGroups() const {
    std::vector<std::string> groups;
    for (const auto& item : groupPorts_) {
        groups.push_back(item.first);
    }
    return groups;
}

const std::vector<Port*>& Processor::getPortsInGroup(std::string_view portGroup) const {
    auto it = groupPorts_.find(std::string(portGroup));
    if (it != groupPorts_.end()) {
        return it->second;
    } else {
        throw Exception(IVW_CONTEXT, "Can't find port group: \"{}\".", portGroup);
    }
}

const std::vector<Port*>& Processor::getPortsInSameGroup(Port* port) const {
    return getPortsInGroup(getPortGroup(port));
}

void Processor::invalidate(InvalidationLevel invalidationLevel, Property* modifiedProperty) {
    notifyObserversInvalidationBegin(this);
    PropertyOwner::invalidate(invalidationLevel, modifiedProperty);
    if (!isValid()) {
        // We need to always propagate the invalidation here even if we have aleady done so before
        // since processors with optional inports can have become valid while this is still
        // invalid. Hence we need to make sure we invalidate them again
        for (auto& port : outports_) port->invalidate(InvalidationLevel::InvalidOutput);
    }
    notifyObserversInvalidationEnd(this);
}

bool Processor::isSource() const { return isSource_; }

bool Processor::isSink() const { return isSink_; }

bool Processor::isReady() const { return isReady_.get(); }

const ProcessorStatus& Processor::status() const { return isReady_.get(); }

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
    std::erase(interactionHandlers_, interactionHandler);
}

bool Processor::hasInteractionHandler() const { return !interactionHandlers_.empty(); }

const std::vector<InteractionHandler*>& Processor::getInteractionHandlers() const {
    return interactionHandlers_;
}

InviwoApplication* Processor::getInviwoApplication() {
    return network_ ? network_->getApplication() : nullptr;
}

void Processor::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("identifier", identifier_, SerializationTarget::Attribute);
    s.serialize("displayName", displayName_, SerializationTarget::Attribute);

    s.serialize("InteractonHandlers", interactionHandlers_, "InteractionHandler");

    s.serialize(
        "PortGroups", portGroups_, "PortGroup",
        [&](const auto& pair) {
            return util::contains_if(ownedInports_,
                                     [&](const auto& p) { return p.get() == pair.first; }) ||
                   util::contains_if(ownedOutports_,
                                     [&](const auto& p) { return p.get() == pair.first; });
        },
        util::identifier{});

    s.serialize("OwnedInportIdentifiers", ownedInports_, "InportIdentifier", util::alwaysTrue{},
                util::identifier{});
    s.serialize("OwnedOutportIdentifiers", ownedOutports_, "OutportIdentifier", util::alwaysTrue{},
                util::identifier{});

    s.serialize("InPorts", ownedInports_, "InPort");
    s.serialize("OutPorts", ownedOutports_, "OutPort");

    PropertyOwner::serialize(s);
    MetaDataOwner::serialize(s);
}

void Processor::deserialize(Deserializer& d) {
    d.deserialize("identifier", identifier_, SerializationTarget::Attribute);

    d.deserialize("displayName", displayName_, SerializationTarget::Attribute);

    d.deserialize("InteractonHandlers", interactionHandlers_, "InteractionHandler");

    UnorderedStringMap<std::string> portGroups;
    d.deserialize("PortGroups", portGroups, "PortGroup");

    {
        std::vector<std::string> ownedInportIds;
        d.deserialize("OwnedInportIdentifiers", ownedInportIds, "InportIdentifier");

        util::identified_deserializer::deserialize(
            d, "InPorts", "InPort", inports_,
            util::identified_deserializer::Functions{
                .getID = [](Inport* const& port) -> std::string_view {
                    return port->getIdentifier();
                },
                .makeNew = []() -> Inport* { return nullptr; },
                .filter = [&](std::string_view id,
                              size_t) { return util::contains(ownedInportIds, id); },
                .onNew =
                    [&](Inport*& port, size_t) {
                        addPort(std::unique_ptr<Inport>(port), portGroups[port->getIdentifier()]);
                    },
                .onRemove =
                    [&](std::string_view id) {
                        if (util::contains_if(ownedInports_, [&](std::unique_ptr<Inport>& op) {
                                return op->getIdentifier() == id;
                            })) {
                            delete removePort(id);
                        }
                    }});
        /*
        auto desInports =
            util::IdentifiedDeserializer<Inport*>("InPorts", "InPort")
                .setGetId(
                    [](Inport* const& port) -> std::string_view { return port->getIdentifier(); })
                .setMakeNew([]() { return nullptr; })
                .setNewFilter(
                    [&](std::string_view id, size_t) { return util::contains(ownedInportIds, id); })
                .onNew([&](Inport*& port) {
                    addPort(std::unique_ptr<Inport>(port), portGroups[port->getIdentifier()]);
                })
                .onRemove([&](std::string_view id) {
                    if (util::contains_if(ownedInports_, [&](std::unique_ptr<Inport>& op) {
                            return op->getIdentifier() == id;
                        })) {
                        delete removePort(id);
                    }
                });
        
        desInports(d, inports_);
        */
    }
    {
        std::vector<std::string> ownedOutportIds;
        d.deserialize("OwnedOutportIdentifiers", ownedOutportIds, "OutportIdentifier");

        util::identified_deserializer::deserialize(
            d, "OutPorts", "OutPort", outports_,
            util::identified_deserializer::Functions{
                .getID = [](Outport* const& port) -> std::string_view {
                    return port->getIdentifier();
                },
                .makeNew = []() -> Outport* { return nullptr; },
                .filter = [&](std::string_view id,
                              size_t) { return util::contains(ownedOutportIds, id); },
                .onNew =
                    [&](Outport*& port, size_t) {
                        addPort(std::unique_ptr<Outport>(port), portGroups[port->getIdentifier()]);
                    },
                .onRemove =
                    [&](std::string_view id) {
                        if (util::contains_if(ownedOutports_, [&](std::unique_ptr<Outport>& op) {
                                return op->getIdentifier() == id;
                            })) {
                            delete removePort(id);
                        }
                    }});

        /*
        auto desOutports =
            util::IdentifiedDeserializer<Outport*>("OutPorts", "OutPort")
                .setGetId(
                    [](Outport* const& port) -> std::string_view { return port->getIdentifier(); })
                .setMakeNew([]() { return nullptr; })
                .setNewFilter([&](std::string_view id, size_t) {
                    return util::contains(ownedOutportIds, id);
                })
                .onNew([&](Outport*& port) {
                    addPort(std::unique_ptr<Outport>(port), portGroups[port->getIdentifier()]);
                })
                .onRemove([&](std::string_view id) {
                    if (util::contains_if(ownedOutports_, [&](std::unique_ptr<Outport>& op) {
                            return op->getIdentifier() == id;
                        })) {
                        delete removePort(id);
                    }
                });

        desOutports(d, outports_);
        */
    }

    PropertyOwner::deserialize(d);
    MetaDataOwner::deserialize(d);
}

void Processor::setValid() {
    PropertyOwner::setValid();
    setInportsChanged(false);
    for (auto* outport : outports_) outport->setValid();
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

void Processor::setInportsChanged(bool changed) {
    for (auto* inport : inports_) inport->setChanged(changed);
}

}  // namespace inviwo
