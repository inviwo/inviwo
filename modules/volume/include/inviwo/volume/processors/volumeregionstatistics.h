/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <inviwo/volume/volumemoduledefine.h>  // for IVW_MODULE_VOLUME_API

#include <inviwo/core/datastructures/coordinatetransformer.h>  // for CoordinateSpace, operator<<
#include <inviwo/core/ports/volumeport.h>                      // for VolumeInport
#include <inviwo/core/processors/poolprocessor.h>              // for PoolProcessor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/properties/optionproperty.h>             // for OptionProperty
#include <inviwo/core/util/staticstring.h>                     // for operator+
#include <inviwo/dataframe/datastructures/dataframe.h>         // for DataFrameOutport

#include <functional>   // for __base
#include <string>       // for operator==, operator+, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operat...

namespace inviwo {

/** \docpage{org.inviwo.VolumeRegionStatistics, Volume Region Statistics}
 * ![](org.inviwo.VolumeRegionStatistics.png?classIdentifier=org.inviwo.VolumeRegionStatistics)
 *
 * Calculate statistics for each volume region/segment in a volume.
 * The following statistics are calculated for each region:
 *   * Volume, given in World space
 *   * Sum for each channel, given in "Value" range
 *   * Mean for each channel, given in "Value" range
 *   * Min for each channel, given in "Value" range
 *   * Max for each channel, given in "Value" range
 *   * Center (x,y,z) mean position in each region, given in `Result Space` coordinates
 *   * Center of Mass for each channel (x, y, z), given in `Result Space` coordinates
 *
 * ### Inports
 *   * __volume__ Segmented input volume
 *   * __atlas__  Index volume, of unsigned integer type, assigning a region index to each voxel.
 *                Has to have the same dimensions as volume. The index range is assumed to
 *                be [0, dataMap.dataRange.y] and without gaps.
 *
 * ### Outports
 *   * __statistics__ Data Frame with the statistics for each region.
 *
 * ### Properties
 *   * __Result Space__ The spatial domain of the resulting statistics. Data, Model, World, or
 *                      Index, defaults to World.
 *
 */
class IVW_MODULE_VOLUME_API VolumeRegionStatistics : public PoolProcessor {
public:
    VolumeRegionStatistics();
    virtual ~VolumeRegionStatistics() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport volume_;
    VolumeInport atlas_;
    DataFrameOutport dataFrame_;

    OptionProperty<CoordinateSpace> space_;
};

}  // namespace inviwo
