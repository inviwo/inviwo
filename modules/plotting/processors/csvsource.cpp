/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/plotting/processors/csvsource.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CSVSource::processorInfo_{
    "org.inviwo.CSVSource",                   // Class identifier
    "CSVSource",                              // Display name
    "Data Input",                             // Category
    CodeState::Stable,                        // Code state
    "CPU, Plotting, Source, CSV, DataFrame",  // Tags
};
const ProcessorInfo CSVSource::getProcessorInfo() const { return processorInfo_; }

CSVSource::CSVSource()
    : Processor()
    , data_("data")
    , firstRowIsHeaders_("firstRowIsHeaders", "First Row Contains Column Headers", true)
    , inputFile_("inputFile_", "CSV File")
    , delimiters_("delimiters", "Delimiters", ",")
    , reloadData_("reloadData", "Reload Data") {

    addPort(data_);

    addProperty(inputFile_);
    addProperty(firstRowIsHeaders_);
    addProperty(delimiters_);
    addProperty(reloadData_);

    reloadData_.onChange([&] {});
}

void CSVSource::process() {
    if (!filesystem::fileExists(inputFile_.get())) {
        LogError("Failed to find file");
        return;
    }

    std::ifstream file(inputFile_.get());

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    util::erase_remove_if(lines, [&](std::string s) {
        if (s == "") return true;
        if (trim(s) == "") return true;
        return false;
    });

    auto size = lines.size();

    std::vector<std::string> headers;

    std::vector<BufferRAMPrecision<float> *> ordinalBuffers;
    std::vector<std::shared_ptr<CategoricalColumn>> nonimalBuffers;

    std::vector<char> delimiters(delimiters_.get().begin(), delimiters_.get().end());

    if (firstRowIsHeaders_) {
        size--;
        headers = splitStringWithMultipleDelimiters(lines[0], delimiters);
        std::transform(headers.begin(), headers.end(), headers.begin(), [](std::string s) {
            util::erase_remove_if(s, [](char cc) {
                return !(cc >= -1) || !(std::isalnum(cc) || cc == '_' || cc == '-');
            });
            return trim(s);
        });
        lines.erase(lines.begin());
    } else {
        for (size_t i = 0; i < splitString(lines[0]).size(); i++) {
            std::ostringstream ss;
            ss << "Column " << i;
            headers.push_back(ss.str());
        }
    }

    // Read headers
    auto dataFrame = std::make_shared<DataFrame>(static_cast<glm::u32>(size));
    int i = 0;
    float d;
    for (auto &s : splitStringWithMultipleDelimiters(lines[0], delimiters)) {
        std::istringstream iss(s);

        if (iss >> d) {  // ordinal buffer
            auto col = dataFrame->addColumn<float>(headers[i++], size);
            ordinalBuffers.push_back(col->getTypedBuffer()->getEditableRAMRepresentation());
        } else {  // nominal buffer
            auto col = dataFrame->addCategoricalColumn(headers[i++], size);
            nonimalBuffers.push_back(col);
        }
    }

    // Read data
    int row = 0;
    for (auto l : lines) {
        int ordinalID = 0;
        int nominalID = 0;
        for (auto v : splitStringWithMultipleDelimiters(l, delimiters)) {
            std::istringstream iss(v);
            if (iss >> d) {
                ordinalBuffers[ordinalID++]->set(row, d);
            } else {
                nonimalBuffers[nominalID++]->set(row, v);
            }
        }
        row++;
    }

    auto zsfd = dataFrame->getDataItem(0);

    data_.setData(dataFrame);
}

}  // namespace plot

}  // namespace inviwo
