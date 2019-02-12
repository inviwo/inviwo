/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/animation/datastructures/basekeyframe.h>

namespace inviwo {

namespace animation {

BaseKeyframe::BaseKeyframe(Seconds time) : time_(time) {}
BaseKeyframe::~BaseKeyframe() = default;

BaseKeyframe::BaseKeyframe(const BaseKeyframe& rhs) = default;
BaseKeyframe& BaseKeyframe::operator=(const BaseKeyframe& that) {
    if (this != &that) {
        Keyframe::operator=(that);
        setTime(that.time_);
        setSelected(that.isSelected_);
    }
    return *this;
}

void BaseKeyframe::setTime(Seconds time) {
    if (time != time_) {
        auto oldTime = time_;
        time_ = time;
        notifyKeyframeTimeChanged(this, oldTime);
    }
}
Seconds BaseKeyframe::getTime() const { return time_; }

bool BaseKeyframe::isSelected() const { return isSelected_; }
void BaseKeyframe::setSelected(bool selected) {
    if (selected != isSelected_) {
        isSelected_ = selected;
        notifyKeyframeSelectionChanged(this);
    }
}

void BaseKeyframe::serialize(Serializer& s) const {
    s.serialize("time", time_.count());
    s.serialize("selected", isSelected_);
}

void BaseKeyframe::deserialize(Deserializer& d) {
    {
        double tmp = time_.count();
        d.deserialize("time", tmp);
        setTime(Seconds{tmp});
    }
    {
        bool isSelected = isSelected_;
        d.deserialize("selected", isSelected);
        setSelected(isSelected);
    }
}

}  // namespace animation

}  // namespace inviwo
