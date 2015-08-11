/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <inviwo/core/properties/positionproperty.h>

namespace inviwo {
PropertyClassIdentifier(PositionProperty, "org.inviwo.PositionProperty");

PositionProperty::PositionProperty(std::string identifier, std::string displayName,
    FloatVec3Property position,
    const Camera* camera,
    InvalidationLevel invalidationLevel,
    PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , referenceFrame_("referenceFrame", "Space")
    , position_(position)
    , camera_(camera) {

    referenceFrame_.addOption("world", "World", static_cast<int>(Space::WORLD));
    if (camera_) {
        referenceFrame_.addOption("view", "View", static_cast<int>(Space::VIEW));
    }
    referenceFrame_.setSelectedValue(static_cast<int>(Space::WORLD));
    referenceFrame_.setCurrentStateAsDefault();

    position_.setSemantics(PropertySemantics::LightPosition);
    position_.setCurrentStateAsDefault();
    // add properties
    addProperty(referenceFrame_);
    addProperty(position_);
}

PositionProperty::PositionProperty(const PositionProperty& rhs) 
    : CompositeProperty(rhs)
    , referenceFrame_(rhs.referenceFrame_)
    , position_(rhs.position_) {
    // add properties
    addProperty(referenceFrame_);
    addProperty(position_);
}

PositionProperty* PositionProperty::clone() const {
    return new PositionProperty(*this);
}

vec3 PositionProperty::get() const {
    switch (static_cast<Space>(referenceFrame_.getSelectedValue())) {
    case Space::VIEW:
        return camera_ ? vec3(camera_->inverseViewMatrix() * vec4(position_.get(), 1.0f))
            : position_.get();
    case Space::WORLD:
    default:
        return position_.get();
    }
}

void PositionProperty::set(const vec3& worldSpacePos) {
    switch (static_cast<Space>(referenceFrame_.getSelectedValue())) {
    case Space::VIEW:
        position_.set( camera_ ? vec3(camera_->viewMatrix() * vec4(worldSpacePos, 1.0f))
            : worldSpacePos);
    case Space::WORLD:
    default:
        position_.set(worldSpacePos);
    }
}

PositionProperty& PositionProperty::operator=(const PositionProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        referenceFrame_ = that.referenceFrame_;
        position_ = that.position_;
    }
    return *this;
}

} // namespace

