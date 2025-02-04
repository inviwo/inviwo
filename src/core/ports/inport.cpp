/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/docutils.h>
#include <inviwo/core/network/networkutils.h>

namespace inviwo {

Inport::Inport(std::string_view identifier, Document help)
    : Port(identifier, std::move(help))
    , isReady_{false, [](const bool& /*isReady*/) {},
               [this]() {
                   return (isConnected() && util::all_of(connectedOutports_,
                                                         [](Outport* p) { return p->isReady(); }));
               }}
    , isOptional_(
          false, [](const bool& /*isOptional*/) {}, []() { return false; })
    , changed_(false)
    , lastInvalidationLevel_(InvalidationLevel::Valid) {}

Inport::~Inport() = default;

bool Inport::isConnected() const { return !connectedOutports_.empty(); }

bool Inport::isReady() const { return isReady_; }

void Inport::readyUpdate() { isReady_.update(); }

bool Inport::isOptional() const { return isOptional_; }

void Inport::setOptional(bool optional) {
    isOptional_.setUpdate([optional]() { return optional; });
}

void Inport::invalidate(InvalidationLevel invalidationLevel) {
    if (lastInvalidationLevel_ == InvalidationLevel::Valid &&
        invalidationLevel >= InvalidationLevel::InvalidOutput)
        onInvalidCallback_.invokeAll();
    lastInvalidationLevel_ = std::max(lastInvalidationLevel_, invalidationLevel);

    if (processor_) processor_->invalidate(invalidationLevel);
}

void Inport::setValid(const Outport* source) {
    lastInvalidationLevel_ = InvalidationLevel::Valid;
    setChanged(true, source);
}

size_t Inport::getNumberOfConnections() const { return connectedOutports_.size(); }

const std::vector<const Outport*>& Inport::getChangedOutports() const { return changedSources_; }

void Inport::propagateEvent(Event* event, Outport* target) {
    if (target) {
        target->propagateEvent(event, this);
    } else {
        bool used = event->hasBeenUsed();
        for (auto outport : getConnectedOutports()) {
            outport->propagateEvent(event, this);
            used |= event->markAsUnused();
        }
        event->setUsed(used);
    }
}

void Inport::setChanged(bool changed, const Outport* source) {
    changed_ = changed;

    if (changed_ == false) {
        if (source == nullptr) {
            changedSources_.clear();
        } else {
            std::erase(changedSources_, source);
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
        isReady_.update();
        onConnectCallback_.invokeAll();
        onConnectDispatcher_.invoke(outport);
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void Inport::disconnectFrom(Outport* outport) {
    auto it = std::find(connectedOutports_.begin(), connectedOutports_.end(), outport);
    if (it != connectedOutports_.end()) {
        connectedOutports_.erase(it);
        outport->disconnectFrom(this);  // remove this from outport.
        setChanged(true, outport);      // mark that we should call onChange.
        isReady_.update();
        onDisconnectCallback_.invokeAll();
        onDisconnectDispatcher_.invoke(outport);
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

const BaseCallBack* Inport::onChange(std::function<void()> lambda) {
    return onChangeCallback_.addLambdaCallback(lambda);
}

std::shared_ptr<std::function<void()>> Inport::onChangeScoped(std::function<void()> lambda) {
    return onChangeCallback_.addLambdaCallbackRaii(lambda);
}

void Inport::removeOnChange(const BaseCallBack* callback) { onChangeCallback_.remove(callback); }

const BaseCallBack* Inport::onInvalid(std::function<void()> lambda) {
    return onInvalidCallback_.addLambdaCallback(lambda);
}
std::shared_ptr<std::function<void()>> Inport::onInvalidScoped(std::function<void()> lambda) {
    return onInvalidCallback_.addLambdaCallbackRaii(lambda);
}
void Inport::removeOnInvalid(const BaseCallBack* callback) { onInvalidCallback_.remove(callback); }

const BaseCallBack* Inport::onConnect(std::function<void()> lambda) {
    return onConnectCallback_.addLambdaCallback(lambda);
}
void Inport::removeOnConnect(const BaseCallBack* callback) { onConnectCallback_.remove(callback); }

const BaseCallBack* Inport::onDisconnect(std::function<void()> lambda) {
    return onDisconnectCallback_.addLambdaCallback(lambda);
}
void Inport::removeOnDisconnect(const BaseCallBack* callback) {
    onDisconnectCallback_.remove(callback);
}

std::shared_ptr<std::function<void()>> Inport::onConnectScoped(std::function<void()> lambda) {
    return onConnectCallback_.addLambdaCallbackRaii(lambda);
}
std::shared_ptr<std::function<void()>> Inport::onDisconnectScoped(std::function<void()> lambda) {
    return onDisconnectCallback_.addLambdaCallbackRaii(lambda);
}

std::shared_ptr<std::function<void(Outport*)>> Inport::onConnectScoped(
    std::function<void(Outport*)> lambda) {
    return onConnectDispatcher_.add(std::move(lambda));
}
std::shared_ptr<std::function<void(Outport*)>> Inport::onDisconnectScoped(
    std::function<void(Outport*)> lambda) {
    return onDisconnectDispatcher_.add(std::move(lambda));
}

void Inport::setIsReadyUpdater(std::function<bool()> updater) {
    isReady_.setUpdate(std::move(updater));
}

bool Inport::circularConnection(const Port* port) const {
    // Check for circular depends.
    auto pd = util::getPredecessors(port->getProcessor());
    return pd.find(getProcessor()) != pd.end();
}

Document Inport::getDefaultPortInfo(const Inport* port, std::string_view portname) {
    Document doc;
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    doc.append("b", portname, {{"class", "name"}});

    if (!port->help_.empty()) {
        doc.append("div", "", {{"class", "help"}}).append(port->help_);
    }

    utildoc::TableBuilder tb(doc.handle(), P::end());
    tb(H("Identifier"), port->getIdentifier());
    tb(H("Class"), port->getClassIdentifier());
    tb(H("Ready"), port->isReady());
    tb(H("Connected"), port->isConnected());

    tb(H("Connections"),
       fmt::format("{} ({})", port->getNumberOfConnections(), port->getMaxNumberOfConnections()));
    tb(H("Optional"), port->isOptional());
    return doc;
}

}  // namespace inviwo
