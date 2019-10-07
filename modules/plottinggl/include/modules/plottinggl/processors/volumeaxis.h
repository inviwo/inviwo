/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_VOLUMEAXIS_H
#define IVW_VOLUMEAXIS_H

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/interaction/cameratrackball.h>

#include <modules/plotting/properties/axisproperty.h>
#include <modules/plotting/properties/axisstyleproperty.h>

#include <modules/plottinggl/utils/axisrenderer.h>

namespace inviwo {

namespace plot {

/** \docpage{org.inviwo.VolumeAxis, Volume Axis}
 * ![](org.inviwo.VolumeAxis.png?classIdentifier=org.inviwo.VolumeAxis)
 * Renders a x, y, and z axis next to the input volume.
 *
 * ### Inports
 *   * __Volume__      input volume
 *   * __imageInport__ background image (optional)
 *
 * ### Outports
 *   * __image__       output image containing the rendered volume axes and the optional input image
 *
 * ### Properties
 *   * __Axis Offset__      offset between each axis and the volume
 *   * __Axis Range Mode__  determines axis ranges (volume dimension, volume basis, or customized)
 *   * __X Axis__           axis properties for x
 *   * __Y Axis__           axis properties for y
 *   * __Z Axis__           axis properties for z
 */

/**
 * \class VolumeAxis
 * \brief Processor for rendering axis annotations next to a volume
 */
class IVW_MODULE_PLOTTINGGL_API VolumeAxis : public Processor {
public:
    enum class AxisRangeMode { VolumeDims, VolumeBasis, VolumeBasisOffset, Custom };

    VolumeAxis();
    virtual ~VolumeAxis() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void adjustRanges();

    VolumeInport inport_;
    ImageInport imageInport_;
    ImageOutport outport_;

    FloatProperty axisOffset_;

    TemplateOptionProperty<AxisRangeMode> rangeMode_;

    CompositeProperty customRanges_;
    DoubleMinMaxProperty rangeXaxis_;
    DoubleMinMaxProperty rangeYaxis_;
    DoubleMinMaxProperty rangeZaxis_;

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

#endif  // IVW_VOLUMEAXIS_H
