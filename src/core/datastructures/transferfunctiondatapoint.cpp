/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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

TransferFunctionDataPoint::TransferFunctionDataPoint(const Point& point) : point_(point) {}

TransferFunctionDataPoint::TransferFunctionDataPoint(const float& pos, const vec4& rgba)
    : TransferFunctionDataPoint(Point{ pos, rgba }) {}

TransferFunctionDataPoint& TransferFunctionDataPoint::operator=(
    const TransferFunctionDataPoint& that) {
    if (point_ != that.point_) {
        point_ = that.point_;
        notifyTransferFunctionPointObservers();
    }
    return *this;
}

void TransferFunctionDataPoint::setPoint(const Point& point) {
    if (point_ != point) {
        point_ = point;
        notifyTransferFunctionPointObservers();
    }
}

void TransferFunctionDataPoint::setPos(const float& pos) {
    if (point_.pos != pos) {
        point_.pos = pos;
        notifyTransferFunctionPointObservers();
    }
}

void TransferFunctionDataPoint::setRGBA(const vec4& rgba) {
    if (point_.color != rgba) {
        point_.color = rgba;
        notifyTransferFunctionPointObservers();
    }
}

void TransferFunctionDataPoint::setRGB(const vec3& rgb) {
    if (rgb.r != point_.color.r || rgb.g != point_.color.g || rgb.b != point_.color.b) {
        point_.color.r = rgb.r;
        point_.color.g = rgb.g;
        point_.color.b = rgb.b;
        notifyTransferFunctionPointObservers();
    }
}

void TransferFunctionDataPoint::setA(float alpha) {
    if (point_.color.a != alpha) {
        point_.color.a = alpha;
        notifyTransferFunctionPointObservers();
    }
}

void TransferFunctionDataPoint::setPosA(const float& pos, float alpha) {
    if (point_.pos != pos || point_.color.a != alpha) {
        point_.pos = pos;
        point_.color.a = alpha;
        notifyTransferFunctionPointObservers();
    }
}

void TransferFunctionDataPoint::notifyTransferFunctionPointObservers() {
    forEachObserver(
        [&](TransferFunctionPointObserver* o) { o->onTransferFunctionPointChange(this); });
}

void TransferFunctionDataPoint::serialize(Serializer& s) const {
    s.serialize("pos", point_.pos);
    s.serialize("rgba", point_.color);
}

void TransferFunctionDataPoint::deserialize(Deserializer& d) {
    auto oldPos = point_.pos;
    auto oldRgba = point_.color;
    d.deserialize("pos", point_.pos);
    d.deserialize("rgba", point_.color);
    if (oldPos != point_.pos || oldRgba != point_.color) {
        notifyTransferFunctionPointObservers();
    }
}

bool operator==(const TransferFunctionDataPoint::Point& lhs,
                const TransferFunctionDataPoint::Point& rhs) {
    return lhs.pos == rhs.pos && lhs.color == rhs.color;
}

bool operator!=(const TransferFunctionDataPoint::Point& lhs,
                const TransferFunctionDataPoint::Point& rhs) {
    return !operator==(lhs, rhs);
}

bool operator<(const TransferFunctionDataPoint::Point& lhs,
               const TransferFunctionDataPoint::Point& rhs) {
    return lhs.pos < rhs.pos;
}

bool operator>(const TransferFunctionDataPoint::Point& lhs,
               const TransferFunctionDataPoint::Point& rhs) {
    return rhs < lhs;
}

bool operator<=(const TransferFunctionDataPoint::Point& lhs,
                const TransferFunctionDataPoint::Point& rhs) {
    return !(rhs < lhs);
}

bool operator>=(const TransferFunctionDataPoint::Point& lhs,
                const TransferFunctionDataPoint::Point& rhs) {
    return !(lhs < rhs);
}

bool operator==(const TransferFunctionDataPoint& lhs, const TransferFunctionDataPoint& rhs) {
    return lhs.getPoint() == rhs.getPoint();
}

bool operator!=(const TransferFunctionDataPoint& lhs, const TransferFunctionDataPoint& rhs) {
    return !operator==(lhs, rhs);
}

bool operator<(const TransferFunctionDataPoint& lhs, const TransferFunctionDataPoint& rhs) {
    return lhs.getPoint() < rhs.getPoint();
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
