/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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

#include <inviwo/core/interaction/pickingobject.h>
#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/eventmatcher.h>

namespace inviwo {

PickingObject::PickingObject(size_t id, size_t size)
    : start_(id)
    , size_(size)
    , capacity_(size) {
}

PickingObject::~PickingObject() = default;

size_t PickingObject::getPickingId(size_t id) const {
    if (id < size_)
        return start_ + id;
    else 
        throw Exception("Out of range", IvwContext);
}

vec3 PickingObject::getColor(size_t id) const {
    return vec3(PickingManager::indexToColor(getPickingId(id))) / 255.0f;
}

size_t PickingObject::getSize() const {
    return size_;
}

size_t PickingObject::getPickedId() const {
    return pickedId_;
}

dvec2 PickingObject::getPosition() const {
    if(event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_)->posNormalized();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_)->getCenterPointNormalized();
        }  
    } 
    return dvec2(0.0f);
}

double PickingObject::getDepth() const {
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_)->depth();
            case TouchEvent::chash():
                return static_cast<TouchEvent*>(event_)->getTouchPoints().front().getDepth();
        }
    }
    return 0.0f;
}

dvec2 PickingObject::getPreviousPosition() const {
    return previousPosition_;
}


double PickingObject::getPreviousDepth() const {
    return previousNDC_.z;
}


dvec2 PickingObject::getPressPosition() const {
    return pressPosition_;
}


double PickingObject::getPressDepth() const {
    return pressNDC_.z;
}

dvec2 PickingObject::getDeltaPosition() const {
    return getPosition() - previousPosition_;
}

double PickingObject::getDeltaDepth() const {
    return getDepth() - previousNDC_.z;
}

dvec2 PickingObject::getDeltaPressPosition() const {
    return getPosition() - pressPosition_;
}

double PickingObject::getDeltaPressDepth() const {
    return getDepth() - pressNDC_.z;
}

dvec3 PickingObject::getNDC() const {
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash():
                return static_cast<MouseEvent*>(event_)->ndc();
            //case TouchEvent::chash():
            //    return static_cast<TouchEvent*>(event_)->getCenterPointNormalized();
        }
    }
    return dvec3(0.0f);
}

dvec3 PickingObject::getPreviousNDC() const {
    return previousNDC_;
}

dvec3 PickingObject::getPressNDC() const {
    return pressNDC_;
}

PickingState PickingObject::getState() const {
    return state_;
}

void PickingObject::picked(Event* e, PickingState state) {
    event_ = e;
    state_ = state;
    if (event_) {
        switch (event_->hash()) {
            case MouseEvent::chash(): {
                auto me = static_cast<MouseEvent*>(event_);
                if (me->state() == MouseState::Press) {
                    pressPosition_ = me->posNormalized();
                    pressNDC_ = me->ndc();
                }

                break;
            }
            case TouchEvent::chash(): {
                //auto te = static_cast<TouchEvent*>(event_);
                LogError("Picking with TouchEvent not implemented yet");
                break;
            }
        }
    }

    if (enabled_)  action_(this);

    previousPosition_ = getPosition();
    previousNDC_ = getNDC();
    event_ = nullptr;
    state_ = PickingState::None;
}

Event* PickingObject::getEvent() const {
    return event_;
}

bool PickingObject::isEnabled() const {
    return enabled_;
}

void PickingObject::setEnabled(bool enabled) {
    enabled_ = enabled;
}

void PickingObject::setAction(Action action) {
    action_ = action;
}

size_t PickingObject::getCapacity() const {
    return capacity_;
}

void PickingObject::setSize(size_t size) {
    if (size <= capacity_)
        size_ = size;
    else
        throw Exception("Out of range", IvwContext);
}

void PickingObject::setPickedId(size_t id) {
    if (id < size_)
        pickedId_ = id;
    else
        throw Exception("Out of range", IvwContext);
}

} // namespace
