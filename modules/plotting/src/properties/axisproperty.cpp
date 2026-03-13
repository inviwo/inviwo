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

#include <modules/plotting/properties/axisproperty.h>

#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/fontrendering/util/fontutils.h>
#include <modules/plotting/algorithm/labeling.h>
#include <modules/plotting/utils/axisutils.h>

#include <inviwo/core/interaction/events/keyboardevent.h>

#include <algorithm>
#include <limits>

namespace inviwo::plot {

class MajorTickSettings;
class MinorTickSettings;
class PlotTextSettings;

namespace {

std::vector<ButtonGroupProperty::Button> buttons(AxisProperty& axisProperty) {

    const auto align = [&](std::string_view icon, std::string_view text, float p,
                           int axis) -> ButtonGroupProperty::Button {
        return {.icon = fmt::format(":svgicons/{}", icon),
                .tooltip = std::string{text},
                .action = [ap = &axisProperty, p, axis]() {
                    ap->labelSettings_.font_.anchorPos_.set(p, axis);
                    ap->captionSettings_.font_.anchorPos_.set(p, axis);
                }};
    };

    return {align("axis-labels-left.svg", "Align labels left", -1.0f, 0),
            align("axis-labels-center.svg", "Center labels", 0.0f, 0),
            align("axis-labels-right.svg", "Align labels right", 1.0f, 0),
            align("axis-labels-top.svg", "Align labels at the top", 1.0f, 1),
            align("axis-labels-middle.svg", "Align labels in the middle", 0.0f, 1),
            align("axis-labels-bottom.svg", "Align labels at the bottom", -1.0f, 1),
            {.name = "A+",
             .tooltip = "Increase font size (Ctrl+Plus)",
             .action =
                 [ap = &axisProperty]() {
                     ap->captionSettings_.font_.fontSize_.set(
                         std::min(1000, ap->captionSettings_.font_.fontSize_.get() + 1));
                     ap->labelSettings_.font_.fontSize_.set(
                         std::min(1000, ap->labelSettings_.font_.fontSize_.get() + 1));
                 }},
            {.name = "A-",
             .tooltip = "Decrease font size (Ctrl+Minus)",
             .action = [ap = &axisProperty]() {
                 ap->captionSettings_.font_.fontSize_.set(
                     std::max(0, ap->captionSettings_.font_.fontSize_.get() - 1));
                 ap->labelSettings_.font_.fontSize_.set(
                     std::max(0, ap->labelSettings_.font_.fontSize_.get() - 1));
             }}};
}

}  // namespace

std::string_view AxisProperty::getClassIdentifier() const { return classIdentifier; }

AxisProperty::AxisProperty(std::string_view identifier, std::string_view displayName, Document help,
                           Orientation orientation, InvalidationLevel invalidationLevel,
                           PropertySemantics semantics)
    : BoolCompositeProperty{identifier, displayName,       std::move(help),
                            true,       invalidationLevel, std::move(semantics)}
    , color_{"color", "Color",
             util::ordinalColor(vec4{0.0f, 0.0f, 0.0f, 1.0f}).set("Color of the axis"_help)}
    , width_{"width", "Width", util::ordinalLength(2.5f, 20.0f).set("Line width of the axis"_help)}
    , overrideRange_{"overrideRange", "Override Axis Range", false}
    , range_{"range",
             "Axis Range",
             "Value range of the axis"_help,
             dvec2{0.0, 100.0},
             {dvec2{std::numeric_limits<double>::lowest()}, ConstraintBehavior::Ignore},
             {dvec2{std::numeric_limits<double>::max()}, ConstraintBehavior::Ignore},
             dvec2{0.001},
             InvalidationLevel::InvalidOutput,
             PropertySemantics::Text}
    , customRange_{"customRange",
                   "Custom Range",
                   "Custom range overriding the range of the axis"_help,
                   dvec2{0.0, 100.0},
                   {dvec2{std::numeric_limits<double>::lowest()}, ConstraintBehavior::Ignore},
                   {dvec2{std::numeric_limits<double>::max()}, ConstraintBehavior::Ignore},
                   dvec2{0.001},
                   InvalidationLevel::InvalidOutput,
                   PropertySemantics::Text}
    , mirrored_{"mirrored", "Mirror ",
                "Swaps the inside and outside of the axis. If not mirrored, the outside will be to "
                "the right of the axis pointing from start point to end point."_help,
                orientation == Orientation::Vertical}
    , labelingAlgorithm_{"labeling",
                         "Labeling Algorithm",
                         {{"heckbert", "Heckbert", LabelingAlgorithm::Heckbert},
                          {"matplotlib", "Matplotlib", LabelingAlgorithm::Matplotlib},
                          {"extentedWilkinson", "Ext. Wilkinson",
                           LabelingAlgorithm::ExtendedWilkinson},
                          {"limits", "Limits only", LabelingAlgorithm::Limits},
                          {"limitsAndZero", "Limits And Zero", LabelingAlgorithm::LimitsAndZero}},
                         1}
    , captionSettings_{"caption", "Caption",
                       "Font and alignment settings for the axis caption"_help, true}
    , labelSettings_{"labels", "Axis Labels",
                     "Settings for axis labels shown next to major ticks"_help, true}
    , majorTicks_{"majorTicks", "Major Ticks"}
    , minorTicks_{"minorTicks", "Minor Ticks"}
    , alignment_{"alignment", "Alignment",
                 "Set axis orientation, label position and the horizontal "
                 "and vertical alignment of both labels and captions."_help,
                 buttons(*this), InvalidationLevel::Valid} {

    range_.setReadOnly(true);

    // change default fonts, make axis labels slightly less pronounced
    captionSettings_.font_.fontFace_.setSelectedIdentifier(
        font::getFont(font::FontType::Caption).string());
    labelSettings_.font_.fontFace_.setSelectedIdentifier(
        font::getFont(font::FontType::Label).string());

    captionSettings_.title_.set("Axis Title");

    labelSettings_.title_.setDisplayName("Format");
    labelSettings_.title_.set(plot::defaultFormat);
    labelSettings_.position_.setVisible(false);

    captionSettings_.setCollapsed(true);
    labelSettings_.setCollapsed(true);
    majorTicks_.setCollapsed(true);
    minorTicks_.setCollapsed(true);

    addProperties(color_, width_, range_, overrideRange_, customRange_, alignment_,
                  mirrored_, labelingAlgorithm_, captionSettings_, labelSettings_, majorTicks_,
                  minorTicks_);

    BoolCompositeProperty::setCollapsed(true);
    set(orientation, mirrored_.get());

    setCurrentStateAsDefault();
}

AxisProperty::AxisProperty(std::string_view identifier, std::string_view displayName,
                           Orientation orientation, InvalidationLevel invalidationLevel,
                           PropertySemantics semantics)
    : AxisProperty{identifier,
                   displayName,
                   "Different settings for an axis including captions, labels, ticks, "
                   "font settings, line widths, colors, and more."_help,
                   orientation,
                   invalidationLevel,
                   std::move(semantics)} {}

AxisProperty::AxisProperty(const AxisProperty& rhs)
    : BoolCompositeProperty{rhs}
    , color_{rhs.color_}
    , width_{rhs.width_}
    , overrideRange_{rhs.overrideRange_}
    , range_{rhs.range_}
    , customRange_{rhs.customRange_}
    , mirrored_{rhs.mirrored_}
    , labelingAlgorithm_{rhs.labelingAlgorithm_}
    , captionSettings_{rhs.captionSettings_}
    , labelSettings_{rhs.labelSettings_}
    , majorTicks_{rhs.majorTicks_}
    , minorTicks_{rhs.minorTicks_}
    , alignment_{rhs.alignment_} {

    addProperties(color_, width_, range_, overrideRange_, customRange_, alignment_, mirrored_,
                  labelingAlgorithm_, captionSettings_, labelSettings_, majorTicks_, minorTicks_);
}

AxisProperty* AxisProperty::clone() const { return new AxisProperty(*this); }

void AxisProperty::set(Orientation orientation, bool mirrored) {
    mirrored_.set(mirrored);
    using enum Orientation;
    captionSettings_.offset_.set(orientation == Vertical ? 50.0f : 35.0f);
    captionSettings_.rotation_.set(0.0f);
    captionSettings_.font_.anchorPos_.set(vec2{0.0f, 1.0f});

    labelSettings_.rotation_.set(orientation == Vertical ? -90.0f : 0.0f);
    labelSettings_.font_.anchorPos_.set(orientation == Vertical ? vec2{-1.0f, 0.0f}
                                                                : vec2{0.0f, 1.0f});
}

AxisProperty& AxisProperty::setCaption(std::string_view title) {
    captionSettings_.title_.set(title);
    return *this;
}

AxisProperty& AxisProperty::setLabelingAlgorithm(LabelingAlgorithm algorithm) {
    labelingAlgorithm_.set(algorithm);
    return *this;
}

AxisProperty& AxisProperty::setNumberOfTicks(int numTicks) {
    majorTicks_.numberOfTicks.set(numTicks);
    return *this;
}

AxisProperty& AxisProperty::setLabelFormat(std::string_view formatStr) {
    labelSettings_.title_.set(formatStr);
    return *this;
}

AxisProperty& AxisProperty::setRange(const dvec2& range) {
    const NetworkLock lock(&range_);
    range_.set(range);

    return *this;
}

AxisProperty& AxisProperty::setColor(const vec4& c) {
    color_.set(c);
    captionSettings_.color_.set(c);
    labelSettings_.color_.set(c);
    majorTicks_.color.set(c);
    minorTicks_.color.set(c);

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
    majorTicks_.length.set(major);
    minorTicks_.length.set(minor);
    return *this;
}

AxisProperty& AxisProperty::setLineWidth(float width) {
    width_.set(width);
    majorTicks_.width.set(width);
    minorTicks_.width.set(width * 0.66667f);
    return *this;
}

dvec2 AxisProperty::getRange() const {
    if (overrideRange_) {
        return customRange_.get();
    } else {
        return range_.get();
    }
}

void AxisProperty::update(AxisData& data) const {
    data.range = getRange();
    data.visible = isChecked();
    data.mirrored = mirrored_.get();
    data.color = color_.get();
    data.width = width_.get();
    data.caption = captionSettings_.title_.get();
    captionSettings_.update(data.captionSettings);

    updateLabelPositions(data.majorPositions, data.minorPositions, labelingAlgorithm_.get(),
                         data.range, majorTicks_.numberOfTicks.get(), minorTicks_.frequency.get(),
                         minorTicks_.fillAxis.get());

    labelSettings_.update(data.labelSettings);
    updateLabels(data.labels, data.majorPositions, labelSettings_.title_.get());

    majorTicks_.update(data.major);
    minorTicks_.update(data.minor);
}

void AxisProperty::invokeEvent(Event* event) {
    if (auto* keyEvent = event->getAs<KeyboardEvent>()) {
        if (keyEvent->key() == IvwKey::Plus && keyEvent->modifiers() == KeyModifier::Control &&
            keyEvent->state() == KeyState::Release) {

            captionSettings_.font_.fontSize_.set(
                std::min(1000, captionSettings_.font_.fontSize_.get() + 1));
            labelSettings_.font_.fontSize_.set(
                std::min(1000, labelSettings_.font_.fontSize_.get() + 1));
        } else if ((keyEvent->key() == IvwKey::Minus || keyEvent->key() == IvwKey::Underscore) &&
                   keyEvent->modifiers() == KeyModifier::Control &&
                   keyEvent->state() == KeyState::Release) {

            captionSettings_.font_.fontSize_.set(
                std::max(0, captionSettings_.font_.fontSize_.get() - 1));
            labelSettings_.font_.fontSize_.set(
                std::max(0, labelSettings_.font_.fontSize_.get() - 1));
        } else if (keyEvent->key() == IvwKey::Num0 &&
                   keyEvent->modifiers() == KeyModifier::Control &&
                   keyEvent->state() == KeyState::Release) {

            captionSettings_.font_.fontSize_.resetToDefaultState();
            labelSettings_.font_.fontSize_.resetToDefaultState();
        }
    }
}

}  // namespace inviwo::plot
