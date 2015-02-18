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
    // Notify observers
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        // static_cast can be used since only template class objects can be added
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkChange();
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkEvaluateRequest() const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkEvaluateRequest();
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkUnlocked() const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkUnlocked();
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillAddProcessor(Processor* processor) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkWillAddProcessor(processor);
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidAddProcessor(Processor* processor) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkDidAddProcessor(processor);
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillRemoveProcessor(Processor* processor) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkWillRemoveProcessor(processor);
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidRemoveProcessor(Processor* processor) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkDidRemoveProcessor(processor);
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillAddConnection(PortConnection* portConnection) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkWillAddConnection(portConnection);
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidAddConnection(PortConnection* portConnection) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkDidAddConnection(portConnection);
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillRemoveConnection(PortConnection* portConnection) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkWillRemoveConnection(portConnection);
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidRemoveConnection(PortConnection* portConnection) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkDidRemoveConnection(portConnection);
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillAddLink(PropertyLink* propertyLink) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkWillAddLink(propertyLink);
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidAddLink(PropertyLink* propertyLink) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkDidAddLink(propertyLink);
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkWillRemoveLink(PropertyLink* propertyLink) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkWillRemoveLink(propertyLink);
    }
}

void ProcessorNetworkObservable::notifyObserversProcessorNetworkDidRemoveLink(PropertyLink* propertyLink) const {
    for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<ProcessorNetworkObserver*>(*it)->onProcessorNetworkDidRemoveLink(propertyLink);
    }
}


} // namespace
