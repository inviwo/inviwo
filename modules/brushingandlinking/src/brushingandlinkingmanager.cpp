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

#include <modules/brushingandlinking/brushingandlinkingmanager.h>
#include <modules/brushingandlinking/processors/brushingandlinkingprocessor.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace inviwo {

BrushingAndLinkingManager::BrushingAndLinkingManager(Processor* p,
                                                     InvalidationLevel validationLevel) {
    auto outPorts = p->getOutports();
    for (auto& op : outPorts) {
        if (dynamic_cast<BrushingAndLinkingOutport*>(op)) {
            op->onDisconnect([=]() {
                selected_.update();
                filtered_.update();
                clusterSelected_.update();
                someOtherSelected_.update();
            });
        }
    }
    callback1_ = selected_.onChange([p, validationLevel]() { p->invalidate(validationLevel); });
    callback2_ = filtered_.onChange([p, validationLevel]() { p->invalidate(validationLevel); });
    callback3_ =
        clusterSelected_.onChange([p, validationLevel]() { p->invalidate(validationLevel); });
    callback4_ =
        someOtherSelected_.onChange([p, validationLevel]() { p->invalidate(validationLevel); });
    callback5_ = ranges_.onChange([p, validationLevel]() { p->invalidate(validationLevel); });
}

BrushingAndLinkingManager::~BrushingAndLinkingManager() {}

size_t BrushingAndLinkingManager::getNumberOfSelected() const { return selected_.getSize(); }

size_t BrushingAndLinkingManager::getNumberOfFiltered() const { return filtered_.getSize(); }

size_t BrushingAndLinkingManager::getNumberOfClusterSelected() const {
    return clusterSelected_.getSize();
}

size_t BrushingAndLinkingManager::getNumberOfSomeOtherSelected() const {
    return someOtherSelected_.getSize();
}

size_t BrushingAndLinkingManager::getNumberOfRanges() const { return ranges_.getSize(); }

void BrushingAndLinkingManager::remove(const BrushingAndLinkingInport* src) {
    selected_.remove(src);
    filtered_.remove(src);
    clusterSelected_.remove(src);
    someOtherSelected_.remove(src);
    ranges_.remove(src);
}

bool BrushingAndLinkingManager::isFiltered(size_t idx) const { return filtered_.has(idx); }

bool BrushingAndLinkingManager::isSelected(size_t idx) const { return selected_.has(idx); }

bool BrushingAndLinkingManager::isClusterSelected(int idx) const {
    return clusterSelected_.has(idx);
}

bool BrushingAndLinkingManager::isSomeOtherSelected(int idx) const {
    return someOtherSelected_.has(idx);
}

void BrushingAndLinkingManager::setSelected(const BrushingAndLinkingInport* src,
                                            const std::unordered_set<size_t>& indices) {
    selected_.set(src, indices);
}

void BrushingAndLinkingManager::setFiltered(const BrushingAndLinkingInport* src,
                                            const std::unordered_set<size_t>& indices) {
    filtered_.set(src, indices);
}

void BrushingAndLinkingManager::setClusterSelected(const BrushingAndLinkingInport* src,
                                                   const std::unordered_set<int>& indices) {
    clusterSelected_.set(src, indices);
}

void BrushingAndLinkingManager::setSomeOtherSelected(const BrushingAndLinkingInport* src,
                                                     const std::unordered_set<int>& indices) {
    someOtherSelected_.set(src, indices);
}

    void BrushingAndLinkingManager::setFilterRanges(const BrushingAndLinkingInport* src,
                                                         const std::vector<vec2>& ranges) {
        ranges_.set(src, ranges);
    }

const std::unordered_set<size_t>& BrushingAndLinkingManager::getSelectedIndices() const {
    return selected_.getIndices();
}

const std::unordered_set<size_t>& BrushingAndLinkingManager::getFilteredIndices() const {
    return filtered_.getIndices();
}

const std::unordered_set<int>& BrushingAndLinkingManager::getClusterSelectedIndices() const {
    return clusterSelected_.getIndices();
}

const std::unordered_set<int>& BrushingAndLinkingManager::getSomeOtherSelectedIndices() const {
    return someOtherSelected_.getIndices();
}

    const std::vector<vec2>& BrushingAndLinkingManager::getFilterRanges() const {
        return ranges_.getRanges();
    }

}  // namespace inviwo
