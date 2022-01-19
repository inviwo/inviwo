/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2021 Inviwo Foundation
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

#include <inviwo/dataframe/datastructures/column.h>

#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/stdextensions.h>

#include <unordered_map>

namespace inviwo {

IndexColumn::IndexColumn(std::string_view header, std::shared_ptr<Buffer<std::uint32_t>> buffer)
    : TemplateColumn<std::uint32_t>(header, buffer) {}

IndexColumn::IndexColumn(std::string_view header, std::vector<std::uint32_t> data)
    : TemplateColumn<std::uint32_t>(header, data) {}

IndexColumn::IndexColumn(const IndexColumn& rhs, const std::vector<std::uint32_t>& rowSelection)
    : TemplateColumn<std::uint32_t>(rhs, rowSelection) {}

IndexColumn* IndexColumn::clone() const { return new IndexColumn(*this); }
IndexColumn* IndexColumn::clone(const std::vector<std::uint32_t>& rowSelection) const {
    return new IndexColumn(*this, rowSelection);
}

ColumnType IndexColumn::getColumnType() const { return ColumnType::Index; }

CategoricalColumn::CategoricalColumn(std::string_view header,
                                     const std::vector<std::string>& values)
    : TemplateColumn<std::uint32_t>(header) {
    append(values);
}

CategoricalColumn::CategoricalColumn(const CategoricalColumn& rhs,
                                     const std::vector<std::uint32_t>& rowSelection)
    : TemplateColumn<std::uint32_t>(rhs, rowSelection)
    , lookUpTable_{rhs.lookUpTable_}
    , lookupMap_{rhs.lookupMap_} {}

CategoricalColumn* CategoricalColumn::clone() const { return new CategoricalColumn(*this); }

CategoricalColumn* CategoricalColumn::clone(const std::vector<std::uint32_t>& rowSelection) const {
    return new CategoricalColumn(*this, rowSelection);
}

ColumnType CategoricalColumn::getColumnType() const { return ColumnType::Categorical; }

std::string CategoricalColumn::getAsString(size_t idx) const {
    auto index = getTypedBuffer()->getRAMRepresentation()->getDataContainer()[idx];
    return lookUpTable_[index];
}

std::shared_ptr<DataPointBase> CategoricalColumn::get(size_t idx, bool getStringsAsStrings) const {
    if (getStringsAsStrings) {
        return std::make_shared<DataPoint<std::string>>(getAsString(idx));
    } else {
        return TemplateColumn<std::uint32_t>::get(idx, getStringsAsStrings);
    }
}

void CategoricalColumn::set(size_t idx, const std::string& str) {
    auto id = addOrGetID(str);
    getTypedBuffer()->getEditableRAMRepresentation()->set(idx, id);
}

std::vector<std::string> CategoricalColumn::getValues() const {
    const auto& data = getTypedBuffer()->getRAMRepresentation()->getDataContainer();
    return util::transform(data, [&](auto idx) { return lookUpTable_[idx]; });
}

void CategoricalColumn::add(std::string_view value) {
    auto id = addOrGetID(value);
    getTypedBuffer()->getEditableRAMRepresentation()->add(id);
}
std::function<void(std::string_view)> CategoricalColumn::addMany() {
    auto rep = buffer_->getEditableRAMRepresentation();
    return [this, rep](std::string_view value) {
        auto id = addOrGetID(value);
        rep->getDataContainer().push_back(id);
    };
}

void CategoricalColumn::append(const Column& col) {
    if (col.getSize() == 0) return;

    if (auto srccol = dynamic_cast<const CategoricalColumn*>(&col)) {
        auto& values = buffer_->getEditableRAMRepresentation()->getDataContainer();

        for (auto idx : srccol->getTypedBuffer()->getRAMRepresentation()->getDataContainer()) {
            const auto& value = srccol->lookUpTable_[idx];
            values.push_back(addOrGetID(value));
        }

    } else {
        throw Exception("data formats of columns do not match", IVW_CONTEXT);
    }
}

void CategoricalColumn::append(const std::vector<std::string>& data) {
    if (data.empty()) return;

    auto& values = buffer_->getEditableRAMRepresentation()->getDataContainer();
    for (const auto& elem : data) {
        values.push_back(addOrGetID(elem));
    }
}

std::uint32_t CategoricalColumn::addCategory(std::string_view cat) { return addOrGetID(cat); }

glm::uint32_t CategoricalColumn::addOrGetID(std::string_view str) {
    if (auto it = lookupMap_.find(str); it != lookupMap_.end()) {
        return it->second;
    } else {
        lookUpTable_.emplace_back(str);
        auto ind = static_cast<glm::uint32_t>(lookUpTable_.size() - 1);
        lookupMap_.emplace(str, ind);
        return ind;
    }
}

}  // namespace inviwo
