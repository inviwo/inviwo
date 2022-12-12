/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>  // for IVW_MODULE_PLOTTINGGL_API

#include <inviwo/core/interaction/cameratrackball.h>        // for CameraTrackball
#include <inviwo/core/ports/imageport.h>                    // for ImageInport, ImageOutport
#include <inviwo/core/ports/volumeport.h>                   // for VolumeInport
#include <inviwo/core/processors/processor.h>               // for Processor
#include <inviwo/core/processors/processorinfo.h>           // for ProcessorInfo
#include <inviwo/core/properties/boolcompositeproperty.h>   // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>            // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>          // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>       // for CompositeProperty
#include <inviwo/core/properties/minmaxproperty.h>          // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>          // for OptionProperty, OptionPropert...
#include <inviwo/core/properties/ordinalproperty.h>         // for FloatProperty
#include <inviwo/core/util/staticstring.h>                  // for operator+
#include <modules/plotting/properties/axisproperty.h>       // for AxisProperty
#include <modules/plotting/properties/axisstyleproperty.h>  // for AxisStyleProperty
#include <modules/plottinggl/utils/axisrenderer.h>          // for AxisRenderer3D

#include <array>        // for array
#include <functional>   // for __base
#include <string>       // for operator==, operator+, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {

namespace plot {

/**
 * \class VolumeAxis
 * \brief Processor for rendering axis annotations next to a volume
 */
class IVW_MODULE_PLOTTINGGL_API VolumeAxis : public Processor {
public:
    enum class AxisRangeMode { VolumeDims, VolumeBasis, VolumeBasisOffset, Custom };
    enum class CaptionType { String, Data, Custom };

    VolumeAxis();
    virtual ~VolumeAxis() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void adjustRanges();
    void updateCaptions();

    VolumeInport inport_;
    ImageInport imageInport_;
    ImageOutport outport_;

    FloatProperty axisOffset_;

    OptionProperty<AxisRangeMode> rangeMode_;

    CompositeProperty customRanges_;
    DoubleMinMaxProperty rangeXaxis_;
    DoubleMinMaxProperty rangeYaxis_;
    DoubleMinMaxProperty rangeZaxis_;

    OptionProperty<CaptionType> captionType_;
    StringProperty customCaption_;

    BoolCompositeProperty visibility_;
    OptionPropertyString presets_;
    std::array<BoolProperty, 12> visibleAxes_;

    AxisStyleProperty axisStyle_;
    AxisProperty xAxis_;
    AxisProperty yAxis_;
    AxisProperty zAxis_;

    CameraProperty camera_;
    CameraTrackball trackball_;

    std::vector<AxisRenderer3D> axisRenderers_;

    bool propertyUpdate_;
};

}  // namespace plot

}  // namespace inviwo
