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
    auto seq = util::make_sequence<std::uint32_t>(0, static_cast<std::uint32_t>(size));
    std::vector<std::uint32_t> indices(seq.begin(), seq.end());
    addColumn(std::make_shared<IndexColumn>("index", std::move(indices)));
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
                                                       std::shared_ptr<const BufferBase> buffer,
                                                       Unit unit, std::optional<dvec2> range) {
    return buffer->getRepresentation<BufferRAM>()
        ->dispatch<std::shared_ptr<Column>, dispatching::filter::Scalars>([&](auto buf) {
            using ValueType = util::PrecisionValueType<decltype(buf)>;
            // this will copy the data container.
            auto col = this->addColumn<ValueType>(identifier, buf->getDataContainer(), unit, range);
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

std::shared_ptr<const Column> DataFrame::getColumn(size_t index) const { return columns_[index]; }

std::shared_ptr<Column> DataFrame::getColumn(std::string_view name) {
    return util::find_if_or_null(columns_, [name](auto c) { return c->getHeader() == name; });
}

std::shared_ptr<const Column> DataFrame::getColumn(std::string_view name) const {
    return util::find_if_or_null(columns_, [name](auto c) { return c->getHeader() == name; });
}

std::shared_ptr<Column> DataFrame::getColumn(size_t index) { return columns_[index]; }

std::shared_ptr<const IndexColumn> DataFrame::getIndexColumn() const {
    if (columns_[0]->getColumnType() == ColumnType::Index) {
        return std::static_pointer_cast<const IndexColumn>(columns_[0]);
    } else {
        return nullptr;
    }
}

std::shared_ptr<IndexColumn> DataFrame::getIndexColumn() {
    if (columns_[0]->getColumnType() == ColumnType::Index) {
        return std::static_pointer_cast<IndexColumn>(columns_[0]);
    } else {
        return nullptr;
    }
}

std::vector<std::shared_ptr<Column>>::iterator DataFrame::begin() { return columns_.begin(); }

std::vector<std::shared_ptr<Column>>::const_iterator DataFrame::begin() const {
    return columns_.begin();
}

std::vector<std::shared_ptr<Column>>::iterator DataFrame::end() { return columns_.end(); }

}  // namespace inviwo
