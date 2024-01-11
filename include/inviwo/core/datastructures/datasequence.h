/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/transformiterator.h>
#include <inviwo/core/datastructures/datatraits.h>

#include <vector>
#include <memory>
#include <utility>
#include <span>

#include <variant>

namespace inviwo {

template <typename Data>
class DataSequence {

    using DataVariant = std::variant<std::shared_ptr<const Data>, std::shared_ptr<Data>>;
    std::vector<DataVariant> data_;

    static constexpr auto getConstData =
        [](const DataVariant& data) -> std::shared_ptr<const Data> {
        return std::visit([](auto& d) -> std::shared_ptr<const Data> { return d; }, data);
    };

    static constexpr auto getData = [](DataVariant& data) -> std::shared_ptr<Data> {
        if (std::holds_alternative<std::shared_ptr<Data>>(data)) {
            return std::get<std::shared_ptr<Data>>(data);
        } else {
            return nullptr;
        }
    };

    std::shared_ptr<const Data> get(const DataVariant& v) const { return getConstData(v); }
    std::shared_ptr<Data> get(DataVariant& v) { return getData(v); }

public:
    using value_type = std::shared_ptr<Data>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator =
        util::TransformIterator<decltype(getData), typename std::vector<DataVariant>::iterator>;
    using const_iterator =
        util::TransformIterator<decltype(getConstData),
                                typename std::vector<DataVariant>::const_iterator>;

    DataSequence() = default;

    DataSequence(std::span<std::shared_ptr<Data>> data) {
        for (auto item : data) {
            data_.emplace_back(item);
        }
    }
    DataSequence(std::span<std::shared_ptr<const Data>> data) {
        for (auto item : data) {
            data_.emplace_back(item);
        }
    }

    DataSequence(const DataSequence& rhs) {
        for (auto item : rhs.data_) {
            data_.emplace_back(std::in_place_type_t<std::shared_ptr<Data>>{}, get(item)->clone());
        }
    }
    DataSequence(DataSequence&&) noexcept = default;
    DataSequence& operator=(const DataSequence& that) {
        if (this != &that) {
            DataSequence tmp(that);
            swap(tmp);
        }
        return this;
    }
    DataSequence& operator=(DataSequence&&) noexcept = delete;

    ~DataSequence() = default;

    void push_back(std::shared_ptr<const Data> data) { data_.emplace_back(data); }
    void push_back(std::shared_ptr<Data> data) { data_.emplace_back(data); }

    template <typename... Args>
    std::shared_ptr<Data> emplace_back(Args&&... args) {
        return get(data_.emplace_back(std::in_place_type_t<std::shared_ptr<Data>>{},
                                      std::forward<Args>(args)...));
    }

    void insert(iterator pos, std::shared_ptr<const Data> data) { data_.insert(pos.base(), data); }
    void insert(iterator pos, std::shared_ptr<Data> data) { data_.insert(pos.base(), data); }

    void insert(const_iterator pos, const_iterator srcBegin, const_iterator srcEnd) {
        auto bpos = pos.base();
        for (auto it = srcBegin; it != srcEnd; ++it) {
            bpos = data_.insert(bpos, *it);
            ++bpos;
        }
    }

    void erase(iterator pos) { data_.erase(pos.base()); }
    void erase(const_iterator begin, const_iterator end) { data_.erase(begin.base(), end.base()); }
    void shrink_to_fit() { data_.shrink_to_fit(); }

    iterator begin() { return util::makeTransformIterator(getData, data_.begin()); }
    iterator end() { return util::makeTransformIterator(getData, data_.end()); }

    const_iterator begin() const {
        return util::makeTransformIterator(getConstData, data_.begin());
    }
    const_iterator end() const { return util::makeTransformIterator(getConstData, data_.end()); }

    std::size_t size() const { return data_.size(); }

    bool empty() const { return data_.empty(); }

    void clear() { data_.clear(); }

    void reserve(std::size_t n) { data_.reserve(n); }

    std::shared_ptr<const Data> operator[](std::size_t n) const { return get(data_[n]); }

    std::shared_ptr<Data> operator[](std::size_t n) { return get(data_[n]); }

    std::shared_ptr<const Data> at(std::size_t n) const { return get(data_.at(n)); }
    std::shared_ptr<Data> at(std::size_t n) { return get(data_.at(n)); }

    std::shared_ptr<const Data> front() const { return get(data_.front()); }
    std::shared_ptr<Data> front() { return get(data_.front()); }

    std::shared_ptr<const Data> back() const { return get(data_.back()); }
    std::shared_ptr<Data> back() { return get(data_.back()); }

    void pop_back() { data_.pop_back(); }

    void swap(DataSequence& other) { data_.swap(other.data_); }

    void swap(std::vector<std::pair<std::shared_ptr<const Data>, std::shared_ptr<Data>>>& other) {
        data_.swap(other);
    }

    void swap(std::vector<std::pair<std::shared_ptr<const Data>, std::shared_ptr<Data>>>&& other) {
        data_.swap(other);
    }
};
template <typename Data>
struct DataTraits<DataSequence<Data>> {
    static std::string classIdentifier() {
        return util::appendIfNotEmpty(DataTraits<Data>::classIdentifier(), ".sequence");
    }
    static std::string dataName() {
        return fmt::format("DataSequence<{}>", DataTraits<Data>::dataName());
    }
    static uvec3 colorCode() { return color::lighter(DataTraits<Data>::colorCode(), 1.12f); }
    static Document info(const DataSequence<Data>& data) {
        return detail::vectorInfo<Data>(data.size(), data.empty() ? nullptr : data.front().get(),
                                        data.empty() ? nullptr : data.back().get());
    }
};

}  // namespace inviwo
