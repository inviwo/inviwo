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

#ifndef IVW_STDEXTENSIONS_H
#define IVW_STDEXTENSIONS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glm.h>
#include <memory>
#include <string>
#include <algorithm>
#include <functional>
#include <vector>

namespace inviwo {

namespace util {
// Since make_unique is a c++14 feature, roll our own in the mean time.
template <class T, class... Args>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T> >::type make_unique(
    Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
template <class T>
typename std::enable_if<std::is_array<T>::value, std::unique_ptr<T> >::type make_unique(
    std::size_t n) {
    typedef typename std::remove_extent<T>::type RT;
    return std::unique_ptr<T>(new RT[n]());
}

// type trait to check if T is derived from std::basic_string
namespace detail {
template <typename T, class Enable = void>
struct is_string : std::false_type {};

template <typename... T>
struct void_helper {
    typedef void type;
};

template <typename T>
struct is_string<T, typename void_helper<typename T::value_type, typename T::traits_type,
                                         typename T::allocator_type>::type>
    : std::is_base_of<std::basic_string<typename T::value_type, typename T::traits_type,
                                        typename T::allocator_type>,
                      T> {};
}
template <typename T>
struct is_string : detail::is_string<T> {};

template <typename T, typename V>
void erase_remove(T& cont, const V& elem) {
    using std::begin;
    using std::end;
    cont.erase(std::remove(begin(cont), end(cont), elem), cont.end());
}

template <typename T, typename Pred>
void erase_remove_if(T& cont, Pred pred) {
    using std::begin;
    using std::end;
    cont.erase(std::remove_if(begin(cont), end(cont), pred), cont.end());
}

template <typename T>
void push_back_unique(T& cont, typename T::value_type elem) {
    using std::begin;
    using std::end;
    if (std::find(begin(cont), end(cont), elem) == cont.end()) {
        cont.push_back(elem);
    }
}

template <typename T, typename V>
bool contains(T& cont, const V& elem) {
    using std::begin;
    using std::end;
    return std::find(begin(cont), end(cont), elem) != end(cont);
}

template <typename T, typename UnaryPredicate>
bool all_of(T& cont, UnaryPredicate pred) {
    using std::begin;
    using std::end;
    return std::all_of(begin(cont), end(cont), pred);
}
template <typename T, typename UnaryPredicate>
bool any_of(T& cont, UnaryPredicate pred) {
    using std::begin;
    using std::end;
    return std::any_of(begin(cont), end(cont), pred);
}
template <typename T, typename UnaryPredicate>
bool none_of(T& cont, UnaryPredicate pred) {
    using std::begin;
    using std::end;
    return std::none_of(begin(cont), end(cont), pred);
}

template <class Iter>
struct iter_range : std::pair<Iter, Iter> {
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


/**
 * A type trait for std container types
 * from: http://stackoverflow.com/a/16316640
 * This is a slightly modified version to avoid constexpr.
 *
 * Requirements on Container T:
 * T::iterator = T::begin();
 * T::iterator = T::end();
 * T::const_iterator = T::begin() const;
 * T::const_iterator = T::end() const;
 * 
 * *T::iterator = T::value_type &
 * *T::const_iterator = T::value_type const &
 */

template <typename T>
class is_container {
    using test_type = typename std::remove_const<T>::type;

    template <
        typename A, class = typename std::enable_if<
            std::is_same<
                decltype(std::declval<A>().begin()),
                typename A::iterator>::value &&
            std::is_same<
                decltype(std::declval<A>().end()),
                typename A::iterator>::value &&
            std::is_same<
                decltype(std::declval<const A>().begin()),
                typename A::const_iterator>::value &&
            std::is_same<
                decltype(std::declval<const A>().end()),
                typename A::const_iterator>::value &&
            std::is_same<
                decltype(*std::declval<typename A::iterator>()),
                typename A::value_type&>::value &&
            std::is_same<
                decltype(*std::declval<const typename A::iterator>()),
                typename A::value_type const&>::value>::type>
    static std::true_type test(int);

    template <class>
    static std::false_type test(...);

public:
    static const bool value = decltype(test<test_type>(0))::value;
};

/**
 *    Function to combine several hash values
 *    http://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
 */
template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}  // namespace util
}  // namespace inviwo

// namespace std {
// template <typename T, glm::precision P, template<typename, glm::precision> class VecType>
// struct hash<VecType<T, P>> {
//     size_t operator()(const VecType<T, P>& v) const {
//         size_t h = 0;
//         for (size_t i = 0; i < inviwo::util::flat_extent<VecType<T, P>>::value; ++i) {
//             T& val = inviwo::util::glmcomp<const VecType<T, P>&>(v, i);
//             inviwo::util::hash_combine(h, val);
//         }
//         return h;
//     }
// };
// 

namespace std {
template <typename T, glm::precision P>
struct hash<glm::detail::tvec2<T, P>> {
    size_t operator()(const glm::detail::tvec2<T, P>& v) const {
        size_t h = 0;
        for (size_t i = 0; i < inviwo::util::flat_extent<glm::detail::tvec2<T, P>>::value; ++i) {
            //T& val = 0 //inviwo::util::glmcomp(v, i);
            inviwo::util::hash_combine(h, v[i]);
        }
        return h;
    }
};

template <typename T, typename U>
struct hash<std::pair<T, U>> {
    size_t operator()(const std::pair<T, U>& p) const {
        size_t h = 0;
        inviwo::util::hash_combine(h, p.first);
        inviwo::util::hash_combine(h, p.second);
        return h;
    }
};


}  // namespace std

#endif  // IVW_STDEXTENSIONS_H
