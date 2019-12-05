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

#include <inviwo/dataframe/datastructures/dataframe.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/dataframe/datastructures/datapoint.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

/*
 * This creates a table with one column: The index column
 */
DataFrame::DataFrame(std::uint32_t size) : columns_() {
    // at the moment, GPUs only support uints up to 32bit
    auto &cont = addColumn<std::uint32_t>("index", size)
                     ->getTypedBuffer()
                     ->getEditableRAMRepresentation()
                     ->getDataContainer();
    std::iota(cont.begin(), cont.end(), 0);
}

std::shared_ptr<Column> DataFrame::addColumn(std::shared_ptr<Column> column) {
    if (column) {
        columns_.push_back(column);
    }
    return column;
}

std::shared_ptr<Column> DataFrame::addColumnFromBuffer(const std::string &identifier,
                                                       std::shared_ptr<const BufferBase> buffer) {
    return buffer->getRepresentation<BufferRAM>()
        ->dispatch<std::shared_ptr<Column>, dispatching::filter::Scalars>([&](auto buf) {
            using ValueType = util::PrecisionValueType<decltype(buf)>;
            auto col = this->addColumn<ValueType>(identifier);
            auto newBuf = col->getTypedBuffer();
            auto &newVec = newBuf->getEditableRAMRepresentation()->getDataContainer();
            auto &oldVec = buf->getDataContainer();
            newVec.insert(newVec.end(), oldVec.begin(), oldVec.end());
            return col;
        });
}

std::shared_ptr<CategoricalColumn> DataFrame::addCategoricalColumn(const std::string &header,
                                                                   size_t size) {
    auto col = std::make_shared<CategoricalColumn>(header);
    col->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer().resize(size);
    columns_.push_back(col);
    return col;
}

void DataFrame::addRow(const std::vector<std::string> &data) {
    if (columns_.size() <= 1) {
        throw NoColumns("DataFrame: DataFrame has no columns", IVW_CONTEXT);
    } else if (columns_.size() != data.size() + 1) {  // consider index column of DataFrame
        std::ostringstream oss;
        oss << "Data does not match column count, DataFrame has " << (columns_.size() - 1)
            << " columns while input data has " << data.size();
        oss << ". Input data is : " << joinString(data, " | ");
        throw InvalidColCount(oss.str(), IVW_CONTEXT);
    }
    // Try to match up input data with columns.
    std::vector<size_t> columnIdForDataTypeErrors;
    for (size_t i = 0; i < data.size(); ++i) {
        try {
            columns_[i + 1]->add(data[i]);
        } catch (InvalidConversion &) {
            columnIdForDataTypeErrors.push_back(i + 1);
        }
    }
    if (!columnIdForDataTypeErrors.empty()) {
        std::stringstream errStr;
        errStr << "Data type mismatch for columns: (";
        auto joiner = util::make_ostream_joiner(errStr, ", ");
        std::copy(columnIdForDataTypeErrors.begin(), columnIdForDataTypeErrors.end(), joiner);
        errStr << ") with values: (";
        std::copy(data.begin(), data.end(), joiner);
        errStr << ")" << std::endl
               << " DataFrame will be in an invalid state since since all columns must be of equal "
                  "size.";
        throw DataTypeMismatch(errStr.str(), IVW_CONTEXT);
    }
}

DataFrame::DataItem DataFrame::getDataItem(size_t index, bool getStringsAsStrings) const {
    DataItem di;
    for (auto column : columns_) {
        di.push_back(column->get(index, getStringsAsStrings));
    }
    return di;
}

void DataFrame::updateIndexBuffer() {
    const size_t nrows = getNumberOfRows();

    auto indexBuffer = std::static_pointer_cast<Buffer<std::uint32_t>>(columns_[0]->getBuffer());
    auto &indexVector = indexBuffer->getEditableRAMRepresentation()->getDataContainer();
    indexVector.resize(nrows);
    std::iota(indexVector.begin(), indexVector.end(), 0);
}

size_t DataFrame::getNumberOfColumns() const { return columns_.size(); }

size_t DataFrame::getNumberOfRows() const {
    size_t size = 0;
    if (columns_.size() > 1) {
        for (size_t i = 1; i < columns_.size(); i++) {
            size = std::max(size, columns_[i]->getSize());
        }
    }
    return size;
}

