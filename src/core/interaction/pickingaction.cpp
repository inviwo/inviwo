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

#include <inviwo/core/interaction/pickingaction.h>
#include <inviwo/core/interaction/pickingmanager.h>

namespace inviwo {

PickingAction::PickingAction(size_t id, size_t size) : start_(id), size_(size), capacity_(size) {}

PickingAction::~PickingAction() = default;

size_t PickingAction::getPickingId(size_t id) const {
    if (id < size_) {
        return start_ + id;
    } else {
        throw RangeException(
            "Local picking ID " + toString(id) + " not in range (0 - " + toString(size_) + ")",
            IVW_CONTEXT);
    }
}

bool PickingAction::isIndex(size_t globalId) const {
    return globalId >= start_ && globalId < start_ + size_;
}

size_t PickingAction::getLocalPickingId(size_t globalId) const {
    if (!isIndex(globalId)) {
        throw RangeException("Global picking ID " + toString(globalId) + " not in range (" +
                                 toString(start_) + " - " + toString(start_ + size_) + ")",
                             IVW_CONTEXT);
    }
    return globalId - start_;
}

vec3 PickingAction::getColor(size_t id) const {
    return vec3(PickingManager::indexToColor(getPickingId(id))) / 255.0f;
}

size_t PickingAction::getSize() const { return size_; }

bool PickingAction::isEnabled() const { return enabled_; }

void PickingAction::setEnabled(bool enabled) { enabled_ = enabled; }

void PickingAction::setAction(Callback action) { action_ = action; }

void PickingAction::operator()(PickingEvent* event) const { action_(event); }

void PickingAction::setProcessor(Processor* processor) { processor_ = processor; }

Processor* PickingAction::getProcessor() const { return processor_; }

size_t PickingAction::getCapacity() const { return capacity_; }

void PickingAction::setSize(size_t size) {
    if (size <= capacity_) {
        size_ = size;
    } else {
        throw Exception("Trying to set size (" + toString(size) + ") larger than capacity (" +
                            toString(capacity_) + ")",
                        IVW_CONTEXT);
    }
}

}  // namespace inviwo
