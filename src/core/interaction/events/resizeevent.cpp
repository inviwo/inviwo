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

#include <inviwo/core/interaction/events/resizeevent.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/imageport.h>

#include <inviwo/core/interaction/events/eventutil.h>

namespace inviwo {

ResizeEvent::ResizeEvent(size2_t canvasSize)
    : Event(), size_(canvasSize), previousSize_(canvasSize) {}

ResizeEvent::ResizeEvent(size2_t canvasSize, size2_t previousSize)
    : Event(), size_(canvasSize), previousSize_(previousSize) {}

ResizeEvent* ResizeEvent::clone() const { return new ResizeEvent(*this); }

bool ResizeEvent::shouldPropagateTo(Inport* inport, Processor* processor, Outport* source) {
    // Only propagate to image ports in the same port group.
    if (processor->getPortGroup(inport) == processor->getPortGroup(source)) {
        if (dynamic_cast<ImagePortBase*>(inport)) return true;
    }
    return false;
}

size2_t ResizeEvent::size() const { return size_; }

size2_t ResizeEvent::previousSize() const { return previousSize_; }

void ResizeEvent::setSize(size2_t csize) { size_ = csize; }

void ResizeEvent::setPreviousSize(size2_t previousSize) { previousSize_ = previousSize; }

uint64_t ResizeEvent::hash() const { return chash(); }

void ResizeEvent::print(std::ostream& ss) const {
    util::printEvent(ss, "ResizeEvent", std::make_pair("size", size_),
                     std::make_pair("prev", previousSize_));
}

}  // namespace inviwo
