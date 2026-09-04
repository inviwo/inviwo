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
#pragma once

#include <modules/plotting/plottingmoduledefine.h>

#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/algorithm/axislabeling.h>

#include <modules/plotting/datastructures/griddata.h>

namespace inviwo::plot {

struct IVW_MODULE_PLOTTING_API GridParams {
    dvec2 range{0.0, 100.0};
    LabelingAlgorithm labeling = LabelingAlgorithm::Matplotlib;
    int maxTicks = 6;
};

class IVW_MODULE_PLOTTING_API PlotGridProperty : public BoolCompositeProperty {
public:
    enum class Positioning : std::uint8_t { MatchingAxis, ZeroOnly };

    PlotGridProperty(std::string_view identifier, std::string_view displayName, Document help,
                     InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                     PropertySemantics semantics = PropertySemantics::Default);
    PlotGridProperty(std::string_view identifier, std::string_view displayName,
                     InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                     PropertySemantics semantics = PropertySemantics::Default);

    PlotGridProperty(const PlotGridProperty& rhs);
    PlotGridProperty(PlotGridProperty&& rhs) = delete;
    PlotGridProperty& operator=(const PlotGridProperty&) = delete;
    PlotGridProperty& operator=(PlotGridProperty&&) = delete;

    virtual PlotGridProperty* clone() const override;
    virtual ~PlotGridProperty() = default;

    using BoolCompositeProperty::set;

    void update(GridData& data, GridParams majorAxis, GridParams minorAxis) const;

    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.PlotGridProperty"};

private:
    FloatVec4Property color_;
    FloatProperty width_;
    OptionProperty<PlotAxis> plotAxis_;
    OptionProperty<Positioning> positioning_;
};

}  // namespace inviwo::plot
