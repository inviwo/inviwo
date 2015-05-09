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

#include <inviwo/core/common/inviwocoredefine.h>
#include <tuple>
#include <iterator>

namespace inviwo {

namespace util {

namespace detail {

template <size_t N, typename R, typename T, typename... V>
typename std::enable_if<N == 0, R>::type beginimpl(T& t, V... v) {
    return std::tuple<V...>(v...);
}
template <size_t N, typename R, typename T, typename... V>
typename std::enable_if<N != 0, R>::type beginimpl(T& t, V... v) {
    return beginimpl<N - 1, R>(t, std::begin(std::get<N-1>(t)), v...);
}
template <typename... T>
auto getBegin(std::tuple<T...>& t) -> std::tuple<decltype(std::begin(std::declval<T>()))...> {
    return beginimpl<std::tuple_size<std::tuple<T...>>::value,
                     std::tuple<decltype(std::begin(std::declval<T>()))...>>(t);
}

template <size_t N, typename R, typename T, typename... V>
typename std::enable_if<N == 0, R>::type endimpl(T& t, V... v) {
    return std::tuple<V...>(v...);
}
template <size_t N, typename R, typename T, typename... V>
typename std::enable_if<N != 0, R>::type endimpl(T& t, V... v) {
    return endimpl<N - 1, R>(t, std::end(std::get<N-1>(t)), v...);
}
template <typename... T>
auto getEnd(std::tuple<T...>& t) -> std::tuple<decltype(std::end(std::declval<T>()))...> {
    return endimpl<std::tuple_size<std::tuple<T...>>::value,
                     std::tuple<decltype(std::end(std::declval<T>()))...>>(t);
}

template <size_t N, typename T>
typename std::enable_if<N == 0, void>::type incrementimpl(T& t) {
    std::get<N>(t)++;
}
template <size_t N, typename T>
typename std::enable_if<N != 0, void>::type incrementimpl(T& t) {
    std::get<N>(t)++;
    incrementimpl<N - 1>(t);
}
template <typename... T>
void increment(std::tuple<T...>& t) {
    incrementimpl<std::tuple_size<std::tuple<T...>>::value - 1>(t);
}

template <size_t N, typename T, typename V>
typename std::enable_if<N == 0, bool>::type equalimpl(T& t, V& v) {
    return std::get<N>(v) == std::get<N>(t);
}
template <size_t N, typename T, typename V>
typename std::enable_if<N != 0, bool>::type equalimpl(T& t, V& v) {
    return std::get<N>(v) == std::get<N>(t) && equalimpl<N - 1>(t, v);
}
template <typename T, typename V>
bool equal(T& t, V& v) {
    return equalimpl<std::tuple_size<T>::value - 1>(t, v);
}

template <size_t N, typename T, typename V>
typename std::enable_if<N == 0, bool>::type notequalimpl(T& t, V& v) {
    return std::get<N>(v) != std::get<N>(t);
}
template <size_t N, typename T, typename V>
typename std::enable_if<N != 0, bool>::type notequalimpl(T& t, V& v) {
    return std::get<N>(v) != std::get<N>(t) && notequalimpl<N - 1>(t, v);
}
template <typename T, typename V>
bool notequal(T& t, V& v) {
    return notequalimpl<std::tuple_size<T>::value - 1>(t, v);
}

template <size_t N, typename R, typename T, typename... V>
typename std::enable_if<N == 0, R>::type refimpl(T& t, V&&... v) {
    return std::tuple<V...>(std::forward<V>(v)...);
}
template <size_t N, typename R, typename T, typename... V>
typename std::enable_if<N != 0, R>::type refimpl(T& t, V&&... v) {
    return refimpl<N - 1, R>(t, *(std::get<N-1>(t)), std::forward<V>(v)...);
}
template <typename... T>
auto getRef(std::tuple<T...>& t) -> std::tuple<decltype(*(std::declval<T>()))...> {
    return refimpl<std::tuple_size<std::tuple<T...>>::value,
                   std::tuple<decltype(*(std::declval<T>()))...>>(t);
}

}

template <typename... Iterable>
struct zipper {
    template <typename... T>
    zipper(T&&... args) : iterables_(std::forward<T>(args)...) {}

    struct iterator {
        using Iterators = std::tuple<decltype(std::begin(std::declval<Iterable>()))...>;
        using Refs = std::tuple<decltype(*(std::begin(std::declval<Iterable>())))...>;
    
        iterator(std::tuple<decltype(std::begin(std::declval<Iterable>()))...> iterators)
            : iterators_(iterators) {}

        iterator& operator++() {
            detail::increment(iterators_);
            return *this;
        }

        iterator operator++(int) {
            iterator i = *this;
            detail::increment(iterators_);
            return i;
        }

        Refs operator*() {
            return detail::getRef(iterators_);
        }
       
        template<size_t N>
        typename std::tuple_element<N, Iterators>::type & get() {
            return std::get<N>(iterators_);
        }
        
        bool operator==(const iterator& rhs) const {
            return detail::equal(iterators_, rhs.iterators_);
        }

        bool operator!=(const iterator& rhs) const {
            return detail::notequal(iterators_, rhs.iterators_);
        }

    private:
        Iterators iterators_;
    };

    iterator begin() { return iterator(detail::getBegin(iterables_)); }
    iterator end() { return iterator(detail::getEnd(iterables_)); }

private:
    std::tuple<Iterable...> iterables_;
};

template <typename... T>
auto zip(T&&... args) -> zipper<T...> {
    return zipper<T...>(std::forward<T>(args)...);
}

template <typename T>
struct range_generator {
    range_generator(T begin, T end, T inc) : begin_(begin), end_(end), inc_(inc) {}
    
    struct iterator {
        
        iterator(T& val, T& inc) : val_(val), inc_(inc) {}

        iterator& operator++() {
            val_+=inc_;
            return *this;
        }

        iterator operator++(int) {
            iterator i = *this;
            val_+=inc_;
            return i;
        }

        T& operator*() {
            return val_;
        }
       
        bool operator==(const iterator& rhs) const {
            return val_ == rhs.val_;
        }

        bool operator!=(const iterator& rhs) const {
            return val_ != rhs.val_;
        }
        
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


}  // namespace util

}  // namespace inviwo

#endif  // IVW_ZIP_H
