/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

#include <modules/plotting/datastructures/axisdata.h>

#include <inviwo/core/util/glmvec.h>                        // for dvec2, vec4
#include <modules/plotting/datastructures/axissettings.h>   // for AxisSettings, AxisSettings::O...
#include <modules/plotting/datastructures/majortickdata.h>  // for MajorTickData
#include <modules/plotting/datastructures/minortickdata.h>  // for MinorTickData
#include <modules/plotting/datastructures/plottextdata.h>   // for PlotTextData

namespace inviwo {

namespace plot {
class MajorTickSettings;
class MinorTickSettings;
class PlotTextSettings;

AxisData::AxisData(const AxisSettings& s)
    : range{s.getRange()}
    , useDataRange{s.getUseDataRange()}
    , visible{s.getAxisVisible()}
    , mirrored{s.getMirrored()}
    , color{s.getColor()}
    , width{s.getWidth()}
    , scalingFactor{s.getScalingFactor()}
    , orientation{s.getOrientation()}
    , caption{s.getCaption()}
    , captionSettings{s.getCaptionSettings()}
    , labels{s.getLabels()}
    , labelSettings{s.getLabelSettings()}
    , majorTicks{s.getMajorTicks()}
    , minorticks{s.getMinorTicks()} {}

bool AxisData::getAxisVisible() const { return visible; }

bool AxisData::getMirrored() const { return mirrored; }

vec4 AxisData::getColor() const { return color; }

float AxisData::getWidth() const { return width; }

float AxisData::getScalingFactor() const { return scalingFactor; }

bool AxisData::getUseDataRange() const { return useDataRange; }

dvec2 AxisData::getRange() const { return range; }

AxisSettings::Orientation AxisData::getOrientation() const { return orientation; }

const std::string& AxisData::getCaption() const { return caption; }

const PlotTextSettings& AxisData::getCaptionSettings() const { return captionSettings; }

const std::vector<std::string>& AxisData::getLabels() const { return labels; }

const PlotTextSettings& AxisData::getLabelSettings() const { return labelSettings; }

const MajorTickSettings& AxisData::getMajorTicks() const { return majorTicks; }

const MinorTickSettings& AxisData::getMinorTicks() const { return minorticks; }

}  // namespace plot

}  // namespace inviwo
