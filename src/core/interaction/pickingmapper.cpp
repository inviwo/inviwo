/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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
#include <inviwo/core/interaction/pickingobject.h>

namespace inviwo {

PickingMapper::PickingMapper(PickingManager* manager) : manager_(manager) {}

PickingMapper::PickingMapper(Processor* processor, size_t size,
                             std::function<void(const PickingObject*)> callback,
                             PickingManager* manager)
    : manager_(manager)
    , processor_(processor)
    , callback_(callback)
    , pickingObject_(manager_->registerPickingCallback(callback, size)) {}

PickingMapper::PickingMapper(PickingMapper&& rhs)
    : manager_(rhs.manager_)
    , processor_(rhs.processor_)
    , callback_(std::move(rhs.callback_))
    , pickingObject_(rhs.pickingObject_) {
    rhs.processor_ = nullptr;
    rhs.pickingObject_ = nullptr;
}

PickingMapper& PickingMapper::operator=(PickingMapper&& that) {
    if (this != &that) {
        std::swap(that.manager_, manager_);
        std::swap(that.processor_, processor_);
        std::swap(that.callback_, callback_);
        std::swap(that.pickingObject_, pickingObject_);

        that.processor_ = nullptr;
        if (manager_) manager_->unregisterPickingObject(that.pickingObject_);
    }
    return *this;
}

PickingMapper::~PickingMapper() {
    if (pickingObject_ && manager_) {
        manager_->unregisterPickingObject(pickingObject_);
    }
}

void PickingMapper::resize(size_t newSize) {
    if (pickingObject_ && manager_) {
        manager_->unregisterPickingObject(pickingObject_);
        pickingObject_ = nullptr;
    }
    if(newSize > 0 && manager_) {
        pickingObject_ = manager_->registerPickingCallback(callback_, newSize);
    }
}

const PickingObject* PickingMapper::getPickingObject() const { return pickingObject_; }

}  // namespace
