/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/tfprimitive.h>

namespace inviwo {

void TFPrimitiveObserver::onTFPrimitiveChange(const TFPrimitive&) {}

TFPrimitive::TFPrimitive(double pos, const vec4& color)
    : Observable<TFPrimitiveObserver>(), data_({pos, color}) {}

// Cannot use default constructors and assignment operator for TFPrimitive!
//
// Default constructors would call the base class constructor of Observable and thereby
// copy all observers. This must be avoided since TFPrimitives are a part of a property
// and when setting/assigning a property, no observers must be copied!
TFPrimitive::TFPrimitive(const TFPrimitiveData& data)
    : Observable<TFPrimitiveObserver>(), data_(data) {}

TFPrimitive::TFPrimitive(const TFPrimitive& rhs)
    : Observable<TFPrimitiveObserver>(), data_(rhs.data_) {}

TFPrimitive::TFPrimitive(TFPrimitive&& rhs) noexcept
    : Observable<TFPrimitiveObserver>(), data_(std::move(rhs.data_)) {}

TFPrimitive& TFPrimitive::operator=(const TFPrimitive& rhs) {
    if ((this != &rhs) && (*this != rhs)) {
        data_ = rhs.data_;
        notifyTFPrimitiveObservers();
    }
    return *this;
}

TFPrimitive& TFPrimitive::operator=(const TFPrimitiveData& rhs) {
    if (data_ != rhs) {
        data_ = rhs;
        notifyTFPrimitiveObservers();
    }
    return *this;
}

TFPrimitive& TFPrimitive::operator=(TFPrimitive&& rhs) noexcept {
    if ((this != &rhs) && (*this != rhs)) {
        data_ = rhs.data_;
        notifyTFPrimitiveObservers();
    }
    return *this;
}

void TFPrimitive::setData(const TFPrimitiveData& data) {
    if ((data.pos != data_.pos) || (data.color != data_.color)) {
        data_.pos = data.pos;
        data_.color = data.color;
        notifyTFPrimitiveObservers();
    }
}

void TFPrimitive::setPosition(double pos) {
    if (pos != data_.pos) {
        data_.pos = pos;
        notifyTFPrimitiveObservers();
    }
}

void TFPrimitive::setAlpha(float alpha) {
    if (alpha != data_.color.a) {
        data_.color.a = alpha;
        notifyTFPrimitiveObservers();
    }
}

void TFPrimitive::setPositionAlpha(double pos, float alpha) {
    if ((pos != data_.pos) || (alpha != data_.color.a)) {
        data_.pos = pos;
        data_.color.a = alpha;
        notifyTFPrimitiveObservers();
    }
}

void TFPrimitive::setPositionAlpha(const dvec2& p) {
    setPositionAlpha(p.x, static_cast<float>(p.y));
}

void TFPrimitive::setColor(const vec3& color) {
    if (vec3(data_.color) != color) {
        for (size_t i = 0; i < 3; ++i) {
            data_.color[i] = color[i];
        }
        notifyTFPrimitiveObservers();
    }
}

void TFPrimitive::setColor(const vec4& color) {
    if (data_.color != color) {
        data_.color = color;
        notifyTFPrimitiveObservers();
    }
}

void TFPrimitive::notifyTFPrimitiveObservers() {
    forEachObserver([&](TFPrimitiveObserver* o) { o->onTFPrimitiveChange(*this); });
}

void TFPrimitive::serialize(Serializer& s) const {
    s.serialize("pos", data_.pos);
    s.serialize("rgba", data_.color);
}

void TFPrimitive::deserialize(Deserializer& d) {
    double pos = 0.0;
    vec4 color;
    d.deserialize("pos", pos);
    d.deserialize("rgba", color);
    setData({pos, color});
}
vec4 util::interpolateColor(const TFPrimitiveData& p1, const TFPrimitiveData& p2, double x) {
    if (std::abs(p1.pos - p2.pos) < 1e-8) return glm::mix(p1.color, p2.color, 0.5);

    const double t = (x - p1.pos) / (p2.pos - p1.pos);
    return glm::mix(p1.color, p2.color, t);
}

vec4 util::interpolateColor(std::span<const TFPrimitiveData> line, double x) {
    if (line.empty()) return vec4{0.0f};

    if (x <= line.front().pos) return line.front().color;
    if (x >= line.back().pos) return line.back().color;

    auto it = std::upper_bound(line.begin(), line.end(), x,
                               [](double x, const TFPrimitiveData& pt) { return x < pt.pos; });

    const auto& p2 = *it;
    const auto& p1 = *(it - 1);
    return interpolateColor(p1, p2, x);
}

}  // namespace inviwo
