/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <inviwo/core/network/networkutils.h>

namespace inviwo {


bool util::ProcessorStates::hasBeenVisited(Processor* processor) const {
    return visited_.find(processor) != visited_.end();
}

void util::ProcessorStates::setProcessorVisited(Processor* processor) { visited_.insert(processor); }

void util::ProcessorStates::clear() { visited_.clear(); }

std::unordered_set<Processor*> util::getDirectPredecessors(Processor* processor) {
    std::unordered_set<Processor*> predecessors;
    for (auto port : processor->getInports()) {
        for (auto connectedPort : port->getConnectedOutports()) {
            predecessors.insert(connectedPort->getProcessor());
        }
    }
    return predecessors;
}

std::unordered_set<Processor*> util::getDirectSuccessors(Processor* processor) {
    std::unordered_set<Processor*> successors;
    for (auto port : processor->getOutports()) {
        for (auto connectedPort : port->getConnectedInports()) {
            successors.insert(connectedPort->getProcessor());
        }
    }
    return successors;
}

std::unordered_set<Processor*> util::getPredecessors(Processor* processor) {
    std::unordered_set<Processor*> predecessors;
    ProcessorStates state;
    traverseNetwork<TraversalDirection::Up, VisitPattern::Post>(
            state, processor, [&predecessors](Processor* p) { predecessors.insert(p); });
    
    return predecessors;
}

std::unordered_set<Processor*> util::getSuccessors(Processor* processor) {
    std::unordered_set<Processor*> successors;
    ProcessorStates state;
    traverseNetwork<TraversalDirection::Down, VisitPattern::Post>(
            state, processor, [&successors](Processor* p) { successors.insert(p); });
    
    return successors;
}

std::vector<Processor*> util::topologicalSort(ProcessorNetwork* network) {
    // perform topological sorting and store processor order in sorted

    std::vector<Processor*> endProcessors;
    util::copy_if(network->getProcessors(), std::back_inserter(endProcessors),
                  [](Processor* p) { return p->isEndProcessor(); });

    ProcessorStates state;
    std::vector<Processor*> sorted;
    for (auto processor : endProcessors) {
        traverseNetwork<TraversalDirection::Up, VisitPattern::Post>(
            state, processor, [&sorted](Processor* p) { sorted.push_back(p); });
    }
    return sorted;
}


}  // namespace
