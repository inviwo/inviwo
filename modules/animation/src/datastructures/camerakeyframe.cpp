/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2020 Inviwo Foundation
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

#include <modules/animation/datastructures/camerakeyframe.h>

namespace inviwo {

namespace animation {

CameraKeyframe::CameraKeyframe(Seconds time) : BaseKeyframe{time} {}

CameraKeyframe::CameraKeyframe(Seconds time, const Camera& value)
    : BaseKeyframe{time}, value_{std::unique_ptr<Camera>(value.clone())} {}

CameraKeyframe::CameraKeyframe(Seconds time, std::unique_ptr<Camera> value)
    : BaseKeyframe{time}, value_{std::move(value)} {}

CameraKeyframe::CameraKeyframe(const CameraKeyframe& that)
    : BaseKeyframe(that), value_{that.value_->clone()} {}

CameraKeyframe& CameraKeyframe::operator=(const CameraKeyframe& that) {
    if (this != &that) {
        BaseKeyframe::operator=(that);
        value_->updateFrom(*that.value_);
    }
    return *this;
}

CameraKeyframe* CameraKeyframe::clone() const { return new CameraKeyframe(*this); }

const Camera& CameraKeyframe::getValue() const { return *value_; }
Camera& CameraKeyframe::getValue() { return *value_; }

void CameraKeyframe::updateFrom(const Camera& value) { value_->updateFrom(value); }

void CameraKeyframe::serialize(Serializer& s) const {
    BaseKeyframe::serialize(s);
    s.serialize("value", value_);
}

void CameraKeyframe::deserialize(Deserializer& d) {
    BaseKeyframe::deserialize(d);
    d.deserialize("value", value_);
}

}  // namespace animation

}  // namespace inviwo
