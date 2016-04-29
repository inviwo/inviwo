/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

PickingObject::~PickingObject() {}

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

vec3 PickingObject::getPickingColor(size_t id) const {
    return vec3(PickingManager::indexToColor(getPickingId(id))) / 255.0f;
}

const vec2& PickingObject::getPickingPosition() const {
    return pos_;
}

const vec2& PickingObject::getPickingMove() const {
    return move_;
}

const double& PickingObject::getPickingDepth() const {
    return depth_;
}

vec2 PickingObject::getPickingTotalDelta() const {
    return mouseEvent_.posNormalized() - pos_;
}

void PickingObject::picked() const {
    callback_(this);
}

PickingObject::InteractionEventType PickingObject::getPickingInteractionType() const {
    return interactionEventType_;
}

void PickingObject::setPickingMouseEvent(MouseEvent e){
    mouseEvent_ = e;
    interactionEventType_ = InteractionEventType::MouseInteraction;
}

const MouseEvent& PickingObject::getPickingMouseEvent() const {
    return mouseEvent_;
}

void PickingObject::setPickingTouchEvent(TouchEvent e){
    touchEvent_ = e;
    interactionEventType_ = InteractionEventType::TouchInteraction;
}

const TouchEvent& PickingObject::getPickingTouchEvent() const {
    return touchEvent_;
}

void PickingObject::setPickingPosition(vec2 pos) {
    pos_ = pos;
}

void PickingObject::setPickingMove(vec2 move) {
    move_ = move;
}

void PickingObject::setPickingDepth(double depth) {
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
