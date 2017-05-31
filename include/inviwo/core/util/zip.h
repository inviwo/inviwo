/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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
#include <iterator>
#include <utility>
#include <inviwo/core/util/stdextensions.h>


namespace inviwo {

namespace util {

namespace detailzip {

template <typename T, std::size_t... I>
auto beginImpl(T& t, std::index_sequence<I...>) {
    return std::make_tuple(std::begin(std::get<I>(t))...);
}
template <typename... T>
auto begin(std::tuple<T...>& t) {
    return beginImpl(t, std::index_sequence_for<T...>{});
}

template <typename T, std::size_t... I>
auto endImpl(T& t, std::index_sequence<I...>) {
    return std::make_tuple(std::end(std::get<I>(t))...);
}
template <typename... T>
auto end(std::tuple<T...>& t) {
    return endImpl(t, std::index_sequence_for<T...>{});
}

template <typename T, std::size_t... I>
auto refImpl(T& t, std::index_sequence<I...>) -> std::tuple<decltype(*(std::get<I>(t)))...> {
    return std::tuple<decltype(*(std::get<I>(t)))...>{*(std::get<I>(t))...};
}
template <typename... T>
auto ref(std::tuple<T...>& t) -> std::tuple<decltype(*(std::declval<T>()))...> {
    return refImpl(t, std::index_sequence_for<T...>{});
}

template <typename T, std::size_t... I>
auto pointerImpl(T& t, std::index_sequence<I...>)
    -> std::tuple<decltype((std::get<I>(t)).operator->())...> {
    return std::tuple<decltype((std::get<I>(t)).operator->())...>{(std::get<I>(t)).operator->()...};
}
template <typename... T>
auto pointer(std::tuple<T...>& t) -> std::tuple<decltype((std::declval<T>()).operator->())...> {
    return pointerImpl(t, std::index_sequence_for<T...>{});
}
}  // namespace detailzip

template <typename... Iterable>
struct zipper {
    template <typename... T>
    zipper(T&&... args) : iterables_(std::forward<T>(args)...) {}

    struct iterator {
        using Iterators = std::tuple<decltype(
            std::begin(std::declval<typename std::add_lvalue_reference<Iterable>::type>()))...>;
        using Refs = std::tuple<decltype(
            *(std::begin(std::declval<typename std::add_lvalue_reference<Iterable>::type>())))...>;
        using Pointers = std::tuple<decltype(
            (std::begin(std::declval<typename std::add_lvalue_reference<Iterable>::type>())
                 .
                 operator->()))...>;
        iterator(Iterators iterators) : iterators_(iterators) {}

        iterator& operator++() {
            for_each_in_tuple([](auto& iter) { ++iter; }, iterators_);
            return *this;
        }

        iterator operator++(int) {
            iterator i = *this;
            for_each_in_tuple([](auto& iter) { ++iter; }, iterators_);
            return i;
        }

        Refs operator*() {
            return detailzip::ref(iterators_);
        }

        Pointers operator->() {
            return detailzip::pointer(iterators_);
        }

        template<size_t N>
        typename std::tuple_element<N, Iterators>::type & get() {
            return std::get<N>(iterators_);
        }

        bool operator==(const iterator& rhs) const { return iterators_ == rhs.iterators_; }

        bool operator!=(const iterator& rhs) const {
            bool equal = false;
            util::for_each_in_tuple([&](auto& i1, auto& i2) { equal |= i1 == i2; }, iterators_,
                                    rhs.iterators_);
            return !equal;
        }

    private:
        Iterators iterators_;
    };

    iterator begin() { return iterator(detailzip::begin(iterables_)); }
    iterator end() { return iterator(detailzip::end(iterables_)); }

private:
    std::tuple<Iterable...> iterables_;
};


/**
 * Iterate over containers in sync.
 * Example use case:
 * std::vector<int> a(10);
 * std::vector<int> b(10);
 * for (auto&& i : util::zip(a, b)) {
 *      std::cout << std::get<0>(i) << " " << std::get<1>(i) << std::endl;
 * }
 */
template <typename... T>
auto zip(T&&... args) -> zipper<T...> {
    return zipper<T...>(std::forward<T>(args)...);
}

template <typename T>
struct range_generator {
    range_generator(const T& begin, const T& end, const T& inc)
        : begin_(begin), end_(end), inc_(inc) {}

    struct iterator {
        iterator(T& val, T& inc) : val_(val), inc_(inc) {}
        iterator& operator++() {
            val_ += inc_;
            return *this;
        }
        iterator operator++(int) {
            iterator i = *this;
            val_ += inc_;
            return i;
        }

        T& operator*() { return val_; }
        T* operator->() { return &val_; }

        bool operator==(const iterator& rhs) const { return val_ == rhs.val_; }
        bool operator!=(const iterator& rhs) const { return val_ != rhs.val_; }

    private:
        T val_;
        T inc_;
    };

    iterator begin() { return iterator(begin_, inc_); }
    iterator end() { return iterator(end_, inc_); }

private:
    T begin_;
    T end_;
    T inc_;
};

template <typename T>
auto make_range(const T& begin, const T& end, const T& inc) -> range_generator<T> {
    return range_generator<T>(begin, end, inc);
}


}  // namespace util

}  // namespace inviwo

#endif  // IVW_ZIP_H
