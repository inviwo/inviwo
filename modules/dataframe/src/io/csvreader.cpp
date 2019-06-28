/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwo/dataframe/io/csvreader.h>

#include <inviwo/dataframe/datastructures/column.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>

#include <fstream>

namespace inviwo {

CSVDataReaderException::CSVDataReaderException(const std::string& message, ExceptionContext context)
    : DataReaderException("CSVReader: " + message, context) {}

CSVReader::CSVReader() : DataReaderType<DataFrame>(), delimiters_(","), firstRowHeader_(true) {
    addExtension(FileExtension("csv", "Comma Separated Values"));
}

CSVReader* CSVReader::clone() const { return new CSVReader(*this); }

void CSVReader::setDelimiters(const std::string& delim) { delimiters_ = delim; }

void CSVReader::setFirstRowHeader(bool hasHeader) { firstRowHeader_ = hasHeader; }

std::shared_ptr<DataFrame> CSVReader::readData(const std::string& fileName) {
    auto file = filesystem::ifstream(fileName);

    if (!file.is_open()) {
        throw FileException(std::string("CSVReader: Could not open file \"" + fileName + "\"."),
                            IVW_CONTEXT);
    }
    file.seekg(0, std::ios::end);
    std::streampos len = file.tellg();
    file.seekg(0, std::ios::beg);

    if (len == std::streampos(0)) {
        throw CSVDataReaderException("Empty file, no data", IVW_CONTEXT);
    }

    return readData(file);
}

std::shared_ptr<DataFrame> CSVReader::readData(std::istream& stream) const {
    // Skip BOM if it exists. Added by for example Excel when saving csv files.
    filesystem::skipByteOrderMark(stream);

    if (stream.bad() || stream.fail()) {
        throw CSVDataReaderException("Input stream in a bad state", IVW_CONTEXT);
    }

    // create a string stream from input stream for buffering
    std::stringstream in;
    in << stream.rdbuf();
    if (in.fail()) {
        throw CSVDataReaderException("No data", IVW_CONTEXT);
    }

    // current line
    size_t lineNumber = 1u;

    // extract exactly one field from the current stream position, the bool return value indicates
    // whether a line break was detected following the field
    auto extractField = [&in, &lineNumber, delims = delimiters_]() -> std::pair<std::string, bool> {
        std::string value;
        size_t quoteCount = 0;
        size_t quoteBeginLine = 0;
        char prev = 0;

        auto isLineBreak = [](const char ch, std::istream& stream) {
            if (ch == '\r') {
                // consume potential LF (\n) following CR (\r)
                if (stream.peek() == '\n') {
                    stream.get();
                }
                return true;
            } else {
                return (ch == '\n');
            }
        };

        char ch;
        while (in.get(ch) && in.good()) {
            bool linebreak = isLineBreak(ch, in);
            if (linebreak) {
                ++lineNumber;  // increase line counter
                // ensure that ch is equal to '\n'
                ch = '\n';
                // consume line break, if inside quotes
                if ((quoteCount & 1) != 0) {
                    value += ch;
                    prev = ch;
                    continue;
                }
            }
            if (ch == '"') {  // found a quote
                if (quoteCount == 0) quoteBeginLine = lineNumber;
                ++quoteCount;
            } else if (util::contains(delims, ch) || linebreak) {
                // found a delimiter/newline, ensure that it isn't enclosed by quotes,
                // i.e. a quote count of 0 or an even count of quotes if the previous
                // character was a quote
                if ((quoteCount == 0) || ((prev == '"') && ((quoteCount & 1) == 0))) {
                    return {value, linebreak};
                }
            }
            prev = ch;
            value += ch;
        }
        if (((quoteCount & 1) != 0) && in.eof()) {
            throw CSVDataReaderException("Unmatched quotes (starting in line " +
                                         std::to_string(quoteBeginLine) + ")");
        }
        return {value, false};
    };

    // extract one row from the current stream position, the bool return value indicates whether
    // the end-of-file was detected
    auto extractRow = [&in, extractField,
                       &lineNumber](size_t maxColCount = std::numeric_limits<size_t>::max())
        -> std::pair<std::vector<std::string>, bool> {
        auto val = extractField();
        if (in.eof() && val.first.empty()) {
            // reached end of file, no more data
            return {{}, true};
        } else if (val.first.empty() && val.second) {
            // empty line, ignore
            return {{}, false};
        }
        std::vector<std::string> values;
        values.push_back(val.first);
        while (!val.second && !in.eof()) {
            val = extractField();
            values.push_back(trim(val.first));
        }
        // ignore last field _if_ it is empty and would be inserted in the maxColCount+1 column
        if (values.back().empty() && (values.size() - 1 == maxColCount)) {
            values.resize(values.size() - 1);
        } else if ((values.size() != maxColCount) &&
                   (maxColCount != std::numeric_limits<size_t>::max())) {
            // mismatch in the number of columns
            throw CSVDataReaderException("Column counts do not match (line " +
                                         std::to_string(lineNumber) + ": " +
                                         std::to_string(values.size()) + " fields; DataFrame has " +
                                         std::to_string(maxColCount) + " columns)");
        }
        return {values, false};
    };

    std::vector<std::string> headers;
    size_t maxColCount = std::numeric_limits<size_t>::max();
    if (firstRowHeader_) {
        // read headers
        auto row = extractRow();
        if (row.second || row.first.empty()) {
            throw CSVDataReaderException("Empty file, column headers not found");
        }
        headers = row.first;
        maxColCount = headers.size();
    }

    std::vector<std::vector<std::string>> exampleRows;
    std::vector<size_t> exampleLineNumbers;  // line numbers matching the example rows
    std::streampos streamPos = in.tellg();
    for (auto exampleRow = 0u; exampleRow < 50u; ++exampleRow) {
        size_t currentLine = lineNumber;
        auto row = extractRow(maxColCount);
        if (row.second) {
            // reached end-of-file
            if (exampleRow == 0) {
                throw CSVDataReaderException("Empty file, no data");
            }
            in.clear();  // clear eof-bit
            break;
        } else if (!row.first.empty()) {  // ignore empty lines
            exampleRows.emplace_back(row.first);
            exampleLineNumbers.emplace_back(currentLine);
        }
    }

    // Rewind to start position
    in.seekg(streamPos, std::ios::beg);
    if (!firstRowHeader_) {
        // assign default column headers
        for (size_t i = 0; i < exampleRows.front().size(); ++i) {
            headers.push_back(std::string("Column ") + std::to_string(i + 1));
        }
        // update column count
        maxColCount = headers.size();
    }

    // figure out column types
    // but check for correct column counts first
    for (size_t i = 0; i < exampleRows.size(); ++i) {
        if (exampleRows[i].size() != maxColCount) {
            throw CSVDataReaderException(
                "Column counts do not match (line " + std::to_string(exampleLineNumbers[i]) + ": " +
                std::to_string(exampleRows[i].size()) + " fields; DataFrame has " +
                std::to_string(maxColCount) + " columns)");
        }
    }

    auto dataFrame = createDataFrame(exampleRows, headers);

    size_t rowIndex = firstRowHeader_ ? 1 : 0;
    auto row = extractRow(maxColCount);
    while (!row.second) {
        // Do not add empty rows, i.e. rows with only delimiters (,,,,) or newline
        auto emptyIt = std::find_if(std::begin(row.first), std::end(row.first),
                                    [](const auto& a) { return !a.empty(); });
        if (emptyIt != row.first.end()) {
            // May throw DataTypeMismatch, but do not catch it here since it indicates
            // that the DataFrame is in an invalid state
            dataFrame->addRow(row.first);
        }
        row = extractRow(maxColCount);
        ++rowIndex;
    }
    dataFrame->updateIndexBuffer();
    return dataFrame;
}

}  // namespace inviwo
