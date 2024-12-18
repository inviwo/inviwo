/*********************************************************************************
/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/properties/columnoptionproperty.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <vector>
#include <unordered_map>

namespace inviwo {

class IVW_MODULE_DATAFRAME_API DataFrameGather : public Processor {
public:
    DataFrameGather();

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataFrameInport inport_;
    BrushingAndLinkingInport bnlInport_;
    DataFrameOutport outport_;
    BrushingAndLinkingOutport bnlOutport_;

    ColumnOptionProperty gatherColumn_;
    OptionProperty<BrushingAction> propagationAction_;

    std::shared_ptr<DataFrame> dataFrame_;
    std::vector<std::uint32_t> matchingGatheredRow_;
    std::unordered_map<std::uint32_t, BitSet> gatheredRowToRowIndices_;
    std::uint32_t maxBrushingIndex_;
};

}  // namespace inviwo
