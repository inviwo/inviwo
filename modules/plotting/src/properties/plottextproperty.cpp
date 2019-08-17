/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/plotting/properties/plottextproperty.h>

namespace inviwo {

namespace plot {

const std::string PlotTextProperty::classIdentifier = "org.inviwo.PlotTextProperty";
std::string PlotTextProperty::getClassIdentifier() const { return classIdentifier; }

PlotTextProperty::PlotTextProperty(const std::string& identifier, const std::string& displayName,
                                   bool checked, InvalidationLevel invalidationLevel,
                                   PropertySemantics semantics)
    : BoolCompositeProperty(identifier, displayName, checked, invalidationLevel, semantics)
    , title_("title", "Title", "")
    , color_("color", "Color", vec4(vec3(0.0f), 1.0f), vec4(0.0f), vec4(1.0f))
    , position_("position", "Position", 0.5f, 0.0f, 1.0f)
    , offset_("offset", "Offset", 10.0f, 0.0f, 100.0f)
    , rotation_("rotation", "Rotation", 0.f, 0.f, 360.f, 10.f, InvalidationLevel::InvalidOutput)
    , font_("font", "Font") {
    color_.setSemantics(PropertySemantics::Color);
    addProperty(title_);
    addProperty(color_);
    addProperty(offset_);
    addProperty(position_);
    addProperty(rotation_);
    addProperty(font_);

    font_.anchorPos_.set(vec2(0.0f, 0.0f));
}

PlotTextProperty::PlotTextProperty(const PlotTextProperty& rhs)
    : BoolCompositeProperty(rhs)
    , title_(rhs.title_)
    , color_(rhs.color_)
    , position_(rhs.position_)
    , offset_(rhs.offset_)
    , rotation_(rhs.rotation_)
    , font_(rhs.font_) {

    color_.setSemantics(PropertySemantics::Color);
    addProperty(title_);
    addProperty(color_);
    addProperty(offset_);
    addProperty(position_);
    addProperty(rotation_);
    addProperty(font_);
}

PlotTextProperty* PlotTextProperty::clone() const { return new PlotTextProperty(*this); }

bool PlotTextProperty::isEnabled() const { return isChecked(); }
vec4 PlotTextProperty::getColor() const { return color_.get(); }
float PlotTextProperty::getPosition() const { return position_.get(); }
vec2 PlotTextProperty::getOffset() const { return {offset_.get(), 0.0f}; }
float PlotTextProperty::getRotation() const { return rotation_.get(); }
const FontSettings& PlotTextProperty::getFont() const { return font_; }

}  // namespace plot

}  // namespace inviwo
