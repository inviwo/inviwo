/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <modules/plotting/plottingmoduledefine.h>  // for IVW_MODULE_PLOTTING_API

#include <inviwo/core/util/glmvec.h>                        // for dvec2, vec4
#include <modules/plotting/datastructures/axissettings.h>   // for AxisSettings::Orientation
#include <modules/plotting/datastructures/majortickdata.h>  // for MajorTickData
#include <modules/plotting/datastructures/minortickdata.h>  // for MinorTickData
#include <modules/plotting/datastructures/plottextdata.h>   // for PlotTextData

#include <string>  // for string, basic_string
#include <vector>  // for vector

namespace inviwo {

namespace plot {
class MajorTickSettings;
class MinorTickSettings;
class PlotTextSettings;

class IVW_MODULE_PLOTTING_API AxisData : public AxisSettings {
public:
    AxisData() = default;
    AxisData(const AxisSettings& settings);
    virtual ~AxisData() = default;

    dvec2 range = dvec2{0.0, 100.0};

    bool visible = true;
    bool mirrored = false;
    vec4 color = vec4{0.0f, 0.0f, 0.0f, 1.0f};
    float width = 2.5f;
    float scalingFactor = 1.0f;
    Orientation orientation = Orientation::Horizontal;

    std::string caption;
    PlotTextData captionSettings;

    std::vector<std::string> labels;
    PlotTextData labelSettings;

    MajorTickData majorTicks;
    MinorTickData minorticks;

    // Inherited via AxisSettings
    virtual dvec2 getRange() const override;

    virtual bool getAxisVisible() const override;
    virtual bool getMirrored() const override;
    virtual vec4 getColor() const override;
    virtual float getWidth() const override;
    virtual float getScalingFactor() const override;
    virtual Orientation getOrientation() const override;

    virtual const std::string& getCaption() const override;
    virtual const PlotTextSettings& getCaptionSettings() const override;

    virtual const std::vector<std::string>& getLabels() const override;
    virtual const PlotTextSettings& getLabelSettings() const override;

    virtual const MajorTickSettings& getMajorTicks() const override;
    virtual const MinorTickSettings& getMinorTicks() const override;
};

}  // namespace plot
}  // namespace inviwo
