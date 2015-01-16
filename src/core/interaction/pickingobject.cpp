/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

namespace inviwo {

PickingObject::PickingObject(size_t id, DataVec3UINT8::type c) : id_(id), colorUINT8_(c),
    pos_(vec2(0.f)), readDepth_(true), depth_(0.f), move_(vec2(0.f)) {
    onPickedCallback_ = new PickingCallback();
    color_ = static_cast<vec3>(DataVec3UINT8::get()->valueToNormalizedVec3Double(&c));
}

PickingObject::~PickingObject() {
    delete onPickedCallback_;
}

const size_t& PickingObject::getPickingId() const {
    return id_;
}

const vec3& PickingObject::getPickingColor() const {
    return color_;
}

const DataVec3UINT8::type& PickingObject::getPickingColorAsUINT8() const {
    return colorUINT8_;
}

const vec2& PickingObject::getPickingPosition() const {
    return pos_;
}

const vec2& PickingObject::getPickingMove() const {
    return move_;
}

void PickingObject::setReadDepth(bool rd) {
    readDepth_ = rd;
}

bool PickingObject::readDepth() {
    return readDepth_;
}

const double& PickingObject::getPickingDepth() const {
    return depth_;
}

void PickingObject::picked() const {
    onPickedCallback_->invoke(this);
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

PickingCallback* PickingObject::getCallbackContainer() {
    return onPickedCallback_;
}

} // namespace
