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
    , processorsSorted_(util::topologicalSort(processorNetwork_))
    , evaulationQueued_(false)
    , exceptionHandler_(StandardExceptionHandler()) {
    
    processorNetwork_->addObserver(this);
}

void ProcessorNetworkEvaluator::setExceptionHandler(ExceptionHandler handler) {
    exceptionHandler_ = handler;
}

void ProcessorNetworkEvaluator::onProcessorNetworkEvaluateRequest() {
    // Direct request, thus we don't want to queue the evaluation anymore
    evaulationQueued_ = false;
    requestEvaluate();
}

void ProcessorNetworkEvaluator::onProcessorNetworkUnlocked() {
    // Only evaluate if an evaluation is queued or the network is modified
    if (evaulationQueued_) {
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

    notifyObserversProcessorNetworkEvaluationBegin();

    RenderContext::getPtr()->activateDefaultRenderContext();

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
                } catch (...) {
                    exceptionHandler_(IvwContext);
                    processor->setValid();
                    continue;
                }

                processor->notifyObserversAboutToProcess(processor);

                try {
                    IVW_CPU_PROFILING_IF(500, "Processed " << processor->getDisplayName());
                    // do the actual processing
                    processor->process();
                } catch (...) {
                    exceptionHandler_(IvwContext);
                }
                // set processor as valid
                processor->setValid();

                processor->notifyObserversFinishedProcess(processor);

            } else {
                processor->notifyObserversAboutToProcess(processor);
                try {
                    processor->doIfNotReady();
                } catch (...) {
                    exceptionHandler_(IvwContext);
                }
                processor->notifyObserversFinishedProcess(processor);
            }
        }
    }

    notifyObserversProcessorNetworkEvaluationEnd();
}

void ProcessorNetworkEvaluator::onProcessorNetworkDidAddProcessor(Processor* processor) {
    processorsSorted_ = util::topologicalSort(processorNetwork_);
}

void ProcessorNetworkEvaluator::onProcessorNetworkDidRemoveProcessor(Processor* processor) {
    processorsSorted_ = util::topologicalSort(processorNetwork_);
}

void ProcessorNetworkEvaluator::onProcessorNetworkDidAddConnection(
    const PortConnection& connection) {
    processorsSorted_ = util::topologicalSort(processorNetwork_);
}

void ProcessorNetworkEvaluator::onProcessorNetworkDidRemoveConnection(
    const PortConnection& connection) {
    processorsSorted_ = util::topologicalSort(processorNetwork_);
}

}  // namespace
