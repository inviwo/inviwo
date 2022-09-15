/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <modules/plotting/properties/axisproperty.h>

#include <inviwo/core/network/networklock.h>                          // for NetworkLock
#include <inviwo/core/properties/boolcompositeproperty.h>             // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>                      // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>                 // for InvalidationLevel
#include <inviwo/core/properties/minmaxproperty.h>                    // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>                    // for OptionPropertyOption
#include <inviwo/core/properties/ordinalproperty.h>                   // for FloatProperty, Floa...
#include <inviwo/core/properties/property.h>                          // for Property
#include <inviwo/core/properties/propertysemantics.h>                 // for PropertySemantics
#include <inviwo/core/properties/stringproperty.h>                    // for StringProperty
#include <inviwo/core/util/glmvec.h>                                  // for vec2, dvec2, vec4
#include <inviwo/core/util/staticstring.h>                            // for operator+
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>  // for FontFaceOptionProperty
#include <modules/fontrendering/properties/fontproperty.h>            // for FontProperty
#include <modules/fontrendering/util/fontutils.h>                     // for getFont, FontType
#include <modules/plotting/datastructures/axissettings.h>             // for AxisSettings::Orien...
#include <modules/plotting/properties/plottextproperty.h>             // for PlotTextProperty
#include <modules/plotting/properties/tickproperty.h>                 // for MajorTickProperty
#include <modules/plotting/utils/axisutils.h>                         // for getMajorTickPositions

#include <algorithm>                                                  // for transform
#include <cstddef>                                                    // for size_t
#include <iterator>                                                   // for back_insert_iterator

#include <fmt/core.h>                                                 // for basic_string_view
#include <fmt/printf.h>                                               // for sprintf
#include <glm/vec2.hpp>                                               // for vec<>::(anonymous)

