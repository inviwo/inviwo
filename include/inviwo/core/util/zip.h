/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef IVW_ZIP_H
#define IVW_ZIP_H

#include <tuple>
#include <utility>
#include <iterator>
#include <type_traits>
#include <limits>
#include <inviwo/core/util/stdextensions.h>
/**
 * This file contains convenience functions for iterating.
 * Examples:
 *
 * \code{.cpp}
 * Iterate over containers in sync (@see inviwo::util::zip).
 * std::vector<int> a(10);
 * std::vector<int> b(10);
 * for (auto&& i : util::zip(a, b)) {
 *      std::cout << i.first() << " " << i.second() << std::endl;
 *      // alternatively, get<0>(i) and get<1>(i) can be used
 * }
 * \endcode
 *
 *
 * Enumerate element in a container (@see inviwo::util::enumerate).
 * \code{.cpp}
 * std::vector<int> vec(10);
 * for (auto&& item : util::enumerate(vec)) {
 *      auto&& ind = item.first();
 *      auto&& elem = item.second();
 * }
 * \endcode
 *
 * Iterate over sequence (@see inviwo::util::make_sequence).
 * \code{.cpp}
 * size_t end = 3;
 * size_t inc = 2;
 * for (auto&& i : util::make_sequence(0, end, inc)) {
 *      // iterates over 0 and 2
 * }
 * \endcode
 *
 */

namespace inviwo {

namespace util {

namespace detailzip {

template <typename... Ts>
struct proxy {
    proxy(const proxy&) = default;
    proxy(proxy&&) = default;
    proxy& operator=(const proxy&) = default;
    proxy& operator=(proxy&&) = default;

    template <typename... Us>
    proxy(Us&&... args) : data{std::forward<Us>(args)...} {}

    template <typename... Us>
    proxy<Ts...>(const proxy<Us...>& rhs) : data(rhs.data) {}
    template <typename... Us>
    proxy<Ts...>(proxy<Us...>&& rhs) : data(std::move(rhs.data)) {}

    template <typename... Us>
    proxy& operator=(const proxy<Us...>& rhs) {
        data = rhs.data;
        return this;
    }
    template <typename... Us>
    proxy& operator=(proxy<Us...>&& rhs) {
        data = std::move(rhs.data);
        return *this;
    }

    operator std::tuple<Ts...>&() { return data; }

    template <std::size_t N>
    decltype(auto) get() const {
        return std::get<N>(data);
    }

    template <typename Us = std::tuple<Ts...>,
              class = typename std::enable_if<1 <= std::tuple_size<Us>::value, void>::type>
    decltype(auto) first() const {
        return std::get<0>(data);
    }

    template <typename Us = std::tuple<Ts...>,
              class = typename std::enable_if<2 <= std::tuple_size<Us>::value, void>::type>
    decltype(auto) second() const {
        return std::get<1>(data);
    }

    template <typename Us = std::tuple<Ts...>,
              class = typename std::enable_if<3 <= std::tuple_size<Us>::value, void>::type>
    decltype(auto) third() const {
        return std::get<2>(data);
    }

    using first_type = std::tuple_element_t<0, std::tuple<Ts..., void>>;
    using second_type = std::tuple_element_t<1, std::tuple<Ts..., void, void>>;
    using third_type = std::tuple_element_t<2, std::tuple<Ts..., void, void, void>>;

