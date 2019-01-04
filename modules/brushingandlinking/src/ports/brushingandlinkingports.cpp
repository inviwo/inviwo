/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace inviwo {

BrushingAndLinkingInport::BrushingAndLinkingInport(std::string identifier)
    : DataInport<BrushingAndLinkingManager>(identifier) {
    setOptional(true);

    onConnect([&]() {
        sendFilterEvent(filterCache_);
        sendSelectionEvent(selectionCache_);
        sendClusterSelectionEvent(clusterSelectionCache_);
        sendSomeOtherSelectionEvent(someOtherSelectionCache_);
        sendFilterRangeChangedEvent(rangeCache_);
    });
}

void BrushingAndLinkingInport::sendFilterEvent(const std::unordered_set<size_t> &indices) {
    if (filterCache_.empty() && indices.empty()) return;
    filterCache_ = indices;
    FilteringEvent event(this, filterCache_);
    getProcessor()->propagateEvent(&event, nullptr);
}

void BrushingAndLinkingInport::sendSelectionEvent(const std::unordered_set<size_t> &indices) {
    if (selectionCache_.empty() && indices.empty()) return;
    selectionCache_ = indices;
    SelectionEvent event(this, selectionCache_);
    getProcessor()->propagateEvent(&event, nullptr);
}

void BrushingAndLinkingInport::sendClusterSelectionEvent(const std::unordered_set<int> &indices) {
    if (clusterSelectionCache_.empty() && indices.empty()) return;
    clusterSelectionCache_ = indices;
    ClusterSelectionEvent event(this, clusterSelectionCache_);
    getProcessor()->propagateEvent(&event, nullptr);
}

void BrushingAndLinkingInport::sendSomeOtherSelectionEvent(const std::unordered_set<int> &indices) {
    if (someOtherSelectionCache_.empty() && indices.empty()) return;
    someOtherSelectionCache_ = indices;
    SomeOtherSelectionEvent event(this, someOtherSelectionCache_);
    getProcessor()->propagateEvent(&event, nullptr);
}

void BrushingAndLinkingInport::sendFilterRangeChangedEvent(const std::vector<vec2> &ranges) {
    if (rangeCache_.empty() && ranges.empty()) return;
    rangeCache_ = ranges;
    FilteringRangeChangedEvent event(this, rangeCache_);
    getProcessor()->propagateEvent(&event, nullptr);
}

bool BrushingAndLinkingInport::isFiltered(size_t idx) const {
    if (isConnected()) {
        return getData()->isFiltered(idx);
    } else {
        return filterCache_.find(idx) != filterCache_.end();
    }
}

bool BrushingAndLinkingInport::isSelected(size_t idx) const {
    if (isConnected()) {
        return getData()->isSelected(idx);
    } else {
        return selectionCache_.find(idx) != selectionCache_.end();
    }
}

bool BrushingAndLinkingInport::isClusterSelected(int idx) const {
    if (isConnected()) {
        return getData()->isClusterSelected(idx);
    } else {
        return clusterSelectionCache_.find(idx) != clusterSelectionCache_.end();
    }
}

bool BrushingAndLinkingInport::isSomeOtherSelected(int idx) const {
    if (isConnected()) {
        return getData()->isSomeOtherSelected(idx);
    } else {
        return someOtherSelectionCache_.find(idx) != someOtherSelectionCache_.end();
    }
}

const std::unordered_set<size_t> &BrushingAndLinkingInport::getSelectedIndices() const {
    if (isConnected()) {
        return getData()->getSelectedIndices();
    } else {
        return selectionCache_;
    }
}

const std::unordered_set<size_t> &BrushingAndLinkingInport::getFilteredIndices() const {
    if (isConnected()) {
        return getData()->getFilteredIndices();
    } else {
        return filterCache_;
    }
}

const std::unordered_set<int> &BrushingAndLinkingInport::getClusterSelectedIndices() const {
    if (isConnected()) {
        return getData()->getClusterSelectedIndices();
    } else {
        return clusterSelectionCache_;
    }
}

const std::unordered_set<int> &BrushingAndLinkingInport::getSomeOtherSelectedIndices() const {
    if (isConnected()) {
        return getData()->getSomeOtherSelectedIndices();
    } else {
        return someOtherSelectionCache_;
    }
}

const std::vector<vec2> &BrushingAndLinkingInport::getFilterRanges() const {
    if (isConnected()) {
        return getData()->getFilterRanges();
    } else {
        return rangeCache_;
    }
}

std::string BrushingAndLinkingInport::getClassIdentifier() const {
    return PortTraits<BrushingAndLinkingInport>::classIdentifier();
}

BrushingAndLinkingOutport::BrushingAndLinkingOutport(std::string identifier)
    : DataOutport<BrushingAndLinkingManager>(identifier) {}

std::string BrushingAndLinkingOutport::getClassIdentifier() const {
    return PortTraits<BrushingAndLinkingOutport>::classIdentifier();
}

}  // namespace inviwo
