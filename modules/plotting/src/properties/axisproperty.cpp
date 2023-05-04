/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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

#include <algorithm>  // for transform
#include <cstddef>    // for size_t
#include <iterator>   // for back_insert_iterator
#include <limits>

#include <fmt/core.h>    // for basic_string_view
#include <fmt/printf.h>  // for sprintf
#include <glm/vec2.hpp>  // for vec<>::(anonymous)
#include <glm/gtx/vec_swizzle.hpp>

namespace inviwo {

namespace plot {
class MajorTickSettings;
class MinorTickSettings;
class PlotTextSettings;

const std::string AxisProperty::classIdentifier = "org.inviwo.AxisProperty";
std::string AxisProperty::getClassIdentifier() const { return classIdentifier; }

AxisProperty::AxisProperty(std::string_view identifier, std::string_view displayName, Document help,
                           Orientation orientation, bool includeOrientationProperty,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : BoolCompositeProperty{identifier, displayName, help, true, invalidationLevel, semantics}
    , color_{"color", "Color",
             util::ordinalColor(vec4{0.0f, 0.0f, 0.0f, 1.0f}).set("Color of the axis"_help)}
    , width_{"width", "Width", util::ordinalLength(2.5f, 20.0f).set("Line width of the axis"_help)}
    , useDataRange_{"useDataRange", "Use Data Range", true}
    , range_{"range",
             "Axis Range",
             "Value range of the axis"_help,
             0.0,
             100.0,
             -1.0e6,
             1.0e6,
             0.01,
             0.0,
             InvalidationLevel::InvalidOutput,
             PropertySemantics::Text}
    , scalingFactor_{"scalingFactor", "Scaling Factor",
                     util::ordinalScale(1.0f, 10.0f)
                         .set("Scaling factor affecting tick lengths and offsets of axis caption "
                              "and labels"_help)
                         .setMin(std::numeric_limits<float>::min())}
    , mirrored_{"mirrored", "Mirror ",
                "Swaps the inside and outside of the axis. If not mirrored, the outside will be to "
                "the right of the axis pointing from start point to end point."_help,
                orientation == Orientation::Vertical}
    , captionSettings_{"caption", "Caption",
                       "Font and alignment settings for the axis caption"_help, true}
    , labelSettings_{"labels", "Axis Labels",
                     "Settings for axis labels shown next to major ticks"_help, true}
    , majorTicks_{"majorTicks", "Major Ticks"}
    , minorTicks_{"minorTicks", "Minor Ticks"}
    , alignment_{
          "alignment", "Alignment",
          "Set axis orientation, label position and the horizontal and vertical alignment of both labels and captions."_help,
          buttons(includeOrientationProperty), InvalidationLevel::Valid} {

    range_.readonlyDependsOn(useDataRange_, [](const auto& p) { return p.get(); });
    scalingFactor_.setVisible(false);

    // change default fonts, make axis labels slightly less pronounced
    captionSettings_.font_.fontFace_.setSelectedIdentifier(
        font::getFont(font::FontType::Caption).string());
    labelSettings_.font_.fontFace_.setSelectedIdentifier(
        font::getFont(font::FontType::Label).string());

    captionSettings_.title_.set("Axis Title");

    labelSettings_.title_.setDisplayName("Format");
    labelSettings_.title_.set("%.1f");
    labelSettings_.position_.setVisible(false);

    captionSettings_.setCollapsed(true);
    labelSettings_.setCollapsed(true);
    majorTicks_.setCollapsed(true);
    minorTicks_.setCollapsed(true);

    addProperties(color_, width_, useDataRange_, range_, alignment_, scalingFactor_, mirrored_,
                  captionSettings_, labelSettings_, majorTicks_, minorTicks_);

    if (includeOrientationProperty) {
        orientation_.emplace(std::string_view{"orientation"}, std::string_view{"Orientation"},
                             "Determines the orientation of the axis (horizontal or vertical)"_help,
                             std::vector<OptionPropertyOption<Orientation>>{
                                 {"horizontal", "Horizontal", Orientation::Horizontal},
                                 {"vertical", "Vertical", Orientation::Vertical}},
                             orientation == Orientation::Horizontal ? size_t{0} : size_t{1});
        insertProperty(7, *orientation_);
    }

    setCollapsed(true);
    defaultAlignLabels();

    setCurrentStateAsDefault();

    majorTicks_.onChange([this]() { updateLabels(); });
    range_.onChange([this]() { updateLabels(); });
    labelSettings_.title_.onChange([this]() { updateLabels(); });
    // update label alignment to match current status
    updateLabels();
}

AxisProperty::AxisProperty(std::string_view identifier, std::string_view displayName,
                           Orientation orientation, bool includeOrientationProperty,
                           InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : AxisProperty{identifier,
                   displayName,
                   "Different settings for an axis including captions, labels, ticks, "
                   "font settings, line widths, colors, and more."_help,
                   orientation,
                   includeOrientationProperty,
                   invalidationLevel,
                   semantics} {}

AxisProperty::AxisProperty(const AxisProperty& rhs)
    : BoolCompositeProperty{rhs}
    , color_{rhs.color_}
    , width_{rhs.width_}
    , useDataRange_{rhs.useDataRange_}
    , range_{rhs.range_}
    , scalingFactor_{rhs.scalingFactor_}
    , mirrored_{rhs.mirrored_}
    , orientation_{rhs.orientation_}
    , captionSettings_{rhs.captionSettings_}
    , labelSettings_{rhs.labelSettings_}
    , majorTicks_{rhs.majorTicks_}
    , minorTicks_{rhs.minorTicks_}
    , alignment_{rhs.alignment_} {

    addProperties(color_, width_, useDataRange_, range_, alignment_, scalingFactor_, mirrored_,
                  captionSettings_, labelSettings_, majorTicks_, minorTicks_);

    // insert orientation property only if rhs owns one, too
    if (orientation_) {
        insertProperty(7, *orientation_);
    }

    range_.readonlyDependsOn(useDataRange_, [](const auto& p) { return p.get(); });

    majorTicks_.onChange([this]() { updateLabels(); });
    range_.onChange([this]() { updateLabels(); });
    labelSettings_.title_.onChange([this]() { updateLabels(); });
    // update label alignment to match current status
    updateLabels();
}

AxisProperty* AxisProperty::clone() const { return new AxisProperty(*this); }

void AxisProperty::defaultAlignLabels() {
    captionSettings_.offset_.set(orientation_ == Orientation::Vertical ? 50.0f : 35.0f);
    captionSettings_.rotation_.set(orientation_ == Orientation::Vertical ? 90.0f : 0.0f);

    const vec2 anchor = [&]() {
        if (orientation_ == Orientation::Vertical) {
            return (mirrored_ ? vec2{1.0f, 0.0f} : vec2{-1.0f, 0.0f});
        } else {
            return (mirrored_ ? vec2{0.0f, -1.0f} : vec2{0.0f, 1.0f});
        }
    }();
    captionSettings_.font_.anchorPos_.set(anchor);
    labelSettings_.font_.anchorPos_.set(anchor);
}

void AxisProperty::set(Orientation orientation, bool mirrored) {
    if (orientation_) {
        orientation_->set(orientation);
    }
    mirrored_.set(mirrored);
    defaultAlignLabels();
}

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

AxisProperty& AxisProperty::setFontFace(const std::filesystem::path& fontFace) {
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

bool AxisProperty::getAxisVisible() const { return isChecked(); }

bool AxisProperty::getMirrored() const { return mirrored_.get(); }

vec4 AxisProperty::getColor() const { return color_.get(); }

float AxisProperty::getWidth() const { return width_.get(); }

float AxisProperty::getScalingFactor() const { return scalingFactor_.get(); }

bool AxisProperty::getUseDataRange() const { return useDataRange_.get(); }

dvec2 AxisProperty::getRange() const { return range_.get(); }

AxisSettings::Orientation AxisProperty::getOrientation() const {
    if (orientation_) {
        return orientation_->getSelectedValue();
    } else {
        return Orientation::Horizontal;
    }
}

const PlotTextSettings& AxisProperty::getCaptionSettings() const { return captionSettings_; }

const std::vector<std::string>& AxisProperty::getLabels() const { return categories_; }

const PlotTextSettings& AxisProperty::getLabelSettings() const { return labelSettings_; }

const MajorTickSettings& AxisProperty::getMajorTicks() const { return majorTicks_; }

const MinorTickSettings& AxisProperty::getMinorTicks() const { return minorTicks_; }

std::vector<ButtonGroupProperty::Button> AxisProperty::buttons(bool hasOrientation) {
    auto createOrientationButton = [&](std::string_view icon, std::string_view text, Orientation o,
                                       const vec2 anchor,
                                       bool mirrored) -> ButtonGroupProperty::Button {
        return {std::nullopt, std::string{icon}, std::string{text}, [this, o, anchor, mirrored]() {
                    if (orientation_) {
                        orientation_->set(o);
                    }
                    mirrored_.set(mirrored);
                    labelSettings_.font_.anchorPos_.set(anchor);
                    captionSettings_.font_.anchorPos_.set(anchor);
                }};
    };
    auto createAlignmentButton = [&](std::string_view icon, std::string_view text, float p,
                                     int axis) -> ButtonGroupProperty::Button {
        return {std::nullopt, std::string{icon}, std::string{text}, [this, p, axis]() {
                    labelSettings_.font_.anchorPos_.set(p, axis);
                    captionSettings_.font_.anchorPos_.set(p, axis);
                }};
    };

    std::vector<ButtonGroupProperty::Button> btns{{
        createOrientationButton(":svgicons/axis-horizontal-bottom.svg",
                                "Horizontal axis with labels below", Orientation::Horizontal,
                                vec2{0.0f, 1.0f}, false),
        createOrientationButton(":svgicons/axis-horizontal-top.svg",
                                "Horizontal axis with labels above", Orientation::Horizontal,
                                vec2{0.0f, -1.0f}, true),
    }};

    if (hasOrientation) {
        btns.push_back(createOrientationButton(":svgicons/axis-vertical-left.svg",
                                               "Vertical axis with labels on the left",
                                               Orientation::Vertical, vec2{1.0f, 0.0f}, true));
        btns.push_back(createOrientationButton(":svgicons/axis-vertical-right.svg",
                                               "Vertical axis with labels on the right",
                                               Orientation::Vertical, vec2{-1.0f, 0.0f}, false));
    }
    btns.push_back(
        createAlignmentButton(":svgicons/axis-labels-left.svg", "Align labels left", -1.0f, 0));
    btns.push_back(
        createAlignmentButton(":svgicons/axis-labels-center.svg", "Center labels", 0.0f, 0));
    btns.push_back(
        createAlignmentButton(":svgicons/axis-labels-right.svg", "Align labels right", 1.0f, 0));
    btns.push_back(
        createAlignmentButton(":svgicons/axis-labels-top.svg", "Align labels at the top", 1.0f, 1));
    btns.push_back(createAlignmentButton(":svgicons/axis-labels-middle.svg",
                                         "Align labels in the middle", 0.0f, 1));
    btns.push_back(createAlignmentButton(":svgicons/axis-labels-bottom.svg",
                                         "Align labels at the bottom", -1.0f, 1));
    return btns;
}

}  // namespace plot

}  // namespace inviwo
