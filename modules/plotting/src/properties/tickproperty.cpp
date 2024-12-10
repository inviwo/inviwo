/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#include <modules/plotting/properties/tickproperty.h>

#include <inviwo/core/properties/boolproperty.h>                // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>           // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>           // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>              // for OptionPropertyOption, Opt...
#include <inviwo/core/properties/ordinalproperty.h>             // for FloatVec4Property, FloatP...
#include <inviwo/core/properties/propertysemantics.h>           // for PropertySemantics, Proper...
#include <inviwo/core/util/glmvec.h>                            // for vec4, vec3
#include <inviwo/core/util/staticstring.h>                      // for operator+
#include <modules/plotting/datastructures/majorticksettings.h>  // for TickStyle, TickStyle::Both

namespace inviwo {

namespace plot {

std::string_view MajorTickProperty::getClassIdentifier() const { return classIdentifier; }

std::string_view MinorTickProperty::getClassIdentifier() const { return classIdentifier; }

MajorTickProperty::MajorTickProperty(std::string_view identifier, std::string_view displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, "Settings for major ticks along the axis"_help,
                        invalidationLevel, semantics)
    , style_("style", "Style",
             {{"none", "None", TickStyle::None},
              {"inside", "Inside", TickStyle::Inside},
              {"outside", "Outside", TickStyle::Outside},
              {"both", "Both", TickStyle::Both}},
             3)
    , color_("color", "Color",
             util::ordinalColor(vec4(0.0f, 0.0f, 0.0f, 1.0f)).set("Color of the ticks"_help))
    , tickLength_("tickLength", "Length",
                  util::ordinalLength(8.0f, 20.0f).set("Length of the ticks"_help))
    , tickWidth_("tickWidth", "Width",
                 util::ordinalLength(2.5f, 20.0f).set("Line width of the ticks"_help))
    , tickDelta_("tickDelta", "Delta",
                 util::ordinalLength(0.0, 100.0)
                     .set("Spacing between two major ticks. In case the delta is larger than the "
                          "axis range, no major ticks will be shown."_help))
    , rangeBasedTicks_("rangeBasedTicks", "Based on Axis Range",
                       "By default (false), the major ticks appear at n * tickDelta and are "
                       "zero-based. If true, major ticks are start at the minimum axis range "
                       "(min + n * tickDelta)."_help,
                       false) {

    addProperties(style_, color_, tickLength_, tickWidth_, tickDelta_, rangeBasedTicks_);
}

MajorTickProperty::MajorTickProperty(const MajorTickProperty& rhs)
    : CompositeProperty(rhs)
    , style_(rhs.style_)
    , color_(rhs.color_)
    , tickLength_(rhs.tickLength_)
    , tickWidth_(rhs.tickWidth_)
    , tickDelta_(rhs.tickDelta_)
    , rangeBasedTicks_(rhs.rangeBasedTicks_) {

    addProperties(style_, color_, tickLength_, tickWidth_, tickDelta_, rangeBasedTicks_);
}

MajorTickProperty* MajorTickProperty::clone() const { return new MajorTickProperty(*this); }

TickStyle MajorTickProperty::getStyle() const { return style_.getSelectedValue(); }

vec4 MajorTickProperty::getColor() const { return color_.get(); }

float MajorTickProperty::getTickLength() const { return tickLength_.get(); }

float MajorTickProperty::getTickWidth() const { return tickWidth_.get(); }

double MajorTickProperty::getTickDelta() const { return tickDelta_.get(); }

bool MajorTickProperty::getRangeBasedTicks() const { return rangeBasedTicks_.get(); }

MinorTickProperty::MinorTickProperty(std::string_view identifier, std::string_view displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName,
                        "Settings for minor ticks (shown between major ticks)"_help,
                        invalidationLevel, semantics)
    , style_("style", "Style",
             {{"none", "None", TickStyle::None},
              {"inside", "Inside", TickStyle::Inside},
              {"outside", "Outside", TickStyle::Outside},
              {"both", "Both", TickStyle::Both}},
             2)
    , fillAxis_("outsideMajorTicks", "Fill Entire Axis", true)
    , color_("color", "Color",
             util::ordinalColor(vec4(0.0f, 0.0f, 0.0f, 1.0f)).set("Color of the ticks"_help))
    , tickLength_("tickLength", "Length",
                  util::ordinalLength(6.0f, 20.0f).set("Length of the ticks"_help))
    , tickWidth_("tickWidth", "Width",
                 util::ordinalLength(1.5f, 20.0f).set("Line width of the ticks"_help))
    , tickFrequency_("tickFrequency_", "Frequency", 2, 2, 20) {

    addProperties(style_, fillAxis_, color_, tickLength_, tickWidth_, tickFrequency_);
}

MinorTickProperty::MinorTickProperty(const MinorTickProperty& rhs)
    : CompositeProperty(rhs)
    , style_(rhs.style_)
    , fillAxis_(rhs.fillAxis_)
    , color_(rhs.color_)
    , tickLength_(rhs.tickLength_)
    , tickWidth_(rhs.tickWidth_)
    , tickFrequency_(rhs.tickFrequency_) {

    addProperties(style_, fillAxis_, color_, tickLength_, tickWidth_, tickFrequency_);
}

MinorTickProperty* MinorTickProperty::clone() const { return new MinorTickProperty(*this); }

TickStyle MinorTickProperty::getStyle() const { return style_.getSelectedValue(); }

bool MinorTickProperty::getFillAxis() const { return fillAxis_.get(); }

vec4 MinorTickProperty::getColor() const { return color_.get(); }

float MinorTickProperty::getTickLength() const { return tickLength_.get(); }

float MinorTickProperty::getTickWidth() const { return tickWidth_.get(); }

int MinorTickProperty::getTickFrequency() const { return tickFrequency_.get(); }

}  // namespace plot

}  // namespace inviwo
