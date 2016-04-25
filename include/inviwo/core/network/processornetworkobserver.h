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

#ifndef IVW_PROCESSOR_NETWORK_OBSERVER_H
#define IVW_PROCESSOR_NETWORK_OBSERVER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class Processor;
class PortConnection;
class PropertyLink;
class ProcessorNetworkObservable;

class IVW_CORE_API ProcessorNetworkObserver: public Observer {
public:
    ProcessorNetworkObserver() = default;
    virtual ~ProcessorNetworkObserver() = default;
    friend ProcessorNetworkObservable;
    /**
    * This method will be called when observed object changes.
    * Override it to add behavior.
    */
    virtual void onProcessorNetworkChange() {};
    virtual void onProcessorNetworkEvaluateRequest() {};
    virtual void onProcessorNetworkUnlocked() {};

    // Processors
    virtual void onProcessorNetworkWillAddProcessor(Processor* processor) {};
    virtual void onProcessorNetworkDidAddProcessor(Processor* processor) {};
    virtual void onProcessorNetworkWillRemoveProcessor(Processor* processor) {};
    virtual void onProcessorNetworkDidRemoveProcessor(Processor* processor) {};

    // Connections
    virtual void onProcessorNetworkWillAddConnection(const PortConnection& connection) {};
    virtual void onProcessorNetworkDidAddConnection(const PortConnection& connection) {};
    virtual void onProcessorNetworkWillRemoveConnection(const PortConnection& connection) {};
    virtual void onProcessorNetworkDidRemoveConnection(const PortConnection& connection) {};

    // Links
    virtual void onProcessorNetworkWillAddLink(const PropertyLink& propertyLink) {};
    virtual void onProcessorNetworkDidAddLink(const PropertyLink& propertyLink) {};
    virtual void onProcessorNetworkWillRemoveLink(const PropertyLink& propertyLink) {};
    virtual void onProcessorNetworkDidRemoveLink(const PropertyLink& propertyLink) {};
};

class IVW_CORE_API ProcessorNetworkObservable: public Observable<ProcessorNetworkObserver> {
public:
    ProcessorNetworkObservable() = default;
    virtual ~ProcessorNetworkObservable() = default;

    void notifyObserversProcessorNetworkChanged();
    void notifyObserversProcessorNetworkEvaluateRequest();
    void notifyObserversProcessorNetworkUnlocked();

    // Processors
    void notifyObserversProcessorNetworkWillAddProcessor(Processor* processor);
    void notifyObserversProcessorNetworkDidAddProcessor(Processor* processor);
    void notifyObserversProcessorNetworkWillRemoveProcessor(Processor* processor);
    void notifyObserversProcessorNetworkDidRemoveProcessor(Processor* processor);

    // Connections
    void notifyObserversProcessorNetworkWillAddConnection(const PortConnection& connection);
    void notifyObserversProcessorNetworkDidAddConnection(const PortConnection& connection);
    void notifyObserversProcessorNetworkWillRemoveConnection(const PortConnection& connection);
    void notifyObserversProcessorNetworkDidRemoveConnection(const PortConnection& connection);

    // Links
    void notifyObserversProcessorNetworkWillAddLink(const PropertyLink& propertyLink);
    void notifyObserversProcessorNetworkDidAddLink(const PropertyLink& propertyLink);
    void notifyObserversProcessorNetworkWillRemoveLink(const PropertyLink& propertyLink);
    void notifyObserversProcessorNetworkDidRemoveLink(const PropertyLink& propertyLink);

};

} // namespace

#endif // IVW_PROCESSOR_NETWORK_OBSERVER_H
