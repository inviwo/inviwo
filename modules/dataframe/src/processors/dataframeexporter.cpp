/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/dataframe/processors/dataframeexporter.h>

#include <inviwo/core/io/datawriter.h>                // for Overwrite, Overwrite::No, Overwrite...
#include <inviwo/core/ports/datainport.h>             // for DataInport
#include <inviwo/core/ports/outportiterable.h>        // for OutportIterable
#include <inviwo/core/processors/processor.h>         // for Processor
#include <inviwo/core/processors/processorinfo.h>     // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>    // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>     // for Tags
#include <inviwo/core/properties/boolproperty.h>      // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>    // for ButtonProperty
#include <inviwo/core/properties/fileproperty.h>      // for FileProperty
#include <inviwo/core/properties/stringproperty.h>    // for StringProperty
#include <inviwo/core/properties/templateproperty.h>  // for operator<<
#include <inviwo/core/util/filedialogstate.h>         // for AcceptMode, AcceptMode::Save
#include <inviwo/core/util/fileextension.h>           // for FileExtension, operator==
#include <inviwo/core/util/glmvec.h>                  // for uvec3
#include <inviwo/core/util/logcentral.h>              // for LogCentral, LogInfo, LogWarn
#include <inviwo/dataframe/datastructures/dataframe.h>  // IWYU pragma: keep
#include <inviwo/dataframe/io/csvwriter.h>              // for CSVWriter
#include <inviwo/dataframe/io/xmlwriter.h>              // for XMLWriter

#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <ostream>      // for operator<<, basic_ostream
#include <string_view>  // for string_view

#include <fmt/core.h>  // for format_to, basic_string_view, format

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameExporter::processorInfo_{
    "org.inviwo.DataFrameExporter",      // Class identifier
    "DataFrame Exporter",                // Display name
    "Data Output",                       // Category
    CodeState::Stable,                   // Code state
    "CPU, DataFrame, Export, CSV, XML",  // Tags
};

const ProcessorInfo DataFrameExporter::getProcessorInfo() const { return processorInfo_; }

FileExtension DataFrameExporter::csvExtension_ = FileExtension("csv", "CSV");
FileExtension DataFrameExporter::xmlExtension_ = FileExtension("xml", "XML");

DataFrameExporter::DataFrameExporter()
    : Processor()
    , dataFrame_("dataFrame")
    , exportFile_("exportFile", "Export file name", "", "dataframe")
    , exportButton_("snapshot", "Export DataFrame")
    , overwrite_("overwrite", "Overwrite", false)
    , exportIndexCol_("exportIndexCol", "Export Index Column", false)
    , separateVectorTypesIntoColumns_("separateVectorTypesIntoColumns",
                                      "Separate Vector Types Into Columns", true)
    , quoteStrings_("quoteStrings", "Quote Strings", true)
    , delimiter_("delimiter", "Delimiter", ",")
    , exportQueued_(false) {

    exportFile_.clearNameFilters();
    exportFile_.addNameFilter(csvExtension_);
    exportFile_.addNameFilter(xmlExtension_);

    addPort(dataFrame_);
    addProperty(exportFile_);
    addProperty(exportButton_);
    addProperty(overwrite_);
    addProperty(exportIndexCol_);
    addProperty(separateVectorTypesIntoColumns_);
    addProperty(quoteStrings_);
    addProperty(delimiter_);

    exportFile_.setAcceptMode(AcceptMode::Save);
    exportFile_.onChange([this]() {
        separateVectorTypesIntoColumns_.setReadOnly(exportFile_.getSelectedExtension().extension_ ==
                                                    xmlExtension_.extension_);
    });
    exportButton_.onChange([&]() {
        if (dataFrame_.hasData()) {
            if (exportFile_.get().empty()) {
                exportFile_.requestFile();
                // file request got canceled, do nothing
                if (exportFile_.get().empty()) return;
            }
            exportQueued_ = true;
        }
    });

    setAllPropertiesCurrentStateAsDefault();
}

void DataFrameExporter::process() {
    if (exportQueued_) exportData();
    exportQueued_ = false;
}

void DataFrameExporter::exportData() {
    if (exportFile_.getSelectedExtension() == xmlExtension_) {
        exportAsXML();
    } else if (exportFile_.getSelectedExtension() == csvExtension_) {
        exportAsCSV();
    } else {
        // use CSV format as fallback
        LogWarn("Could not determine export format from file "
                << exportFile_ << ", exporting as comma-separated values (csv).");
        exportAsCSV();
    }

    // update widgets as the file might now exist
    exportFile_.clearInitiatingWidget();
    exportFile_.updateWidgets();
}

void DataFrameExporter::exportAsCSV() {
    CSVWriter writer{};
    writer.setOverwrite(overwrite_ ? Overwrite::Yes : Overwrite::No);
    writer.delimiter = delimiter_.get();
    writer.quoteStrings = quoteStrings_.get();
    writer.exportIndexCol = exportIndexCol_.get();
    writer.separateVectorTypesIntoColumns = separateVectorTypesIntoColumns_.get();

    writer.writeData(dataFrame_.getData().get(), exportFile_.get());

    LogInfo("CSV file exported to " << exportFile_);
}

void DataFrameExporter::exportAsXML() {
    XMLWriter writer{};
    writer.setOverwrite(overwrite_ ? Overwrite::Yes : Overwrite::No);
    writer.exportIndexCol = exportIndexCol_.get();

    writer.writeData(dataFrame_.getData().get(), exportFile_.get());
    LogInfo("XML file exported to " << exportFile_);
}

}  // namespace inviwo
