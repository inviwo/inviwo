/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>       // for BufferRAMPrecision
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/unitsystem.h>                      // for Unit
#include <inviwo/core/metadata/metadataowner.h>                         // for MetaDataOwner
#include <inviwo/core/util/formatdispatching.h>                         // for PrecisionValueType
#include <inviwo/core/util/glmvec.h>                                    // for dvec2
#include <inviwo/core/util/stdextensions.h>                             // for find_if_or_null
#include <inviwo/core/util/zip.h>                                       // for make_sequence
#include <inviwo/dataframe/datastructures/column.h>                     // for Column, Categoric...

#include <algorithm>      // for max, remove_if
#include <iterator>       // for begin, end
#include <numeric>        // for iota
#include <unordered_set>  // for unordered_set
#include <utility>        // for move

namespace inviwo {

/*
 * This creates a table with one column: The index column
 */
DataFrame::DataFrame(std::uint32_t size) : columns_{} { createIndexBuffer(size); }

DataFrame::DataFrame(std::vector<std::shared_ptr<Column>> columns) : columns_{std::move(columns)} {
    if (columns_.empty()) {
        createIndexBuffer(0);
    } else {
        if (auto it = std::ranges::adjacent_find(columns_, std::ranges::not_equal_to{},
                                                 [](auto& item) { return item->getSize(); });
            it != columns_.end()) {
            throw Exception(SourceContext{},
                            "Columns have different lengths {}: {} rows and {} {} rows",
                            (*it)->getHeader(), (*it)->getSize(), (*(it + 1))->getHeader(),
                            (*(it + 1))->getSize());
        }
        if (!getIndexColumn()) {
            createIndexBuffer(columns_.front()->getSize());
        }
    }
}

DataFrame::DataFrame(const DataFrame& rhs) : MetaDataOwner{rhs}, columns_{} {
    for (const auto& col : rhs.columns_) {
        columns_.emplace_back(col->clone());
    }
}
DataFrame::DataFrame(const DataFrame& rhs, std::span<const std::uint32_t> rowSelection)
    : MetaDataOwner{rhs}, columns_{} {
    for (const auto& col : rhs.columns_) {
        columns_.emplace_back(col->clone(rowSelection));
    }
}

DataFrame::DataFrame(const DataFrame& rhs, std::span<const std::string> columnSelection)
    : MetaDataOwner{rhs}, columns_{} {
    for (const auto& col : columnSelection) {
        columns_.emplace_back(rhs.getColumnRef(col).clone());
    }
    if (columns_.empty()) {
        createIndexBuffer(0);
    } else if (!getIndexColumn()) {
        createIndexBuffer(columns_.front()->getSize());
    }
}

DataFrame::DataFrame(const DataFrame& rhs, std::span<const std::string> columnSelection,
                     std::span<const std::uint32_t> rowSelection)
    : MetaDataOwner{rhs}, columns_{} {
    for (const auto& col : columnSelection) {
        columns_.emplace_back(rhs.getColumnRef(col).clone(rowSelection));
    }
}

DataFrame& DataFrame::operator=(const DataFrame& that) {
    if (this != &that) {
        DataFrame tmp(that);
        std::swap(tmp.columns_, columns_);
        std::swap(tmp.metaData_, metaData_);
    }
    return *this;
}
DataFrame& DataFrame::operator=(DataFrame&&) noexcept = default;
DataFrame::DataFrame(DataFrame&&) noexcept = default;

void DataFrame::createIndexBuffer(size_t rows) {
    // at the moment, GPUs only support uints up to 32bit
    auto seq = util::make_sequence<std::uint32_t>(0, static_cast<std::uint32_t>(rows));
    std::vector<std::uint32_t> indices(seq.begin(), seq.end());
    columns_.insert(columns_.begin(), std::make_shared<IndexColumn>("index", std::move(indices)));
}
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

void DataFrame::updateIndexBuffer() {
    const size_t nrows = getNumberOfRows();

    auto& indexVector = getIndexColumnRef().getEditableContainer();

    const auto oldSize = indexVector.size();
    indexVector.resize(nrows);
    if (oldSize == 0) {
        std::iota(indexVector.begin(), indexVector.end(), 0);
    } else if (nrows > oldSize) {
        std::iota(indexVector.begin() + oldSize, indexVector.end(), indexVector[oldSize - 1]);
    }
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

Column& DataFrame::getColumnRef(std::string_view name) {
    if (auto col = getColumn(name)) {
        return *col;
    } else {
        throw Exception(SourceContext{}, "No Column with name '{}' found", name);
    }
}

const Column& DataFrame::getColumnRef(std::string_view name) const {
    if (auto col = getColumn(name)) {
        return *col;
    } else {
        throw Exception(SourceContext{}, "No Column with name '{}' found", name);
    }
}

std::shared_ptr<CategoricalColumn> DataFrame::getCategoricalColumn(std::string_view name) {
    return std::dynamic_pointer_cast<CategoricalColumn>(getColumn(name));
}
std::shared_ptr<const CategoricalColumn> DataFrame::getCategoricalColumn(
    std::string_view name) const {
    return std::dynamic_pointer_cast<const CategoricalColumn>(getColumn(name));
}

CategoricalColumn& DataFrame::getCategoricalColumnRef(std::string_view name) {
    if (auto col = getCategoricalColumn(name)) {
        return *col;
    } else {
        throw Exception(SourceContext{}, "No CategoricalColumn with name '{}' found", name);
    }
}
const CategoricalColumn& DataFrame::getCategoricalColumnRef(std::string_view name) const {
    if (auto col = getCategoricalColumn(name)) {
        return *col;
    } else {
        throw Exception(SourceContext{}, "No CategoricalColumn with name '{}' found", name);
    }
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

const IndexColumn& DataFrame::getIndexColumnRef() const {
    if (auto col = getIndexColumn()) {
        return *col;
    } else {
        throw Exception(SourceContext{}, "No IndexColumn found");
    }
}

IndexColumn& DataFrame::getIndexColumnRef() {
    if (auto col = getIndexColumn()) {
        return *col;
    } else {
        throw Exception(SourceContext{}, "No IndexColumn found");
    }
}

std::vector<std::shared_ptr<Column>>::iterator DataFrame::begin() { return columns_.begin(); }

std::vector<std::shared_ptr<Column>>::const_iterator DataFrame::begin() const {
    return columns_.begin();
}

std::vector<std::shared_ptr<Column>>::iterator DataFrame::end() { return columns_.end(); }

}  // namespace inviwo
