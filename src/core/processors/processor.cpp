/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

namespace inviwo {

std::set<std::string> Processor::usedIdentifiers_;

ProcessorClassIdentifier(Processor, "org.inviwo.Processor");
ProcessorDisplayName(Processor,  "Processor");
ProcessorTags(Processor, Tags::None);
ProcessorCategory(Processor, "undefined");
ProcessorCodeState(Processor, CODE_STATE_EXPERIMENTAL);

Processor::Processor()
    : PropertyOwner(), ProcessorObservable()
    , processorWidget_(NULL)
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
        processorWidget_->setProcessor(NULL);
    }
}

void Processor::addPort(Inport* port, const std::string &portDependencySet) {
    // TODO: check if port with same name has been added before
    port->setProcessor(this);
    inports_.push_back(port);
    portDependencySets_.insert(portDependencySet, port);
    notifyObserversProcessorPortAdded(this,port);
}

void Processor::addPort(Inport& port, const std::string &portDependencySet) {
    addPort(&port, portDependencySet);
}

void Processor::addPort(Outport* port, const std::string &portDependencySet) {
    // TODO: check if port with same name has been added before
    port->setProcessor(this);
    outports_.push_back(port);
    portDependencySets_.insert(portDependencySet, port);
    notifyObserversProcessorPortAdded(this,port);
}

void Processor::addPort(Outport& port, const std::string &portDependencySet) {
    addPort(&port, portDependencySet);
}

