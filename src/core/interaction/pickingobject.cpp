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

namespace inviwo {

PickingObject::PickingObject(size_t id, size_t size)
    : start_(id)
    , size_(size)
    , capacity_(size)
    , interactionEventType_(InteractionEventType::NoneSupported)
    , pos_(vec2(0.f))
    , depth_(0.f)
    , move_(vec2(0.f)) {
}

PickingObject::~PickingObject() = default;

size_t PickingObject::getPickingId(size_t id) const {
    if (id < size_)
        return start_ + id;
    else 
        throw Exception("Out of range", IvwContext);
}

size_t PickingObject::getPickedId() const {
    return pickedId_;
}

size_t PickingObject::getSize() const {
    return size_;
}

vec3 PickingObject::getColor(size_t id) const {
    return vec3(PickingManager::indexToColor(getPickingId(id))) / 255.0f;
}

vec2 PickingObject::getInitialPosition() const {
    return pos_;
}

vec2 PickingObject::getPosition() const {
    return mouseEvent_.posNormalized();
}

vec2 PickingObject::getDelta() const {
    return move_;
}

double PickingObject::getDepth() const {
    return depth_;
}

vec2 PickingObject::getTotalDelta() const {
    return mouseEvent_.posNormalized() - pos_;
}

void PickingObject::picked() const {
    callback_(this);
}

PickingObject::InteractionEventType PickingObject::getInteractionType() const {
    return interactionEventType_;
}

void PickingObject::setMouseEvent(MouseEvent e){
    mouseEvent_ = e;
    interactionEventType_ = InteractionEventType::MouseInteraction;
}

const MouseEvent& PickingObject::getMouseEvent() const {
    return mouseEvent_;
}

void PickingObject::setTouchEvent(TouchEvent e){
    touchEvent_ = e;
    interactionEventType_ = InteractionEventType::TouchInteraction;
}

const TouchEvent& PickingObject::getTouchEvent() const {
    return touchEvent_;
}

void PickingObject::setPosition(vec2 pos) {
    pos_ = pos;
}

void PickingObject::setDelta(vec2 move) {
    move_ = move;
}

void PickingObject::setDepth(double depth) {
    depth_ = depth;
}

void PickingObject::setCallback(std::function<void(const PickingObject*)> func) {
    callback_ = func;
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
