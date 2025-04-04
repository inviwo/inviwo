/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/clock.h>

namespace inviwo {

ProcessorNetworkEvaluator::ProcessorNetworkEvaluator(ProcessorNetwork* processorNetwork)
    : processorNetwork_(processorNetwork)
    , processorsSorted_(util::topologicalSortFiltered(processorNetwork_))
    , needsSorting_(true)
    , evaluationQueued_(false)
    , exceptionHandler_(StandardEvaluationErrorHandler()) {

    processorNetwork_->addObserver(this);
}

void ProcessorNetworkEvaluator::setExceptionHandler(EvaluationErrorHandler handler) {
    exceptionHandler_ = handler;
}

void ProcessorNetworkEvaluator::onProcessorNetworkEvaluateRequest() {
    // Direct request, thus we don't want to queue the evaluation anymore
    evaluationQueued_ = false;
    requestEvaluate();
}

void ProcessorNetworkEvaluator::onProcessorNetworkUnlocked() {
    // Only evaluate if an evaluation is queued or the network is modified
    if (evaluationQueued_) {
        evaluationQueued_ = false;
        requestEvaluate();
    }
}

void ProcessorNetworkEvaluator::requestEvaluate() {
    // evaluation has been triggered but is currently queued
    // requestEvaluate needs to be called with evaluationQueued_ false to continue.
    if (evaluationQueued_) return;

    // wait for linking to finish
    if (processorNetwork_->isLinking()) {
        evaluationQueued_ = true;
        return;
    }

    // evaluation disabled
    if (processorNetwork_->islocked()) {
        evaluationQueued_ = true;
        return;
    }

    // wait for invalidation to finish before evaluating
    if (processorNetwork_->isInvalidating()) {
        evaluationQueued_ = true;
        return;
    }

    evaluationQueued_ = false;
    // if we haven't returned yet, perform evaluation of the network
    evaluate();
}

void ProcessorNetworkEvaluator::evaluate() {
    // lock processor network to avoid concurrent evaluation
    NetworkLock lock(processorNetwork_);

    notifyObserversProcessorNetworkEvaluationBegin();

    if (needsSorting_) {
        processorsSorted_ = util::topologicalSortFiltered(processorNetwork_);
        needsSorting_ = false;
    }

    IVW_CPU_PROFILING_IF(500, "Evaluated Processor Network");

    for (auto processor : processorsSorted_) {
        if (!processor->isValid()) {
            if (processor->isReady()) {
                try {
                    // re-initialize resources (e.g., shaders) if necessary
                    if (processor->getInvalidationLevel() >= InvalidationLevel::InvalidResources) {
                        processor->initializeResources();
                    }
                } catch (...) {
                    exceptionHandler_(processor, EvaluationType::InitResource, SourceContext{});
                    continue;
                }

                try {
                    // call onChange for all invalid inports
                    for (auto inport : processor->getInports()) {
                        inport->callOnChangeIfChanged();
                    }
                } catch (...) {
                    exceptionHandler_(processor, EvaluationType::PortOnChange, SourceContext{});
                    continue;
                }

                processor->notifyObserversAboutToProcess(processor);

                try {
                    IVW_CPU_PROFILING_IF(500, "Processed " << processor->getIdentifier());
                    // do the actual processing
                    processor->process();

                    // Set processor as valid only if we still are ready.
                    // Callbacks might have made our inports invalid, if so abort
                    // the evaluation by not setting the processor valid.
                    if (processor->isReady()) processor->setValid();

                } catch (...) {
                    exceptionHandler_(processor, EvaluationType::Process, SourceContext{});
                }

                processor->notifyObserversFinishedProcess(processor);

            } else {
                try {
                    processor->doIfNotReady();
                } catch (...) {
                    exceptionHandler_(processor, EvaluationType::NotReady, SourceContext{});
                }
            }
        }
    }

    notifyObserversProcessorNetworkEvaluationEnd();
}

void ProcessorNetworkEvaluator::onProcessorSinkChanged(Processor*) { needsSorting_ = true; }

void ProcessorNetworkEvaluator::onProcessorActiveConnectionsChanged(Processor*) {
    needsSorting_ = true;
}

void ProcessorNetworkEvaluator::onProcessorNetworkDidAddProcessor(Processor* p) {
    p->ProcessorObservable::addObserver(this);
    needsSorting_ = true;
}

void ProcessorNetworkEvaluator::onProcessorNetworkDidRemoveProcessor(Processor* p) {
    p->ProcessorObservable::removeObserver(this);
    needsSorting_ = true;
}

void ProcessorNetworkEvaluator::onProcessorNetworkDidAddConnection(const PortConnection&) {
    needsSorting_ = true;
}

void ProcessorNetworkEvaluator::onProcessorNetworkDidRemoveConnection(const PortConnection&) {
    needsSorting_ = true;
}

}  // namespace inviwo
