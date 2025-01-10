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

#pragma once

#include <inviwo/core/ports/port.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class Processor;
class ProcessorObservable;
class ProcessorNetworkEvaluator;
class Property;

/** \class ProcessorObserver
 *
 * Processor observer that knows which processor did the notification.
 * Has notification for begin and end of it's invalidation.
 * Inherits from VoidObserver.
 *
 * @see ProcessorObservable
 */
class IVW_CORE_API ProcessorObserver : public Observer {
public:
    friend ProcessorObservable;
    ProcessorObserver() = default;
    virtual ~ProcessorObserver() = default;

    virtual void onAboutPropertyChange(Property*){};
    virtual void onProcessorInvalidationBegin(Processor*){};
    virtual void onProcessorInvalidationEnd(Processor*){};

    virtual void onProcessorPortAdded(Processor*, Port*){};
    virtual void onProcessorPortRemoved(Processor*, Port*){};

    virtual void onProcessorAboutToProcess(Processor*){};
    virtual void onProcessorFinishedProcess(Processor*){};

    /**
     * Called after the processor has changed its source state.
     */
    virtual void onProcessorSourceChanged(Processor*){};
    /**
     * Called after the processor has changed its sink state.
     */
    virtual void onProcessorSinkChanged(Processor*){};
    /**
     * Called after the processor has changed its ready state.
     */
    virtual void onProcessorReadyChanged(Processor*){};
    /**
     * Called after a processor inport and its connected outport(s) changed active state.
     * @see Processor::isConnectionActive
     */
    virtual void onProcessorActiveConnectionsChanged(Processor*){};

    /**
     * @brief Called when a processor initiates a background computation
     */
    virtual void onProcessorStartBackgroundWork(Processor*, size_t /*jobs*/){};
    /**
     * @brief Called when a processor finishes a background computation
     */
    virtual void onProcessorFinishBackgroundWork(Processor*, size_t /*jobs*/){};
};

/** \class ProcessorObservable
 * @see ProcessorObserver
 */
class IVW_CORE_API ProcessorObservable : public Observable<ProcessorObserver> {
protected:
    friend ProcessorNetworkEvaluator;
    friend Property;

    ProcessorObservable() = default;
    virtual ~ProcessorObservable() = default;

    void notifyObserversAboutPropertyChange(Property* p) {
        forEachObserver([&](ProcessorObserver* o) { o->onAboutPropertyChange(p); });
    }

    void notifyObserversInvalidationBegin(Processor* p) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorInvalidationBegin(p); });
    }

    void notifyObserversInvalidationEnd(Processor* p) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorInvalidationEnd(p); });
    }

    void notifyObserversProcessorPortAdded(Processor* p, Port* port) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorPortAdded(p, port); });
    }

    void notifyObserversProcessorPortRemoved(Processor* p, Port* port) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorPortRemoved(p, port); });
    }

    void notifyObserversAboutToProcess(Processor* p) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorAboutToProcess(p); });
    }
    void notifyObserversFinishedProcess(Processor* p) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorFinishedProcess(p); });
    }
    void notifyObserversSourceChange(Processor* p) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorSourceChanged(p); });
    }
    void notifyObserversSinkChange(Processor* p) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorSinkChanged(p); });
    }
    void notifyObserversReadyChange(Processor* p) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorReadyChanged(p); });
    }
    void notifyObserversActiveConnectionsChange(Processor* p) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorActiveConnectionsChanged(p); });
    }

    void notifyObserversStartBackgroundWork(Processor* p, size_t jobs = 1) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorStartBackgroundWork(p, jobs); });
    }
    void notifyObserversFinishBackgroundWork(Processor* p, size_t jobs = 1) {
        forEachObserver([&](ProcessorObserver* o) { o->onProcessorFinishBackgroundWork(p, jobs); });
    }
};

}  // namespace inviwo