    std::tuple<Ts...> data;
};

template <typename... Ts, typename... Us>
bool operator==(const proxy<Ts...>& lhs, const proxy<Us...>& rhs) {
    return lhs.data == rhs.data;
}
template <typename... Ts, typename... Us>
bool operator!=(const proxy<Ts...>& lhs, const proxy<Us...>& rhs) {
    return lhs.data != rhs.data;
}
template <typename... Ts, typename... Us>
bool operator>(const proxy<Ts...>& lhs, const proxy<Us...>& rhs) {
    return lhs.data > rhs.data;
}
template <typename... Ts, typename... Us>
bool operator<(const proxy<Ts...>& lhs, const proxy<Us...>& rhs) {
    return lhs.data < rhs.data;
}
template <typename... Ts, typename... Us>
bool operator>=(const proxy<Ts...>& lhs, const proxy<Us...>& rhs) {
    return lhs.data >= rhs.data;
}
template <typename... Ts, typename... Us>
bool operator<=(const proxy<Ts...>& lhs, const proxy<Us...>& rhs) {
    return lhs.data <= rhs.data;
}

template <typename... Ts>
void swap(proxy<Ts...>&& a, proxy<Ts...>&& b) {
    std::swap(a.data, b.data);
}

template <typename T, std::size_t... I>
auto beginImpl(T& t, std::index_sequence<I...>) {
    return std::make_tuple(std::begin(std::get<I>(t))...);
}
template <typename... T>
auto getBegin(std::tuple<T...>& t) {
    return beginImpl(t, std::index_sequence_for<T...>{});
}

template <typename... T>
auto getBegin(const std::tuple<T...>& t) {
    return beginImpl(t, std::index_sequence_for<T...>{});
}

template <typename T, std::size_t... I>
auto endImpl(T& t, std::index_sequence<I...>) {
    return std::make_tuple(std::end(std::get<I>(t))...);
}
template <typename... T>
auto getEnd(std::tuple<T...>& t) {
    return endImpl(t, std::index_sequence_for<T...>{});
}

template <typename... T>
auto getEnd(const std::tuple<T...>& t) {
    return endImpl(t, std::index_sequence_for<T...>{});
}

template <typename T, std::size_t... I>
auto refImpl(const T& t, std::index_sequence<I...>) -> proxy<decltype(*(std::get<I>(t)))...> {
    return proxy<decltype(*(std::get<I>(t)))...>{*(std::get<I>(t))...};
}
template <typename... T>
auto ref(const std::tuple<T...>& t) -> proxy<decltype(*(std::declval<T>()))...> {
    return refImpl(t, std::index_sequence_for<T...>{});
}

template <typename T, typename ptrdiff_t, std::size_t... I>
auto indexImpl(const T& t, ptrdiff_t i, std::index_sequence<I...>)
    -> proxy<decltype(std::get<I>(t)[std::declval<ptrdiff_t>()])...> {
    return proxy<decltype(std::get<I>(t)[std::declval<ptrdiff_t>()])...>{std::get<I>(t)[i]...};
}
template <typename... T, typename ptrdiff_t>
auto index(const std::tuple<T...>& t, ptrdiff_t i)
    -> proxy<decltype(std::declval<T>()[std::declval<ptrdiff_t>()])...> {
    return indexImpl(t, i, std::index_sequence_for<T...>{});
}

template <typename T, std::size_t... I>
auto pointerImpl(const T& t, std::index_sequence<I...>)
    -> std::tuple<decltype((std::get<I>(t)).operator->())...> {
    return std::tuple<decltype((std::get<I>(t)).operator->())...>{(std::get<I>(t)).operator->()...};
}
template <typename... T>
auto pointer(const std::tuple<T...>& t)
    -> std::tuple<decltype((std::declval<T>()).operator->())...> {
    return pointerImpl(t, std::index_sequence_for<T...>{});
}

template <typename T>
struct get_iterator {
    using type = decltype(std::begin(std::declval<std::add_lvalue_reference_t<T>>()));
};
template <typename T>
using get_iterator_t = typename get_iterator<T>::type;

template <typename T>
struct iterator_tools;

template <typename... Ts>
struct iterator_tools<std::tuple<Ts...>> {
    using difference_type = std::ptrdiff_t;
    using iterator_category = typename std::common_type<
        typename std::iterator_traits<get_iterator_t<Ts>>::iterator_category...>::type;
    using value_type = proxy<typename std::iterator_traits<get_iterator_t<Ts>>::value_type...>;
    using pointer = std::tuple<typename std::iterator_traits<get_iterator_t<Ts>>::pointer...>;
    using reference = proxy<typename std::iterator_traits<get_iterator_t<Ts>>::reference...>;
    using iterators = std::tuple<get_iterator_t<Ts>...>;
};

template <typename Tag, typename IterTuple>
struct require
    : std::enable_if<
          std::is_base_of<Tag, typename iterator_tools<IterTuple>::iterator_category>::value> {};
template <typename Tag, typename IterTuple>
using require_t = typename require<Tag, IterTuple>::type;

template <typename Iterables>
struct zipIterator {
    using Iterators = typename detailzip::iterator_tools<Iterables>::iterators;
    using difference_type = typename detailzip::iterator_tools<Iterables>::difference_type;
    using iterator_category = typename detailzip::iterator_tools<Iterables>::iterator_category;
    using value_type = typename detailzip::iterator_tools<Iterables>::value_type;
    using pointer = typename detailzip::iterator_tools<Iterables>::pointer;
    using reference = typename detailzip::iterator_tools<Iterables>::reference;

