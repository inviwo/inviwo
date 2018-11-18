/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#ifndef IVW_VOLUMETODATAFRAME_H
#define IVW_VOLUMETODATAFRAME_H

#include <modules/plotting/plottingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/datastructures/volume/volume.h>

#include <modules/plotting/datastructures/dataframe.h>

namespace inviwo {
namespace plot {

/** \docpage{org.inviwo.VolumeToDataFrame, Volume To DataFrame}
 * ![](org.inviwo.VolumeToDataFrame.png?classIdentifier=org.inviwo.VolumeToDataFrame)
 * This processor converts a volume into a DataFrame.
 *
 * ### Inports
 *   * __volume__  source volume
 *
 * ### Outports
 *   * __outport__  generated DataFrame
 *
 * ### Properties
 *   * __Mode__ The processor can operate in 4 modes: Analytics, where data for each voxel in the
 *              specified ranges is outputted, or XDir, YDir, and ZDir where one column for each
 *              line of voxels in the specified direction is outputted.
 *   * __X Range__ x range of voxels to use.
 *   * __Y Range__ y range of voxels to use.
 *   * __Z Range__ z range of voxels to use.
 */
class IVW_MODULE_PLOTTING_API VolumeToDataFrame : public Processor {
public:
    VolumeToDataFrame();
    virtual ~VolumeToDataFrame() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    enum class Mode { Analytics, XDir, YDir, ZDir };

    DataInport<Volume> inport_;
    DataOutport<DataFrame> outport_;

    TemplateOptionProperty<Mode> mode_;
    IntSizeTMinMaxProperty rangeX_;
    IntSizeTMinMaxProperty rangeY_;
    IntSizeTMinMaxProperty rangeZ_;
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_VOLUMETODATAFRAME_H
