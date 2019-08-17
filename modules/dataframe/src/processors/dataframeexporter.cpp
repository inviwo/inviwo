/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <inviwo/dataframe/datastructures/dataframeutil.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/io/serialization/serializer.h>

#include <fstream>

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
    , export_(false) {

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
    exportButton_.onChange([&]() { export_ = true; });

    setAllPropertiesCurrentStateAsDefault();
}

void DataFrameExporter::process() {
    if (export_) exportNow();
    export_ = false;
}

void DataFrameExporter::exportNow() {
    if (filesystem::fileExists(exportFile_) && !overwrite_.get()) {
        LogWarn("File already exists: " << exportFile_);
        return;
    }
    if (exportFile_.getSelectedExtension() == xmlExtension_) {
        exportAsXML();
    } else if (exportFile_.getSelectedExtension() == csvExtension_) {
        exportAsCSV(separateVectorTypesIntoColumns_);
    } else {
        // use CSV format as fallback
        LogWarn("Could not determine export format from extension '"
                << filesystem::getFileExtension(exportFile_)
                << "', exporting as comma-separated values (csv).");
        exportAsCSV(separateVectorTypesIntoColumns_);
    }
}

void DataFrameExporter::exportAsCSV(bool separateVectorTypesIntoColumns) {
    std::ofstream file(exportFile_);
    auto dataFrame = dataFrame_.getData();

    const std::string delimiter = delimiter_.get();
    std::string citation = "\"";
    if (!quoteStrings_.get()) citation = "";
    const char lineterminator = '\n';
    const std::array<char, 4> componentNames = {'X', 'Y', 'Z', 'W'};

    // headers
    auto oj = util::make_ostream_joiner(file, delimiter);
    for (const auto& col : *dataFrame) {
        if ((col == dataFrame->getIndexColumn()) && !exportIndexCol_) {
            continue;
        }
        const auto components = col->getBuffer()->getDataFormat()->getComponents();
        if (components > 1 && separateVectorTypesIntoColumns) {
            for (size_t k = 0; k < components; k++) {
                oj = citation + col->getHeader() + ' ' + componentNames[k] + citation;
            }
        } else {
            oj = col->getHeader();
        }
    }
    file << lineterminator;

    std::vector<std::function<void(std::ostream&, size_t)>> printers;
    for (const auto& col : *dataFrame) {
        if ((col == dataFrame->getIndexColumn()) && !exportIndexCol_) {
            continue;
        }
        auto df = col->getBuffer()->getDataFormat();
        if (auto cc = dynamic_cast<const CategoricalColumn*>(col.get())) {
            printers.push_back([cc, citation](std::ostream& os, size_t index) {
                os << citation << cc->getAsString(index) << citation;
            });
        } else if (df->getComponents() == 1) {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Scalars>([&printers](auto br) {
                    printers.push_back([br](std::ostream& os, size_t index) {
                        os << br->getDataContainer()[index];
                    });
                });
        } else if (df->getComponents() > 1 && separateVectorTypesIntoColumns) {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Vecs>([&printers, delimiter](auto br) {
                    using ValueType = util::PrecisionValueType<decltype(br)>;
                    printers.push_back([br, delimiter](std::ostream& os, size_t index) {
                        auto oj = util::make_ostream_joiner(os, delimiter);
                        for (size_t i = 0; i < util::flat_extent<ValueType>::value; ++i) {
                            oj = br->getDataContainer()[index][i];
                        }
                    });
                });
        } else {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Vecs>([&printers, citation](auto br) {
                    printers.push_back([br, citation](std::ostream& os, size_t index) {
                        os << citation << br->getDataContainer()[index] << citation;
                    });
                });
        }
    }

    for (size_t j = 0; j < dataFrame->getNumberOfRows(); j++) {
        if (j != 0) {
            file << lineterminator;
        }
        bool firstCol = true;
        for (auto& printer : printers) {
            if (!firstCol) {
                file << delimiter;
            }
            firstCol = false;
            printer(file, j);
        }
    }

    LogInfo("CSV file exported to " << exportFile_);
}

void DataFrameExporter::exportAsXML() {
    auto dataFrame = dataFrame_.getData();

    std::ofstream file(exportFile_);
    Serializer serializer("");

    for (const auto& col : *dataFrame) {
        if ((col == dataFrame->getIndexColumn()) && !exportIndexCol_) {
            continue;
        }
        col->getBuffer()->getRepresentation<BufferRAM>()->dispatch<void>([&](auto br) {
            serializer.serialize(col->getHeader(), br->getDataContainer(), "Item");
        });
    }

    serializer.writeFile(file);
    LogInfo("XML file exported to " << exportFile_);
}

}  // namespace inviwo