namespace inviwo {

namespace plot {
class MajorTickSettings;
class MinorTickSettings;
class PlotTextSettings;

const std::string AxisProperty::classIdentifier = "org.inviwo.AxisProperty";
std::string AxisProperty::getClassIdentifier() const { return classIdentifier; }

AxisProperty::AxisProperty(std::string_view identifier, std::string_view displayName,
                           Orientation orientation, InvalidationLevel invalidationLevel,
                           PropertySemantics semantics)
    : BoolCompositeProperty{identifier, displayName, true, invalidationLevel, semantics}
    , color_{"color",    "Color",     vec4{vec3{0.0f}, 1.0f},           vec4{0.0f},
             vec4{1.0f}, vec4{0.01f}, InvalidationLevel::InvalidOutput, PropertySemantics::Color}
    , width_{"width", "Width", 2.5f, 0.0f, 20.0f}
    , useDataRange_{"useDataRange", "Use Data Range", true}
    , range_("range", "Axis Range", 0.0, 100.0, -1.0e6, 1.0e6, 0.01, 0.0,
             InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , flipped_{"flipped", "Swap Label Position", false}
    , orientation_{"orientation",
                   "Orientation",
                   {{"horizontal", "Horizontal", Orientation::Horizontal},
                    {"vertical", "Vertical", Orientation::Vertical}},
                   orientation == Orientation::Horizontal ? size_t{0} : size_t{1}}
    , placement_{"placement",
                 "Placement",
                 {{"outside", "Bottom / Left", Placement::Outside},
                  {"inside", "Top / Right", Placement::Inside}},
                 0}
    , captionSettings_{"caption", "Caption", false}
    , labelSettings_{"labels", "Axis Labels", true}
    , majorTicks_{"majorTicks", "Major Ticks"}
    , minorTicks_{"minorTicks", "Minor Ticks"} {

    color_.setSemantics(PropertySemantics::Color).setCurrentStateAsDefault();
    range_.readonlyDependsOn(useDataRange_, [](const auto& p) { return p.get(); });

    // change default fonts, make axis labels slightly less pronounced
    captionSettings_.font_.fontFace_.setSelectedIdentifier(font::getFont(font::FontType::Caption));
    labelSettings_.font_.fontFace_.setSelectedIdentifier(font::getFont(font::FontType::Label));

    captionSettings_.title_.set("Axis Title");
    captionSettings_.offset_.set(35.0f);

    labelSettings_.title_.setDisplayName("Format");
    labelSettings_.title_.set("%.1f");
    labelSettings_.position_.setVisible(false);

    captionSettings_.setCollapsed(true);
    labelSettings_.setCollapsed(true);
    majorTicks_.setCollapsed(true);
    minorTicks_.setCollapsed(true);

    addProperties(color_, width_, useDataRange_, range_, flipped_, orientation_, placement_,
                  captionSettings_, labelSettings_, majorTicks_, minorTicks_);

    setCollapsed(true);

    orientation_.onChange([this]() { adjustAlignment(); });
    placement_.onChange([this]() { adjustAlignment(); });
    majorTicks_.onChange([this]() { updateLabels(); });
    range_.onChange([this]() { updateLabels(); });
    labelSettings_.title_.onChange([this]() { updateLabels(); });
    // update label alignment to match current status
    adjustAlignment();
    updateLabels();

    setCurrentStateAsDefault();
}

AxisProperty::AxisProperty(const AxisProperty& rhs)
    : BoolCompositeProperty{rhs}
    , color_{rhs.color_}
    , width_{rhs.width_}
    , useDataRange_{rhs.useDataRange_}
    , range_{rhs.range_}
    , flipped_{rhs.flipped_}
    , orientation_{rhs.orientation_}
    , placement_{rhs.placement_}
    , captionSettings_{rhs.captionSettings_}
    , labelSettings_{rhs.labelSettings_}
    , majorTicks_{rhs.majorTicks_}
    , minorTicks_{rhs.minorTicks_} {

    addProperties(color_, width_, useDataRange_, range_, flipped_, orientation_, placement_);

    range_.setReadOnly(useDataRange_.get());
    useDataRange_.onChange([this]() { range_.setReadOnly(useDataRange_.get()); });

    addProperties(captionSettings_, labelSettings_, majorTicks_, minorTicks_);

    orientation_.onChange([this]() { adjustAlignment(); });
    placement_.onChange([this]() { adjustAlignment(); });
    majorTicks_.onChange([this]() { updateLabels(); });
    range_.onChange([this]() { updateLabels(); });
    labelSettings_.title_.onChange([this]() { updateLabels(); });
    // update label alignment to match current status
    adjustAlignment();
    updateLabels();
}

AxisProperty* AxisProperty::clone() const { return new AxisProperty(*this); }

AxisProperty& AxisProperty::setCaption(std::string_view title) {
    captionSettings_.title_.set(title);
    return *this;
}

const std::string& AxisProperty::getCaption() const { return captionSettings_.title_.get(); }

AxisProperty& AxisProperty::setLabelFormat(std::string_view formatStr) {
    labelSettings_.title_.set(formatStr);
    return *this;
}

AxisProperty& AxisProperty::setRange(const dvec2& range) {
    NetworkLock lock(&range_);
    if (range_.getRangeMin() > range.x) {
        range_.setRangeMin(range.x);
    }
    if (range_.getRangeMax() < range.y) {
        range_.setRangeMax(range.y);
    }
    if (useDataRange_) {
        range_.set(range);
    }
    return *this;
}

AxisProperty& AxisProperty::setColor(const vec4& c) {
    color_.set(c);
    captionSettings_.color_.set(c);
    labelSettings_.color_.set(c);
    majorTicks_.color_.set(c);
    minorTicks_.color_.set(c);

    return *this;
}

AxisProperty& AxisProperty::setFontFace(std::string_view fontFace) {
    captionSettings_.font_.fontFace_.set(fontFace);
    labelSettings_.font_.fontFace_.set(fontFace);
    return *this;
}

AxisProperty& AxisProperty::setFontSize(int fontsize) {
    captionSettings_.font_.fontSize_.set(fontsize);
    labelSettings_.font_.fontSize_.set(fontsize);
    return *this;
}

AxisProperty& AxisProperty::setTickLength(float major, float minor) {
    majorTicks_.tickLength_.set(major);
    minorTicks_.tickLength_.set(minor);
    return *this;
}

AxisProperty& AxisProperty::setLineWidth(float width) {
    width_.set(width);
    majorTicks_.tickWidth_.set(width);
    minorTicks_.tickWidth_.set(width * 0.66667f);
    return *this;
}

void AxisProperty::updateLabels() {
    const auto tickmarks = plot::getMajorTickPositions(majorTicks_, range_);
    categories_.clear();
    const auto& format = labelSettings_.title_.get();
    std::transform(tickmarks.begin(), tickmarks.end(), std::back_inserter(categories_),
                   [&](auto tick) { return fmt::sprintf(format, tick); });
}

void AxisProperty::adjustAlignment() {
    vec2 labelAnchor = [this]() {
        if (orientation_.get() == Orientation::Horizontal) {
            return vec2(0.0f, (placement_.get() == Placement::Outside) ? 1.0 : -1.0);
        } else {
            return vec2((placement_.get() == Placement::Outside) ? 1.0 : -1.0, 0.0f);
        }
    }();
    vec2 captionAnchor = [this]() {
        vec2 anchor = vec2(0.0f, (placement_.get() == Placement::Outside) ? 1.0 : -1.0);
        if (orientation_.get() == Orientation::Vertical) {
            anchor.y = -anchor.y;
        }
        return anchor;
    }();

    labelSettings_.font_.anchorPos_.set(labelAnchor);
    captionSettings_.font_.anchorPos_.set(captionAnchor);
}

bool AxisProperty::getAxisVisible() const { return isChecked(); }

bool AxisProperty::getFlipped() const { return flipped_.get(); }

vec4 AxisProperty::getColor() const { return color_.get(); }

float AxisProperty::getWidth() const { return width_.get(); }

bool AxisProperty::getUseDataRange() const { return useDataRange_.get(); }

dvec2 AxisProperty::getRange() const { return range_.get(); }

AxisSettings::Orientation AxisProperty::getOrientation() const {
    return orientation_.getSelectedValue();
}

AxisSettings::Placement AxisProperty::getPlacement() const { return placement_.getSelectedValue(); }

const PlotTextSettings& AxisProperty::getCaptionSettings() const { return captionSettings_; }

const std::vector<std::string>& AxisProperty::getLabels() const { return categories_; }

const PlotTextSettings& AxisProperty::getLabelSettings() const { return labelSettings_; }

const MajorTickSettings& AxisProperty::getMajorTicks() const { return majorTicks_; }

const MinorTickSettings& AxisProperty::getMinorTicks() const { return minorTicks_; }

}  // namespace plot

}  // namespace inviwo
