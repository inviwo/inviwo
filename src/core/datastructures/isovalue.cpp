/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <inviwo/core/datastructures/isovalue.h>

#include <glm/gtc/epsilon.hpp>

namespace inviwo {

void IsoValueObserver::onIsoValueChange(const IsoValue&) {}

IsoValue::IsoValue(float value, const vec4& color) : data_({value, color}) {}

IsoValue::IsoValue(const IsoValue& rhs) : data_(rhs.data_) {}

IsoValue& IsoValue::operator=(const IsoValue& rhs) {
    if ((this != &rhs) && (*this != rhs)) {
        data_ = rhs.data_;
        notifyIsoValueObservers();
    }
    return *this;
}

void IsoValue::set(float value, const vec4& color) {
    if ((color != data_.color) ||
        glm::epsilonNotEqual(value, data_.isovalue, glm::epsilon<float>())) {
        data_.isovalue = value;
        data_.color = color;
        notifyIsoValueObservers();
    }
}

void IsoValue::set(const IsoValueData& value) { set(value.isovalue, value.color); }

void IsoValue::setIsoValue(const float& value) {
    if (glm::epsilonNotEqual(value, data_.isovalue, glm::epsilon<float>())) {
        data_.isovalue = value;
        notifyIsoValueObservers();
    }
}

void IsoValue::setColor(const vec4& color) {
    if (data_.color != color) {
        data_.color = color;
        notifyIsoValueObservers();
    }
}

void IsoValue::notifyIsoValueObservers() {
    forEachObserver([&](IsoValueObserver* o) { o->onIsoValueChange(*this); });
}

void IsoValue::serialize(Serializer& s) const {
    s.serialize("isovalue", data_.isovalue);
    s.serialize("color", data_.color);
}

void IsoValue::deserialize(Deserializer& d) {
    float value = 0.0f;
    vec4 color;
    d.deserialize("isovalue", value);
    d.deserialize("color", color);
    set(value, color);
}

bool operator==(const IsoValue& lhs, const IsoValue& rhs) {
    return glm::epsilonEqual(lhs.getIsoValue(), rhs.getIsoValue(), glm::epsilon<float>()) &&
           (lhs.getColor() == rhs.getColor());
}

bool operator!=(const IsoValue& lhs, const IsoValue& rhs) { return !operator==(lhs, rhs); }

bool operator<(const IsoValue& lhs, const IsoValue& rhs) {
    return lhs.getIsoValue() < rhs.getIsoValue();
}

bool operator>(const IsoValue& lhs, const IsoValue& rhs) { return rhs < lhs; }

bool operator<=(const IsoValue& lhs, const IsoValue& rhs) { return !(rhs < lhs); }

bool operator>=(const IsoValue& lhs, const IsoValue& rhs) { return !(lhs < rhs); }

bool operator==(const IsoValueData& lhs, const IsoValueData& rhs) {
    return glm::epsilonEqual(lhs.isovalue, rhs.isovalue, glm::epsilon<float>()) &&
           (lhs.color == rhs.color);
}

bool operator!=(const IsoValueData& lhs, const IsoValueData& rhs) { return !operator==(lhs, rhs); }

bool operator<(const IsoValueData& lhs, const IsoValueData& rhs) {
    return lhs.isovalue < rhs.isovalue;
}

bool operator>(const IsoValueData& lhs, const IsoValueData& rhs) { return rhs < lhs; }

bool operator<=(const IsoValueData& lhs, const IsoValueData& rhs) { return !(rhs < lhs); }

bool operator>=(const IsoValueData& lhs, const IsoValueData& rhs) { return !(lhs < rhs); }

}  // namespace inviwo
