/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

bool Event::shouldPropagateTo(Inport* /*inport*/, Processor* /*processor*/, Outport* /*source*/) {
    return true;
}

bool Event::markAsVisited(Processor* p) { return util::push_back_unique(visitedProcessors_, p); }

void Event::markAsVisited(Event& e) {
    visitedProcessors_.reserve(visitedProcessors_.size() + e.visitedProcessors_.size());
    for (auto p : e.visitedProcessors_) {
        util::push_back_unique(visitedProcessors_, p);
    }
}

bool Event::hasVisitedProcessor(Processor* p) const {
    return util::contains(visitedProcessors_, p);
}

const std::vector<Processor*>& Event::getVisitedProcessors() const { return visitedProcessors_; }

void Event::print(std::ostream& ss) const { ss << "Unknown Event. Hash:" << hash(); }

}  // namespace inviwo
