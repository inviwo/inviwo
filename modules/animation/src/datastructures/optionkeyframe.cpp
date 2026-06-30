/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/animation/datastructures/optionkeyframe.h>

#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>

#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/basekeyframe.h>

namespace inviwo {

namespace animation {

OptionKeyframe::OptionKeyframe(Seconds time) : BaseKeyframe{time} {}

OptionKeyframe::OptionKeyframe(Seconds time, size_t index) : BaseKeyframe{time}, index_{index} {}

OptionKeyframe& OptionKeyframe::operator=(const OptionKeyframe& that) {
    if (this != &that) {
        BaseKeyframe::operator=(that);
        index_ = that.index_;
    }
    return *this;
}

OptionKeyframe* OptionKeyframe::clone() const { return new OptionKeyframe(*this); }

size_t OptionKeyframe::getIndex() const { return index_; }

void OptionKeyframe::setIndex(size_t index) { index_ = index; }

void OptionKeyframe::serialize(Serializer& s) const {
    BaseKeyframe::serialize(s);
    s.serialize("index", index_);
}

void OptionKeyframe::deserialize(Deserializer& d) {
    BaseKeyframe::deserialize(d);
    d.deserialize("index", index_);
}

bool operator==(const OptionKeyframe& a, const OptionKeyframe& b) {
    return a.getTime() == b.getTime() && a.getIndex() == b.getIndex();
}

bool operator!=(const OptionKeyframe& a, const OptionKeyframe& b) { return !(a == b); }

}  // namespace animation

}  // namespace inviwo
