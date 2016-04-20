/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/network/processornetworkobserver.h>

namespace inviwo {

void ProcessorNetworkObservable::notifyObserversProcessorNetworkChanged() const {
    for (auto o : observers_) o->onProcessorNetworkChange();
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkEvaluateRequest() const {
    for (auto o : observers_) o->onProcessorNetworkEvaluateRequest();
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkUnlocked() const {
    for (auto o : observers_) o->onProcessorNetworkUnlocked();
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillAddProcessor(
    Processor* processor) const {
    for (auto o : observers_) o->onProcessorNetworkWillAddProcessor(processor);
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidAddProcessor(
    Processor* processor) const {
    for (auto o : observers_) o->onProcessorNetworkDidAddProcessor(processor);
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillRemoveProcessor(
    Processor* processor) const {
    for (auto o : observers_) o->onProcessorNetworkWillRemoveProcessor(processor);
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidRemoveProcessor(
    Processor* processor) const {
    for (auto o : observers_) o->onProcessorNetworkDidRemoveProcessor(processor);
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillAddConnection(
    const PortConnection& portConnection) const {
    for (auto o : observers_) o->onProcessorNetworkWillAddConnection(portConnection);
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidAddConnection(
    const PortConnection& portConnection) const {
    for (auto o : observers_) o->onProcessorNetworkDidAddConnection(portConnection);
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillRemoveConnection(
    const PortConnection& portConnection) const {
    for (auto o : observers_) o->onProcessorNetworkWillRemoveConnection(portConnection);
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidRemoveConnection(
    const PortConnection& portConnection) const {
    for (auto o : observers_) o->onProcessorNetworkDidRemoveConnection(portConnection);
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillAddLink(
    const PropertyLink& propertyLink) const {
    for (auto o : observers_) o->onProcessorNetworkWillAddLink(propertyLink);
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidAddLink(
    const PropertyLink& propertyLink) const {
    for (auto o : observers_) o->onProcessorNetworkDidAddLink(propertyLink);
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillRemoveLink(
    const PropertyLink& propertyLink) const {
    for (auto o : observers_) o->onProcessorNetworkWillRemoveLink(propertyLink);
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidRemoveLink(
    const PropertyLink& propertyLink) const {
    for (auto o : observers_) o->onProcessorNetworkDidRemoveLink(propertyLink);
}

}  // namespace
