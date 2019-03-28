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

#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace inviwo {

BrushingAndLinkingInport::BrushingAndLinkingInport(std::string identifier)
    : DataInport<BrushingAndLinkingManager>(identifier) {
    setOptional(true);

    onConnect([&]() {
        sendFilterEvent(filterCache_);
        sendSelectionEvent(selectionCache_);
    });
}

void BrushingAndLinkingInport::sendFilterEvent(const std::unordered_set<size_t> &indices) {
    if (filterCache_.size() == 0 && indices.size() == 0) return;
    filterCache_ = indices;
    FilteringEvent event(this, filterCache_);
    propagateEvent(&event, nullptr);
}

void BrushingAndLinkingInport::sendSelectionEvent(const std::unordered_set<size_t> &indices) {
    bool noRemoteSelections = false;
    if (isConnected() && hasData()) {
        noRemoteSelections = getData()->getSelectedIndices().empty();
    }
    if (selectionCache_.empty() && indices.empty() && noRemoteSelections) {
        return;
    }
    selectionCache_ = indices;
    SelectionEvent event(this, selectionCache_);
    propagateEvent(&event, nullptr);
}

void BrushingAndLinkingInport::sendColumnSelectionEvent(const std::unordered_set<size_t> &indices) {
    bool noRemoteSelections = false;
    if (isConnected() && hasData()) {
        noRemoteSelections = getData()->getSelectedColumns().empty();
    }
    if (selectionColumnCache_.empty() && indices.empty() && noRemoteSelections) {
        return;
    }
    selectionColumnCache_ = indices;
    ColumnSelectionEvent event(this, selectionColumnCache_);
    propagateEvent(&event, nullptr);
}

bool BrushingAndLinkingInport::isColumnSelected(size_t idx) const {
    if (isConnected()) {
        return getData()->isColumnSelected(idx);
    } else {
        return selectionColumnCache_.find(idx) != selectionColumnCache_.end();
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

const std::unordered_set<size_t> &BrushingAndLinkingInport::getSelectedColumns() const {
    if (isConnected()) {
        return getData()->getSelectedColumns();
    } else {
        return selectionColumnCache_;
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
