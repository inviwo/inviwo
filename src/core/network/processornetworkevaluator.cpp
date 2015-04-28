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
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/canvas.h>
#include <inviwo/core/util/rendercontext.h>

namespace inviwo {


std::map<ProcessorNetwork*,ProcessorNetworkEvaluator*> ProcessorNetworkEvaluator::processorNetworkEvaluators_;

ProcessorNetworkEvaluator::ProcessorNetworkEvaluator(ProcessorNetwork* processorNetwork)
    : processorNetwork_(processorNetwork)
    , evaulationQueued_(false)
    , evaluationDisabled_(false)
    , processorStatesDirty_(true) 
    , exceptionHandler_(StandardExceptionHandler()) {

    initializeNetwork();
    
    ivwAssert(processorNetworkEvaluators_.find(processorNetwork) == processorNetworkEvaluators_.end() ,
              "A ProcessorNetworkEvaluator for the given ProcessorNetwork is already created");
    processorNetworkEvaluators_[processorNetwork] = this;
    processorNetwork_->addObserver(this);
}

ProcessorNetworkEvaluator::~ProcessorNetworkEvaluator() {
    std::map<ProcessorNetwork*,ProcessorNetworkEvaluator*>::iterator it = processorNetworkEvaluators_.find(processorNetwork_);

    if (it != processorNetworkEvaluators_.end())
        processorNetworkEvaluators_.erase(it);
}

void ProcessorNetworkEvaluator::topologyUpdated() {
    processorStatesDirty_ = true;
}

void ProcessorNetworkEvaluator::initializeNetwork() {
    ivwAssert(processorNetwork_!=0, "processorNetwork_ not initialized, call setProcessorNetwork()");
    // initialize network
    std::vector<Processor*> processors = processorNetwork_->getProcessors();


    for (size_t i=0; i<processors.size(); i++) {
        try {
            if (!processors[i]->isInitialized())
                processors[i]->initialize();
        } catch (Exception& e) {
            exceptionHandler_(IvwContext);
        }
    }
}

void ProcessorNetworkEvaluator::saveSnapshotAllCanvases(std::string dir, std::string default_name, std::string ext) {
    std::vector<inviwo::CanvasProcessor*> pv = processorNetwork_->getProcessorsByType<inviwo::CanvasProcessor>();
    int i = 0;

    for (std::vector<inviwo::CanvasProcessor*>::iterator it = pv.begin(); it != pv.end(); it++) {
        std::stringstream ss;

        if (default_name == "" || default_name == "UPN")
            ss << (*it)->getIdentifier();
        else
            ss << default_name << i+1;

        std::string path(dir + ss.str() + ext);
        LogInfo("Saving canvas to: " + path);
        (*it)->saveImageLayer(path);
        ++i;
    }
}

void ProcessorNetworkEvaluator::setProcessorVisited(Processor* processor, bool visited) {
    ProcMapIt it = processorStates_.find(processor);
    if (it != processorStates_.end())
        it->second.visited = visited;
}

bool ProcessorNetworkEvaluator::hasBeenVisited(Processor* processor) const {
    const_ProcMapIt it = processorStates_.find(processor);
    if (it != processorStates_.end())
        return it->second.visited;
    else
        return false;
}

void ProcessorNetworkEvaluator::setPropertyVisited(Property* property, bool visited) {
    PropertyMapIt it = propertiesVisited_.find(property);
    if (it != propertiesVisited_.end())
        it->second.visited = visited;
}

bool ProcessorNetworkEvaluator::hasBeenVisited(Property* property) const {
    const_PropertyMapIt it = propertiesVisited_.find(property);
    if (it != propertiesVisited_.end())
        return it->second.visited;
    else
        return false;
}

const ProcessorNetworkEvaluator::ProcessorList& 
ProcessorNetworkEvaluator::getStoredPredecessors(Processor* processor) const {
    const_ProcMapIt it = processorStates_.find(processor);
    if (it != processorStates_.end()) {
        return it->second.pred;
    }
    else {
        // processor not found, return reference to empty list of dummy element
        return processorStates_.find(nullptr)->second.pred;
    }
}

ProcessorNetworkEvaluator::ProcessorList ProcessorNetworkEvaluator::getDirectPredecessors(
    Processor* processor) const {
    ProcessorList predecessors;

    for (auto port : processor->getInports()) {
        if (!port->isConnected()) continue;

        for (auto connectedPort : port->getConnectedOutports()) {
            if (connectedPort) predecessors.insert(connectedPort->getProcessor());
        }
    }

    return predecessors;
}

void ProcessorNetworkEvaluator::traversePredecessors(Processor* processor) {
    if (!hasBeenVisited(processor)) {
        setProcessorVisited(processor);
        
        for (auto p : getStoredPredecessors(processor)) traversePredecessors(p);

        processorsSorted_.push_back(processor);
    }
}

void ProcessorNetworkEvaluator::determineProcessingOrder() {
    std::vector<Processor*> endProcessors;

    for (auto processor: processorNetwork_->getProcessors())  {
        if (processor->isEndProcessor()) endProcessors.push_back(processor);
    }

    // perform topological sorting and store processor order
    // in processorsSorted_
    processorsSorted_.clear();
    resetProcessorVisitedStates();

    for (auto processor : endProcessors) traversePredecessors(processor);
}

void ProcessorNetworkEvaluator::updateProcessorStates() {
    std::vector<Processor*> endProcessors;

    processorStates_.clear();
    // insert dummy processor to be able to return a reference to an 
    // empty predecessor list, if a processor does not exist (getStoredPredecessors())
    processorStates_.insert(ProcMapPair(nullptr, ProcessorState()));

    // update all processor states, i.e. collecting predecessors
    for (auto processor: processorNetwork_->getProcessors())  {
        // register processor in global state map
        if (!processorStates_.insert(ProcMapPair(processor, ProcessorState(getDirectPredecessors(processor)))).second)
            LogError("Processor State was already registered.");

        if (processor->isEndProcessor())
            endProcessors.push_back(processor);
    }

    // perform topological sorting and store processor order in processorsSorted_
    processorsSorted_.clear();

    for (auto processor : endProcessors) traversePredecessors(processor);
}

void ProcessorNetworkEvaluator::resetProcessorVisitedStates() {
    ProcMapIt it = processorStates_.begin();
    while (it != processorStates_.end()) {
        it->second.visited = false;
        ++it;
    }
}

void ProcessorNetworkEvaluator::setExceptionHandler(ExceptionHandler handler) {
    exceptionHandler_ = handler;
}

bool ProcessorNetworkEvaluator::isPortConnectedToProcessor(Port* port, Processor* processor) {
    bool isConnected = false;
    std::vector<PortConnection*> portConnections = processorNetwork_->getConnections();
    std::vector<Outport*> outports = processor->getOutports();

    for (size_t i=0; i<outports.size(); i++) {
        for (size_t j=0; j<portConnections.size(); j++) {
            const Port* curOutport = portConnections[j]->getOutport();

            if (curOutport == outports[i]) {
                const Port* connectedInport = portConnections[j]->getInport();

                if (connectedInport == port) {
                    isConnected = true;
                    break;
                }
            }
        }
    }

    if (isConnected) return isConnected;

    std::vector<Inport*> inports = processor->getInports();

    for (size_t i=0; i<inports.size(); i++) {
        for (size_t j=0; j<portConnections.size(); j++) {
            const Port* curInport = portConnections[j]->getInport();

            if (curInport == inports[i]) {
                const Outport* connectedOutport = portConnections[j]->getOutport();

                if (connectedOutport == port) {
                    isConnected = true;
                    break;
                }
            }
        }
    }

    return isConnected;
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
    //Direct request, thus we don't want to queue the evaluation anymore
    if (evaulationQueued_)
        evaulationQueued_ = false;

    requestEvaluate();
}

void ProcessorNetworkEvaluator::onProcessorNetworkUnlocked() {
    // Only evaluate if an evaluation is queued or the network is modified
    if (evaulationQueued_ || processorNetwork_->isModified()) {
        evaulationQueued_ = false;
        requestEvaluate();
    }
}

void ProcessorNetworkEvaluator::disableEvaluation() {
    evaluationDisabled_ = true;
}

void ProcessorNetworkEvaluator::enableEvaluation() {
    evaluationDisabled_ = false;

    if (evaulationQueued_) {
        evaulationQueued_ = false;
        requestEvaluate();
    }
}

ProcessorNetworkEvaluator*
ProcessorNetworkEvaluator::getProcessorNetworkEvaluatorForProcessorNetwork(
    ProcessorNetwork* network) {
    std::map<ProcessorNetwork*, ProcessorNetworkEvaluator*>::iterator it =
        processorNetworkEvaluators_.find(network);

    if (it == processorNetworkEvaluators_.end()) return new ProcessorNetworkEvaluator(network);

    return it->second;
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
    if (processorNetwork_->islocked() || evaluationDisabled_) {
        evaulationQueued_ = true;
        return;
    }

    // wait for invalidation to finish before evaluating
    if (processorNetwork_->isInvalidating()) {
        evaulationQueued_ = true;
        return;
    }

    evaulationQueued_ = false;
    //if we haven't returned yet, perform evaluation of the network
    evaluate();
}

void ProcessorNetworkEvaluator::evaluate() {
    // lock processor network to avoid concurrent evaluation
    processorNetwork_->lock();
    RenderContext::getPtr()->activateDefaultRenderContext();

    // if the processor network has changed determine the new processor order
    if (processorNetwork_->isModified()) {
        initializeNetwork();
        processorNetwork_->setModified(false);
        processorStatesDirty_ = true;
    }

    if (processorStatesDirty_) {
        // network topology has changed, update internal processor states
        updateProcessorStates();
    }

    for (auto processor : processorsSorted_) {
        if (!processor->isValid()) {
            if (processor->isReady()) {
                try {
                    // re-initialize resources (e.g., shaders) if necessary
                    if (processor->getInvalidationLevel() >= INVALID_RESOURCES) {
                        processor->initializeResources();
                    }
                    // call onChange for all invalid inports
                    for (auto inport : processor->getInports()) {
                        inport->callOnChangeIfChanged();
                    }
                } catch (Exception& e) {
                    exceptionHandler_(IvwContext);
                    processor->setValid();
                    continue;
                }

                #if IVW_PROFILING
                processor->notifyObserversAboutToProcess(processor);
                #endif

                try {
                    // do the actual processing
                    processor->process();
                } catch (Exception& e) {
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
    resetProcessorVisitedStates();

    // unlock processor network to allow next evaluation
    processorNetwork_->unlock();
}

} // namespace
