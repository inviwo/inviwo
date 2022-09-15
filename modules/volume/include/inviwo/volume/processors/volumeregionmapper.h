/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/volume/volumemoduledefine.h>                  // for IVW_MODULE_VOLUME_API

#include <inviwo/core/ports/volumeport.h>                      // for VolumeInport, VolumeOutport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>               // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>            // for IntProperty
#include <inviwo/dataframe/datastructures/dataframe.h>         // for DataFrameInport
#include <inviwo/dataframe/properties/columnoptionproperty.h>  // for ColumnOptionProperty

namespace inviwo {

/** \docpage{org.inviwo.VolumeRegionMapper, Volume Region Map}
 * ![](org.inviwo.VolumeRegionMapper.png?classIdentifier=org.inviwo.VolumeRegionMapper)
 * Voxel values are remapped to values provided in columns of a DataFrame. Requires two columns.
 * First column contains the source indices and the second column contains the matching destination
 * indices.
 *
 * ### Inports
 *   * __inport__          Input volume
 *   * __mappingIndices__  DataFrame with at least two columns used for mapping the input volume
 *
 * ### Outports
 *   * __outport__         Resulting volume after mapping the voxels of the input volume.
 *
 * ### Properties
 * * __from__             DataFrame column index for source values
 * * __to__               DataFrame column index for destination values
 * * __useMissingValue__  If set, unmapped values are set to __missingValue__
 * * __missingValue__     Value specifying missing values
 */
class IVW_MODULE_VOLUME_API VolumeRegionMapper : public Processor {
public:
    VolumeRegionMapper();
    virtual ~VolumeRegionMapper() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport inport_;
    DataFrameInport dataFrame_;
    VolumeOutport outport_;
    ColumnOptionProperty from_;
    ColumnOptionProperty to_;
    BoolProperty useMissingValue_;
    IntProperty missingValue_;
};

}  // namespace inviwo
