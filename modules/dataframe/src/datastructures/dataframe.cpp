/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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
#include <inviwo/core/util/zip.h>

#include <utility>

#include <fmt/format.h>

namespace inviwo {

/*
 * This creates a table with one column: The index column
 */
DataFrame::DataFrame(std::uint32_t size) : columns_{} {
    // at the moment, GPUs only support uints up to 32bit
    auto& cont = addColumn<std::uint32_t>("index", size)
                     ->getTypedBuffer()
                     ->getEditableRAMRepresentation()
                     ->getDataContainer();
    std::iota(cont.begin(), cont.end(), 0);
}
DataFrame::DataFrame(const DataFrame& rhs) : columns_{} {
    for (const auto& col : rhs.columns_) {
        columns_.emplace_back(col->clone());
    }
}
DataFrame& DataFrame::operator=(const DataFrame& that) {
    if (this != &that) {
        DataFrame tmp(that);
        std::swap(tmp.columns_, columns_);
    }
    return *this;
}
DataFrame& DataFrame::operator=(DataFrame&&) = default;
DataFrame::DataFrame(DataFrame&&) = default;

std::shared_ptr<Column> DataFrame::addColumn(std::shared_ptr<Column> column) {
    if (column) {
        columns_.push_back(column);
    }
    return column;
}

std::shared_ptr<Column> DataFrame::addColumnFromBuffer(std::string_view identifier,
                                                       std::shared_ptr<const BufferBase> buffer) {
    return buffer->getRepresentation<BufferRAM>()
        ->dispatch<std::shared_ptr<Column>, dispatching::filter::Scalars>([&](auto buf) {
            using ValueType = util::PrecisionValueType<decltype(buf)>;
            auto col = this->addColumn<ValueType>(identifier);
            auto newBuf = col->getTypedBuffer();
            auto& newVec = newBuf->getEditableRAMRepresentation()->getDataContainer();
            auto& oldVec = buf->getDataContainer();
            newVec.insert(newVec.end(), oldVec.begin(), oldVec.end());
            return col;
        });
}

void DataFrame::dropColumn(std::string_view header) {
    columns_.erase(std::remove_if(std::begin(columns_), std::end(columns_),
                                  [&](std::shared_ptr<Column> col) -> bool {
                                      return col->getHeader() == header;
                                  }),
                   std::end(columns_));
}

void DataFrame::dropColumn(const size_t index) {
    if (index >= columns_.size()) {
        return;
    }
    columns_.erase(std::begin(columns_) + index);
}

std::shared_ptr<CategoricalColumn> DataFrame::addCategoricalColumn(std::string_view header,
                                                                   size_t size) {
    auto col = std::make_shared<CategoricalColumn>(header);
    col->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer().resize(size);
    columns_.push_back(col);
    return col;
}

std::shared_ptr<CategoricalColumn> DataFrame::addCategoricalColumn(
    std::string_view header, const std::vector<std::string>& values) {
    auto col = std::make_shared<CategoricalColumn>(header, values);
    columns_.push_back(col);
    return col;
}

void DataFrame::addRow(const std::vector<std::string>& data) {
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
        } catch (InvalidConversion&) {
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
    auto& indexVector = indexBuffer->getEditableRAMRepresentation()->getDataContainer();
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

std::vector<std::shared_ptr<Column>>::const_iterator DataFrame::end() const {
    return columns_.end();
}

const std::vector<std::pair<std::string, const DataFormatBase*>> DataFrame::getHeaders() const {
    std::vector<std::pair<std::string, const DataFormatBase*>> headers;
    for (const auto& c : columns_) {
        headers.emplace_back(c->getHeader(), c->getBuffer()->getDataFormat());
    }
    return headers;
}

std::string DataFrame::getHeader(size_t idx) const { return columns_[idx]->getHeader(); }

std::shared_ptr<const Column> DataFrame::getColumn(size_t index) const { return columns_[index]; }

std::shared_ptr<Column> DataFrame::getColumn(std::string_view name) {
    return util::find_if_or_null(columns_, [name](auto c) { return c->getHeader() == name; });
}

std::shared_ptr<const Column> DataFrame::getColumn(std::string_view name) const {
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

namespace detail {

bool isIntegral(const std::string& str) {
    try {
        // check for integral type, if there are parts left over it is most likely a
        // floating point number
        std::size_t pos;
        std::stoll(str, &pos);
        if (pos == str.size()) {
            return true;
        }
    } catch (const std::exception&) {
    }
    return false;
}

bool isFloat(const std::string& str) {
    try {
        // check for floating point number
        std::size_t pos;
        std::stod(str, &pos);
        if (pos == str.size()) {
            return true;
        }
    } catch (const std::exception&) {
    }
    return false;
}

}  // namespace detail

std::shared_ptr<DataFrame> createDataFrame(const std::vector<std::vector<std::string>>& exampleRows,
                                           const std::vector<std::string>& colHeaders,
                                           bool doublePrecision) {

    if (exampleRows.empty()) {
        throw InvalidColCount("No example data to derive columns from",
                              IVW_CONTEXT_CUSTOM("DataFrame::createDataFrame"));
    }

    // Guess type of each column, distinguish between ordinal (int, ) and nominal (string)
    struct ColCounts {
        unsigned int integral = 0u;
        unsigned int floatingPoint = 0u;
        unsigned int nominal = 0u;
    };

    std::vector<ColCounts> stats(colHeaders.size());
    for (size_t i = 0; i < exampleRows.size(); ++i) {
        const auto& rowData = exampleRows[i];
        if (rowData.size() != colHeaders.size()) {
            throw InvalidColCount(
                fmt::format("Number of headers ({}) does not match column count ({})",
                            colHeaders.size(), rowData.size()),
                IVW_CONTEXT_CUSTOM("DataFrame::createDataFrame"));
        }
        for (auto column = 0u; column < rowData.size(); ++column) {
            if (rowData[column].empty()) {
                continue;
            } else if (detail::isIntegral(rowData[column])) {
                ++stats[column].integral;
                continue;
            } else if (detail::isFloat(rowData[column])) {
                ++stats[column].floatingPoint;
                continue;
            }
            ++stats[column].nominal;
        }
    }

    auto dataframe = std::make_shared<DataFrame>();
    for (auto&& [col, elem] : util::enumerate(util::zip(colHeaders, stats))) {
        const auto header =
            (!get<0>(elem).empty() ? get<0>(elem) : fmt::format("Column {}", col + 1));
        const auto& counts = get<1>(elem);
        if (counts.nominal > 0) {
            dataframe->addCategoricalColumn(header);
        } else if (counts.floatingPoint > 0) {
            if (doublePrecision) {
                dataframe->addColumn<double>(header);
            } else {
                dataframe->addColumn<float>(header);
            }
        } else if (counts.integral > 0) {
            dataframe->addColumn<int>(header);
        } else {
            dataframe->addCategoricalColumn(header);
        }
    }
    return dataframe;
}

}  // namespace inviwo
