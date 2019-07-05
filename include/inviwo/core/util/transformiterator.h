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

#include <iterator>
#include <type_traits>
#include <utility>

namespace inviwo {

namespace util {

namespace detail {

template <typename Tag, typename Iter>
struct require : std::enable_if<std::is_base_of<Tag, typename Iter::iterator_category>::value> {};
template <typename Tag, typename Iter>
using require_t = typename require<Tag, Iter>::type;

}  // namespace detail

template <typename Transform, typename Iter>
struct TransformIterator {
    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using iterator_category = typename std::iterator_traits<Iter>::iterator_category;

    using base_value = typename std::iterator_traits<Iter>::value_type;
    using base_reference = typename std::iterator_traits<Iter>::reference;
    using base_pointer = typename std::iterator_traits<Iter>::pointer;

    using reference = std::invoke_result_t<Transform, base_reference>;
    using value_type = std::remove_reference_t<reference>;
    using pointer = std::add_pointer_t<value_type>;

    TransformIterator() = default;
    TransformIterator(Iter iterator) : transform_{}, iterator_(iterator) {}
    TransformIterator(Transform transform, Iter iterator)
        : transform_{std::move(transform)}, iterator_(iterator) {
        static_assert(std::is_same<base_reference, decltype(*std::declval<Iter>())>::value, "");
    }

    TransformIterator& operator++() {
        ++iterator_;
        return *this;
    }
    TransformIterator operator++(int) { return {transform_, iterator_++}; }

    template <typename I = Iter, typename = detail::require_t<std::bidirectional_iterator_tag, I>>
    TransformIterator& operator--() {
        --iterator_;
        return *this;
    }
    template <typename I = Iter, typename = detail::require_t<std::bidirectional_iterator_tag, I>>
    TransformIterator operator--(int) {
        return {transform_, iterator_--};
    }

    template <typename I = Iter, typename = detail::require_t<std::random_access_iterator_tag, I>>
    TransformIterator& operator+=(difference_type rhs) {
        iterator_ += rhs;
        return *this;
    }
    template <typename I = Iter, typename = detail::require_t<std::random_access_iterator_tag, I>>
    TransformIterator& operator-=(difference_type rhs) {
        iterator_ -= rhs;
        return *this;
    }

    template <typename I = Iter, typename = detail::require_t<std::random_access_iterator_tag, I>>
    difference_type operator-(const TransformIterator& rhs) const {
        return iterator_ - rhs.iterator_;
    }
    template <typename I = Iter, typename = detail::require_t<std::random_access_iterator_tag, I>>
    TransformIterator operator+(difference_type i) const {
        auto iter = *this;
        return iter += i;
    }
    template <typename I = Iter, typename = detail::require_t<std::random_access_iterator_tag, I>>
    TransformIterator operator-(difference_type i) const {
        auto iter = *this;
        return iter -= i;
    }

    template <typename I = Iter, typename = detail::require_t<std::random_access_iterator_tag, I>>
    reference operator[](difference_type i) const {
        return *iterator_[i];
    }

    reference operator*() const { return transform_(*iterator_); }
    pointer operator->() const { return &transform_(*iterator_); }

    const Iter& base() const { return iterator_; }
    Iter& base() { return iterator_; }

    bool operator==(const TransformIterator& rhs) const { return iterator_ == rhs.iterator_; }

    bool operator!=(const TransformIterator& rhs) const { return iterator_ != rhs.iterator_; }

    template <typename I = Iter, typename = detail::require_t<std::random_access_iterator_tag, I>>
    bool operator>(const TransformIterator& rhs) const {
        return iterator_ > rhs.iterator_;
    }
    template <typename I = Iter, typename = detail::require_t<std::random_access_iterator_tag, I>>
    bool operator<(const TransformIterator& rhs) const {
        return iterator_ < rhs.iterator_;
    }
    template <typename I = Iter, typename = detail::require_t<std::random_access_iterator_tag, I>>
    bool operator>=(const TransformIterator& rhs) const {
        return iterator_ >= rhs.iterator_;
    }
    template <typename I = Iter, typename = detail::require_t<std::random_access_iterator_tag, I>>
    bool operator<=(const TransformIterator& rhs) const {
        return iterator_ <= rhs.iterator_;
    }

private:
    Transform transform_;
    Iter iterator_;
};

template <typename Transform, typename Iter>
auto makeTransformIterator(Transform&& transform, Iter iter) {
    return TransformIterator<std::remove_reference_t<Transform>, Iter>(
        std::forward<Transform>(transform), iter);
}

}  // namespace util

}  // namespace inviwo
