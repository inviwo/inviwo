/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <modules/plotting/properties/axisproperty.h>
#include <modules/plotting/properties/axisstyleproperty.h>
#include <modules/plottinggl/utils/axisrenderer.h>

namespace inviwo {

namespace plot {

class IVW_MODULE_PLOTTINGGL_API Axis2DProcessorHelper {
public:
    enum class AxisRangeMode { Dims, Basis, BasisOffset, World, Custom };
    enum class OffsetScaling { None, MinExtent, MaxExtent, MeanExtent, Diagonal };

    enum class DimsRangeMode { No, Yes };

    Axis2DProcessorHelper(std::function<std::optional<mat4>()> getBoundingBox,
                          DimsRangeMode useDimsRange = DimsRangeMode::No);

    void renderAxes(size2_t outputDims, const SpatialEntity& entity);

    void adjustScalingFactor(const SpatialEntity* entity = nullptr);
    void adjustRanges(const SpatialEntity* entity);

    auto props() {
        return std::tie(offsetScaling_, axisOffset_, rangeMode_, customRanges_, visibility_,
                        axisStyle_, xAxis_, yAxis_, camera_, trackball_);
    }
    auto props() const {
        return std::tie(offsetScaling_, axisOffset_, rangeMode_, customRanges_, visibility_,
                        axisStyle_, xAxis_, yAxis_, camera_, trackball_);
    }

    OptionProperty<OffsetScaling> offsetScaling_;
    FloatProperty axisOffset_;

    OptionProperty<AxisRangeMode> rangeMode_;

    CompositeProperty customRanges_;
    DoubleMinMaxProperty rangeXaxis_;
    DoubleMinMaxProperty rangeYaxis_;

    BoolCompositeProperty visibility_;
    OptionPropertyString presets_;
    std::array<BoolProperty, 4> visibleAxes_;

    AxisStyleProperty axisStyle_;
    AxisProperty xAxis_;
    AxisProperty yAxis_;

    CameraProperty camera_;
    CameraTrackball trackball_;

    std::array<AxisRenderer3D, 2> axisRenderers_;

protected:
    bool propertyUpdate_;
};

}  // namespace plot

}  // namespace inviwo
