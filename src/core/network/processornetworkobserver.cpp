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

void ProcessorNetworkObservable::notifyObserversProcessorNetworkChanged() {
    forEachObserver([](ProcessorNetworkObserver* o) { o->onProcessorNetworkChange(); });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkEvaluateRequest() {
    forEachObserver([](ProcessorNetworkObserver* o) { o->onProcessorNetworkEvaluateRequest(); });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkUnlocked() {
    forEachObserver([](ProcessorNetworkObserver* o) { o->onProcessorNetworkUnlocked(); });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillAddProcessor(
    Processor* processor) {
    forEachObserver(
        [&](ProcessorNetworkObserver* o) { o->onProcessorNetworkWillAddProcessor(processor); });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidAddProcessor(
    Processor* processor) {
    forEachObserver(
        [&](ProcessorNetworkObserver* o) { o->onProcessorNetworkDidAddProcessor(processor); });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillRemoveProcessor(
    Processor* processor) {
    forEachObserver(
        [&](ProcessorNetworkObserver* o) { o->onProcessorNetworkWillRemoveProcessor(processor); });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidRemoveProcessor(
    Processor* processor) {
    forEachObserver(
        [&](ProcessorNetworkObserver* o) { o->onProcessorNetworkDidRemoveProcessor(processor); });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillAddConnection(
    const PortConnection& portConnection) {
    forEachObserver([&](ProcessorNetworkObserver* o) {
        o->onProcessorNetworkWillAddConnection(portConnection);
    });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidAddConnection(
    const PortConnection& portConnection) {
    forEachObserver([&](ProcessorNetworkObserver* o) {
        o->onProcessorNetworkDidAddConnection(portConnection);
    });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillRemoveConnection(
    const PortConnection& portConnection) {
    forEachObserver([&](ProcessorNetworkObserver* o) {
        o->onProcessorNetworkWillRemoveConnection(portConnection);
    });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidRemoveConnection(
    const PortConnection& portConnection) {
    forEachObserver([&](ProcessorNetworkObserver* o) {
        o->onProcessorNetworkDidRemoveConnection(portConnection);
    });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillAddLink(
    const PropertyLink& propertyLink) {
    forEachObserver(
        [&](ProcessorNetworkObserver* o) { o->onProcessorNetworkWillAddLink(propertyLink); });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidAddLink(
    const PropertyLink& propertyLink) {
    forEachObserver(
        [&](ProcessorNetworkObserver* o) { o->onProcessorNetworkDidAddLink(propertyLink); });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillRemoveLink(
    const PropertyLink& propertyLink) {
    forEachObserver(
        [&](ProcessorNetworkObserver* o) { o->onProcessorNetworkWillRemoveLink(propertyLink); });
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidRemoveLink(
    const PropertyLink& propertyLink) {
    forEachObserver(
        [&](ProcessorNetworkObserver* o) { o->onProcessorNetworkDidRemoveLink(propertyLink); });
}

}  // namespace
