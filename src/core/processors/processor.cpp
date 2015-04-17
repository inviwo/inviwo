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

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/metadata/metadatafactory.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

std::set<std::string> Processor::usedIdentifiers_;

ProcessorClassIdentifier(Processor, "org.inviwo.Processor");
ProcessorDisplayName(Processor, "Processor");
ProcessorTags(Processor, Tags::None);
ProcessorCategory(Processor, "undefined");
ProcessorCodeState(Processor, CODE_STATE_EXPERIMENTAL);

Processor::Processor()
    : PropertyOwner()
    , ProcessorObservable()
    , processorWidget_(nullptr)
    , identifier_("")
    , initialized_(false)
    , invalidationEnabled_(true)
    , invalidationRequestLevel_(VALID) {
    createMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
}

Processor::~Processor() {
    usedIdentifiers_.erase(identifier_);
    portDependencySets_.deinitialize();
    if (processorWidget_) {
        processorWidget_->setProcessor(nullptr);
    }
}

void Processor::addPort(Inport* port, const std::string& portDependencySet) {
    // TODO: check if port with same name has been added before
    port->setProcessor(this);
    inports_.push_back(port);
    portDependencySets_.insert(portDependencySet, port);
    notifyObserversProcessorPortAdded(this, port);
}

void Processor::addPort(Inport& port, const std::string& portDependencySet) {
    addPort(&port, portDependencySet);
}

void Processor::addPort(Outport* port, const std::string& portDependencySet) {
    // TODO: check if port with same name has been added before
    port->setProcessor(this);
    outports_.push_back(port);
    portDependencySets_.insert(portDependencySet, port);
    notifyObserversProcessorPortAdded(this, port);
}

void Processor::addPort(Outport& port, const std::string& portDependencySet) {
    addPort(&port, portDependencySet);
}

std::string Processor::setIdentifier(const std::string& identifier) {
    if (identifier == identifier_)  // nothing changed
        return identifier_;

    if (usedIdentifiers_.find(identifier_) != usedIdentifiers_.end()) {
        usedIdentifiers_.erase(identifier_);  // remove old identifier
    }

    std::string baseIdentifier = identifier;
    std::string newIdentifier = identifier;
    int i = 2;

    while (usedIdentifiers_.find(newIdentifier) != usedIdentifiers_.end()) {
        newIdentifier = baseIdentifier + " " + toString(i++);
    }

    usedIdentifiers_.insert(newIdentifier);
    identifier_ = newIdentifier;

    notifyObserversIdentifierChange(this);
    return identifier_;
}

std::string Processor::getIdentifier() {
    if (identifier_.empty()) setIdentifier(getDisplayName());
    return identifier_;
}

void Processor::setProcessorWidget(ProcessorWidget* processorWidget) {
    processorWidget_ = processorWidget;
}

ProcessorWidget* Processor::getProcessorWidget() const { return processorWidget_; }

bool Processor::hasProcessorWidget() const { return (processorWidget_ != nullptr); }

Port* Processor::getPort(const std::string& identifier) const {
    for (auto port : inports_) if (port->getIdentifier() == identifier) return port;
    for (auto port : outports_) if (port->getIdentifier() == identifier) return port;
    return nullptr;
}

Inport* Processor::getInport(const std::string& identifier) const {
    for (auto port : inports_) if (port->getIdentifier() == identifier) return port;
    return nullptr;
}

Outport* Processor::getOutport(const std::string& identifier) const {
    for (auto port : outports_) if (port->getIdentifier() == identifier) return port;
    return nullptr;
}

const std::vector<Inport*>& Processor::getInports() const { return inports_; }

const std::vector<Outport*>& Processor::getOutports() const { return outports_; }

const std::vector<Inport*>& Processor::getInports(Event*) const { return inports_; }

std::vector<Port*> Processor::getPortsByDependencySet(const std::string& portDependencySet) const {
    return portDependencySets_.getGroupedData(portDependencySet);
}

