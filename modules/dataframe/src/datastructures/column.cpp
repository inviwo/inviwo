/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>       // for BufferRAMPrecision
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/unitsystem.h>                      // for Unit
#include <inviwo/core/util/exception.h>                                 // for Exception, RangeE...
#include <inviwo/core/util/glmvec.h>                                    // for dvec2
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT, IVW_...
#include <inviwo/core/util/stdextensions.h>                             // for transform
#include <inviwo/core/util/zip.h>

#include <sstream>        // for basic_stringbuf<>...
#include <unordered_map>  // for unordered_map

#include <glm/gtc/type_precision.hpp>  // for uint32_t

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
                                     const std::vector<std::string>& values, Unit unit,
                                     std::optional<dvec2> range)
    : header_{header}
    , unit_{unit}
    , range_{range}
    , buffer_{std::make_shared<Buffer<std::uint32_t>>(0)} {
    append(values);
}

CategoricalColumn::CategoricalColumn(std::string_view header, std::vector<type> data,
                                     std::vector<std::string> lookup, Unit unit,
                                     std::optional<dvec2> range)
    : header_{header}
    , unit_{unit}
    , range_{range}
    , buffer_{util::makeBuffer(std::move(data))}
    , lookUpTable_{std::move(lookup)} {

    for (auto&& [i, str] : util::enumerate<type>(lookUpTable_)) {
        lookupMap_[str] = i;
    }
}

CategoricalColumn::CategoricalColumn(const CategoricalColumn& rhs)
    : header_{rhs.header_}
    , unit_{rhs.unit_}
    , range_{rhs.range_}
    , buffer_{std::shared_ptr<Buffer<std::uint32_t>>(rhs.buffer_->clone())}
    , lookUpTable_{rhs.lookUpTable_}
    , lookupMap_{rhs.lookupMap_} {}

CategoricalColumn::CategoricalColumn(const CategoricalColumn& rhs,
                                     const std::vector<std::uint32_t>& rowSelection)
    : header_{rhs.header_}
    , unit_{rhs.unit_}
    , range_{rhs.range_}
    , buffer_{std::make_shared<Buffer<std::uint32_t>>(rowSelection.size())}
    , lookUpTable_{rhs.lookUpTable_}
    , lookupMap_{rhs.lookupMap_} {

    const auto& src = rhs.buffer_->getRAMRepresentation()->getDataContainer();
    auto& dst = buffer_->getEditableRAMRepresentation()->getDataContainer();
    for (size_t i = 0; i < rowSelection.size(); ++i) {
        dst[i] = src[rowSelection[i]];
    }
}

CategoricalColumn& CategoricalColumn::operator=(const CategoricalColumn& rhs) {
    if (this != &rhs) {
        header_ = rhs.getHeader();
        unit_ = rhs.unit_;
        range_ = rhs.range_;
        buffer_ = std::shared_ptr<Buffer<std::uint32_t>>(rhs.buffer_->clone());
        lookUpTable_ = rhs.lookUpTable_;
        lookupMap_ = rhs.lookupMap_;
    }
    return *this;
}

CategoricalColumn& CategoricalColumn::operator=(CategoricalColumn&& rhs) {
    if (this != &rhs) {
        header_ = std::move(rhs.header_);
        unit_ = rhs.unit_;
        range_ = rhs.range_;
        buffer_ = std::move(rhs.buffer_);
        lookUpTable_ = std::move(rhs.lookUpTable_);
        lookupMap_ = std::move(rhs.lookupMap_);
    }
    return *this;
}

CategoricalColumn* CategoricalColumn::clone() const { return new CategoricalColumn(*this); }

CategoricalColumn* CategoricalColumn::clone(const std::vector<std::uint32_t>& rowSelection) const {
    return new CategoricalColumn(*this, rowSelection);
}

ColumnType CategoricalColumn::getColumnType() const { return ColumnType::Categorical; }

