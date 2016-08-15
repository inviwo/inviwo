/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

namespace inviwo{

BrushingAndLinkingInport::BrushingAndLinkingInport(std::string identifier)
    : DataInport<BrushingAndLinkingManager>(identifier) {
    setOptional(true);

    onConnect([&]() {
        sendFilterEvent(filterCache_);
        sendSelectionEvent(selctionCache_);
    });
}

void BrushingAndLinkingInport::sendFilterEvent(const std::unordered_set<size_t> &indices) {
    filterCache_ = indices;
    FilteringEvent event(this, filterCache_);
    getProcessor()->propagateEvent(&event, nullptr);
}

void BrushingAndLinkingInport::sendSelectionEvent(const std::unordered_set<size_t> &indices) {
    selctionCache_ = indices;
    SelectionEvent event(this, selctionCache_);
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
        return selctionCache_.find(idx) != selctionCache_.end();
    }
}

const std::unordered_set<size_t> &BrushingAndLinkingInport::getSelectedIndices() const {
    if (isConnected()) {
        return getData()->getSelectedIndices();
    }
    else {
        return selctionCache_;
    }
}


const std::unordered_set<size_t> &BrushingAndLinkingInport::getFilteredIndices() const {
    if (isConnected()) {
        return getData()->getFilteredIndices();
    }
    else {
        return filterCache_;
    }
}

BrushingAndLinkingOutport::BrushingAndLinkingOutport(std::string identifier)
    : DataOutport<BrushingAndLinkingManager>(identifier) {}

}  // namespace
