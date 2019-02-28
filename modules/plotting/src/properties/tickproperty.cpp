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

#include <modules/plotting/properties/tickproperty.h>

namespace inviwo {

namespace plot {

const std::string TickProperty::classIdentifier = "org.inviwo.TickProperty";
std::string TickProperty::getClassIdentifier() const { return classIdentifier; }

const std::string MajorTickProperty::classIdentifier = "org.inviwo.MajorTickProperty";
std::string MajorTickProperty::getClassIdentifier() const { return classIdentifier; }

const std::string MinorTickProperty::classIdentifier = "org.inviwo.MinorTickProperty";
std::string MinorTickProperty::getClassIdentifier() const { return classIdentifier; }

MajorTickProperty::MajorTickProperty(const std::string& identifier, const std::string& displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , style_("style", "Style",
             {{"none", "None", TickStyle::None},
              {"inside", "Inside", TickStyle::Inside},
              {"outside", "Outside", TickStyle::Outside},
              {"both", "Both", TickStyle::Both}},
             3)
    , color_("color", "Color", vec4(vec3(0.0f), 1.0f), vec4(0.0f), vec4(1.0f))
    , tickLength_("tickLength", "Length", 8.0f, 0.0f, 20.0f)
    , tickWidth_("tickWidth", "Width", 2.5f, 0.0f, 20.0f)
    , tickDelta_("tickDelta", "Delta", 0.0f, 0.0f, 100.0f, 0.1f)
    , rangeBasedTicks_("rangeBasedTicks", "Based on Axis Range", false) {
    color_.setSemantics(PropertySemantics::Color);
    addProperty(style_);
    addProperty(color_);
    addProperty(tickLength_);
    addProperty(tickWidth_);
    addProperty(tickDelta_);
    addProperty(rangeBasedTicks_);
}

MajorTickProperty::MajorTickProperty(const MajorTickProperty& rhs)
    : CompositeProperty(rhs)
    , style_(rhs.style_)
    , color_(rhs.color_)
    , tickLength_(rhs.tickLength_)
    , tickWidth_(rhs.tickWidth_)
    , tickDelta_(rhs.tickDelta_)
    , rangeBasedTicks_(rhs.rangeBasedTicks_) {
    color_.setSemantics(PropertySemantics::Color);
    addProperty(style_);
    addProperty(color_);
    addProperty(tickLength_);
    addProperty(tickWidth_);
    addProperty(tickDelta_);
    addProperty(rangeBasedTicks_);
}

MajorTickProperty* MajorTickProperty::clone() const { return new MajorTickProperty(*this); }

TickStyle MajorTickProperty::getStyle() const { return style_.getSelectedValue(); }

vec4 MajorTickProperty::getColor() const { return color_.get(); }

float MajorTickProperty::getTickLength() const { return tickLength_.get(); }

float MajorTickProperty::getTickWidth() const { return tickWidth_.get(); }

double MajorTickProperty::getTickDelta() const { return tickDelta_.get(); }

bool MajorTickProperty::getRangeBasedTicks() const { return rangeBasedTicks_.get(); }

MinorTickProperty::MinorTickProperty(const std::string& identifier, const std::string& displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , style_("style", "Style",
             {{"none", "None", TickStyle::None},
              {"inside", "Inside", TickStyle::Inside},
              {"outside", "Outside", TickStyle::Outside},
              {"both", "Both", TickStyle::Both}},
             2)
    , fillAxis_("outsideMajorTicks", "Fill Entire Axis", true)
    , color_("color", "Color", vec4(vec3(0.0f), 1.0f), vec4(0.0f), vec4(1.0f))
    , tickLength_("tickLength", "Length", 6.0f, 0.0f, 20.0f)
    , tickWidth_("tickWidth", "Width", 1.5f, 0.0f, 20.0f)
    , tickFrequency_("tickFrequency_", "Frequency", 2, 2, 20) {
    color_.setSemantics(PropertySemantics::Color);
    addProperty(style_);
    addProperty(fillAxis_);
    addProperty(color_);
    addProperty(tickLength_);
    addProperty(tickWidth_);
    addProperty(tickFrequency_);
}

MinorTickProperty::MinorTickProperty(const MinorTickProperty& rhs)
    : CompositeProperty(rhs)
    , style_(rhs.style_)
    , fillAxis_(rhs.fillAxis_)
    , color_(rhs.color_)
    , tickLength_(rhs.tickLength_)
    , tickWidth_(rhs.tickWidth_)
    , tickFrequency_(rhs.tickFrequency_) {
    color_.setSemantics(PropertySemantics::Color);
    addProperty(style_);
    addProperty(fillAxis_);
    addProperty(color_);
    addProperty(tickLength_);
    addProperty(tickWidth_);
    addProperty(tickFrequency_);
}

MinorTickProperty* MinorTickProperty::clone() const { return new MinorTickProperty(*this); }

TickStyle MinorTickProperty::getStyle() const { return style_.getSelectedValue(); }

bool MinorTickProperty::getFillAxis() const { return fillAxis_.get(); }

vec4 MinorTickProperty::getColor() const { return color_.get(); }

float MinorTickProperty::getTickLength() const { return tickLength_.get(); }

float MinorTickProperty::getTickWidth() const { return tickWidth_.get(); }

int MinorTickProperty::getTickFrequency() const { return tickFrequency_.get(); }

TickProperty::TickProperty(const std::string& identifier, const std::string& displayName,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , majorTicks_("majorTicks", "Major Ticks")
    , minorTicks_("minorTicks", "Minor Ticks") {
    addProperty(majorTicks_);
    addProperty(minorTicks_);

    majorTicks_.setCollapsed(true);
    minorTicks_.setCollapsed(true);
}

TickProperty::TickProperty(const TickProperty& rhs)
    : CompositeProperty(rhs), majorTicks_(rhs.majorTicks_), minorTicks_(rhs.minorTicks_) {
    addProperty(majorTicks_);
    addProperty(minorTicks_);

    majorTicks_.setCollapsed(true);
    minorTicks_.setCollapsed(true);
}

TickProperty* TickProperty::clone() const { return new TickProperty(*this); }

}  // namespace plot

}  // namespace inviwo
