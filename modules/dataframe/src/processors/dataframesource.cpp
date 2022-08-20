/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <inviwo/dataframe/processors/dataframesource.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameSource::processorInfo_{
    "org.inviwo.DataFrameSource",                                // Class identifier
    "DataFrame Source",                                          // Display name
    "Data Input",                                                // Category
    CodeState::Stable,                                           // Code state
    "CPU, Plotting, Source, CSV, JSON, DataFrame, Spreadsheet",  // Tags
};
const ProcessorInfo DataFrameSource::getProcessorInfo() const { return processorInfo_; }

DataFrameSource::DataFrameSource(InviwoApplication* app, const std::string& file)
    : DataSource<DataFrame, DataFrameOutport>(app, file, "spreadsheet")
    , columns_("columns", "Column MetaData") {

    DataSource<DataFrame, DataFrameOutport>::file_.setDisplayName("Spreadsheet file");
}

void DataFrameSource::dataLoaded(std::shared_ptr<DataFrame> data) {
    columns_.updateForNewDataFrame(*loadedData_, util::OverwriteState::Yes);
    columns_.updateDataFrame(*loadedData_);
}
void DataFrameSource::dataDeserialized(std::shared_ptr<DataFrame> data) {
    columns_.updateForNewDataFrame(*loadedData_, util::OverwriteState::No);
    columns_.updateDataFrame(*loadedData_);
}

}  // namespace inviwo
