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
#include <modules/base/algorithm/dataminmax.h>

#include <unordered_map>

namespace inviwo {

void Column::setRange(dvec2 range) { columnRange_ = range; }

void Column::unsetRange() { columnRange_ = std::nullopt; }

std::optional<dvec2> Column::getRange() const {
    if (columnRange_.has_value()) {
        return columnRange_;
    }
    return std::nullopt;
}

namespace columnutil {

dvec2 getRange(const Column& col) {
    if (col.getRange()) {
        return col.getRange().value();
    }
    auto [min, max] = util::bufferMinMax(col.getBuffer().get(), IgnoreSpecialValues::Yes);
    return dvec2(glm::compMin(min), glm::compMax(max));
}

}  // namespace columnutil

IndexColumn::IndexColumn(std::string_view header, std::shared_ptr<Buffer<std::uint32_t>> buffer)
    : TemplateColumn<std::uint32_t>(header, buffer) {}

IndexColumn::IndexColumn(std::string_view header, std::vector<std::uint32_t> data)
    : TemplateColumn<std::uint32_t>(header, data) {}

IndexColumn* IndexColumn::clone() const { return new IndexColumn(*this); }

ColumnType IndexColumn::getColumnType() const { return ColumnType::Index; }

CategoricalColumn::CategoricalColumn(std::string_view header,
                                     const std::vector<std::string>& values)
    : TemplateColumn<std::uint32_t>(header) {
    append(values);
}

CategoricalColumn* CategoricalColumn::clone() const { return new CategoricalColumn(*this); }

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

namespace detail {

struct AppendHelper {
    AppendHelper(CategoricalColumn* col, const std::vector<std::string>& lookupTable) : col{col} {
        for (auto&& [idx, str] : util::enumerate(lookupTable)) {
            dict[str] = static_cast<std::uint32_t>(idx);
        }
    }

    void append(const std::string& value) {
        auto it = dict.find(value);
        if (it != dict.end()) {
            data.push_back(it->second);
        } else {
            auto newIdx = col->addCategory(value);
            dict[value] = newIdx;
            data.push_back(newIdx);
        }
    }

    CategoricalColumn* col;
    std::unordered_map<std::string, std::uint32_t> dict;
    std::vector<std::uint32_t> data;
};

}  // namespace detail

void CategoricalColumn::append(const Column& col) {
    if (col.getSize() == 0) return;

    if (auto srccol = dynamic_cast<const CategoricalColumn*>(&col)) {
        detail::AppendHelper helper{this, lookUpTable_};
        for (auto idx : srccol->getTypedBuffer()->getRAMRepresentation()->getDataContainer()) {
            const auto& value = srccol->lookUpTable_[idx];
            helper.append(value);
        }
        buffer_->getEditableRAMRepresentation()->append(helper.data);
    } else {
        throw Exception("data formats of columns do not match", IVW_CONTEXT);
    }
}

void CategoricalColumn::append(const std::vector<std::string>& data) {
    if (data.empty()) return;

    detail::AppendHelper helper{this, lookUpTable_};
    for (auto& value : data) {
        helper.append(value);
    }
    buffer_->getEditableRAMRepresentation()->append(helper.data);
}

std::uint32_t CategoricalColumn::addCategory(std::string_view cat) { return addOrGetID(cat); }

glm::uint32_t CategoricalColumn::addOrGetID(std::string_view str) {
    auto it = std::find(lookUpTable_.begin(), lookUpTable_.end(), str);
    if (it != lookUpTable_.end()) {
        return static_cast<glm::uint32_t>(std::distance(lookUpTable_.begin(), it));
    }
    lookUpTable_.emplace_back(str);
    return static_cast<glm::uint32_t>(lookUpTable_.size() - 1);
}

}  // namespace inviwo
