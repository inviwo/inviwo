/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

Inport::Inport(std::string identifier)
    : Port(identifier), changed_(false), optional_(false), lastInvalidationLevel_(InvalidationLevel::Valid) {}

Inport::~Inport() {}

bool Inport::isConnected() const { return !connectedOutports_.empty(); }

bool Inport::isReady() const {
    return isConnected() &&
           util::all_of(connectedOutports_, [](Outport* p) { return p->isReady(); });
}

bool Inport::isOptional() const { return optional_; }
void Inport::setOptional(bool optional) { optional_ = optional; }

void Inport::invalidate(InvalidationLevel invalidationLevel) {
    if (lastInvalidationLevel_ == InvalidationLevel::Valid && invalidationLevel >= InvalidationLevel::InvalidOutput)
        onInvalidCallback_.invokeAll();
    lastInvalidationLevel_ = std::max(lastInvalidationLevel_, invalidationLevel);

    if (processor_) processor_->invalidate(invalidationLevel);
}

void Inport::setValid(const Outport* source) {
    lastInvalidationLevel_ = InvalidationLevel::Valid;
    setChanged(true, source);
}

size_t Inport::getNumberOfConnections() const { return connectedOutports_.size(); }

std::vector<const Outport*> Inport::getChangedOutports() const { return changedSources_; }

void Inport::propagateEvent(Event* event, Outport* target) {
    if (target) {
        target->propagateEvent(event);
    } else {
        for (auto outport : getConnectedOutports()) {
            outport->propagateEvent(event);
        }
    }
}

void Inport::setChanged(bool changed, const Outport* source) {
    changed_ = changed;

    if (changed_ == false) {
        if (source == nullptr) {
            changedSources_.clear();
        } else {
            util::erase_remove(changedSources_, source);
        }
    } else if (source) {
        util::push_back_unique(changedSources_, source);
    }
}

bool Inport::isChanged() const { return changed_; }

void Inport::connectTo(Outport* outport) {
    if (!isConnectedTo(outport)) {
        connectedOutports_.push_back(outport);
        outport->connectTo(this);   // add this to the outport.
        setChanged(true, outport);  // mark that we should call onChange.
        onConnectCallback_.invokeAll();
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void Inport::disconnectFrom(Outport* outport) {
    auto it = std::find(connectedOutports_.begin(), connectedOutports_.end(), outport);
    if (it != connectedOutports_.end()) {
        connectedOutports_.erase(it);
        outport->disconnectFrom(this);  // remove this from outport.
        setChanged(true, outport);      // mark that we should call onChange.
        onDisconnectCallback_.invokeAll();
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

bool Inport::isConnectedTo(const Outport* outport) const {
    return std::find(connectedOutports_.begin(), connectedOutports_.end(), outport) !=
           connectedOutports_.end();
}

Outport* Inport::getConnectedOutport() const {
    if (!connectedOutports_.empty()) {
        return connectedOutports_.front();
    } else {
        return nullptr;
    }
}

const std::vector<Outport*>& Inport::getConnectedOutports() const { return connectedOutports_; }

void Inport::callOnChangeIfChanged() const {
    if (isChanged()) {
        onChangeCallback_.invokeAll();
    }
}

const BaseCallBack* Inport::onChange(std::function<void()> lambda) const {
    return onChangeCallback_.addLambdaCallback(lambda);
}

void Inport::removeOnChange(const BaseCallBack* callback) const {
    onChangeCallback_.remove(callback);
}

const BaseCallBack* Inport::onInvalid(std::function<void()> lambda) const {
    return onInvalidCallback_.addLambdaCallback(lambda);
}
void Inport::removeOnInvalid(const BaseCallBack* callback) const {
    onInvalidCallback_.remove(callback);
}

const BaseCallBack* Inport::onConnect(std::function<void()> lambda) const {
    return onConnectCallback_.addLambdaCallback(lambda);
}
void Inport::removeOnConnect(const BaseCallBack* callback) const {
    onConnectCallback_.remove(callback);
}
const BaseCallBack* Inport::onDisconnect(std::function<void()> lambda) const {
    return onDisconnectCallback_.addLambdaCallback(lambda);
}
void Inport::removeOnDisconnect(const BaseCallBack* callback) const {
    onDisconnectCallback_.remove(callback);
}

}  // namespace
