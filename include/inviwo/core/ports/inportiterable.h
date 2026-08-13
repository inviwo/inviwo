/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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
#include <inviwo/core/util/stdextensions.h>

#include <ranges>

namespace inviwo {

class Outport;

template <typename T>
class FlatInportIterator {
    using PortIter = typename std::vector<DataOutportInterface<T>*>::const_iterator;

public:
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::shared_ptr<const T>;
    using pointer = std::shared_ptr<const T>;
    using reference = std::shared_ptr<const T>;

    FlatInportIterator() = default;

    explicit FlatInportIterator(PortIter pIterBegin, PortIter pIterEnd)
        : portIter_{pIterBegin}, portEnd_{pIterEnd}, dataIndex_{0}, dataSize_{0} {
        if (portIter_ != portEnd_) {
            auto* ptr = *portIter_;
            dataIndex_ = 0;
            dataSize_ = ptr->size();
            maybeAdvanceData();
            maybeAdvancePort();
        }
    }
    FlatInportIterator& operator++() {
        ++dataIndex_;
        maybeAdvancePort();
        return *this;
    }
    FlatInportIterator operator++(int) {
        auto i = *this;
        ++(*this);
        return i;
    }

    reference operator*() const { return (*portIter_)->getElement(dataIndex_); }
    pointer operator->() const { return (*portIter_)->getElement(dataIndex_); }

    friend bool operator==(const FlatInportIterator& lhs, const FlatInportIterator& rhs) {
        return lhs.portIter_ == rhs.portIter_ && lhs.dataIndex_ == rhs.dataIndex_;
    }
    friend bool operator!=(const FlatInportIterator& lhs, const FlatInportIterator& rhs) {
        return !(lhs == rhs);
    }

protected:
    void maybeAdvancePort() {
        while (dataIndex_ == dataSize_ && portIter_ != portEnd_) {
            ++portIter_;
            dataIndex_ = 0;
            if (portIter_ != portEnd_) {
                dataSize_ = (*portIter_)->size();
            } else {
                dataSize_ = 0;
            }
            maybeAdvanceData();
        }
    }
    void maybeAdvanceData() {
        while (dataIndex_ < dataSize_ && (*portIter_)->getElement(dataIndex_) == nullptr) {
            ++dataIndex_;
        }
    }

    PortIter portIter_;
    PortIter portEnd_;
    size_t dataIndex_;
    size_t dataSize_;
};

template <typename T>
class RegularInportIterator {
    using PortIter = typename std::vector<DataOutportInterface<T>*>::const_iterator;

public:
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::shared_ptr<const T>;
    using pointer = std::shared_ptr<const T>;
    using reference = std::shared_ptr<const T>;

    RegularInportIterator() = default;
    explicit RegularInportIterator(PortIter pIterBegin, PortIter pIterEnd)
        : portIter_{pIterBegin}, portEnd_{pIterEnd} {
        maybeAdvancePort();
    }

    reference operator*() const { return (*portIter_)->getElement(0); }
    pointer operator->() const { return (*portIter_)->getElement(0); }

    RegularInportIterator& operator++() {
        ++portIter_;
        maybeAdvancePort();
        return *this;
    }
    RegularInportIterator operator++(int) {
        RegularInportIterator i = *this;
        ++(*this);
        return i;
    }

    auto operator<=>(const RegularInportIterator& rhs) const = default;

protected:
    void maybeAdvancePort() {
        while (portIter_ != portEnd_ && (*portIter_)->getElement(0) == nullptr) {
            ++portIter_;
        }
    }

    PortIter portIter_;
    PortIter portEnd_;
};

}  // namespace inviwo
