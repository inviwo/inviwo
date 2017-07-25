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

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <modules/plotting/datastructures/dataframe.h>
#include <modules/plotting/datastructures/datapoint.h>
#include <inviwo/core/util/formatdispatching.h>

namespace inviwo {

namespace plot {

/*
 * This creates a table with one column: The index column
 */
DataFrame::DataFrame(std::uint32_t size) : columns_() {
    // at the moment, GPUs only support uints up to 32bit
    auto indexColumn = addColumn<std::uint32_t>("index");

    auto ib = indexColumn->getTypedBuffer();
    auto buffer = ib->getEditableRAMRepresentation();
    for (std::uint32_t i = 0; i < size; i++) {
        buffer->add(i);
    }
}

/*
In order to prevent data loss, external data will be casted to glm::f64
*/
std::shared_ptr<Column> DataFrame::addColumnFromBuffer(const std::string &identifier,
                                                       std::shared_ptr<const BufferBase> buffer) {
    return buffer->getRepresentation<BufferRAM>()->dispatch<std::shared_ptr<Column>, dispatching::filter::Scalars>([&](auto buf) {
        using BufferType = std::remove_cv_t<decltype(buf)>;
        using ValueType = util::PrecsionValueType<BufferType>;
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
        throw NoColumns("DataFrame: DataFrame has no columns");
    } else if (columns_.size() != data.size() + 1) { // consider index column of DataFrame
        throw InvalidColCount("DataFrame: data does not match column count");
    }
    // try to match up input data with columns.
    for (size_t i = 0; i < data.size(); ++i) {
        try {
            columns_[i+1]->add(data[i]);
        } catch (InvalidConversion &) {
            throw DataTypeMismatch("DataFrame: data type does not match (col. " +
                                   std::to_string(i + 1) + ")");
        }
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
    size_t size = 0;
    if (columns_.size() > 1) {
        for (size_t i = 1; i < columns_.size(); i++) {
            size = std::max(size, columns_[i]->getSize());
        }
        auto indexBuffer = std::dynamic_pointer_cast<Buffer<std::uint32_t>>(
            columns_[0]->getBuffer());  // change to static cast after tested
        auto &indexVector = indexBuffer->getEditableRAMRepresentation()->getDataContainer();
        indexVector.resize(size);
        std::iota(indexVector.begin(), indexVector.end(), 0);
    }
}

size_t DataFrame::getSize() const {
    size_t size = 0;
    if (columns_.size() > 1) {
        for (size_t i = 1; i < columns_.size(); i++) {
            size = std::max(size, columns_[i]->getSize());
        }
    }
    return size;
}

size_t DataFrame::getNumberOfColumns() const { return columns_.size(); }

size_t DataFrame::getNumberOfRows() const { return getSize(); }

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

std::shared_ptr<DataFrame> createDataFrame(const std::vector<std::string> &exampleData,
                                         const std::vector<std::string> &colHeaders) {

    if (!colHeaders.empty() && (colHeaders.size() != exampleData.size())) {
        throw InvalidColCount("Number of headers does not match column count");
    }

    auto dataFrame = std::make_shared<DataFrame>(0u);
    for (size_t i = 0; i < exampleData.size(); ++i) {
        const auto header =
            (!colHeaders.empty() ? colHeaders[i] : std::string("Column ") + std::to_string(i + 1));

        std::istringstream iss(exampleData[i]);

        float d;
        if (iss >> d) {  // ordinal buffer
            dataFrame->addColumn<float>(header, 0u);
        } else {  // nominal buffer
            dataFrame->addCategoricalColumn(header, 0);
        }
    }
    return dataFrame;
}

}  // namespace plot

}  // namespace inviwo
