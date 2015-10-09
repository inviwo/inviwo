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

#ifndef IVW_PROCESSORNETWORKEVALUATOR_H
#define IVW_PROCESSORNETWORKEVALUATOR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/network/processornetwork.h>

#include <unordered_map>

namespace inviwo {

class IVW_CORE_API ProcessorNetworkEvaluator : public ProcessorNetworkObserver,
                                               public ProcessorObserver {
    friend class Processor;

public:
    ProcessorNetworkEvaluator(ProcessorNetwork* processorNetwork);
    virtual ~ProcessorNetworkEvaluator();

    void disableEvaluation();
    void enableEvaluation();
    void requestEvaluate();

    void setExceptionHandler(ExceptionHandler handler);

    virtual void onProcessorInvalidationEnd(Processor*) override;
    virtual void onProcessorNetworkEvaluateRequest() override;
    virtual void onProcessorNetworkUnlocked() override;

    static ProcessorNetworkEvaluator* getProcessorNetworkEvaluatorForProcessorNetwork(
        ProcessorNetwork* network);

private:
    using ProcessorList = std::unordered_set<Processor*>;

    void evaluate();
    // retrieve predecessors based on given event
    ProcessorList getDirectPredecessors(Processor* processor) const;
 
    void updateProcessorStates();
    void resetProcessorVisitedStates();

    struct ProcessorState {
        ProcessorState() : visited(false) {}
        ProcessorState(const ProcessorList& predecessors) : visited(false), pred(predecessors) {}
        bool visited;
        ProcessorList pred;  // list of all predecessors
    };

    struct ProcessorStates {
        bool hasBeenVisited(Processor* processor) const {
            auto it = processorStates_.find(processor);
            if (it != processorStates_.end())
                return it->second.visited;
            else
                return false;
        }

        void setProcessorVisited(Processor* processor, bool visited = true) {
            auto it = processorStates_.find(processor);
            if (it != processorStates_.end()) it->second.visited = visited;
        }

        void clear() {
            processorStates_.clear();
        }

        bool insert(Processor* p, ProcessorState s) {
            return processorStates_.insert(std::make_pair(p, s)).second;
        }

        // retrieve predecessors from global processor state list (look-up)
        const ProcessorList& getStoredPredecessors(Processor* processor) const {
            auto it = processorStates_.find(processor);
            if (it != processorStates_.end()) {
                return it->second.pred;
            } else {
                // processor not found, return reference to empty list
                return empty;
            }
        }


        static ProcessorList empty;
    private:
        std::unordered_map<Processor*, ProcessorState> processorStates_;
    };

    void traversePredecessors(ProcessorStates& state, Processor* processor);

    ProcessorNetwork* processorNetwork_;
    // the sorted list of processors obtained through topological sorting
    std::vector<Processor*> processorsSorted_;
    
    bool evaulationQueued_;
    bool evaluationDisabled_;

    static std::map<ProcessorNetwork*, ProcessorNetworkEvaluator*> processorNetworkEvaluators_;
    ExceptionHandler exceptionHandler_;
};

}  // namespace

#endif  // IVW_PROCESSORNETWORKEVALUATOR_H
