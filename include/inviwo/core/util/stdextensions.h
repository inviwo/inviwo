/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/hashcombine.h>
#include <inviwo/core/util/typetraits.h>
#include <inviwo/core/util/iterrange.h>

#include <warn/push>
#include <warn/ignore/all>
#include <memory>
#include <string>
#include <iterator>
#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>
#include <type_traits>
#include <utility>
#include <tuple>
#include <warn/pop>

namespace inviwo {

namespace util {

template <typename Derived, typename Base, typename Del>
std::unique_ptr<Derived, Del> static_unique_ptr_cast(std::unique_ptr<Base, Del>&& p) {
    auto d = static_cast<Derived*>(p.release());
    return std::unique_ptr<Derived, Del>(d, std::move(p.get_deleter()));
}

template <typename Derived, typename Base, typename Del>
std::unique_ptr<Derived, Del> dynamic_unique_ptr_cast(std::unique_ptr<Base, Del>&& p) {
    if (Derived* result = dynamic_cast<Derived*>(p.get())) {
        p.release();
        return std::unique_ptr<Derived, Del>(result, std::move(p.get_deleter()));
    }
    return std::unique_ptr<Derived, Del>(nullptr, p.get_deleter());
}

template <typename Derived, typename Base>
std::unique_ptr<Derived> dynamic_unique_ptr_cast(
    std::unique_ptr<Base, std::default_delete<Base>>&& p) {
    if (Derived* result = dynamic_cast<Derived*>(p.get())) {
        p.release();
        return std::unique_ptr<Derived>(result);
    }
    return std::unique_ptr<Derived>(nullptr);
}

namespace detail {

template <typename Index, typename Functor, Index... Is>
constexpr auto make_array(Functor&& func, std::integer_sequence<Index, Is...>) noexcept
    -> std::array<decltype(func(std::declval<Index>())), sizeof...(Is)> {
    return {{func(Is)...}};
}

}  // namespace detail

template <std::size_t N, typename Index = size_t, typename Functor>
constexpr auto make_array(Functor&& func) noexcept
    -> std::array<decltype(func(std::declval<Index>())), N> {
    return detail::make_array<Index>(std::forward<Functor>(func),
                                     std::make_integer_sequence<Index, N>());
}

/*
 * Like std::ref but for multiple things, returns an array of reference_wrapper<Common>
 */
template <typename Common, typename... Ts>
auto ref(Ts&... args) {
    return std::array<std::reference_wrapper<Common>, sizeof...(Ts)>{args...};
}

// Default construct if possible otherwise return nullptr;
template <typename T, typename std::enable_if<!std::is_abstract<T>::value &&
                                                  std::is_default_constructible<T>::value,
                                              int>::type = 0>
T* defaultConstructType() {
    return new T();
}

template <typename T, typename std::enable_if<std::is_abstract<T>::value ||
                                                  !std::is_default_constructible<T>::value,
                                              int>::type = 0>
T* defaultConstructType() {
    return nullptr;
}

/**
* Helper struct to allow passing multiple lambda expressions to std::visit.
* Example useage:
* \code{.cpp}
*  std::variant<int, std::string, float, double> data = ...;
*  std::visit(util::overloaded{[](const int& arg) {   }, // called if data contains an int
                               [](const std::string &arg) {  }, // called if data contains a string
                               [](const auto& arg) {  }} // use auto to capture "the other types"
                               , data);
*
* \endcode
*
*/
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename T, typename V>
[[deprecated("use std::erase")]] auto erase_remove(T& cont, const V& elem)
    -> decltype(std::distance(std::declval<T>().begin(), std::declval<T>().end())) {
    using std::begin;
    using std::end;
    auto it = std::remove(begin(cont), end(cont), elem);
    auto nelem = std::distance(it, cont.end());
    cont.erase(it, cont.end());
    return nelem;
}

template <typename T, typename Pred>
auto erase_remove_if(T& cont, Pred pred)
    -> decltype(std::distance(std::declval<T>().begin(), std::declval<T>().end())) {
    using std::begin;
    using std::end;
    auto it = std::remove_if(begin(cont), end(cont), pred);
    auto nelem = std::distance(it, cont.end());
    cont.erase(it, cont.end());
    return nelem;
}

template <typename T>
void reverse_erase(T& cont) {
    using std::rbegin;
    using std::rend;
    for (auto it = rbegin(cont); it != rend(cont);) {
        // Erase does not take reverse_iterator so we need to convert it
        it = decltype(it)(cont.erase((++it).base()));
    }
}

template <typename T, typename Pred>
void reverse_erase_if(T& cont, Pred pred) {
    using std::begin;
    using std::end;
    for (auto it = rbegin(cont); it != rend(cont);) {
        if (pred(*it)) {
            // Erase does not take reverse_iterator so we need to convert it
            it = decltype(it)(cont.erase((++it).base()));
        } else {
            ++it;
        }
    }
}

template <typename T, typename Pred>
[[deprecated("use std::erase_if")]] size_t map_erase_remove_if(T& cont, Pred pred) {
    using std::begin;
    using std::end;
    size_t removed{0};
    for (auto it = begin(cont); it != end(cont);) {
        if (pred(*it)) {
            it = cont.erase(it);
            removed++;
        } else {
            ++it;
        }
    }
    return removed;
}

template <typename T>
bool push_back_unique(T& cont, typename T::value_type elem) {
    using std::begin;
    using std::end;
    if (std::find(begin(cont), end(cont), elem) == cont.end()) {
        cont.push_back(elem);
        return true;
    } else {
        return false;
    }
}

template <typename Dst, typename... Srcs>
Dst& append(Dst& dest, Srcs&&... sources) {
    for_each_argument(
        [&](auto&& source) {
            using std::begin;
            using std::end;
            dest.insert(end(dest), begin(source), end(source));
        },
        std::forward<Srcs>(sources)...);
    return dest;
}

template <typename T, typename V>
auto find(T& cont, const V& elem) {
    using std::begin;
    using std::end;
    return std::find(begin(cont), end(cont), elem);
}

template <typename T, typename Pred>
auto find_if(T& cont, Pred pred) -> typename T::iterator {
    using std::begin;
    using std::end;
    return std::find_if(begin(cont), end(cont), pred);
}

template <typename T, typename Pred>
auto find_if(const T& cont, Pred pred) -> typename T::const_iterator {
    return std::find_if(cont.cbegin(), cont.cend(), pred);
}

template <typename T, typename V>
bool contains(T& cont, const V& elem) {
    using std::begin;
    using std::end;
    return std::find(begin(cont), end(cont), elem) != end(cont);
}

template <typename T, typename Pred>
bool contains_if(T& cont, Pred pred) {
    using std::begin;
    using std::end;
    return std::find_if(begin(cont), end(cont), pred) != end(cont);
}

template <typename T, typename V>
bool contains(const T& cont, const V& elem) {
    using std::cbegin;
    using std::cend;
    return std::find(cbegin(cont), cend(cont), elem) != end(cont);
}

template <typename T, typename Pred>
bool contains_if(const T& cont, Pred pred) {
    using std::cbegin;
    using std::cend;
    return std::find_if(cbegin(cont), cend(cont), pred) != end(cont);
}

template <typename T, typename P>
auto find_if_or_null(T& cont, P pred) -> typename T::value_type {
    using std::begin;
    using std::end;

    auto it = std::find_if(begin(cont), end(cont), pred);
    if (it != end(cont)) {
        return *it;
    } else {
        return nullptr;
    }
}

template <typename T, typename V>
auto find_or_null(T& cont, const V& elem) -> typename T::value_type {
    using std::begin;
    using std::end;

    auto it = std::find(begin(cont), end(cont), elem);
    if (it != end(cont)) {
        return *it;
    } else {
        return nullptr;
    }
}

template <typename T, typename V, typename Callable>
auto find_or_null(T& cont, const V& elem, Callable f) -> typename T::value_type {
    using std::begin;
    using std::end;

    auto it = std::find(begin(cont), end(cont), elem);
    if (it != end(cont)) {
        return f(*it);
    } else {
        return nullptr;
    }
}

template <typename T>
bool has_key(T& map, const typename T::key_type& key) {
    return map.find(key) != map.end();
}
template <typename T>
bool insert_unique(T& map, const typename T::key_type& key, typename T::mapped_type& value) {
    return map.insert(std::make_pair(key, value)).second;
}

template <typename T, typename V>
auto map_find_or_null(T& cont, const V& elem) -> typename T::mapped_type {
    auto it = cont.find(elem);
    if (it != end(cont)) {
        return it->second;
    } else {
        return nullptr;
    }
}

template <typename T, typename V, typename Callable>
auto map_find_or_null(T& cont, const V& elem, Callable f) ->
    typename std::invoke_result_t<Callable, typename T::mapped_type> {
    auto it = cont.find(elem);
    if (it != end(cont)) {
        return f(it->second);
    } else {
        return nullptr;
    }
}

template <typename InputIter, typename UnaryPredicate>
[[nodiscard]] constexpr bool all_of(InputIter begin, InputIter end, UnaryPredicate pred) {
    for (; begin != end; ++begin) {
        if (!std::invoke(pred, *begin)) {
            return false;
        }
    }
    return true;
}
template <typename T, typename UnaryPredicate>
[[nodiscard]] constexpr bool all_of(const T& cont, UnaryPredicate pred) {
    using std::begin;
    using std::end;
    return ::inviwo::util::all_of(begin(cont), end(cont), pred);
}

template <typename InputIter, typename UnaryPredicate>
[[nodiscard]] constexpr bool any_of(InputIter begin, InputIter end, UnaryPredicate pred) {
    for (; begin != end; ++begin) {
        if (std::invoke(pred, *begin)) {
            return true;
        }
    }
    return false;
}
template <typename T, typename UnaryPredicate>
[[nodiscard]] constexpr bool any_of(const T& cont, UnaryPredicate pred) {
    using std::begin;
    using std::end;
    return ::inviwo::util::any_of(begin(cont), end(cont), pred);
}

template <typename InputIter, typename UnaryPredicate>
[[nodiscard]] constexpr bool none_of(InputIter begin, InputIter end, UnaryPredicate pred) {
    for (; begin != end; ++begin) {
        if (std::invoke(pred, *begin)) {
            return false;
        }
    }
    return true;
}
template <typename T, typename UnaryPredicate>
[[nodiscard]] constexpr bool none_of(const T& cont, UnaryPredicate pred) {
    using std::begin;
    using std::end;
    return ::inviwo::util::none_of(begin(cont), end(cont), pred);
}

template <typename Iter, typename Proj = identity>
Iter find_not_equal(Iter begin, Iter end, Proj proj = {}) {
    if (begin == end) return end;

    decltype(auto) val = std::invoke(proj, *begin);

    for (auto it = std::next(begin); it != end; ++it) {
        if (val != std::invoke(proj, *it)) return it;
    }
    return end;
}

template <typename T, typename OutIt, typename P>
OutIt copy_if(const T& cont, OutIt out, P pred) {
    using std::begin;
    using std::end;
    return std::copy_if(begin(cont), end(cont), out, pred);
}

template <typename T, typename P>
auto copy_if(const T& cont, P pred) -> std::vector<typename T::value_type> {
    using std::begin;
    using std::end;
    std::vector<typename T::value_type> res;
    std::copy_if(begin(cont), end(cont), std::back_inserter(res), pred);
    return res;
}

template <typename T, typename UnaryOperation>
auto transform(const T& cont, UnaryOperation op)
    -> std::vector<std::invoke_result_t<UnaryOperation, const typename T::value_type>> {
    using std::begin;
    using std::end;

    std::vector<std::invoke_result_t<UnaryOperation, const typename T::value_type>> res;
    res.reserve(std::distance(begin(cont), end(cont)));
    std::transform(begin(cont), end(cont), std::back_inserter(res), op);
    return res;
}

template <typename T, typename Pred>
auto ordering(T& cont, Pred pred) -> std::vector<size_t> {
    using std::begin;
    using std::end;

    std::vector<size_t> res(std::distance(begin(cont), end(cont)));
    std::iota(res.begin(), res.end(), 0);
    std::sort(res.begin(), res.end(), [&](const size_t& a, const size_t& b) {
        return pred(begin(cont)[a], begin(cont)[b]);
    });

    return res;
}

template <typename T>
auto ordering(T& cont) -> std::vector<size_t> {
    return ordering(cont, std::less<typename T::value_type>());
}

template <typename Generator>
auto table(Generator gen, int start, int end, int step = 1)
    -> std::vector<decltype(gen(std::declval<int>()))> {
    using type = decltype(gen(std::declval<int>()));
    std::vector<type> res((end - start) / step);
    size_t count = 0;
    for (int i = start; i < end; i += step) {
        res[count] = gen(i);
        count++;
    }
    return res;
}

/**
 * Get the index of a type in a tuple, returns the index of the first matching type
 */
template <class T, typename Tuple, size_t count = 0>
constexpr size_t index_of() {
    static_assert(count < std::tuple_size_v<Tuple>, "Type T not found in Tuple");
    if constexpr (std::is_same_v<T, std::tuple_element_t<count, Tuple>>) {
        return count;
    } else {
        return index_of<T, Tuple, count + 1>();
    }
}

/**
 * Get the index of the first type in the Tuple that is derived from T
 */
template <class T, typename Tuple, size_t count = 0>
constexpr size_t index_of_derived() {
    static_assert(count < std::tuple_size_v<Tuple>, "Type T not found in Tuple");
    if constexpr (std::is_base_of_v<T, std::tuple_element_t<count, Tuple>>) {
        return count;
    } else {
        return index_of_derived<T, Tuple, count + 1>();
    }
}

namespace hashtuple {

/**
 * Hashing for tuples
 * https://stackoverflow.com/questions/20834838/using-tuple-in-unordered-map
 */
template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
struct HashValueImpl {
    static void apply(size_t& seed, Tuple const& tuple) {
        HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
        hash_combine(seed, std::get<Index>(tuple));
    }
};

template <class Tuple>
struct HashValueImpl<Tuple, 0> {
    static void apply(size_t& seed, Tuple const& tuple) { hash_combine(seed, std::get<0>(tuple)); }
};

}  // namespace hashtuple
}  // namespace util
}  // namespace inviwo

namespace std {

template <typename T, typename U>
struct hash<std::pair<T, U>> {
    size_t operator()(const std::pair<T, U>& p) const {
        size_t h = 0;
        inviwo::util::hash_combine(h, p.first);
        inviwo::util::hash_combine(h, p.second);
        return h;
    }
};

template <typename... TT>
struct hash<std::tuple<TT...>> {
    size_t operator()(std::tuple<TT...> const& tt) const {
        size_t seed = 0;
        inviwo::util::hashtuple::HashValueImpl<std::tuple<TT...>>::apply(seed, tt);
        return seed;
    }
};

template <class ForwardIt>
ForwardIt rotateRetval(ForwardIt first, ForwardIt n_first, ForwardIt last) {
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 9)
    // gcc 4.8.x has no implementation for std::rotate with a return value
    std::rotate(first, n_first, last);
    return first + (last - n_first);
#else
    return std::rotate(first, n_first, last);
#endif
}

}  // namespace std
