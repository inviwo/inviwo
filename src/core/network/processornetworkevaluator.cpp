/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/canvas.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/util/clock.h>

namespace inviwo {

ProcessorNetworkEvaluator::ProcessorNetworkEvaluator(ProcessorNetwork* processorNetwork)
    : processorNetwork_(processorNetwork)
    , evaulationQueued_(false)
    , exceptionHandler_(StandardExceptionHandler()) {
    
    processorNetwork_->addObserver(this);
}

ProcessorNetworkEvaluator::~ProcessorNetworkEvaluator() {}

void ProcessorNetworkEvaluator::setExceptionHandler(ExceptionHandler handler) {
    exceptionHandler_ = handler;
}

void ProcessorNetworkEvaluator::onProcessorInvalidationEnd(Processor* p) {
    processorNetwork_->onProcessorInvalidationEnd(p);
    p->ProcessorObservable::removeObserver(this);

    if (evaulationQueued_) {
        evaulationQueued_ = false;
        requestEvaluate();
    }
}

void ProcessorNetworkEvaluator::onProcessorNetworkEvaluateRequest() {
    // Direct request, thus we don't want to queue the evaluation anymore
    if (evaulationQueued_) evaulationQueued_ = false;

    requestEvaluate();
}

void ProcessorNetworkEvaluator::onProcessorNetworkUnlocked() {
    // Only evaluate if an evaluation is queued or the network is modified
    if (evaulationQueued_ || processorNetwork_->isModified()) {
        evaulationQueued_ = false;
        requestEvaluate();
    }
}

void ProcessorNetworkEvaluator::requestEvaluate() {
    // evaluation has been triggered but is currently queued
    // requestEvaluate needs to be called with evaulationQueued_ false to continue.
    if (evaulationQueued_) return;

    // wait for linking to finish
    if (processorNetwork_->isLinking()) {
        evaulationQueued_ = true;
        return;
    }

    // evaluation disabled
    if (processorNetwork_->islocked()) {
        evaulationQueued_ = true;
        return;
    }

    // wait for invalidation to finish before evaluating
    if (processorNetwork_->isInvalidating()) {
        evaulationQueued_ = true;
        return;
    }

    evaulationQueued_ = false;
    // if we haven't returned yet, perform evaluation of the network
    evaluate();
}

void ProcessorNetworkEvaluator::evaluate() {
    // lock processor network to avoid concurrent evaluation
    NetworkLock lock(processorNetwork_);
    RenderContext::getPtr()->activateDefaultRenderContext();

    // if the processor network has changed determine the new processor order
    if (processorNetwork_->isModified()) {
        // network topology has changed, update internal processor states
        processorsSorted_ = util::topologicalSort(processorNetwork_);
        processorNetwork_->setModified(false);
    }

    for (auto processor : processorsSorted_) {
        if (!processor->isValid()) {
            if (processor->isReady()) {
                try {
                    // re-initialize resources (e.g., shaders) if necessary
                    if (processor->getInvalidationLevel() >= InvalidationLevel::InvalidResources) {
                        processor->initializeResources();
                    }
                    // call onChange for all invalid inports
                    for (auto inport : processor->getInports()) {
                        inport->callOnChangeIfChanged();
                    }
                } catch (Exception&) {
                    exceptionHandler_(IvwContext);
                    processor->setValid();
                    continue;
                }

#if IVW_PROFILING
                processor->notifyObserversAboutToProcess(processor);
#endif

                try {
                    IVW_CPU_PROFILING_IF(500, "Processed " << processor->getDisplayName());
                    // do the actual processing
                    processor->process();
                } catch (Exception&) {
                    exceptionHandler_(IvwContext);
                }
                // set processor as valid
                processor->setValid();

#if IVW_PROFILING
                processor->notifyObserversFinishedProcess(processor);
#endif
            } else {
                processor->doIfNotReady();
            }
        }
    }
}

}  // namespace