const std::string& CategoricalColumn::getHeader() const { return header_; }

void CategoricalColumn::setHeader(std::string_view header) { header_ = header; }

Unit CategoricalColumn::getUnit() const { return unit_; }

void CategoricalColumn::setUnit(Unit unit) { unit_ = unit; }

void CategoricalColumn::setCustomRange(std::optional<dvec2> range) { range_ = range; }

std::optional<dvec2> CategoricalColumn::getCustomRange() const { return range_; }

dvec2 CategoricalColumn::getDataRange() const {
    const auto& cont = buffer_->getRAMRepresentation()->getDataContainer();
    auto&& [minIt, maxIt] = std::minmax_element(cont.begin(), cont.end());
    return {*minIt, *maxIt};
}

dvec2 CategoricalColumn::getRange() const {
    if (range_) {
        return *range_;
    } else {
        return getDataRange();
    }
}

size_t CategoricalColumn::getSize() const { return buffer_->getSize(); }

void CategoricalColumn::set(size_t idx, std::string_view str) {
    auto id = addOrGetID(str);
    buffer_->getEditableRAMRepresentation()->set(idx, id);
}

void CategoricalColumn::set(size_t idx, std::uint32_t id) {
    if (id >= lookUpTable_.size()) {
        throw RangeException(IVW_CONTEXT, "Invalid categorical index: {}", id);
    }
    buffer_->getEditableRAMRepresentation()->set(idx, id);
}

const std::string& CategoricalColumn::get(size_t idx) const { return lookUpTable_[getId(idx)]; }

std::uint32_t CategoricalColumn::getId(size_t idx) const {
    return buffer_->getRAMRepresentation()->get(idx);
}

double CategoricalColumn::getAsDouble(size_t idx) const { return static_cast<double>(getId(idx)); }

std::string CategoricalColumn::getAsString(size_t idx) const { return get(idx); }

std::vector<std::string> CategoricalColumn::getValues() const {
    const auto& data = buffer_->getRAMRepresentation()->getDataContainer();
    return util::transform(data, [&](auto idx) { return lookUpTable_[idx]; });
}

void CategoricalColumn::add(std::string_view value) {
    auto id = addOrGetID(value);
    buffer_->getEditableRAMRepresentation()->add(id);
}

CategoricalColumn::AddMany CategoricalColumn::addMany() {
    auto rep = buffer_->getEditableRAMRepresentation();
    return AddMany{this, rep};
}

void CategoricalColumn::append(const Column& col) {
    if (col.getSize() == 0) return;

    if (auto srccol = dynamic_cast<const CategoricalColumn*>(&col)) {
        auto& values = buffer_->getEditableRAMRepresentation()->getDataContainer();

        for (auto idx : srccol->buffer_->getRAMRepresentation()->getDataContainer()) {
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
    values.reserve(values.size() + data.size());
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
        auto ind = static_cast<std::uint32_t>(lookUpTable_.size() - 1);
        lookupMap_.emplace(str, ind);
        return ind;
    }
}

std::shared_ptr<BufferBase> CategoricalColumn::getBuffer() { return buffer_; }

std::shared_ptr<const BufferBase> CategoricalColumn::getBuffer() const { return buffer_; }

std::shared_ptr<Buffer<std::uint32_t>> CategoricalColumn::getTypedBuffer() { return buffer_; }

std::shared_ptr<const Buffer<std::uint32_t>> CategoricalColumn::getTypedBuffer() const {
    return buffer_;
}

std::string_view enumToStr(ColumnType type) {
    switch (type) {
        case ColumnType::Index:
            return "Index";
        case ColumnType::Ordinal:
            return "Ordinal";
        case ColumnType::Categorical:
            return "Categorical";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid ColumnType enum value '{}'",
                    static_cast<int>(type));
}
std::ostream& operator<<(std::ostream& ss, ColumnType type) { return ss << enumToStr(type); }

}  // namespace inviwo