std::string Processor::setIdentifier(const std::string& identifier) {
    if (identifier == identifier_) //nothing changed
        return identifier_;

    if (usedIdentifiers_.find(identifier_) != usedIdentifiers_.end()) {
        usedIdentifiers_.erase(identifier_); //remove old identifier
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

ProcessorWidget* Processor::getProcessorWidget() const {
    return processorWidget_;
}

bool Processor::hasProcessorWidget() const {
    return (processorWidget_ != NULL);
}

Port* Processor::getPort(const std::string &identifier) const {
    for (std::vector<Inport*>::const_iterator it = inports_.begin(); it != inports_.end(); ++it)
        if ((*it)->getIdentifier() == identifier)
            return (*it);

    for (std::vector<Outport*>::const_iterator it = outports_.begin(); it != outports_.end(); ++it)
        if ((*it)->getIdentifier() == identifier)
            return (*it);

    return NULL;
}

Inport* Processor::getInport(const std::string &identifier) const {
    for (std::vector<Inport*>::const_iterator it = inports_.begin(); it != inports_.end(); ++it)
        if ((*it)->getIdentifier() == identifier)
            return (*it);

    return NULL;
}

Outport* Processor::getOutport(const std::string &identifier) const {
    for (std::vector<Outport*>::const_iterator it = outports_.begin(); it != outports_.end(); ++it)
        if ((*it)->getIdentifier() == identifier)
            return (*it);

    return NULL;
}

const std::vector<Inport*>& Processor::getInports() const {
    return inports_;
}

const std::vector<Outport*>& Processor::getOutports() const {
    return outports_;
}

const std::vector<Inport*>& Processor::getInports(Event*) const {
    return inports_;
}

std::vector<Port*> Processor::getPortsByDependencySet(const std::string &portDependencySet) const {
    return portDependencySets_.getGroupedData(portDependencySet);
}

std::vector<std::string> Processor::getPortDependencySets() const {
    return portDependencySets_.getGroupKeys();
}

std::string Processor::getPortDependencySet(Port* port) const {
    return portDependencySets_.getKey(port);
}

bool Processor::allInportsConnected() const {
    for (std::vector<Inport*>::const_iterator it = inports_.begin(); it != inports_.end(); ++it)
        if (!(*it)->isConnected())
            return false;

    return true;
}

bool Processor::allInportsAreReady() const {
    for (std::vector<Inport*>::const_iterator it = inports_.begin(); it != inports_.end(); ++it)
        if (!(*it)->isReady())
            return false;

    return true;
}

void Processor::initialize() {
    for (std::vector<Inport*>::iterator it = inports_.begin(); it != inports_.end(); ++it)
        (*it)->initialize();

    for (std::vector<Outport*>::iterator it = outports_.begin(); it != outports_.end(); ++it)
        (*it)->initialize();

    initialized_ = true;
}

void Processor::deinitialize() {
    for (std::vector<Inport*>::iterator it = inports_.begin(); it != inports_.end(); ++it)
        (*it)->deinitialize();

    for (std::vector<Outport*>::iterator it = outports_.begin(); it != outports_.end(); ++it)
        (*it)->deinitialize();

    initialized_ = false;
}

bool Processor::isInitialized() const {
    return initialized_;
}

void Processor::invalidate(InvalidationLevel invalidationLevel, Property* modifiedProperty) {
    if(!invalidationEnabled_){
        invalidationRequestLevel_ = invalidationLevel;
        return;
    }

    notifyObserversInvalidationBegin(this);
    PropertyOwner::invalidate(invalidationLevel, modifiedProperty);


    if (PropertyOwner::isValid()) {
        notifyObserversInvalidationEnd(this);
        return;
    }

    invalidateSuccesors(invalidationLevel, modifiedProperty);

    notifyObserversInvalidationEnd(this);

    if (isEndProcessor()) {
        performEvaluateRequest();
    }
}

void Processor::invalidateSuccesors(InvalidationLevel invalidationLevel, Property* modifiedProperty) {
    for (std::vector<Outport*>::iterator it = outports_.begin(); it != outports_.end(); ++it){
        (*it)->invalidate(INVALID_OUTPUT);
    }
}

bool Processor::isEndProcessor() const {
    return outports_.empty();
}

bool Processor::isReady() const {
    return allInportsAreReady();
}

void Processor::process() {
}

void Processor::addInteractionHandler(InteractionHandler* interactionHandler) {
    // Make sure that the interaction handler is not added twice
    if (std::find(interactionHandlers_.begin(), interactionHandlers_.end(), interactionHandler) ==
        interactionHandlers_.end()) {
        interactionHandlers_.push_back(interactionHandler);
    }
}

void Processor::removeInteractionHandler(InteractionHandler* interactionHandler) {
    std::vector<InteractionHandler*>::iterator it =
        std::find(interactionHandlers_.begin(), interactionHandlers_.end(), interactionHandler);

    if (it != interactionHandlers_.end()) {
        interactionHandlers_.erase(it);
    }
}

bool Processor::hasInteractionHandler() const {
    return !interactionHandlers_.empty();
}

const std::vector<InteractionHandler*>& Processor::getInteractionHandlers() const {
    return interactionHandlers_;
}

void Processor::invokeInteractionEvent(Event* event) {
    PropertyOwner::invokeInteractionEvent(event);

    for (size_t i=0; i<interactionHandlers_.size(); i++)
        interactionHandlers_[i]->invokeEvent(event);
}

void Processor::serialize(IvwSerializer& s) const {
    s.serialize("type", getClassIdentifier(), true);
    s.serialize("identifier", identifier_, true);

    if (!interactionHandlers_.empty())
        s.serialize("InteractonHandlers", interactionHandlers_, "InteractionHandler");

    PropertyOwner::serialize(s);
    MetaDataOwner::serialize(s);
}

void Processor::deserialize(IvwDeserializer& d) {
    std::string identifier;
    d.deserialize("identifier", identifier, true);
    setIdentifier(identifier); // Need to use setIdentifier to make sure we get a unique id.

    if (interactionHandlers_.size() != 0)
        d.deserialize("InteractonHandlers", interactionHandlers_, "InteractionHandler");

    PropertyOwner::deserialize(d);
    MetaDataOwner::deserialize(d);
}

void Processor::setValid() {
    PropertyOwner::setValid();

    for (std::vector<Inport*>::iterator it = inports_.begin(); it != inports_.end(); ++it)
        (*it)->setChanged(false);

    for (std::vector<Outport*>::iterator it = outports_.begin(); it != outports_.end(); ++it)
        (*it)->setInvalidationLevel(VALID);
}

void Processor::enableInvalidation() {
    invalidationEnabled_ = true;
    if(invalidationRequestLevel_ > VALID){
        invalidate(invalidationRequestLevel_);
        invalidationRequestLevel_ = VALID;
    }
}

void Processor::disableInvalidation(){
    invalidationRequestLevel_ = VALID;
    invalidationEnabled_ = false;
}

void Processor::performEvaluateRequest() {
    notifyObserversRequestEvaluate(this);
}

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

} // namespace
