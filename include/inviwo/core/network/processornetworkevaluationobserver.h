/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_PROCESSORNETWORKEVALUATIONOBSERVER_H
#define IVW_PROCESSORNETWORKEVALUATIONOBSERVER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class ProcessorNetworkEvaluationObservable;

/**
 * \class ProcessorNetworkEvaluationObserver
 */
class IVW_CORE_API ProcessorNetworkEvaluationObserver : public Observer {
public:
    friend ProcessorNetworkEvaluationObservable;
    ProcessorNetworkEvaluationObserver() = default;
    virtual ~ProcessorNetworkEvaluationObserver() = default;

    virtual void onProcessorNetworkEvaluationBegin(){};
    virtual void onProcessorNetworkEvaluationEnd(){};
};

class IVW_CORE_API ProcessorNetworkEvaluationObservable
    : public Observable<ProcessorNetworkEvaluationObserver> {
public:
    ProcessorNetworkEvaluationObservable() = default;

    virtual void notifyObserversProcessorNetworkEvaluationBegin();
    virtual void notifyObserversProcessorNetworkEvaluationEnd();
};

}  // namespace

#endif  // IVW_PROCESSORNETWORKEVALUATIONOBSERVER_H
