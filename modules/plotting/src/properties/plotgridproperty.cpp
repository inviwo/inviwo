/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/plotting/properties/plotgridproperty.h>

#include <inviwo/core/algorithm/axislabeling.h>

namespace inviwo::plot {

std::string_view PlotGridProperty::getClassIdentifier() const { return classIdentifier; }

PlotGridProperty::PlotGridProperty::PlotGridProperty(std::string_view identifier,
                                                     std::string_view displayName, Document help,
                                                     InvalidationLevel invalidationLevel,
                                                     PropertySemantics semantics)
    : BoolCompositeProperty{identifier, displayName,       std::move(help),
                            false,      invalidationLevel, std::move(semantics)}
    , color_{"color", "Color",
             util::ordinalColor(vec4{0.45f, 0.45f, 0.48f, 1.0f}).set("Color of the axis"_help)}
    , width_{"width", "Width", util::ordinalLength(1.5f, 20.0f).set("Line width of the axis"_help)}
    , plotAxis_{"plotAxis",
                "Plot Axis",
                {
                    {"none", "None", PlotAxis::None},
                    {"major", "Major", PlotAxis::Major},
                    {"minor", "Minor", PlotAxis::Minor},
                    {"both", "Both", PlotAxis::Both},
                },
                3}
    , positioning_{"positioning",
                   "Positioning",
                   {{"matchingAxis", "Matching Axis", Positioning::MatchingAxis},
                    {"zeroOnly", "Zero only", Positioning::ZeroOnly}},
                   0} {

    addProperties(color_, width_, plotAxis_, positioning_);

    BoolCompositeProperty::setCollapsed(true);
    setCurrentStateAsDefault();
}

PlotGridProperty::PlotGridProperty(std::string_view identifier, std::string_view displayName,
                                   InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : PlotGridProperty{identifier, displayName,
                       "Different settings for a grid covering the plot area spanned by the major "
                       "and minor axes including placement, line widths, colors, and more."_help,
                       invalidationLevel, std::move(semantics)} {}

PlotGridProperty::PlotGridProperty(const PlotGridProperty& rhs)
    : BoolCompositeProperty{rhs}
    , color_{rhs.color_}
    , width_{rhs.width_}
    , plotAxis_{rhs.plotAxis_}
    , positioning_{rhs.positioning_} {
    addProperties(color_, width_, plotAxis_, positioning_);
}

PlotGridProperty* PlotGridProperty::clone() const { return new PlotGridProperty{*this}; }

void PlotGridProperty::update(GridData& data, GridParams majorAxis, GridParams minorAxis) const {
    data.majorRange = majorAxis.range;
    data.minorRange = minorAxis.range;

    data.visible = isChecked();
    data.axis = plotAxis_.get();
    data.color = color_.get();
    data.width = width_.get();

    if (positioning_.get() == Positioning::MatchingAxis) {
        std::vector<double> dummy;
        updateLabelPositions(data.horizontalPositions, dummy, majorAxis.labeling, data.majorRange,
                             majorAxis.maxTicks, 0, false);
        updateLabelPositions(data.verticalPositions, dummy, minorAxis.labeling, data.minorRange,
                             minorAxis.maxTicks, 0, false);
    } else {
        data.horizontalPositions = {0.0};
        data.verticalPositions = {0.0};
    }
}

}  // namespace inviwo::plot