    static_assert(std::is_base_of<std::input_iterator_tag, iterator_category>::value,
                  "All iterators have to be at least input iterators");

    template <typename Tag, typename IterTuple>
    using require_t = detailzip::require_t<Tag, IterTuple>;

    zipIterator() = default;
    zipIterator(Iterators iterators) : iterators_(iterators) {}

    zipIterator& operator++() {
        for_each_in_tuple([](auto& iter) { ++iter; }, iterators_);
        return *this;
    }
    zipIterator operator++(int) {
        auto i = *this;
        for_each_in_tuple([](auto& iter) { ++iter; }, iterators_);
        return i;
    }

    template <typename I = Iterables, typename = require_t<std::bidirectional_iterator_tag, I>>
    zipIterator& operator--() {
        for_each_in_tuple([](auto& iter) { --iter; }, iterators_);
        return *this;
    }
    template <typename I = Iterables, typename = require_t<std::bidirectional_iterator_tag, I>>
    zipIterator operator--(int) {
        auto i = *this;
        for_each_in_tuple([](auto& iter) { --iter; }, iterators_);
        return i;
    }

    template <typename I = Iterables, typename = require_t<std::random_access_iterator_tag, I>>
    zipIterator& operator+=(difference_type rhs) {
        for_each_in_tuple([&](auto& iter) { iter += rhs; }, iterators_);
        return *this;
    }
    template <typename I = Iterables, typename = require_t<std::random_access_iterator_tag, I>>
    zipIterator& operator-=(difference_type rhs) {
        for_each_in_tuple([&](auto& iter) { iter -= rhs; }, iterators_);
        return *this;
    }

    template <typename I = Iterables, typename = require_t<std::random_access_iterator_tag, I>>
    difference_type operator-(const zipIterator& rhs) const {
        difference_type dist{std::numeric_limits<difference_type>::max()};
        for_each_in_tuple([&](auto& i1, auto& i2) { dist = std::min(dist, i1 - i2); }, iterators_,
                          rhs.iterators_);
        return dist;
    }
    template <typename I = Iterables, typename = require_t<std::random_access_iterator_tag, I>>
    zipIterator operator+(difference_type rhs) const {
        auto i = *this;
        return i += rhs;
    }
    template <typename I = Iterables, typename = require_t<std::random_access_iterator_tag, I>>
    zipIterator operator-(difference_type rhs) const {
        auto i = *this;
        return i -= rhs;
    }

    template <typename I = Iterables, typename = require_t<std::random_access_iterator_tag, I>>
    reference operator[](difference_type rhs) const {
        return detailzip::index(iterators_, rhs);
    }

    reference operator*() const { return detailzip::ref(iterators_); }

    pointer operator->() const { return detailzip::pointer(iterators_); }

    template <size_t N>
    typename std::tuple_element<N, Iterators>::type& get() {
        return std::get<N>(iterators_);
    }

    /**
     * Should be true if __any__ underlaying iterators are equal.
     */
    bool operator==(const zipIterator& rhs) const {
        bool equal = false;
        for_each_in_tuple([&](auto& i1, auto& i2) { equal |= i1 == i2; }, iterators_,
                          rhs.iterators_);
        return equal;
    }

