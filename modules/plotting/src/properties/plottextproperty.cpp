/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <inviwo/core/properties/boolcompositeproperty.h>   // for BoolCompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>       // for InvalidationLevel, Invalidati...
#include <inviwo/core/properties/ordinalproperty.h>         // for FloatProperty, FloatVec4Property
#include <inviwo/core/properties/propertysemantics.h>       // for PropertySemantics, PropertySe...
#include <inviwo/core/properties/stringproperty.h>          // for StringProperty
#include <inviwo/core/util/glmvec.h>                        // for vec4, vec2, vec3
#include <modules/fontrendering/properties/fontproperty.h>  // for FontProperty

namespace inviwo {
class FontSettings;

namespace plot {

std::string_view PlotTextProperty::getClassIdentifier() const { return classIdentifier; }

PlotTextProperty::PlotTextProperty(std::string_view identifier, std::string_view displayName,
                                   Document help, bool checked, InvalidationLevel invalidationLevel,
                                   PropertySemantics semantics)
    : BoolCompositeProperty(identifier, displayName, help, checked, invalidationLevel, semantics)
    , title_("title", "Title", "")
    , placement_{"placement",
                 "Placement",
                 "Allows to place the text on either the outside or inside of the axis"_help,
                 {{"outside", "Outside", LabelPlacement::Outside},
                  {"inside", "Inside", LabelPlacement::Inside}},
                 0}
    , color_("color", "Color", util::ordinalColor(vec4(vec3(0.0f), 1.0f)).set("Text color"_help))
    , position_("position", "Position",
                "Relative position of the text between start and end point of the axis"_help, 0.5f,
                {0.0f, ConstraintBehavior::Editable}, {1.0f, ConstraintBehavior::Editable})
    , offset_("offset", "Offset",
              util::ordinalSymmetricVector(20.0f, 100.0f)
                  .set(PropertySemantics::Default)
                  .set("Offset between axis and text in pixel"_help))
    , rotation_("rotation", "Rotation",
                util::ordinalSymmetricVector(0.0f, 360.0f)
                    .setInc(10.0f)
                    .set("Text rotation in degree"_help))
    , font_("font", "Font") {

    color_.setSemantics(PropertySemantics::Color);

    addProperties(title_, placement_, color_, offset_, position_, rotation_, font_);

    font_.anchorPos_.set(vec2(0.0f, 0.0f));
}

PlotTextProperty::PlotTextProperty(std::string_view identifier, std::string_view displayName,
                                   bool checked, InvalidationLevel invalidationLevel,
                                   PropertySemantics semantics)
    : PlotTextProperty(identifier, displayName,
                       "Font and alignment settings for text elements of an axis"_help, checked,
                       invalidationLevel, semantics) {}

PlotTextProperty::PlotTextProperty(const PlotTextProperty& rhs)
    : BoolCompositeProperty(rhs)
    , title_(rhs.title_)
    , placement_(rhs.placement_)
    , color_(rhs.color_)
    , position_(rhs.position_)
    , offset_(rhs.offset_)
    , rotation_(rhs.rotation_)
    , font_(rhs.font_) {

    addProperties(title_, placement_, color_, offset_, position_, rotation_, font_);
}

PlotTextProperty* PlotTextProperty::clone() const { return new PlotTextProperty(*this); }

bool PlotTextProperty::isEnabled() const { return isChecked(); }
LabelPlacement PlotTextProperty::getPlacement() const { return placement_.getSelectedValue(); }
vec4 PlotTextProperty::getColor() const { return color_.get(); }
float PlotTextProperty::getPosition() const { return position_.get(); }
vec2 PlotTextProperty::getOffset() const { return {offset_.get(), 0.0f}; }
float PlotTextProperty::getRotation() const { return rotation_.get(); }
const FontSettings& PlotTextProperty::getFont() const { return font_; }

}  // namespace plot

}  // namespace inviwo
