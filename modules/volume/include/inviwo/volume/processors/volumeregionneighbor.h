/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/volume/volumemoduledefine.h>

#include <inviwo/core/ports/volumeport.h>               // for VolumeInport, VolumeOutport
#include <inviwo/core/processors/poolprocessor.h>       // for PoolProcessor
#include <inviwo/core/processors/processorinfo.h>       // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>        // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>     // for OrdinalProperty
#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrameInport

namespace inviwo {

class IVW_MODULE_VOLUME_API VolumeRegionNeighbor : public PoolProcessor {
public:
    VolumeRegionNeighbor();

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport inport_;
    VolumeInport scalarField_;
    DataFrameOutport outport_;

    BoolProperty useCutoff_;
    DoubleProperty cutoff_;
};

}  // namespace inviwo
