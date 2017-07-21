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

/*
 * This creates a table with one column: The index column
 */
DataFrame::DataFrame(glm::u32 size) : columns_() {
    auto indexColumn = addColumn<glm::u32>("index");  // 32 is the biggest possible on gpu atm

    auto ib = indexColumn->getTypedBuffer();
    auto buffer = ib->getEditableRAMRepresentation();
    for (glm::u32 i = 0; i < size; i++) {
        buffer->add(i);
    }
}

/*
In order to prevent data loss, external data will be casted to glm::f64
*/
std::shared_ptr<DataFrame::Column> DataFrame::addColumnFromBuffer(
    const std::string &identifier, std::shared_ptr<const BufferBase> buffer) {
    return buffer->getRepresentation<BufferRAM>()->dispatch<std::shared_ptr<DataFrame::Column>>(
        [&](auto buf) {
            using BufferType = std::remove_cv_t<decltype(buf)>;
            using ValueType = util::PrecsionValueType<BufferType>;
            //  using ValueType = decltype(buf)::type;
            auto col = this->addColumn<ValueType>(identifier);
            auto newBuf = col->getTypedBuffer();  // std::make_shared<Buffer<ValueType>>( );
            auto &newVec = newBuf->getEditableRAMRepresentation()->getDataContainer();
            auto &oldVec = buf->getDataContainer();
            newVec.insert(newVec.end(), oldVec.begin(), oldVec.end());
            // col->setBuffer( newBuf );
            return col;
        });
}

std::shared_ptr<DataFrame::CategoricalColumn> DataFrame::addCategoricalColumn(
    const std::string &header, size_t size /*= 0*/) {
    auto col = std::make_shared<DataFrame::CategoricalColumn>(header);
    col->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer().resize(size);
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
    size_t size = 0;
    if (columns_.size() > 1) {
        for (size_t i = 1; i < columns_.size(); i++) {
            size = std::max(size, columns_[i]->getSize());
        }
        auto indexBuffer = std::dynamic_pointer_cast<Buffer<glm::u32>>(
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

}  // namespace inviwo
