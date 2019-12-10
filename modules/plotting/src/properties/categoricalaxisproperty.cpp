/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/plotting/properties/categoricalaxisproperty.h>

#include <inviwo/core/network/networklock.h>

#include <modules/plotting/utils/axisutils.h>

namespace inviwo {

namespace plot {

const std::string CategoricalAxisProperty::classIdentifier = "org.inviwo.CategoricalAxisProperty";
std::string CategoricalAxisProperty::getClassIdentifier() const { return classIdentifier; }

CategoricalAxisProperty::CategoricalAxisProperty(const std::string& identifier,
                                                 const std::string& displayName,
                                                 std::vector<std::string> categories,
                                                 Orientation orientation,
                                                 InvalidationLevel invalidationLevel,
                                                 PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , visible_{"visible", "Visible", true}
    , color_{"color",    "Color",     vec4{vec3{0.0f}, 1.0f},           vec4{0.0f},
             vec4{1.0f}, vec4{0.01f}, InvalidationLevel::InvalidOutput, PropertySemantics::Color}
    , width_{"width", "Width", 2.5f, 0.0f, 20.0f}
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
    , majorTicks_{"majorTicks", "Major Ticks"} {

    color_.setSemantics(PropertySemantics::Color);

    addProperty(visible_);
    addProperty(color_);
    addProperty(width_);
    addProperty(flipped_);
    addProperty(orientation_);
    addProperty(placement_);

    // change default fonts, make axis labels slightly less pronounced
    captionSettings_.font_.fontFace_.setSelectedIdentifier("Montserrat-Regular");
    labelSettings_.font_.fontFace_.setSelectedIdentifier("Montserrat-Light");

    captionSettings_.title_.set("Axis Title");
    captionSettings_.offset_.set(35.0f);

    labelSettings_.title_.setVisible(false);
    labelSettings_.position_.setVisible(false);
    labelSettings_.setCurrentStateAsDefault();

    addProperty(captionSettings_);
    addProperty(labelSettings_);
    addProperty(majorTicks_);

    captionSettings_.setCollapsed(true);
    labelSettings_.setCollapsed(true);

    setCollapsed(true);

    orientation_.onChange([this]() { adjustAlignment(); });
    placement_.onChange([this]() { adjustAlignment(); });

    // update label alignment to match current status
    adjustAlignment();

    setCategories(std::move(categories));

    minorTicks_.style = TickStyle::None;
    majorTicks_.tickDelta_.set(1.0);
    majorTicks_.tickDelta_.setReadOnly(true);
}

CategoricalAxisProperty::CategoricalAxisProperty(const CategoricalAxisProperty& rhs)
    : CompositeProperty{rhs}
    , visible_{rhs.visible_}
    , color_{rhs.color_}
    , width_{rhs.width_}
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
    addProperty(flipped_);
    addProperty(orientation_);
    addProperty(placement_);

    addProperty(captionSettings_);
    addProperty(labelSettings_);
    addProperty(majorTicks_);

    orientation_.onChange([this]() { adjustAlignment(); });
    placement_.onChange([this]() { adjustAlignment(); });

    // update label alignment to match current status
    adjustAlignment();
}

CategoricalAxisProperty* CategoricalAxisProperty::clone() const {
    return new CategoricalAxisProperty(*this);
}

const std::vector<std::string>& CategoricalAxisProperty::getCategories() const {
    return categories_;
}

void CategoricalAxisProperty::setCategories(std::vector<std::string> categories) {
    categories_ = std::move(categories);
}

const std::string& CategoricalAxisProperty::getCaption() const {
    return captionSettings_.title_.get();
}

void CategoricalAxisProperty::adjustAlignment() {
    vec2 anchor;
    if (orientation_.get() == Orientation::Horizontal) {
        // horizontal axis, center labels
        anchor = vec2(0.0f, (placement_.get() == Placement::Outside) ? 1.0 : -1.0);
    } else {
        // vertical axis
        anchor = vec2((placement_.get() == Placement::Outside) ? 1.0 : -1.0, 0.0f);
    }
    labelSettings_.font_.anchorPos_.set(anchor);
    captionSettings_.font_.anchorPos_.set(anchor);
}

bool CategoricalAxisProperty::getAxisVisible() const { return visible_.get(); }

bool CategoricalAxisProperty::getFlipped() const { return flipped_.get(); }

vec4 CategoricalAxisProperty::getColor() const { return color_.get(); }

float CategoricalAxisProperty::getWidth() const { return width_.get(); }

bool CategoricalAxisProperty::getUseDataRange() const { return false; }

dvec2 CategoricalAxisProperty::getRange() const {
    return {0, static_cast<double>(categories_.size()) - 1.0};
}

AxisSettings::Orientation CategoricalAxisProperty::getOrientation() const {
    return orientation_.getSelectedValue();
}

AxisSettings::Placement CategoricalAxisProperty::getPlacement() const {
    return placement_.getSelectedValue();
}

const PlotTextSettings& CategoricalAxisProperty::getCaptionSettings() const {
    return captionSettings_;
}

const std::vector<std::string>& CategoricalAxisProperty::getLabels() const { return categories_; }

const PlotTextSettings& CategoricalAxisProperty::getLabelSettings() const { return labelSettings_; }

const MajorTickSettings& CategoricalAxisProperty::getMajorTicks() const { return majorTicks_; }

const MinorTickSettings& CategoricalAxisProperty::getMinorTicks() const { return minorTicks_; }

}  // namespace plot

}  // namespace inviwo
