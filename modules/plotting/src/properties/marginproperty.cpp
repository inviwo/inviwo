/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <inviwo/core/network/networklock.h>

namespace inviwo {

namespace plot {

const std::string MarginProperty::classIdentifier = "org.inviwo.MarginProperty";
std::string MarginProperty::getClassIdentifier() const { return classIdentifier; }

MarginProperty::MarginProperty(
    std::string identifier, std::string displayName, float top, float right, float bottom,
    float left, InvalidationLevel invalidationLevel /*= InvalidationLevel::InvalidOutput*/,
    PropertySemantics semantics /*= PropertySemantics::Default*/)
    : CompositeProperty(identifier, displayName)
    , top_("top", "Top", top, 0.0f, 100.0f, 1.0f, invalidationLevel, semantics)
    , right_("right_", "Right", right, 0.0f, 100.0f, 1.0f, invalidationLevel, semantics)
    , bottom_("bottom", "Bottom", bottom, 0.0f, 100.0f, 1.0f, invalidationLevel, semantics)
    , left_("left", "Left", left, 0.0f, 100.0f, 1.0f, invalidationLevel, semantics) {
    addProperty(top_);
    addProperty(right_);
    addProperty(bottom_);
    addProperty(left_);
}

MarginProperty::MarginProperty(const MarginProperty& rhs)
    : CompositeProperty(rhs)
    , top_{rhs.top_}
    , right_{rhs.right_}
    , bottom_{rhs.bottom_}
    , left_{rhs.left_} {

    addProperty(top_);
    addProperty(right_);
    addProperty(bottom_);
    addProperty(left_);
}

MarginProperty* MarginProperty::clone() const { return new MarginProperty(*this); }

void MarginProperty::setMargins(float top, float right, float bottom, float left) {
    NetworkLock lock(this);
    top_.set(top, std::min(top_.getMinValue(), top), std::max(top_.getMaxValue(), top * 2.0f),
             top_.getIncrement());
    right_.set(right, std::min(right_.getMinValue(), right),
               std::max(right_.getMaxValue(), right * 2.0f), right_.getIncrement());
    bottom_.set(bottom, std::min(bottom_.getMinValue(), bottom),
                std::max(bottom_.getMaxValue(), bottom * 2.0f), bottom_.getIncrement());
    left_.set(left, std::min(left_.getMinValue(), left), std::max(left_.getMaxValue(), left * 2.0f),
              left_.getIncrement());
}

void MarginProperty::setTop(float top) {
    top_.set(top, std::min(top_.getMinValue(), top), std::max(top_.getMaxValue(), top * 2.0f),
             top_.getIncrement());
}

void MarginProperty::setRight(float right) {
    right_.set(right, std::min(right_.getMinValue(), right),
               std::max(right_.getMaxValue(), right * 2.0f), right_.getIncrement());
}

void MarginProperty::setBottom(float bottom) {
    bottom_.set(bottom, std::min(bottom_.getMinValue(), bottom),
                std::max(bottom_.getMaxValue(), bottom * 2.0f), bottom_.getIncrement());
}

void MarginProperty::setLeft(float left) {
    left_.set(left, std::min(left_.getMinValue(), left), std::max(left_.getMaxValue(), left * 2.0f),
              left_.getIncrement());
}

float MarginProperty::getTop() const { return top_.get(); }

float MarginProperty::getRight() const { return right_.get(); }

float MarginProperty::getBottom() const { return bottom_.get(); }

float MarginProperty::getLeft() const { return left_.get(); }

vec2 MarginProperty::getLowerLeftMargin() const { return {left_, bottom_}; }

vec2 MarginProperty::getUpperRightMargin() const { return {right_, top_}; }

void MarginProperty::setLowerLeftMargin(vec2 lowerLeft) {
    auto left = lowerLeft.x;
    auto bottom = lowerLeft.y;

    NetworkLock lock(this);
    left_.set(left, std::min(left_.getMinValue(), left), std::max(left_.getMaxValue(), left * 2.0f),
              left_.getIncrement());
    bottom_.set(bottom, std::min(bottom_.getMinValue(), bottom),
                std::max(bottom_.getMaxValue(), bottom * 2.0f), bottom_.getIncrement());
}

void MarginProperty::setUpperRightMargin(vec2 upperRight) {
    auto right = upperRight.x;
    auto top = upperRight.y;

    NetworkLock lock(this);
    right_.set(right, std::min(right_.getMinValue(), right),
               std::max(right_.getMaxValue(), right * 2.0f), right_.getIncrement());
    top_.set(top, std::min(top_.getMinValue(), top), std::max(top_.getMaxValue(), top * 2.0f),
             top_.getIncrement());
}

vec4 MarginProperty::getAsVec4() const {
    return vec4(top_.get(), right_.get(), bottom_.get(), left_.get());
}

std::pair<vec2, vec2> MarginProperty::getRect(vec2 size) const {
    return {vec2{left_, bottom_}, size - vec2{right_, top_}};
}

vec2 MarginProperty::getSize(vec2 size) const {
    return size - vec2{left_, bottom_} - vec2{right_, top_};
}

}  // namespace plot

}  // namespace inviwo
