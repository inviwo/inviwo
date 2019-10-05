/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/brushingandlinking/events/brushingandlinkingevent.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/interaction/events/eventutil.h>

#include <algorithm>

namespace inviwo {

BrushingAndLinkingEvent::BrushingAndLinkingEvent(const BrushingAndLinkingInport* src,
                                                 const std::unordered_set<size_t>& indices)
    : source_(src), indices_(indices) {}

BrushingAndLinkingEvent* BrushingAndLinkingEvent::clone() const {
    return new BrushingAndLinkingEvent(*this);
}

const inviwo::BrushingAndLinkingInport* BrushingAndLinkingEvent::getSource() const {
    return source_;
}

const std::unordered_set<size_t>& BrushingAndLinkingEvent::getIndices() const { return indices_; }

uint64_t BrushingAndLinkingEvent::hash() const { return chash(); }

void BrushingAndLinkingEvent::print(std::ostream& os) const {
    printEvent("BrushingAndLinkingEvent", os);
}

void BrushingAndLinkingEvent::printEvent(const std::string& eventType, std::ostream& os) const {
    using namespace std::string_literals;

    std::vector<size_t> indices(indices_.begin(), indices_.end());
    std::sort(indices.begin(), indices.end());
    const std::string indicesStr = [&]() -> std::string {
        if (indices.empty()) return "none"s;
        std::string str = joinString(indices.begin(),
                                     indices.begin() + std::min<size_t>(indices.size(), 10), ", ");
        if (indices_.size() > 10) {
            str.append("...");
        }
        return str;
    }();

    util::printEvent(
        os, eventType,
        std::make_pair("source", (source_ ? source_->getProcessor()->getIdentifier() : "unknown"s)),
        std::make_pair("indices", indicesStr));
}

}  // namespace inviwo
