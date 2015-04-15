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

#include <map>

namespace inviwo {

class Canvas;

class IVW_CORE_API ProcessorNetworkEvaluator : public ProcessorNetworkObserver, public ProcessorObserver {

    friend class Processor;

public:
    ProcessorNetworkEvaluator(ProcessorNetwork* processorNetwork);
    virtual ~ProcessorNetworkEvaluator();

    // this function is to be called when the network topology was changed
    void topologyUpdated();
    void saveSnapshotAllCanvases(std::string dir, std::string default_name = "", std::string ext = ".png");
    
    void initializeNetwork();

    void disableEvaluation();
    void enableEvaluation();
    void requestEvaluate();

    void propagateInteractionEvent(Processor* processor, InteractionEvent* event);

    void setExceptionHandler(ExceptionHandler handler);

    void onProcessorInvalidationEnd(Processor*);
    void onProcessorNetworkEvaluateRequest();
    void onProcessorNetworkUnlocked();

    static ProcessorNetworkEvaluator* getProcessorNetworkEvaluatorForProcessorNetwork(ProcessorNetwork* network);

private:
    typedef std::set<Processor *> ProcessorList;

    void evaluate();

    void setProcessorVisited(Processor* processor, bool visited=true);
    bool hasBeenVisited(Processor* processor) const;
    void setPropertyVisited(Property* property, bool visited=true);
    bool hasBeenVisited(Property* property) const;
    // retrieve predecessors from global processor state list (look-up)
    const ProcessorList& getStoredPredecessors(Processor* processor) const;
    // retrieve predecessors based on given event
    ProcessorList getDirectPredecessors(Processor* processor, Event* event=nullptr) const;
    void traversePredecessors(Processor* processor);
    void determineProcessingOrder();
    void updateProcessorStates();
    void resetProcessorVisitedStates();

    void propagateInteractionEventImpl(Processor* processor, InteractionEvent* event);
    bool isPortConnectedToProcessor(Port* port, Processor* processor);

    ProcessorNetwork* processorNetwork_;

    // the sorted list of processors obtained through topological sorting
    std::vector<Processor *> processorsSorted_;
    
    struct ProcessorState {
        ProcessorState() : visited(false) {}
        ProcessorState(const ProcessorList &predecessors) : visited(false), pred(predecessors) {}
        bool visited;
        ProcessorList pred; // list of all predecessors
        // additional information?
    };

    // map for managing processor states (predecessors, visited flags, etc.)
    // TODO: replace std::map with std::unordered_map when using C++11!
    //
    // map contains a dummy element for nullptr processor
    typedef std::map<Processor*, ProcessorState> ProcMap;
    typedef ProcMap::iterator ProcMapIt;
    typedef ProcMap::const_iterator const_ProcMapIt;
    typedef std::pair<Processor*, ProcessorState> ProcMapPair;

    ProcMap processorStates_;

    struct PropertyState {
        bool visited;
        // additional information?
    };

    // map for visited state of properties
    // TODO: replace std::map with std::unordered_map when using C++11!
    typedef std::map<Property *, PropertyState> PropertyMap;
    typedef PropertyMap::iterator PropertyMapIt;
    typedef PropertyMap::const_iterator const_PropertyMapIt;
    typedef std::pair<Property *, PropertyState> PropertyMapPair;

    PropertyMap propertiesVisited_;

    bool evaulationQueued_;
    bool evaluationDisabled_;

    bool processorStatesDirty_; // flag for lazy topology evaluation

    static std::map<ProcessorNetwork*, ProcessorNetworkEvaluator*> processorNetworkEvaluators_;

    ExceptionHandler exceptionHandler_;
};

} // namespace

#endif // IVW_PROCESSORNETWORKEVALUATOR_H