DataFrame::DataFrame(const DataFrame &df) {
    for (const auto &col : df.columns_) {
        columns_.emplace_back(col->clone());
    }
}

std::vector<std::shared_ptr<Column>>::const_iterator DataFrame::end() const {
    return columns_.end();
}

const std::vector<std::pair<std::string, const DataFormatBase *>> DataFrame::getHeaders() const {
    std::vector<std::pair<std::string, const DataFormatBase *>> headers;
    for (const auto &c : columns_) {
        headers.emplace_back(c->getHeader(), c->getBuffer()->getDataFormat());
    }
    return headers;
}

std::string DataFrame::getHeader(size_t idx) const { return columns_[idx]->getHeader(); }

std::shared_ptr<const Column> DataFrame::getColumn(size_t index) const { return columns_[index]; }

std::shared_ptr<Column> DataFrame::getColumn(const std::string &name) {
    return util::find_if_or_null(columns_, [name](auto c) { return c->getHeader() == name; });
}

std::shared_ptr<const Column> DataFrame::getColumn(const std::string &name) const {
    return util::find_if_or_null(columns_, [name](auto c) { return c->getHeader() == name; });
}

std::shared_ptr<Column> DataFrame::getColumn(size_t index) { return columns_[index]; }

std::shared_ptr<const TemplateColumn<std::uint32_t>> DataFrame::getIndexColumn() const {
    return std::dynamic_pointer_cast<const TemplateColumn<std::uint32_t>>(columns_[0]);
}

std::shared_ptr<TemplateColumn<std::uint32_t>> DataFrame::getIndexColumn() {
    return std::dynamic_pointer_cast<TemplateColumn<std::uint32_t>>(columns_[0]);
}

std::vector<std::shared_ptr<Column>>::iterator DataFrame::begin() { return columns_.begin(); }

std::vector<std::shared_ptr<Column>>::const_iterator DataFrame::begin() const {
    return columns_.begin();
}

std::vector<std::shared_ptr<Column>>::iterator DataFrame::end() { return columns_.end(); }

std::shared_ptr<DataFrame> createDataFrame(const std::vector<std::vector<std::string>> &exampleRows,
                                           const std::vector<std::string> &colHeaders) {

    if (exampleRows.empty()) {
        throw InvalidColCount("No example data to derive columns from",
                              IVW_CONTEXT_CUSTOM("DataFrame::createDataFrame"));
    }

    // Guess type of each column, ordinal or nominal
    std::vector<std::pair<unsigned int, unsigned int>> columnTypeStatistics(colHeaders.size(),
                                                                            std::make_pair(0, 0));
    for (size_t i = 0; i < exampleRows.size(); ++i) {
        const auto &rowData = exampleRows[i];
        if (rowData.size() != colHeaders.size()) {
            std::ostringstream oss;
            oss << "Number of headers does not match column count, number of headers: "
                << colHeaders.size() << ", number of columns: " << rowData.size();
            throw InvalidColCount(oss.str(), IVW_CONTEXT_CUSTOM("DataFrame::createDataFrame"));
        }
        for (auto column = 0u; column < rowData.size(); ++column) {
            std::istringstream iss(trim(rowData[column]));
            float d;
            // try extracting floating point number. If the stream is not at its end, then there was
            // trailing data. Hence this value does not represent a floating point number.
            if ((iss >> d) && iss.eof()) {
                // ordinal buffer
                columnTypeStatistics[column].first++;
            } else {  // nominal buffer
                columnTypeStatistics[column].second++;
            }
        }
    }

    auto dataFrame = std::make_shared<DataFrame>(0u);
    for (size_t column = 0; column < exampleRows.front().size(); ++column) {
        const auto header =
            (!colHeaders.empty() ? colHeaders[column]
                                 : std::string("Column ") + std::to_string(column + 1));
        if (columnTypeStatistics[column].first > columnTypeStatistics[column].second) {
            // ordinal buffer
            dataFrame->addColumn<float>(header, 0u);
        } else {  // nominal buffer
            dataFrame->addCategoricalColumn(header, 0);
        }
    }
    return dataFrame;
}

}  // namespace inviwo
