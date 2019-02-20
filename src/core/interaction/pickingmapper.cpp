/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/pickingaction.h>

namespace inviwo {

PickingMapper::PickingMapper(PickingManager* manager) : manager_(manager) {}

PickingMapper::PickingMapper(Processor* processor, size_t size,
                             std::function<void(PickingEvent*)> callback, PickingManager* manager)
    : manager_(manager)
    , processor_(processor)
    , callback_(callback)
    , pickingAction_(manager_->registerPickingAction(processor, callback, size)) {}

PickingMapper::PickingMapper(PickingMapper&& rhs)
    : manager_(rhs.manager_)
    , processor_(rhs.processor_)
    , callback_(std::move(rhs.callback_))
    , pickingAction_(rhs.pickingAction_) {
    rhs.processor_ = nullptr;
    rhs.pickingAction_ = nullptr;
}

PickingMapper& PickingMapper::operator=(PickingMapper&& that) {
    if (this != &that) {
        std::swap(that.manager_, manager_);
        std::swap(that.processor_, processor_);
        std::swap(that.callback_, callback_);
        std::swap(that.pickingAction_, pickingAction_);

        that.processor_ = nullptr;
        if (manager_) manager_->unregisterPickingAction(that.pickingAction_);
    }
    return *this;
}

PickingMapper::~PickingMapper() {
    if (pickingAction_ && manager_) {
        manager_->unregisterPickingAction(pickingAction_);
    }
}

void PickingMapper::resize(size_t newSize) {
    if (pickingAction_ && newSize == getPickingAction()->getSize()) {
        // Same size or size zero, do nothing
        return;
    }
    bool enabled = true;
    if (pickingAction_ && manager_) {
        enabled = pickingAction_->isEnabled();
        manager_->unregisterPickingAction(pickingAction_);
        pickingAction_ = nullptr;
    }
    if (newSize > 0 && manager_) {
        pickingAction_ = manager_->registerPickingAction(processor_, callback_, newSize);
        pickingAction_->setEnabled(enabled);
    }
}

bool PickingMapper::isEnabled() const {
    if (pickingAction_)
        return pickingAction_->isEnabled();
    else
        return false;
}

void PickingMapper::setEnabled(bool enabled) {
    if (pickingAction_) pickingAction_->setEnabled(enabled);
}

size_t PickingMapper::getPickingId(size_t id) const { return pickingAction_->getPickingId(id); }

inviwo::vec3 PickingMapper::getColor(size_t id) const { return pickingAction_->getColor(id); }

size_t PickingMapper::getSize() const { return pickingAction_->getSize(); }

const PickingAction* PickingMapper::getPickingAction() const { return pickingAction_; }

}  // namespace inviwo
