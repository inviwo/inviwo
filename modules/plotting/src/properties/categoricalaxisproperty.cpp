/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/staticstring.h>
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>
#include <modules/fontrendering/properties/fontproperty.h>
#include <modules/fontrendering/util/fontutils.h>
#include <modules/plotting/algorithm/labeling.h>
#include <modules/plotting/datastructures/axisdata.h>
#include <modules/plotting/datastructures/tickdata.h>
#include <modules/plotting/properties/plottextproperty.h>
#include <modules/plotting/properties/tickproperty.h>

#include <cstddef>
#include <utility>

namespace inviwo {

namespace plot {
class MinorTickSettings;
class PlotTextSettings;

std::string_view CategoricalAxisProperty::getClassIdentifier() const { return classIdentifier; }

CategoricalAxisProperty::CategoricalAxisProperty(
    std::string_view identifier, std::string_view displayName, std::vector<std::string> categories,
    Orientation orientation, InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , visible_{"visible", "Visible", true}
    , color_{"color", "Color",
             util::ordinalColor(vec4{0.0f, 0.0f, 0.0f, 1.0f}).set("Color of the axis"_help)}
    , width_{"width", "Width", util::ordinalLength(2.5f, 20.0f).set("Line width of the axis"_help)}
    , mirrored_{"flipped", "Swap Label Position",
                "Show labels on the opposite side of the axis"_help, false}

    , captionSettings_{"caption", "Caption", "Caption settings"_help, false}
    , labelSettings_{"labels", "Axis Labels",
                     "Settings for axis labels shown next to major ticks"_help, true}
    , majorTicks_{"majorTicks", "Major Ticks"} {

    addProperties(visible_, color_, width_, mirrored_);

    // change default fonts, make axis labels slightly less pronounced
    captionSettings_.font_.fontFace_.setSelectedIdentifier(
        font::getFont(font::FontType::Caption).string());
    labelSettings_.font_.fontFace_.setSelectedIdentifier(
        font::getFont(font::FontType::Label).string());

    captionSettings_.title_.set("Axis Title");

    labelSettings_.title_.setVisible(false);
    labelSettings_.position_.setVisible(false);
    labelSettings_.setCurrentStateAsDefault();

    addProperties(captionSettings_, labelSettings_, majorTicks_);

    captionSettings_.setCollapsed(true);
    labelSettings_.setCollapsed(true);

    setCollapsed(true);

    setOrientation(orientation, mirrored_.get());

    setCategories(std::move(categories));
}

CategoricalAxisProperty::CategoricalAxisProperty(const CategoricalAxisProperty& rhs)
    : CompositeProperty{rhs}
    , visible_{rhs.visible_}
    , color_{rhs.color_}
    , width_{rhs.width_}
    , mirrored_{rhs.mirrored_}
    , captionSettings_{rhs.captionSettings_}
    , labelSettings_{rhs.labelSettings_}
    , majorTicks_{rhs.majorTicks_} {

    addProperties(visible_, color_, width_, mirrored_, captionSettings_, labelSettings_,
                  majorTicks_);
}

CategoricalAxisProperty* CategoricalAxisProperty::clone() const {
    return new CategoricalAxisProperty(*this);
}

const std::vector<std::string>& CategoricalAxisProperty::getCategories() const {
    return categories_;
}

void CategoricalAxisProperty::setCategories(std::span<const std::string> categories) {
    categories_.resize(categories.size());
    std::ranges::copy(categories, categories_.begin());
}

void CategoricalAxisProperty::setOrientation(Orientation orientation, bool mirrored) {
    mirrored_.set(mirrored);
    using enum Orientation;
    captionSettings_.offset_.set(orientation == Vertical ? 50.0f : 35.0f);
    captionSettings_.rotation_.set(0.0f);
    captionSettings_.font_.anchorPos_.set(vec2{0.0f, 1.0f});

    labelSettings_.rotation_.set(orientation == Vertical ? -90.0f : 0.0f);
    labelSettings_.font_.anchorPos_.set(orientation == Vertical ? vec2{-1.0f, 0.0f}
                                                                : vec2{0.0f, 1.0f});
}

void CategoricalAxisProperty::update(AxisData& data) const {
    data.range = dvec2{0.0, static_cast<double>(categories_.size()) - 1.0};
    data.visible = visible_.get();
    data.mirrored = mirrored_.get();
    data.color = color_.get();
    data.width = width_.get();
    data.caption = captionSettings_.title_.get();
    captionSettings_.update(data.captionSettings);

    linearRange({.start = 0.0, .stop = static_cast<double>(categories_.size()) - 1.0, .step = 1.0},
                data.majorPositions);
    data.minorPositions.clear();
    data.labels.assign(categories_.begin(), categories_.end());

    labelSettings_.update(data.labelSettings);
    majorTicks_.update(data.major);
    data.minor.style = TickData::Style::None;
}
}  // namespace plot

}  // namespace inviwo
