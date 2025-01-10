/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/io/datareader.h>                               // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                      // for DataReaderException
#include <inviwo/core/ports/outportiterable.h>                       // for OutportIterableImpl<...
#include <inviwo/core/processors/processorinfo.h>                    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                   // for CodeState, CodeState...
#include <inviwo/core/processors/processortags.h>                    // for Tags
#include <inviwo/core/properties/fileproperty.h>                     // for FileProperty
#include <inviwo/core/properties/property.h>                         // for OverwriteState, Over...
#include <inviwo/dataframe/datastructures/dataframe.h>               // for DataFrameOutport
#include <inviwo/dataframe/properties/columnmetadatalistproperty.h>  // for ColumnMetaDataListPr...
#include <modules/base/processors/datasource.h>                      // for DataSource

#include <functional>   // for __base
#include <string_view>  // for string_view

namespace inviwo {
class InviwoApplication;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameSource::processorInfo_{
    "org.inviwo.DataFrameSource",                                // Class identifier
    "DataFrame Source",                                          // Display name
    "Data Input",                                                // Category
    CodeState::Stable,                                           // Code state
    "CPU, Plotting, Source, CSV, JSON, DataFrame, Spreadsheet",  // Tags
    "Loads a DataFrame from file."_help};
const ProcessorInfo& DataFrameSource::getProcessorInfo() const { return processorInfo_; }

DataFrameSource::DataFrameSource(InviwoApplication* app, const std::filesystem::path& filePath)
    : DataSource<DataFrame, DataFrameOutport>(util::getDataReaderFactory(app), filePath,
                                              "spreadsheet")
    , columns_("columns", "Column MetaData") {

    DataSource<DataFrame, DataFrameOutport>::filePath.setDisplayName("Spreadsheet file");
}

void DataFrameSource::dataLoaded(std::shared_ptr<DataFrame> data) {
    columns_.updateForNewDataFrame(*data, util::OverwriteState::Yes);
    columns_.updateDataFrame(*data);
}
void DataFrameSource::dataDeserialized(std::shared_ptr<DataFrame> data) {
    columns_.updateForNewDataFrame(*data, util::OverwriteState::No);
    columns_.updateDataFrame(*data);
}

}  // namespace inviwo
