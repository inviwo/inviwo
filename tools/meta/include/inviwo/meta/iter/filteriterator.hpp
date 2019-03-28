/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/meta/iter/iterutil.hpp>

#include <iterator>
#include <type_traits>

namespace inviwo::meta {

template <typename Pred, typename Iter>
class FilterIterator {
public:
    using base_category = typename std::iterator_traits<Iter>::iterator_category;

    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using iterator_category =
        std::conditional_t<std::is_convertible_v<base_category, std::bidirectional_iterator_tag>,
                           std::bidirectional_iterator_tag, base_category>;
    using value_type = typename std::iterator_traits<Iter>::value_type;
    using pointer = typename std::iterator_traits<Iter>::pointer;
    using reference = typename std::iterator_traits<Iter>::reference;

    FilterIterator() = default;
    FilterIterator(Iter iterator, Iter end) : predicate_{}, iterator_{iterator}, end_{end} {
        satisfy_predicate();
    }

    FilterIterator(Pred predicate, Iter iterator, Iter end)
        : predicate_{std::move(predicate)}, iterator_{iterator}, end_{end} {
        satisfy_predicate();
    }

    FilterIterator& operator++() {
        ++iterator_;
        satisfy_predicate();
        return *this;
    }
    FilterIterator operator++(int) {
        auto it = this;
        operator++();
        return it;
    }

    template <typename I = Iter, typename = iterutil::require_t<std::bidirectional_iterator_tag, I>>
    FilterIterator& operator--() {
        while (!predicate_(*--iterator_)) {
        }
        return *this;
    }
    template <typename I = Iter, typename = iterutil::require_t<std::bidirectional_iterator_tag, I>>
    FilterIterator operator--(int) {
        auto it = this;
        operator--();
        return it;
    }

    reference operator*() const { return iterator_.operator*(); }
    pointer operator->() const { return iterator_.operator->(); }

    bool operator==(const FilterIterator& rhs) const { return iterator_ == rhs.iterator_; }
    bool operator!=(const FilterIterator& rhs) const { return iterator_ != rhs.iterator_; }

    const Iter& base() const { return iterator_; }
    Iter& base() { return iterator_; }

    const Pred& predicate() const { return predicate_; }
    Pred& predicate() { return predicate_; }

    FilterIterator end() const { return {predicate_, end_, end_}; }
    FilterIterator<Pred, Iter> begin() const { return *this; }

private:
    void satisfy_predicate() {
        while (iterator_ != end_ && !predicate_(*iterator_)) {
            ++iterator_;
        }
    }
    Pred predicate_;
    Iter iterator_;
    Iter end_;
};

template <typename Pred, typename Iter>
auto makeFilterIterator(Pred&& predicate, Iter begin, Iter end) {
    return FilterIterator<std::remove_reference_t<Pred>, Iter>(std::forward<Pred>(predicate), begin,
                                                               end);
}

}  // namespace inviwo::meta