std::vector<std::string> Processor::getPortDependencySets() const {
    return portDependencySets_.getGroupKeys();
}

std::string Processor::getPortDependencySet(Port* port) const {
    return portDependencySets_.getKey(port);
}

void Processor::initialize() { initialized_ = true; }

void Processor::deinitialize() { initialized_ = false; }

bool Processor::isInitialized() const { return initialized_; }

void Processor::invalidate(InvalidationLevel invalidationLevel, Property* modifiedProperty) {
    if (!invalidationEnabled_) {
        invalidationRequestLevel_ = invalidationLevel;
        return;
    }

    notifyObserversInvalidationBegin(this);
    PropertyOwner::invalidate(invalidationLevel, modifiedProperty);

    if (PropertyOwner::isValid()) {
        notifyObserversInvalidationEnd(this);
        return;
    }

    for (auto& port : outports_) port->invalidate(INVALID_OUTPUT);

    notifyObserversInvalidationEnd(this);

    if (isEndProcessor()) {
        performEvaluateRequest();
    }
}

bool Processor::isEndProcessor() const { return outports_.empty(); }

bool Processor::isReady() const { return allInportsAreReady(); }

bool Processor::allInportsAreReady() const {
    return util::all_of(inports_, [](Inport* p) { return p->isReady(); });
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

void Processor::invokeInteractionEvent(Event* event) {
    PropertyOwner::invokeInteractionEvent(event);
    for (auto& elem : interactionHandlers_) elem->invokeEvent(event);
}

void Processor::serialize(IvwSerializer& s) const {
    s.serialize("type", getClassIdentifier(), true);
    s.serialize("identifier", identifier_, true);

    s.serialize("InteractonHandlers", interactionHandlers_, "InteractionHandler");
    s.serialize("InPorts", inports_, "InPort");
    s.serialize("OutPorts", outports_, "OutPort");

    PropertyOwner::serialize(s);
    MetaDataOwner::serialize(s);
}

void Processor::deserialize(IvwDeserializer& d) {
    std::string identifier;
    d.deserialize("identifier", identifier, true);
    setIdentifier(identifier);  // Need to use setIdentifier to make sure we get a unique id.

    d.deserialize("InteractonHandlers", interactionHandlers_, "InteractionHandler");

    StandardIdentifier<Port> inportIdentifier;
    d.deserialize("InPorts", inports_, "InPort", inportIdentifier);
    d.deserialize("OutPorts", outports_, "OutPort", inportIdentifier);

    for (auto elem : inports_) {
        elem->setProcessor(this);
    }
    for (auto elem : outports_) {
        elem->setProcessor(this);
    }

    PropertyOwner::deserialize(d);
    MetaDataOwner::deserialize(d);
}

void Processor::setValid() {
    PropertyOwner::setValid();
    for (auto port : inports_) port->setChanged(false);
    for (auto port : outports_) port->setValid();
}

void Processor::enableInvalidation() {
    invalidationEnabled_ = true;
    if (invalidationRequestLevel_ > VALID) {
        invalidate(invalidationRequestLevel_);
        invalidationRequestLevel_ = VALID;
    }
}

void Processor::disableInvalidation() {
    invalidationRequestLevel_ = VALID;
    invalidationEnabled_ = false;
}

void Processor::performEvaluateRequest() { notifyObserversRequestEvaluate(this); }

const std::string Processor::getCodeStateString(CodeState state) {
    switch (state) {
        case CODE_STATE_STABLE:
            return "Stable";
        case CODE_STATE_BROKEN:
            return "Broken";
        case CODE_STATE_EXPERIMENTAL:
            return "Experimental";
        default:
            return "Unknown";
    }
}

std::vector<std::string> Processor::getPath() const {
    std::vector<std::string> path;
    path.push_back(identifier_);
    return path;
}

}  // namespace
