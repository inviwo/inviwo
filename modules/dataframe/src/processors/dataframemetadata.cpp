/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <inviwo/dataframe/processors/dataframemetadata.h>

#include <inviwo/core/processors/processor.h>                        // for Processor
#include <inviwo/core/processors/processorinfo.h>                    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                   // for CodeState, CodeState...
#include <inviwo/core/processors/processortags.h>                    // for Tags
#include <inviwo/dataframe/datastructures/dataframe.h>               // for DataFrame, DataFrame...
#include <inviwo/dataframe/properties/columnmetadatalistproperty.h>  // for ColumnMetaDataListPr...

#include <memory>       // for make_shared, shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameMetaData::processorInfo_{
    "org.inviwo.DataFrameMetaData",  // Class identifier
    "DataFrame MetaData",            // Display name
    "DataFrame",                     // Category
    CodeState::Experimental,         // Code state
    "CPU, DataFrame",                // Tags
    "Augment a DataFrame with column-specific metadata like minimum and maximum values."_help,
};
const ProcessorInfo& DataFrameMetaData::getProcessorInfo() const { return processorInfo_; }

DataFrameMetaData::DataFrameMetaData()
    : Processor()
    , inport_("inport", "Input DataFrame to be augmented"_help)
    , outport_("outport", "Copy of the input DataFrame along with column MetaData"_help)
    , columns_("columns", "Column MetaData", inport_) {

    addPort(inport_);
    addPort(outport_);
    addProperty(columns_);
}

void DataFrameMetaData::process() {
    auto dataFrame = std::make_shared<DataFrame>(*inport_.getData().get());
    columns_.updateDataFrame(*dataFrame);
    outport_.setData(dataFrame);
}

}  // namespace inviwo
