/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

const std::string PlaneProperty::classIdentifier = "org.inviwo.PlaneProperty";
std::string PlaneProperty::getClassIdentifier() const { return classIdentifier; }

PlaneProperty::PlaneProperty(const std::string& identifier, const std::string& displayName,
                             vec3 position, vec3 normal, vec4 color,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : BoolCompositeProperty(identifier, displayName, true, invalidationLevel, semantics)
    , position_("position", "Position", position, vec3(0.0f), vec3(1.0f), vec3(0.01f),
                InvalidationLevel::InvalidOutput, PropertySemantics("SpinBox"))
    , normal_("normal", "Normal", normal, vec3(-1.0f), vec3(1.0f), vec3(0.01f),
              InvalidationLevel::InvalidOutput, PropertySemantics("SpinBox"))
    , color_("color", "Color", color, vec4(0.0f), vec4(1.0f), vec4(0.01f),
             InvalidationLevel::InvalidOutput, PropertySemantics::Color) {

    getBoolProperty()->setIdentifier("enable");

    addProperty(position_);
    addProperty(normal_);
    addProperty(color_);
}

PlaneProperty::PlaneProperty(const std::string& identifier, const std::string& displayName,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : PlaneProperty(identifier, displayName, vec3(0.5f), vec3(0.0f, 0.0f, 1.0f), vec4(1.0f),
                    invalidationLevel, semantics) {}

PlaneProperty::PlaneProperty(const PlaneProperty& rhs)
    : BoolCompositeProperty(rhs)
    , position_(rhs.position_)
    , normal_(rhs.normal_)
    , color_(rhs.color_) {

    addProperty(position_);
    addProperty(normal_);
    addProperty(color_);
}

PlaneProperty* PlaneProperty::clone() const { return new PlaneProperty(*this); }

PlaneProperty::~PlaneProperty() = default;

}  // namespace inviwo
