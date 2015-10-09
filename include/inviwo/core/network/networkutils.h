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

#ifndef IVW_NETWORKUTILS_H
#define IVW_NETWORKUTILS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

namespace util {

struct IVW_CORE_API ProcessorStates {
    bool hasBeenVisited(Processor* processor) const;
    void setProcessorVisited(Processor* processor);
    void clear();

private:
    std::unordered_set<Processor*> visited_;
};

IVW_CORE_API std::unordered_set<Processor*> getDirectPredecessors(Processor* processor);
IVW_CORE_API std::unordered_set<Processor*> getDirectSuccessors(Processor* processor);

IVW_CORE_API std::unordered_set<Processor*> getPredecessors(Processor* processor);
IVW_CORE_API std::unordered_set<Processor*> getSuccessors(Processor* processor);

enum class TraversalDirection {Up, Down};
enum class VisitPattern {Pre, Post};

template <TraversalDirection D, VisitPattern V, typename Func>
void traverseNetwork(ProcessorStates& state, Processor* processor, Func f) {
    if (!state.hasBeenVisited(processor)) {
        state.setProcessorVisited(processor);

        if (V == VisitPattern::Pre) f(processor);

        switch (D) {
            case TraversalDirection::Up: {
                for (auto port : processor->getInports()) {
                    for (auto connectedPort : port->getConnectedOutports()) {
                        traverseNetwork<D, V, Func>(state, connectedPort->getProcessor(), f);
                    }
                }
                break;
            }
            
            case TraversalDirection::Down: {
                for (auto port : processor->getOutports()) {
                    for (auto connectedPort : port->getConnectedInports()) {
                        traverseNetwork<D, V, Func>(state, connectedPort->getProcessor(), f);
                    }
                }
                break;
            }
        }

        if (V == VisitPattern::Post) f(processor);
    }
}

IVW_CORE_API std::vector<Processor*> topologicalSort(ProcessorNetwork* network);

}  // namespace
}  // namespace

#endif  // IVW_NETWORKUTILS_H
