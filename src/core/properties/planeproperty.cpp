/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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
#include <inviwo/core/properties/planeproperty.h>

namespace inviwo {

PlaneProperty::PlaneProperty(std::string identifier, std::string displayName,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , enable_("enable", "Enable", true, INVALID_RESOURCES)
    , mode_("mode", "Mode", INVALID_RESOURCES)
    , position_("position", "Position", vec3(0.5f), vec3(0.0f), vec3(1.0f))
    , normal_("normal", "Normal", vec3(0.0f, 0.0f, 1.0f), vec3(-1.0f), vec3(1.0f))
    , color_("color", "Color", vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f), vec4(1.0f), vec4(0.01f),
             INVALID_OUTPUT, PropertySemantics::Color) {
    addProperty(enable_);
    addProperty(mode_);
    addProperty(position_);
    addProperty(normal_);
    addProperty(color_);

    mode_.onChange(this, &PlaneProperty::onModeChange);
    mode_.addOption("plane", "Plane", 0);

    setAllPropertiesCurrentStateAsDefault();
}

PropertyClassIdentifier(PlaneProperty, "org.inviwo.PlaneProperty");

PlaneProperty::PlaneProperty(const PlaneProperty& rhs)
    : CompositeProperty(rhs)
    , enable_(rhs.enable_)
    , mode_(rhs.mode_)
    , position_(rhs.position_)
    , normal_(rhs.normal_)
    , color_(rhs.color_) {

    addProperty(enable_);
    addProperty(mode_);
    addProperty(position_);
    addProperty(normal_);
    addProperty(color_);

    mode_.onChange(this, &PlaneProperty::onModeChange);
    setAllPropertiesCurrentStateAsDefault();
}

PlaneProperty& PlaneProperty::operator=(const PlaneProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        enable_ = that.enable_;
        mode_ = that.mode_;
        position_ = that.position_;
        normal_ = that.normal_;
        color_ = that.color_;
    }
    return *this;
}

PlaneProperty* PlaneProperty::clone() const { return new PlaneProperty(*this); }

PlaneProperty::~PlaneProperty() {}

void PlaneProperty::onModeChange() {}

}  // namespace
