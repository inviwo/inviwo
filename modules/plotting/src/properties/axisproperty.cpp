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

#include <modules/plotting/properties/axisproperty.h>

#include <inviwo/core/network/networklock.h>

#include <modules/plotting/utils/axisutils.h>

#include <fmt/format.h>
#include <fmt/printf.h>

namespace inviwo {

namespace plot {

const std::string AxisProperty::classIdentifier = "org.inviwo.AxisProperty";
std::string AxisProperty::getClassIdentifier() const { return classIdentifier; }

AxisProperty::AxisProperty(const std::string& identifier, const std::string& displayName,
                           Orientation orientation, InvalidationLevel invalidationLevel,
                           PropertySemantics semantics)
    : CompositeProperty{identifier, displayName, invalidationLevel, semantics}
    , visible_{"visible", "Visible", true}
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

    color_.setSemantics(PropertySemantics::Color);

    addProperty(visible_);
    addProperty(color_);
    addProperty(width_);
    addProperty(useDataRange_);
    addProperty(range_);
    addProperty(flipped_);
    addProperty(orientation_);
    addProperty(placement_);

    range_.setReadOnly(useDataRange_.get());
    useDataRange_.onChange([this]() { range_.setReadOnly(useDataRange_.get()); });

    // change default fonts, make axis labels slightly less pronounced
    captionSettings_.font_.fontFace_.setSelectedIdentifier("Montserrat-Regular");
    labelSettings_.font_.fontFace_.setSelectedIdentifier("Montserrat-Light");

    captionSettings_.title_.set("Axis Title");
    captionSettings_.offset_.set(35.0f);

    labelSettings_.title_.setDisplayName("Format");
    labelSettings_.title_.set("%.1f");
    labelSettings_.position_.setVisible(false);
    labelSettings_.setCurrentStateAsDefault();

    addProperty(captionSettings_);
    addProperty(labelSettings_);
    addProperty(majorTicks_);
    addProperty(minorTicks_);

    captionSettings_.setCollapsed(true);
    labelSettings_.setCollapsed(true);

    majorTicks_.setCollapsed(true);
    minorTicks_.setCollapsed(true);

    setCollapsed(true);

    orientation_.onChange([this]() { adjustAlignment(); });
    placement_.onChange([this]() { adjustAlignment(); });
    majorTicks_.onChange([this]() { updateLabels(); });
    range_.onChange([this]() { updateLabels(); });
    labelSettings_.title_.onChange([this]() { updateLabels(); });
    // update label alignment to match current status
    adjustAlignment();
    updateLabels();
}

AxisProperty::AxisProperty(const AxisProperty& rhs)
    : CompositeProperty{rhs}
    , visible_{rhs.visible_}
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

    addProperty(visible_);
    addProperty(color_);
    addProperty(width_);
    addProperty(useDataRange_);
    addProperty(range_);
    addProperty(flipped_);
    addProperty(orientation_);
    addProperty(placement_);

    range_.setReadOnly(useDataRange_.get());
    useDataRange_.onChange([this]() { range_.setReadOnly(useDataRange_.get()); });

    addProperty(captionSettings_);
    addProperty(labelSettings_);
    addProperty(majorTicks_);
    addProperty(minorTicks_);

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

void AxisProperty::setCaption(const std::string& title) { captionSettings_.title_.set(title); }

const std::string& AxisProperty::getCaption() const { return captionSettings_.title_.get(); }

void AxisProperty::setLabelFormat(const std::string& formatStr) {
    labelSettings_.title_.set(formatStr);
}

void AxisProperty::setRange(const dvec2& range) {
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
}

void AxisProperty::setColor(const vec4& c) {
    color_.set(c);
    captionSettings_.color_.set(c);
    labelSettings_.color_.set(c);
    majorTicks_.color_.set(c);
    minorTicks_.color_.set(c);
}

void AxisProperty::setFontFace(const std::string& fontFace) {
    captionSettings_.font_.fontFace_.set(fontFace);
    labelSettings_.font_.fontFace_.set(fontFace);
}

void AxisProperty::setFontSize(int fontsize) {
    captionSettings_.font_.fontSize_.set(fontsize);
    labelSettings_.font_.fontSize_.set(fontsize);
}

void AxisProperty::setTickLength(float major, float minor) {
    majorTicks_.tickLength_.set(major);
    minorTicks_.tickLength_.set(minor);
}

void AxisProperty::setLineWidth(float width) {
    width_.set(width);
    majorTicks_.tickWidth_.set(width);
    minorTicks_.tickWidth_.set(width * 0.66667f);
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

bool AxisProperty::getAxisVisible() const { return visible_.get(); }

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
