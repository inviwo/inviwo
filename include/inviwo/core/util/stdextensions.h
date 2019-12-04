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

#ifndef IVW_STDEXTENSIONS_H
#define IVW_STDEXTENSIONS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/hashcombine.h>
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
#include <future>
#include <utility>
#include <tuple>
#include <warn/pop>

namespace inviwo {

namespace util {
// Since make_unique is a c++14 feature, roll our own in the mean time.
template <class T, class... Args>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type make_unique(
    Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
template <class T>
typename std::enable_if<std::is_array<T>::value, std::unique_ptr<T>>::type make_unique(
    std::size_t n) {
    typedef typename std::remove_extent<T>::type RT;
    return std::unique_ptr<T>(new RT[n]());
}

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

template <class...>
using void_t = void;

/**
* Helper struct to allow passing multiple lambda expressions to std::visit.
* Example useage:
* \code{.cpp}
*  std::variant<int, std::string, float, double> data = ...;
*  std::visit(overloaded{[](const int& arg) {   }, // called if data contains an int
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
overloaded(Ts...)->overloaded<Ts...>;

// type trait to check if T is derived from std::basic_string
namespace detail {
template <typename T, class Enable = void>
struct is_string : std::false_type {};

template <typename T>
struct is_string<
    T, void_t<typename T::value_type, typename T::traits_type, typename T::allocator_type>>
    : std::is_base_of<std::basic_string<typename T::value_type, typename T::traits_type,
                                        typename T::allocator_type>,
                      T> {};
}  // namespace detail
template <typename T>
struct is_string : detail::is_string<T> {};

template <typename T, typename V>
auto erase_remove(T& cont, const V& elem)
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
size_t map_erase_remove_if(T& cont, Pred pred) {
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
    typename std::result_of<Callable(typename T::mapped_type)>::type {
    auto it = cont.find(elem);
    if (it != end(cont)) {
        return f(it->second);
    } else {
        return nullptr;
    }
}

template <typename T, typename UnaryPredicate>
bool all_of(const T& cont, UnaryPredicate pred) {
    using std::begin;
    using std::end;
    return std::all_of(begin(cont), end(cont), pred);
}
template <typename T, typename UnaryPredicate>
bool any_of(const T& cont, UnaryPredicate pred) {
    using std::begin;
    using std::end;
    return std::any_of(begin(cont), end(cont), pred);
}
template <typename T, typename UnaryPredicate>
bool none_of(const T& cont, UnaryPredicate pred) {
    using std::begin;
    using std::end;
    return std::none_of(begin(cont), end(cont), pred);
}

template <class Iter>
struct iter_range : std::pair<Iter, Iter> {
    using value_type = typename Iter::value_type;
    using const_iterator = Iter;
    using iterator = Iter;
    using std::pair<Iter, Iter>::pair;
    iter_range(std::pair<Iter, Iter> const& x) : std::pair<Iter, Iter>(x) {}
    Iter begin() const { return this->first; }
    Iter end() const { return this->second; }
};

template <class Iter>
inline iter_range<Iter> as_range(Iter begin, Iter end) {
    return iter_range<Iter>(std::make_pair(begin, end));
}

template <class Iter>
inline iter_range<Iter> as_range(std::pair<Iter, Iter> const& x) {
    return iter_range<Iter>(x);
}

template <class Container>
inline iter_range<typename Container::iterator> as_range(Container& c) {
    using std::begin;
    using std::end;
    return iter_range<typename Container::iterator>(std::make_pair(begin(c), end(c)));
}
template <class Container>
inline iter_range<typename Container::const_iterator> as_range(const Container& c) {
    using std::begin;
    using std::end;
    return iter_range<typename Container::const_iterator>(std::make_pair(begin(c), end(c)));
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
    -> std::vector<typename std::result_of<UnaryOperation(typename T::value_type)>::type> {
    using std::begin;
    using std::end;

    std::vector<typename std::result_of<UnaryOperation(typename T::value_type)>::type> res;
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

template <typename T>
bool is_future_ready(const std::future<T>& future) {
    return (future.valid() && future.wait_for(std::chrono::duration<int, std::milli>(0)) ==
                                  std::future_status::ready);
}

/**
 * A type trait for std container types
 * from: http://stackoverflow.com/a/16316640
 * This is a slightly modified version to avoid constexpr.
 *
 * Requirements on Container T:
 * \code{.cpp}
 *     T::iterator = T::begin();
 *     T::iterator = T::end();
 *     T::const_iterator = T::begin() const;
 *     T::const_iterator = T::end() const;
 *
 *     *T::iterator = T::value_type &
 *     *T::const_iterator = T::value_type const &
 * \endcode
 */

template <typename T>
class is_container {
    using test_type = typename std::remove_const<T>::type;

    template <typename A,
              class = typename std::enable_if<
                  std::is_same<decltype(std::declval<A>().begin()), typename A::iterator>::value &&
                  std::is_same<decltype(std::declval<A>().end()), typename A::iterator>::value &&
                  std::is_same<decltype(std::declval<const A>().begin()),
                               typename A::const_iterator>::value &&
                  std::is_same<decltype(std::declval<const A>().end()),
                               typename A::const_iterator>::value &&
                  std::is_same<decltype(*std::declval<typename A::iterator>()),
                               typename A::value_type&>::value &&
                  std::is_same<decltype(*std::declval<const typename A::iterator>()),
                               typename A::value_type const&>::value>::type>
    static std::true_type test(int);

    template <class>
    static std::false_type test(...);

public:
    static const bool value = decltype(test<test_type>(0))::value;
};

template <class T>
class is_stream_insertable {
    template <typename U, class = typename std::enable_if<std::is_convertible<
                              decltype(std::declval<std::ostream&>() << std::declval<U>()),
                              std::ostream&>::value>::type>
    static std::true_type check(int);
    template <class>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<T>(0))::value;
};

// primary template handles types that do not support dereferencing:
template <class, class = void_t<>>
struct is_dereferenceable : std::false_type {};
// specialization recognizes types that do support dereferencing:
template <class T>
struct is_dereferenceable<T, void_t<decltype(*std::declval<T>())>> : std::true_type {};

/**
 * A type trait to determine if type "Type" is constructible from arguments "Arguments...".
 * Example:
 *     util::is_constructible<MyType, FirstArg, SecondArg>::value
 */
template <typename Type, typename... Arguments>
struct is_constructible {
    template <typename U, decltype(U(std::declval<Arguments>()...))* = nullptr>
    static std::true_type check(int);
    template <class>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<Type>(0))::value;
};

template <typename F, typename... Args>
using is_invocable[[deprecated("Use `std::is_invocable` instead")]] = std::is_invocable<F, Args...>;

template <typename R, typename F, typename... Args>
using is_invocable_r[[deprecated("Use `std::is_invocable_r` instead")]] =
    std::is_invocable_r<F, Args...>;

template <typename F, typename... Args>
using is_callable[[deprecated("Use `std::is_invocable` instead")]] = std::is_invocable<F, Args...>;

/**
 * A type trait to determine if type "callback" cann be called with certain arguments.
 * Example:
 *     util::is_callable_with<float>(callback)
 *     where
 *        callback = [](float){}  -> true
 *        callback = [](std::string){}  -> false
 */
template <typename... A, typename F>
constexpr bool is_callable_with(F&&) {
    return std::is_invocable_v<F, A...>;
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
};

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
};

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

#endif  // IVW_STDEXTENSIONS_H