    /**
     * Should be true if __all__ underlaying iterators are not equal.
     */
    bool operator!=(const zipIterator& rhs) const {
        bool notEqual = true;
        for_each_in_tuple([&](auto& i1, auto& i2) { notEqual &= i1 != i2; }, iterators_,
                          rhs.iterators_);
        return notEqual;
    }

    /*
     * Since all underlaying iterators are random access iterators, >,<,>=,<= should return the same
     * result for all of the iterators, for convenience we just use the first one.
     */
    template <typename I = Iterables, typename = require_t<std::random_access_iterator_tag, I>>
    bool operator>(const zipIterator& rhs) const {
        return std::get<0>(iterators_) > std::get<0>(rhs.iterators_);
    }
    template <typename I = Iterables, typename = require_t<std::random_access_iterator_tag, I>>
    bool operator<(const zipIterator& rhs) const {
        return std::get<0>(iterators_) < std::get<0>(rhs.iterators_);
    }
    template <typename I = Iterables, typename = require_t<std::random_access_iterator_tag, I>>
    bool operator>=(const zipIterator& rhs) const {
        return std::get<0>(iterators_) >= std::get<0>(rhs.iterators_);
    }
    template <typename I = Iterables, typename = require_t<std::random_access_iterator_tag, I>>
    bool operator<=(const zipIterator& rhs) const {
        return std::get<0>(iterators_) <= std::get<0>(rhs.iterators_);
    }

    Iterators iterators_;
};

template <typename Iterables, typename = require_t<std::random_access_iterator_tag, Iterables>>
zipIterator<Iterables> operator+(typename detailzip::iterator_tools<Iterables>::difference_type lhs,
                                 const zipIterator<Iterables>& rhs) {
    return rhs + lhs;
}

template <typename... Iterable>
struct zipper {
    using Iterables = std::tuple<Iterable...>;
    using iterator = zipIterator<Iterables>;
    using const_iterator = zipIterator<Iterables>;

    using value_type = typename zipIterator<Iterables>::value_type;

    template <typename... T>
    zipper(T&&... args) : iterables_(std::forward<T>(args)...) {}

    auto begin() -> iterator { return iterator(getBegin(iterables_)); }
    auto end() -> iterator { return iterator(getEnd(iterables_)); }

    auto begin() const -> const_iterator { return const_iterator(getBegin(iterables_)); }
    auto end() const -> const_iterator { return const_iterator(getEnd(iterables_)); }

    Iterables iterables_;
};

}  // namespace detailzip

/**
 * Iterate over containers in sync.
 * Example use case 1:
 * \code{.cpp}
 * std::vector<int> a(10);
 * std::vector<int> b(10);
 * for (auto&& i : util::zip(a, b)) {
 *      std::cout << i.first() << " " << i.second() << std::endl;
 *      // alternatively, get<0>(i) and get<1>(i) can be used
 * }
 * \endcode
 *
 * with C++17 structured bindings:
 * \code{.cpp}
 * for (auto&& [i, j] : util::enumerate(vec)) {
 *      std::cout << i << " " << j << std::endl;
 * }
 * \endcode
 */
template <typename... T>
auto zip(T&&... args) -> detailzip::zipper<T...> {
    return detailzip::zipper<T...>(std::forward<T>(args)...);
}

template <typename T>
struct sequence {
    sequence(const T& begin, const T& end, const T& inc) : begin_(begin), end_(end), inc_(inc) {}

    struct iterator {
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::random_access_iterator_tag;

        iterator() = default;

        iterator(const T& val, const T& end, const T& inc) : val_{val}, end_{end}, inc_{inc} {}
        iterator& operator++() {
            val_ += inc_;
            clamp();
            return *this;
        }
        iterator operator++(int) {
            auto i = *this;
            val_ += inc_;
            clamp();
            return i;
        }
        iterator& operator--() {
            val_ -= inc_;
            clamp();
            return *this;
        }
        iterator operator--(int) {
            auto i = *this;
            val_ -= inc_;
            clamp();
            return i;
        }

