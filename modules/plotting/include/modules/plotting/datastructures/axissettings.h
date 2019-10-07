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
#pragma once

#include <modules/plotting/plottingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/plotting/datastructures/plottextsettings.h>
#include <modules/plotting/datastructures/majorticksettings.h>
#include <modules/plotting/datastructures/minorticksettings.h>

namespace inviwo {

namespace plot {

class IVW_MODULE_PLOTTING_API AxisSettings {
public:
    enum class Orientation { Horizontal, Vertical };
    enum class Placement { Outside, Inside };

    AxisSettings() = default;
    virtual ~AxisSettings() = default;

    virtual dvec2 getRange() const = 0;
    virtual bool getUseDataRange() const = 0;

    virtual bool getAxisVisible() const = 0;
    virtual bool getFlipped() const = 0;
    virtual vec4 getColor() const = 0;
    virtual float getWidth() const = 0;

    virtual Placement getPlacement() const = 0;
    virtual Orientation getOrientation() const = 0;

    // caption besides axis
    virtual const std::string& getCaption() const = 0;
    virtual const PlotTextSettings& getCaptionSettings() const = 0;

    // labels showing numbers along axis
    virtual const std::vector<std::string>& getLabels() const = 0;
    virtual const PlotTextSettings& getLabelSettings() const = 0;

    virtual const MajorTickSettings& getMajorTicks() const = 0;
    virtual const MinorTickSettings& getMinorTicks() const = 0;

    // Utility functions
    bool isVertical() const;
};

IVW_MODULE_PLOTTING_API bool operator==(const AxisSettings& a, const AxisSettings& b);
IVW_MODULE_PLOTTING_API bool operator!=(const AxisSettings& a, const AxisSettings& b);

}  // namespace plot

}  // namespace inviwo
