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

#include <inviwo/core/datastructures/transferfunctiondatapoint.h>

namespace inviwo {

TransferFunctionDataPoint::TransferFunctionDataPoint(const vec2& pos, const vec4& rgba)
    : pos_(pos)
    , rgba_(rgba)
    , notify_(true) {
}

TransferFunctionDataPoint::TransferFunctionDataPoint(const TransferFunctionDataPoint& rhs) 
    : pos_(rhs.pos_), rgba_(rhs.rgba_), notify_(rhs.notify_) {
}

TransferFunctionDataPoint& TransferFunctionDataPoint::operator=(const TransferFunctionDataPoint& that) {
    if (this != &that) {
        pos_ = that.pos_;
        rgba_ = that.rgba_;
        notify_ = that.notify_;
    }
    return *this;
}

TransferFunctionDataPoint::~TransferFunctionDataPoint() {}

void TransferFunctionDataPoint::setPos(const vec2& pos) {
    if(pos==pos_ && pos.y == rgba_.a) return;
    pos_ = pos;
    rgba_.a = pos.y;
    notifyTransferFunctionPointObservers();
}

void TransferFunctionDataPoint::setRGBA(const vec4& rgba) {
    if(rgba==rgba_ && rgba.a == pos_.y) return;
    pos_.y = rgba.a;
    rgba_ = rgba;
    notifyTransferFunctionPointObservers();
}

void TransferFunctionDataPoint::setRGB(const vec3& rgb) {
    if(rgb.r == rgba_.r && rgb.g == rgba_.g && rgb.b == rgba_.b) return;
    rgba_.r = rgb.r;
    rgba_.g = rgb.g;
    rgba_.b = rgb.b;
    notifyTransferFunctionPointObservers();
}

void TransferFunctionDataPoint::setA(float alpha) {
    if(pos_.y == alpha && rgba_.a == alpha) return;
    pos_.y = alpha;
    rgba_.a = alpha;
    notifyTransferFunctionPointObservers();
}

void TransferFunctionDataPoint::setPosA(const vec2& pos, float alpha) {
    if(pos_ == pos && rgba_.a == alpha) return;
    pos_ = pos;
    rgba_.a = alpha;
    notifyTransferFunctionPointObservers();
}

void TransferFunctionDataPoint::notifyTransferFunctionPointObservers() const {
    if(notify_) {
        // Notify observers
        for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
            // static_cast can be used since only template class objects can be added
            static_cast<TransferFunctionPointObserver*>(*it)->onTransferFunctionPointChange(this);
        }
    }
}

void TransferFunctionDataPoint::serialize(IvwSerializer& s) const {
    s.serialize("pos", pos_);
    s.serialize("rgba", rgba_);
}

void TransferFunctionDataPoint::deserialize(IvwDeserializer& d) {
    d.deserialize("pos", pos_);
    d.deserialize("rgba", rgba_);
}

bool operator==(const TransferFunctionDataPoint& lhs, const TransferFunctionDataPoint& rhs) {
    return lhs.pos_ == rhs.pos_ && lhs.rgba_ == rhs.rgba_;
}

bool operator!=(const TransferFunctionDataPoint& lhs, const TransferFunctionDataPoint& rhs) {
    return !operator==(lhs, rhs);
}

bool operator<(const TransferFunctionDataPoint& lhs, const TransferFunctionDataPoint& rhs) {
    return lhs.pos_.x < rhs.pos_.x;
}

bool operator>(const TransferFunctionDataPoint& lhs, const TransferFunctionDataPoint& rhs) {
    return rhs < lhs;
}

bool operator<=(const TransferFunctionDataPoint& lhs, const TransferFunctionDataPoint& rhs) {
    return !(rhs < lhs);
}

bool operator>=(const TransferFunctionDataPoint& lhs, const TransferFunctionDataPoint& rhs) {
    return !(lhs < rhs);
}

} // namespace