        iterator& operator+=(difference_type rhs) {
            val_ += rhs * inc_;
            clamp();
            return *this;
        }
        iterator& operator-=(difference_type rhs) {
            val_ -= rhs * inc_;
            clamp();
            return *this;
        }

        difference_type operator-(const iterator& rhs) const { return ((val_ - rhs.val_) / inc_); }
        iterator operator+(difference_type rhs) const {
            auto i = *this;
            return i += rhs;
        }
        iterator operator-(difference_type rhs) const {
            auto i = *this;
            return i -= rhs;
        }
        friend iterator operator+(difference_type lhs, const iterator& rhs) { return rhs + lhs; }

        value_type operator[](difference_type rhs) const { return val_ + rhs * inc_; }

        reference operator*() const { return val_; }

        pointer operator->() const { return &val_; }

        bool operator==(const iterator& rhs) const { return val_ == rhs.val_; }
        bool operator!=(const iterator& rhs) const { return val_ != rhs.val_; }
        bool operator>(const iterator& rhs) const { return val_ > rhs.val_; }
        bool operator<(const iterator& rhs) const { return val_ < rhs.val_; }
        bool operator>=(const iterator& rhs) const { return val_ >= rhs.val_; }
        bool operator<=(const iterator& rhs) const { return val_ <= rhs.val_; }

    private:
        void clamp() {
            using std::max;
            using std::min;
            val_ = inc_ > 0 ? min(val_, end_) : max(val_, end_);
        }
        T val_{};
        T end_{};
        T inc_{};
    };

    using value_type = typename iterator::value_type;
    using const_iterator = iterator;

    iterator begin() const { return iterator(begin_, end_, inc_); }
    iterator end() const { return iterator(end_, end_, inc_); }

private:
    T begin_;
    T end_;
    T inc_;
};
/**
 * Convenvience function for creating a sequence.
 * Use case example:
 * \code{.cpp}
 * auto inc = 2; auto end = 3;
 * for (auto&& i : util::make_sequence(0, end, inc)) {
 *   // Iterates over 0 and 2
 * }
 * \endcode
 */
template <typename T>
auto make_sequence(const T& begin, const T& end, const T& inc = T{1}) -> sequence<T> {
    return sequence<T>(begin, end, inc);
}

/**
 * Enumerate element in a container.
 * Example use case:
 * std::vector<int> vec(10);
 * for (auto&& item : util::enumerate(vec)) {
 *      auto&& ind = item.first();
 *      auto&& elem = item.second();
 *      // alternatively, get<0>(item) and get<1>(item) can be used
 * }
 *
 * with C++17 structured bindings
 * for (auto&& [ind, elem] : util::enumerate(vec)) {
 *
 * }
 */
template <typename T, typename... Ts>
auto enumerate(T&& cont, Ts&&... conts) {
    return zip(sequence<size_t>(0u, std::numeric_limits<size_t>::max(), 1u), std::forward<T>(cont),
               std::forward<Ts>(conts)...);
}

}  // namespace util

// Get function for the proxy class. std::get is not a customization point
template <std::size_t N, typename... Ts>
decltype(auto) get(const util::detailzip::proxy<Ts...>& p) {
    return p.template get<N>();
}

}  // namespace inviwo

namespace std {

// Needed to get structured bindings to work for the proxy class
// enables for(auto&& [i, j] : zip(a, b)) std::cout << i << " " << j << std::endl;
// in C++17
template <typename... Ts>
struct tuple_size<::inviwo::util::detailzip::proxy<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <std::size_t N, typename... Ts>
struct tuple_element<N, ::inviwo::util::detailzip::proxy<Ts...>> {
    using type =
        decltype(std::declval<::inviwo::util::detailzip::proxy<Ts...>>().template get<N>());
};
}  // namespace std

#endif  // IVW_ZIP_H
