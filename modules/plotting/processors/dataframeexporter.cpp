/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#include <modules/plotting/processors/dataframeexporter.h>
#include <modules/plotting/datastructures/dataframeutil.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/serialization/serializer.h>

#include <fstream>

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameExporter::processorInfo_{
    "org.inviwo.DataFrameExporter",      // Class identifier
    "DataFrame Exporter",                // Display name
    "Data Output",                       // Category
    CodeState::Experimental,             // Code state
    "CPU, DataFrame, Export, CSV, XML",  // Tags
};

const ProcessorInfo DataFrameExporter::getProcessorInfo() const { return processorInfo_; }

DataFrameExporter::DataFrameExporter()
    : Processor()
    , dataFrame_("dataFrame")
    , exportFile_("exportFile", "Export file name", "",
                  "dataframeCSV")  //, filesystem::getPath(PathType::Data, "/mesh"),
    , exportButton_("snapshot", "Export DataFrame")
    , overwrite_("overwrite", "Overwrite", false)
    , forceDoublePrecision_("forceDoublePrecision", "Force Double Precision", true)
    , export_(false) {
    exportFile_.clearNameFilters();
    exportFile_.addNameFilter("CSV (*.csv)");
    exportFile_.addNameFilter("XML (*.xml)");

    addPort(dataFrame_);
    addProperty(exportFile_);
    addProperty(exportButton_);
    addProperty(overwrite_);
    addProperty(forceDoublePrecision_);

    exportFile_.setAcceptMode(AcceptMode::Save);
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

    bool forceDoublePrecision = forceDoublePrecision_.get();
    exportAsCSV(forceDoublePrecision);
}

void DataFrameExporter::exportAsCSV(bool forceDoublePrecision) {
    std::ofstream file(exportFile_);
    auto dataFrame = dataFrame_.getData();

    static std::string delimiter = ",";
    static std::string lineterminator = "\n";
    std::vector<std::string> componentNames = {"X", "Y", "Z", "W"};

    // headers
    for (size_t j = 0; j < dataFrame->getNumberOfColumns(); j++) {
        auto c = dataFrame->getColumn(j);
        auto components = c->getBuffer()->getDataFormat()->getComponents();

        if (components > 1 && forceDoublePrecision) {
            for (size_t k = 0; k < components; k++) {
                file << c->getHeader();
                if (k < (components - 1)) file << delimiter;
            }

            if (j < (dataFrame->getNumberOfColumns() - 1)) file << delimiter;
            continue;
        }

        file << c->getHeader();
        if (j < (dataFrame->getNumberOfColumns() - 1)) file << delimiter;
    }
    file << lineterminator;

    // content
    std::map<std::string, std::vector<std::string>> dataColumStringMap;
    for (size_t j = 0; j < dataFrame->getNumberOfColumns(); j++) {
        auto c = dataFrame->getColumn(j);
        auto bufferFormat = c->getBuffer()->getDataFormat();

        dataframeutil::BufferToStringDispatcher bio;
        std::vector<std::string> buffStr1;
        std::vector<std::string> buffStr2;
        bufferFormat->dispatch(bio, c->getBuffer(), buffStr1, buffStr2, delimiter);

        if (forceDoublePrecision)
            dataColumStringMap[c->getHeader()] = buffStr2;  // double precision, no tuple
        else
            dataColumStringMap[c->getHeader()] = buffStr1;  // glm types
    }

    for (size_t i = 0; i < dataFrame->getNumberOfRows(); i++) {
        for (size_t j = 0; j < dataFrame->getNumberOfColumns(); j++) {
            auto c = dataFrame->getColumn(j);
            auto& strBuffer = dataColumStringMap[c->getHeader()];
            if (strBuffer.size() && i < strBuffer.size())
                file << strBuffer[i];
            else
                file << "0.0000";  // throw exception
            if (j < (dataFrame->getNumberOfColumns() - 1)) file << delimiter;
        }
        file << lineterminator;
    }

    LogInfo("CSV file exported to " << exportFile_);
}

void DataFrameExporter::exportAsXML(bool forceDoublePrecision /*= true*/) {
    auto dataFrame = dataFrame_.getData();

    std::ofstream file(exportFile_);
    Serializer serializer("");

    for (size_t j = 0; j < dataFrame->getNumberOfColumns(); j++) {
        auto c = dataFrame->getColumn(j);
        auto bufferFormat = c->getBuffer()->getDataFormat();
        dataframeutil::BufferSerializerDispatcher bdisp;
        bufferFormat->dispatch(bdisp, c->getBuffer(), serializer, c->getHeader(),
                               std::string("Item"));
    }

    serializer.writeFile(file);
}

}  // namespace plot

}  // namespace inviwo
