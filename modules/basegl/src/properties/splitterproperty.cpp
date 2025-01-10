/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <modules/basegl/properties/splitterproperty.h>

#include <inviwo/core/properties/boolcompositeproperty.h>    // for BoolCompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>        // for InvalidationLevel, Invalidat...
#include <inviwo/core/properties/optionproperty.h>           // for OptionPropertyOption, Option...
#include <inviwo/core/properties/ordinalproperty.h>          // for ordinalColor, FloatProperty
#include <inviwo/core/properties/propertysemantics.h>        // for PropertySemantics
#include <inviwo/core/util/glmvec.h>                         // for vec4
#include <inviwo/core/util/staticstring.h>                   // for operator+
#include <modules/basegl/datastructures/splittersettings.h>  // for Style, Style::Divider, Style...

namespace inviwo {

std::string_view SplitterProperty::getClassIdentifier() const { return classIdentifier; }

SplitterProperty::SplitterProperty(std::string_view identifier, std::string_view displayName,
                                   bool checked, splitter::Style style, vec4 color, vec4 bgColor,
                                   InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : BoolCompositeProperty(identifier, displayName, checked, invalidationLevel, semantics)
    , style_("style", "Style",
             {{"handle", "Handle", splitter::Style::Handle},
              {"divider", "Divider", splitter::Style::Divider},
              {"line", "Line", splitter::Style::Line},
              {"invisible", "Invisible", splitter::Style::Invisible}})
    , color_("color", "Color", util::ordinalColor(color))
    , bgColor_("bgColor", "Background Color", util::ordinalColor(bgColor))
    , hoverColor_("hoverColor", "Hover Color", util::ordinalColor(1.0f, 0.666f, 0.0f, 1.0f))
    , length_("length", "Length (pixel)", 75.0f, 0.0f, 200.0f, 1.0f,
              InvalidationLevel::InvalidOutput, PropertySemantics("SpinBox"))
    , width_("width", "Width (pixel)", 9.0f, 0.0f, 50.0f, 0.25f, InvalidationLevel::InvalidOutput,
             PropertySemantics("SpinBox"))
    , triSize_("triSize", "Triangle Size", 10.0f, 0.0f, 50.0f, 0.5f,
               InvalidationLevel::InvalidOutput, PropertySemantics("SpinBox")) {

    style_.setSelectedValue(style);
    style_.setCurrentStateAsDefault();

    addProperties(style_, color_, bgColor_, hoverColor_, length_, width_, triSize_);
}

SplitterProperty::SplitterProperty(const SplitterProperty& rhs)
    : BoolCompositeProperty(rhs)
    , style_(rhs.style_)
    , color_(rhs.color_)
    , bgColor_(rhs.bgColor_)
    , hoverColor_(rhs.hoverColor_)
    , length_(rhs.length_)
    , width_(rhs.width_)
    , triSize_(rhs.triSize_) {

    addProperties(style_, color_, bgColor_, hoverColor_, length_, width_, triSize_);
}

SplitterProperty* SplitterProperty::clone() const { return new SplitterProperty(*this); }

bool SplitterProperty::enabled() const { return isChecked(); }

splitter::Style SplitterProperty::getStyle() const { return style_; }

vec4 SplitterProperty::getColor() const { return color_; }

vec4 SplitterProperty::getBackgroundColor() const { return bgColor_; }

vec4 SplitterProperty::getHoverColor() const { return hoverColor_; }

float SplitterProperty::getLength() const { return length_; }

float SplitterProperty::getWidth() const { return width_; }

float SplitterProperty::getTriangleSize() const { return triSize_; }

}  // namespace inviwo
