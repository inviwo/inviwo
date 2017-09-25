/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/plotting/properties/marginproperty.h>

namespace inviwo {

namespace plot {

PropertyClassIdentifier(MarginProperty, "org.inviwo.MarginProperty");

MarginProperty::MarginProperty(
    std::string identifier, std::string displayName, float top, float right, float bottom,
    float left, InvalidationLevel invalidationLevel /*= InvalidationLevel::InvalidOutput*/,
    PropertySemantics semantics /*= PropertySemantics::Default*/)
    : CompositeProperty(identifier, displayName)
    , top_("top", "Top", top, 0, 100, 1, invalidationLevel, semantics)
    , right_("right_", "Right", right, 0, 100, 1, invalidationLevel, semantics)
    , bottom_("bottom", "Bottom", bottom, 0, 100, 1, invalidationLevel, semantics)
    , left_("left", "Left", left, 0, 100, 1, invalidationLevel, semantics) {
    addProperty(top_);
    addProperty(right_);
    addProperty(bottom_);
    addProperty(left_);
}

void MarginProperty::setMargins(float top, float right, float bottom, float left) {
    top_.set(top);
    right_.set(right);
    bottom_.set(bottom);
    left_.set(left);
}

void MarginProperty::setTop(float top) { top_.set(top); }

void MarginProperty::setRight(float right) { right_.set(right); }

void MarginProperty::setBottom(float bottom) { bottom_.set(bottom); }

void MarginProperty::setLeft(float left) { left_.set(left); }

float MarginProperty::getTop() const { return top_.get(); }

float MarginProperty::getRight() const { return right_.get(); }

float MarginProperty::getBottom() const { return bottom_.get(); }

float MarginProperty::getLeft() const { return left_.get(); }

inviwo::vec4 MarginProperty::getAsVec4() const {
    return vec4(top_.get(), right_.get(), bottom_.get(), left_.get());
}

}  // namespace plot

}  // namespace inviwo
