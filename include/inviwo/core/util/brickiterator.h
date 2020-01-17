/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/assertion.h>

#include <iterator>
#include <algorithm>

namespace inviwo {

namespace util {

template <typename Iter>
class BrickIterator {
public:
    using base_category = typename std::iterator_traits<Iter>::iterator_category;

    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename std::iterator_traits<Iter>::value_type;
    using pointer = typename std::iterator_traits<Iter>::pointer;
    using reference = typename std::iterator_traits<Iter>::reference;

    BrickIterator() = default;
    BrickIterator(Iter iterator, size3_t dims, size3_t offset, size3_t extent)
        : current_{0, 0, 0}, start_{offset}, extent_{extent}, im_{dims}, iterator_{iterator} {

        IVW_ASSERT(glm::all(glm::lessThanEqual(start_ + extent_, dims)), "Invalid extents");
    }

    BrickIterator& operator++() {
        ++current_.x;
        if (current_.x == extent_.x) {
            current_.x = 0;
            ++current_.y;
        }
        if (current_.y == extent_.y) {
            current_.y = 0;
            ++current_.z;
        }
        return *this;
    }
    BrickIterator operator++(int) {
        auto it = this;
        operator++();
        return it;
    }

    BrickIterator& operator--() {
        if (current_.x == 0) {
            current_.x = extent_.x - 1;
            if (current_.y == 0) {
                current_.y = extent_.y - 1;
                --current_.z;
            } else {
                --current_.y;
            }
        } else {
            --current_.x;
        }

        return *this;
    }
    BrickIterator operator--(int) {
        auto it = this;
        operator--();
        return it;
    }

    reference operator*() const { return (iterator_ + im_(start_ + current_)).operator*(); }
    pointer operator->() const { return (iterator_ + im_(start_ + current_)).operator->(); }

    bool operator==(const BrickIterator& rhs) const { return current_ == rhs.current_; }
    bool operator!=(const BrickIterator& rhs) const { return current_ != rhs.current_; }

    const Iter& base() const { return iterator_ + im_(start_ + current_); }
    Iter& base() { return iterator_ + im_(start_ + current_); }

    size3_t globalPos() const { return start_ + current_; }
    size3_t blockPos() const { return current_; }

    BrickIterator begin() const { return {*this}; }

    BrickIterator end() const {
        auto it{*this};
        it.current_ = size3_t{0, 0, extent_.z};
        return it;
    }

private:
    size3_t current_;
    size3_t start_;
    size3_t extent_;
    util::IndexMapper3D im_;
    Iter iterator_;
};

}  // namespace util

}  // namespace inviwo
