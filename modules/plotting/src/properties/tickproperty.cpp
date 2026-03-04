/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

namespace inviwo::plot {

std::string_view MajorTickProperty::getClassIdentifier() const { return classIdentifier; }

std::string_view MinorTickProperty::getClassIdentifier() const { return classIdentifier; }

MajorTickProperty::MajorTickProperty(std::string_view identifier, std::string_view displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, "Settings for major ticks along the axis"_help,
                        invalidationLevel, std::move(semantics))
    , style("style", "Style",
            {{"none", "None", TickStyle::None},
             {"inside", "Inside", TickStyle::Inside},
             {"outside", "Outside", TickStyle::Outside},
             {"both", "Both", TickStyle::Both}},
            3)
    , color("color", "Color",
            util::ordinalColor(vec4(0.0f, 0.0f, 0.0f, 1.0f)).set("Color of the ticks"_help))
    , length("tickLength", "Length",
             util::ordinalLength(8.0f, 20.0f).set("Length of the ticks"_help))
    , width("tickWidth", "Width",
            util::ordinalLength(2.5f, 20.0f).set("Line width of the ticks"_help))
    , numberOfTicks(
          "numberOfTicks", "Max Number of Ticks",
          util::ordinalCount(6, 20).setMin(2).set("Maximum number of labels/ticks."_help)) {

    addProperties(style, color, length, width, numberOfTicks);
}

MajorTickProperty::MajorTickProperty(const MajorTickProperty& rhs)
    : CompositeProperty(rhs)
    , style(rhs.style)
    , color(rhs.color)
    , length(rhs.length)
    , width(rhs.width)
    , numberOfTicks(rhs.numberOfTicks) {

    addProperties(style, color, length, width, numberOfTicks);
}

MajorTickProperty* MajorTickProperty::clone() const { return new MajorTickProperty(*this); }

void MajorTickProperty::update(TickData& data) const {
    data.style = style.getSelectedValue();
    data.color = color.get();
    data.length = length.get();
    data.width = width.get();
}

MinorTickProperty::MinorTickProperty(std::string_view identifier, std::string_view displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName,
                        "Settings for minor ticks (shown between major ticks)"_help,
                        invalidationLevel, std::move(semantics))
    , style("style", "Style",
            {{"none", "None", TickStyle::None},
             {"inside", "Inside", TickStyle::Inside},
             {"outside", "Outside", TickStyle::Outside},
             {"both", "Both", TickStyle::Both}},
            2)
    , fillAxis("fillAxis", "Fill Entire Axis",
               "Minor ticks will cover the entire axis range if true, otherwise they will only "
               "appear in between major ticks."_help,
               true)
    , color("color", "Color",
            util::ordinalColor(vec4(0.0f, 0.0f, 0.0f, 1.0f)).set("Color of the ticks"_help))
    , length("tickLength", "Length",
             util::ordinalLength(6.0f, 20.0f).set("Length of the ticks"_help))
    , width("tickWidth", "Width",
            util::ordinalLength(1.5f, 20.0f).set("Line width of the ticks"_help))
    , frequency("tickFrequency_", "Frequency", 2, 1, 20) {

    addProperties(style, fillAxis, color, length, width, frequency);
}

MinorTickProperty::MinorTickProperty(const MinorTickProperty& rhs)
    : CompositeProperty(rhs)
    , style(rhs.style)
    , fillAxis(rhs.fillAxis)
    , color(rhs.color)
    , length(rhs.length)
    , width(rhs.width)
    , frequency(rhs.frequency) {

    addProperties(style, fillAxis, color, length, width, frequency);
}

MinorTickProperty* MinorTickProperty::clone() const { return new MinorTickProperty(*this); }

void MinorTickProperty::update(TickData& data) const {
    data.style = style.getSelectedValue();
    data.color = color.get();
    data.length = length.get();
    data.width = width.get();
}

}  // namespace inviwo::plot
