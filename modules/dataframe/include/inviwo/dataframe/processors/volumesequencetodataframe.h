/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>     // for IVW_MODULE_DATAFRAME_API

#include <inviwo/core/datastructures/volume/volume.h>   // for Volume
#include <inviwo/core/ports/datainport.h>               // for DataInport
#include <inviwo/core/ports/dataoutport.h>              // for DataOutport
#include <inviwo/core/ports/outportiterable.h>          // for OutportIterable
#include <inviwo/core/processors/processor.h>           // for Processor
#include <inviwo/core/processors/processorinfo.h>       // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>        // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>     // for FloatProperty
#include <inviwo/core/util/glmvec.h>                    // for uvec3
#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrame

#include <cstddef>                                      // for size_t
#include <set>                                          // for set
#include <string>                                       // for operator+, string
#include <unordered_map>                                // for operator!=
#include <vector>                                       // for vector

#include <fmt/core.h>                                   // for format, format_to, basic_string_view

namespace inviwo {

/** \docpage{org.inviwo.VolumeSequenceToDataFrame, Volume To DataFrame}
 * ![](org.inviwo.VolumeSequenceToDataFrame.png?classIdentifier=org.inviwo.VolumeSequenceToDataFrame)
 * This processor converts a volume sequence into a DataFrame.
 *
 * ### Inports
 *   * __volume__  source volume
 *
 * ### Outports
 *   * __outport__  generated DataFrame
 */

class IVW_MODULE_DATAFRAME_API VolumeSequenceToDataFrame : public Processor {
public:
    VolumeSequenceToDataFrame();
    virtual ~VolumeSequenceToDataFrame() = default;

    virtual void initializeResources() override;
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<Volume, 0, true> inport_;
    DataOutport<DataFrame> outport_;

    std::set<size_t> filteredIDs_;
    BoolProperty reduce_;
    FloatProperty probability_;

    BoolProperty omitOutliers_;
    FloatProperty threshold_;

    void recomputeReduceBuffer();
};

}  // namespace inviwo
