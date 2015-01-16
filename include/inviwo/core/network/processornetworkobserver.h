/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

class IVW_CORE_API ProcessorNetworkObserver: public Observer {
public:
    ProcessorNetworkObserver(): Observer() {};

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
    virtual void onProcessorNetworkWillAddConnection(PortConnection* connection) {};
    virtual void onProcessorNetworkDidAddConnection(PortConnection* connection) {};
    virtual void onProcessorNetworkWillRemoveConnection(PortConnection* connection) {};
    virtual void onProcessorNetworkDidRemoveConnection(PortConnection* connection) {};

    // Links
    virtual void onProcessorNetworkWillAddLink(PropertyLink* propertyLink) {};
    virtual void onProcessorNetworkDidAddLink(PropertyLink* propertyLink) {};
    virtual void onProcessorNetworkWillRemoveLink(PropertyLink* propertyLink) {};
    virtual void onProcessorNetworkDidRemoveLink(PropertyLink* propertyLink) {};
};

class IVW_CORE_API ProcessorNetworkObservable: public Observable<ProcessorNetworkObserver> {
public:
    ProcessorNetworkObservable(): Observable<ProcessorNetworkObserver>() {};

    void notifyObserversProcessorNetworkChanged() const;
    void notifyObserversProcessorNetworkEvaluateRequest() const;
    void notifyObserversProcessorNetworkUnlocked() const;

    // Processors
    void notifyObserversProcessorNetworkWillAddProcessor(Processor* processor) const;
    void notifyObserversProcessorNetworkDidAddProcessor(Processor* processor) const;
    void notifyObserversProcessorNetworkWillRemoveProcessor(Processor* processor) const;
    void notifyObserversProcessorNetworkDidRemoveProcessor(Processor* processor) const;

    // Connections
    void notifyObserversProcessorNetworkWillAddConnection(PortConnection* connection) const;
    void notifyObserversProcessorNetworkDidAddConnection(PortConnection* connection) const;
    void notifyObserversProcessorNetworkWillRemoveConnection(PortConnection* connection) const;
    void notifyObserversProcessorNetworkDidRemoveConnection(PortConnection* connection) const;

    // Links
    void notifyObserversProcessorNetworkWillAddLink(PropertyLink* propertyLink) const;
    void notifyObserversProcessorNetworkDidAddLink(PropertyLink* propertyLink) const;
    void notifyObserversProcessorNetworkWillRemoveLink(PropertyLink* propertyLink) const;
    void notifyObserversProcessorNetworkDidRemoveLink(PropertyLink* propertyLink) const;

};

} // namespace

#endif // IVW_PROCESSOR_NETWORK_OBSERVER_H
