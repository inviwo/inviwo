/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/plotting/utils/csvreader.h>

#include <modules/plotting/datastructures/column.h>
#include <inviwo/core/util/filesystem.h>

#include <fstream>

namespace inviwo {

CSVReader::CSVReader() : delimiters_(","), firstRowHeader_(true) {}

CSVReader* CSVReader::clone() const { return new CSVReader(*this); }

void CSVReader::setDelimiters(const std::string& delim) { delimiters_ = delim; }

void CSVReader::setFirstRowHeader(bool hasHeader) {
    firstRowHeader_ = true;
}

std::shared_ptr<plot::DataFrame> CSVReader::readData(const std::string& fileName) {
    std::ifstream file(fileName);

    if (!file.is_open()) {
        throw FileException(std::string("CSVReader: Could not open file \"" + fileName + "\"."));
    }

    file.seekg(0, std::ios::end);
    std::streampos len = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(len);
    // read in entire file
    file.read(buffer.data(), len);

    // create a string stream for easier handling
    std::stringstream in(buffer.data());
    //in.rdbuf()->pubsetbuf(buffer.data(), len);

    // current line
    size_t line = 1u;

    auto extractColumn = [&]() -> std::pair<std::string, bool> {
        std::string value;
        size_t quoteCount = 0;
        char prev = 0;
        const size_t startLine = line;
        // ignore empty lines
        while (!in.eof() && (in.peek() == '\n')) {
            in.get();
        }
        char ch;
        while (in.get(ch)) {
            if (ch == '\n') {
                ++line;  // increase line counter
                // consume line break, if inside quotes
                if ((quoteCount & 1) != 0) {
                    value += ch;
                    prev = ch;
                    continue;
                }
            }
            if (ch == '"') {
                ++quoteCount;
            } else if (util::contains(delimiters_, ch) || (ch == '\n')) {
                // found a delimiter/newline, ensure that it isn't enclosed by quotes,
                // i.e. an even count of quotes
                if ((quoteCount == 0) || ((prev == '"') && ((quoteCount & 1) == 0))) {
                    return{ value, (ch == '\n') };
                }
            }
            prev = ch;
            value += ch;
        }
        if (((quoteCount & 1) != 0) && in.eof()) {
            throw Exception("CSVReader: unmatched quotes (line " + std::to_string(startLine) + ")");
        }
        return {value, false};
    };

    auto extractRow = [&]() -> std::vector<std::string> {        
        auto val = extractColumn();
        if (in.eof() && val.first.empty()) {
            // reached end of file, no more data
            return{};
        }
        std::vector<std::string> values;
        values.push_back(val.first);
        while (!val.second && !in.eof()) {
            val = extractColumn();
            values.push_back(val.first);
        }
        return values;
    };

    std::vector<std::string> headers;
    if (firstRowHeader_) {
        // read headers
        headers = extractRow();
        if (headers.empty()) {
            throw Exception("CSVReader: no column headers found.");
        }
    }

    std::vector<std::string> data = extractRow();
    if (data.empty()) {
        throw Exception("CSVReader: empty file, no data");
    }

    if (!firstRowHeader_) {
        // assign default column headers
        for (size_t i = 0; i < data.size(); ++i) {
            headers.push_back(std::string("Column ") + std::to_string(i + 1));
        }
    }
    // figure out column types
    auto dataFrame = plot::createDataFrame(data, headers);
    
    while (!data.empty()) {
        dataFrame->addRow(data);

        data = extractRow();
    }
    dataFrame->updateIndexBuffer();
    return dataFrame;
}

}  // namespace inviwo
