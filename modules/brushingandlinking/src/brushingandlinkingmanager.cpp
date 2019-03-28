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

#include <modules/brushingandlinking/brushingandlinkingmanager.h>
#include <modules/brushingandlinking/processors/brushingandlinkingprocessor.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace inviwo {

BrushingAndLinkingManager::BrushingAndLinkingManager(Processor* p,
                                                     InvalidationLevel validationLevel)
    : owner_{p}, invalidationLevel_{validationLevel} {
    auto outPorts = p->getOutports();
    for (auto& op : outPorts) {
        if (dynamic_cast<BrushingAndLinkingOutport*>(op)) {
            op->onDisconnect([=]() { filtered_.update(); });
        }
    }
    onFilteringChangeCallback_ =
        filtered_.onChange([p, validationLevel]() { p->invalidate(validationLevel); });
}

BrushingAndLinkingManager::~BrushingAndLinkingManager() {}

size_t BrushingAndLinkingManager::getNumberOfSelected() const { return selected_.size(); }

size_t BrushingAndLinkingManager::getNumberOfFiltered() const { return filtered_.getSize(); }

void BrushingAndLinkingManager::remove(const BrushingAndLinkingInport* src) {
    filtered_.remove(src);
}

bool BrushingAndLinkingManager::isColumnSelected(size_t idx) const {
    return selectedColumns_.find(idx) != selectedColumns_.end();
}

void BrushingAndLinkingManager::setSelected(const BrushingAndLinkingInport*,
                                            const std::unordered_set<size_t>& indices) {
    selected_ = indices;
    owner_->invalidate(invalidationLevel_);
}

void BrushingAndLinkingManager::setFiltered(const BrushingAndLinkingInport* src,
                                            const std::unordered_set<size_t>& indices) {
    filtered_.set(src, indices);
}

void BrushingAndLinkingManager::setSelectedColumn(const BrushingAndLinkingInport*,
                                                  const std::unordered_set<size_t>& indices) {
    selectedColumns_ = indices;
    owner_->invalidate(invalidationLevel_);
}

const std::unordered_set<size_t>& BrushingAndLinkingManager::getSelectedIndices() const {
    return selected_;
}

const std::unordered_set<size_t>& BrushingAndLinkingManager::getFilteredIndices() const {
    return filtered_.getIndices();
}

const std::unordered_set<size_t>& BrushingAndLinkingManager::getSelectedColumns() const {
    return selectedColumns_;
}

}  // namespace inviwo
