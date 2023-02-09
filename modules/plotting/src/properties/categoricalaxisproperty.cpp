/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <inviwo/core/properties/boolproperty.h>                      // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>                 // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                 // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>                    // for OptionPropertyOption
#include <inviwo/core/properties/ordinalproperty.h>                   // for FloatProperty, Floa...
#include <inviwo/core/properties/propertysemantics.h>                 // for PropertySemantics
#include <inviwo/core/properties/stringproperty.h>                    // for StringProperty
#include <inviwo/core/util/glmvec.h>                                  // for vec2, vec4, dvec2
#include <inviwo/core/util/staticstring.h>                            // for operator+
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>  // for FontFaceOptionProperty
#include <modules/fontrendering/properties/fontproperty.h>            // for FontProperty
#include <modules/fontrendering/util/fontutils.h>                     // for getFont, FontType
#include <modules/plotting/datastructures/axissettings.h>             // for AxisSettings::Orien...
#include <modules/plotting/datastructures/majorticksettings.h>        // for TickStyle, TickStyl...
#include <modules/plotting/datastructures/minortickdata.h>            // for MinorTickData
#include <modules/plotting/properties/plottextproperty.h>             // for PlotTextProperty
#include <modules/plotting/properties/tickproperty.h>                 // for MajorTickProperty

#include <cstddef>  // for size_t
#include <utility>  // for move

namespace inviwo {

namespace plot {
class MinorTickSettings;
class PlotTextSettings;

const std::string CategoricalAxisProperty::classIdentifier = "org.inviwo.CategoricalAxisProperty";
std::string CategoricalAxisProperty::getClassIdentifier() const { return classIdentifier; }

CategoricalAxisProperty::CategoricalAxisProperty(
    std::string_view identifier, std::string_view displayName, std::vector<std::string> categories,
    Orientation orientation, InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , visible_{"visible", "Visible", true}
    , color_{"color", "Color",
             util::ordinalColor(vec4{0.0f, 0.0f, 0.0f, 1.0f}).set("Color of the axis"_help)}
    , width_{"width", "Width", util::ordinalLength(2.5f, 20.0f).set("Line width of the axis"_help)}
    , scalingFactor_{"scalingFactor", "Scaling Factor",
                     util::ordinalScale(1.0f, 10.0f)
                         .set("Scaling factor affecting tick lengths and offsets of axis caption "
                              "and labels"_help)}
    , mirrored_{"flipped", "Swap Label Position",
                "Show labels on the opposite side of the axis"_help, false}
    , orientation_{"orientation",
                   "Orientation",
                   "Determines the orientation of the axis (horizontal or vertical)"_help,
                   {{"horizontal", "Horizontal", Orientation::Horizontal},
                    {"vertical", "Vertical", Orientation::Vertical}},
                   orientation == Orientation::Horizontal ? size_t{0} : size_t{1}}
    , captionSettings_{"caption", "Caption", "Caption settings"_help, false}
    , labelSettings_{"labels", "Axis Labels",
                     "Settings for axis labels shown next to major ticks"_help, true}
    , majorTicks_{"majorTicks", "Major Ticks", "Settings for major ticks along the axis"_help} {

    scalingFactor_.setVisible(false);
    addProperties(visible_, color_, width_, scalingFactor_, mirrored_, orientation_);

    // change default fonts, make axis labels slightly less pronounced
    captionSettings_.font_.fontFace_.setSelectedIdentifier(font::getFont(font::FontType::Caption));
    labelSettings_.font_.fontFace_.setSelectedIdentifier(font::getFont(font::FontType::Label));

    captionSettings_.title_.set("Axis Title");

    labelSettings_.title_.setVisible(false);
    labelSettings_.position_.setVisible(false);
    labelSettings_.setCurrentStateAsDefault();

    addProperties(captionSettings_, labelSettings_, majorTicks_);

    captionSettings_.setCollapsed(true);
    labelSettings_.setCollapsed(true);

    setCollapsed(true);

    orientation_.onChange([this]() { adjustAlignment(); });
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
    , scalingFactor_{rhs.scalingFactor_}
    , mirrored_{rhs.mirrored_}
    , orientation_{rhs.orientation_}
    , captionSettings_{rhs.captionSettings_}
    , labelSettings_{rhs.labelSettings_}
    , majorTicks_{rhs.majorTicks_}
    , minorTicks_{rhs.minorTicks_} {

    addProperties(visible_, color_, width_, scalingFactor_, mirrored_, orientation_,
                  captionSettings_, labelSettings_, majorTicks_);

    orientation_.onChange([this]() { adjustAlignment(); });

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

    auto updateAlignment = [](PlotTextProperty& p, Orientation o) {
        if (o == Orientation::Horizontal) {
            p.font_.anchorPos_.set(
                vec2{0.0f, (p.placement_ == LabelPlacement::Outside) ? 1.0f : -1.0f});
        } else {
            p.font_.anchorPos_.set(
                vec2{(p.placement_ == LabelPlacement::Outside) ? 1.0f : -1.0f, 0.0f});
        }
    };

    updateAlignment(labelSettings_, getOrientation());
    updateAlignment(captionSettings_, getOrientation());
}

bool CategoricalAxisProperty::getAxisVisible() const { return visible_.get(); }

bool CategoricalAxisProperty::getMirrored() const { return mirrored_.get(); }

vec4 CategoricalAxisProperty::getColor() const { return color_.get(); }

float CategoricalAxisProperty::getWidth() const { return width_.get(); }

float CategoricalAxisProperty::getScalingFactor() const { return scalingFactor_.get(); }

bool CategoricalAxisProperty::getUseDataRange() const { return false; }

dvec2 CategoricalAxisProperty::getRange() const {
    return {0, static_cast<double>(categories_.size()) - 1.0};
}

AxisSettings::Orientation CategoricalAxisProperty::getOrientation() const {
    return orientation_.getSelectedValue();
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
