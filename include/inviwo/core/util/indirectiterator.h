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

#ifndef IVW_INDIRECTITERATOR_H
#define IVW_INDIRECTITERATOR_H

#include <iterator>
#include <type_traits>
#include <memory>

namespace inviwo {

namespace util {

namespace detail_indirect {
template <typename Tag, typename Iter>
struct require : std::enable_if<std::is_base_of<Tag, typename Iter::iterator_category>::value> {};
template <typename Tag, typename Iter>
using require_t = typename require<Tag, Iter>::type;

template <typename Iterator>
struct is_const_iterator {
    typedef typename std::iterator_traits<Iterator>::pointer pointer;
    static constexpr bool value = std::is_const<typename std::remove_pointer<pointer>::type>::value;
};

template <typename T>
struct add_const_to_reference {
    using type = const T;
};

template <typename T>
struct add_const_to_reference<T&> {
    using type = const T&;
};

template <typename T>
struct add_const_to_reference<T&&> {
    using type = const T&&;
};

template <typename T>
using add_const_to_reference_t = typename add_const_to_reference<T>::type;

// a utility to get a raw pointer as a const or mutable
template <bool asConst>
struct asPointer {};

template <>
struct asPointer<true> {
    template <typename T>
    static constexpr const T* get(const T* p) {
        return p;
    }
    template <typename T, typename D>
    static constexpr const T* get(const std::unique_ptr<T, D>& p) {
        return p.get();
    }
    template <typename T>
    static constexpr const T* get(const std::shared_ptr<T>& p) {
        return p.get();
    }
};
template <>
struct asPointer<false> {
    template <typename T>
    static constexpr T* get(const T* p) {
        return p;
    }
    template <typename T, typename D>
    static constexpr T* get(const std::unique_ptr<T, D>& p) {
        return p.get();
    }
    template <typename T>
    static constexpr T* get(const std::shared_ptr<T>& p) {
        return p.get();
    }
};

}  // namespace detail_indirect

/**
 * IndirectIterator<typename Iter, bool PropagateConst = true>
 * Iter is underlying iterator to a pointer like type
 * PropagateConst decides if we should treat the value as const if the pointer is const.
 *
 * Example:
 * \code{.cpp}
 * std::vector<std::unique_ptr<int>> vec;
 * for (int i = 0; i < 5; ++i) {
 *     vec.push_back(std::make_unique<int>(i));
 * }
 *
 * auto it = util::makeIndirectIterator(vec.begin());
 * *it = 5; // *it is a int& not a std::make_unique<int>&
 *
 * // note cbegin() return a const_iterator
 * auto const_it = util::makeIndirectIterator<true>(vec.cbegin());
 * *const_it = 5; // will fail since we propagate const from the pointer to the value
 *
 * auto mutable_it = util::makeIndirectIterator<false>(vec.cbegin());
 * *mutable_it = 5; // will work since __don't__ propagate const from the pointer to the value
 *
 * \endcode
 *
 * The use case is to container types that stores items using a vector of pointers, but want to
 * expose an iterator directly to the item not to the pointer.
 * @see makeIndirectIterator
 */
template <typename Iter, bool PropagateConst = true>
struct IndirectIterator {
    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using iterator_category = typename std::iterator_traits<Iter>::iterator_category;

    using base_value = typename std::iterator_traits<Iter>::value_type;
    using value_type = decltype(*std::declval<base_value>());

    static constexpr bool is_const =
        std::conditional_t<PropagateConst, detail_indirect::is_const_iterator<Iter>,
                           std::false_type>::value;

    using base_pointer = typename std::iterator_traits<Iter>::pointer;
    using pointer =
        decltype(detail_indirect::asPointer<is_const>::get(*std::declval<base_pointer>()));

    using base_reference = typename std::iterator_traits<Iter>::reference;
    using reference = std::conditional_t<
        is_const,
        detail_indirect::add_const_to_reference_t<decltype(*std::declval<base_reference>())>,
        decltype(*std::declval<base_reference>())>;

    template <typename Tag, typename Iterables>
    using require_t = detail_indirect::require_t<Tag, Iter>;

    IndirectIterator() = default;
    IndirectIterator(Iter iterator) : iterator_(iterator) {}

    IndirectIterator& operator++() {
        ++iterator_;
        return *this;
    }
    IndirectIterator operator++(int) { return {iterator_++}; }

    template <typename I = Iter, typename = require_t<std::bidirectional_iterator_tag, I>>
    IndirectIterator& operator--() {
        --iterator_;
        return *this;
    }
    template <typename I = Iter, typename = require_t<std::bidirectional_iterator_tag, I>>
    IndirectIterator operator--(int) {
        return {iterator_--};
    }

    template <typename I = Iter, typename = require_t<std::random_access_iterator_tag, I>>
    IndirectIterator& operator+=(difference_type rhs) {
        iterator_ += rhs;
        return *this;
    }
    template <typename I = Iter, typename = require_t<std::random_access_iterator_tag, I>>
    IndirectIterator& operator-=(difference_type rhs) {
        iterator_ -= rhs;
        return *this;
    }

    template <typename I = Iter, typename = require_t<std::random_access_iterator_tag, I>>
    difference_type operator-(const IndirectIterator& rhs) const {
        return iterator_ - rhs.iterator_;
    }
    template <typename I = Iter, typename = require_t<std::random_access_iterator_tag, I>>
    IndirectIterator operator+(difference_type i) const {
        auto iter = *this;
        return iter += i;
    }
    template <typename I = Iter, typename = require_t<std::random_access_iterator_tag, I>>
    IndirectIterator operator-(difference_type i) const {
        auto iter = *this;
        return iter -= i;
    }

    template <typename I = Iter, typename = require_t<std::random_access_iterator_tag, I>>
    reference operator[](difference_type i) const {
        return *iterator_[i];
    }

    reference operator*() const { return **iterator_; }

    pointer operator->() const {
        return detail_indirect::asPointer<is_const>::get(*(iterator_.operator->()));
    }

    const Iter& base() const { return iterator_; }
    Iter& base() { return iterator_; }

    bool operator==(const IndirectIterator& rhs) const { return iterator_ == rhs.iterator_; }

    bool operator!=(const IndirectIterator& rhs) const { return iterator_ != rhs.iterator_; }

    template <typename I = Iter, typename = require_t<std::random_access_iterator_tag, I>>
    bool operator>(const IndirectIterator& rhs) const {
        return iterator_ > rhs.iterator_;
    }
    template <typename I = Iter, typename = require_t<std::random_access_iterator_tag, I>>
    bool operator<(const IndirectIterator& rhs) const {
        return iterator_ < rhs.iterator_;
    }
    template <typename I = Iter, typename = require_t<std::random_access_iterator_tag, I>>
    bool operator>=(const IndirectIterator& rhs) const {
        return iterator_ >= rhs.iterator_;
    }
    template <typename I = Iter, typename = require_t<std::random_access_iterator_tag, I>>
    bool operator<=(const IndirectIterator& rhs) const {
        return iterator_ <= rhs.iterator_;
    }

private:
    Iter iterator_;
};

/**
 * Create an IndirectIterator
 * @see IndirectIterator
 */
template <bool PropagateConst = true, typename Iter>
IndirectIterator<Iter, PropagateConst> makeIndirectIterator(Iter&& iter) {
    return IndirectIterator<Iter, PropagateConst>(std::forward<Iter>(iter));
}

}  // namespace util

}  // namespace inviwo

#endif  // IVW_INDIRECTITERATOR_H
