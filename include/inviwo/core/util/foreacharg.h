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

#ifndef IVW_FOREACHARG_H
#define IVW_FOREACHARG_H

#include <tuple>
#include <initializer_list>
#include <utility>
#include <algorithm>
#include <type_traits>

namespace inviwo {

namespace util {

// https://isocpp.org/blog/2015/01/for-each-arg-eric-niebler
template <class F, class... Args>
auto for_each_argument(F&& f, Args&&... args) {
    return (void)std::initializer_list<int>{0, (f(std::forward<Args>(args)), 0)...},
           std::forward<F>(f);
}

namespace detail {

template <typename F, typename TupleType, size_t... Is>
auto for_each_in_tuple_impl(F&& f, std::index_sequence<Is...>, TupleType&& t) {
    return (void)std::initializer_list<int>{0, (f(std::get<Is>(t)), 0)...}, std::forward<F>(f);
}
template <typename F, typename TupleType1, typename TupleType2, size_t... Is>
auto for_each_in_tuple_impl(F&& f, std::index_sequence<Is...>, TupleType1&& t1, TupleType2&& t2) {
    return (void)std::initializer_list<int>{0, (f(std::get<Is>(t1), std::get<Is>(t2)), 0)...},
           std::forward<F>(f);
}

}  // namespace detail

template <typename F, typename TupleType>
void for_each_in_tuple(F&& f, TupleType&& t) {
    detail::for_each_in_tuple_impl(
        std::forward<F>(f),
        std::make_index_sequence<
            std::tuple_size<typename std::remove_reference<TupleType>::type>::value>{},
        std::forward<TupleType>(t));
}
template <typename F, typename TupleType1, typename TupleType2>
void for_each_in_tuple(F&& f, TupleType1&& t1, TupleType2&& t2) {
    detail::for_each_in_tuple_impl(
        std::forward<F>(f),
        std::make_index_sequence<std::min(
            std::tuple_size<typename std::remove_reference<TupleType1>::type>::value,
            std::tuple_size<typename std::remove_reference<TupleType2>::type>::value)>{},
        std::forward<TupleType1>(t1), std::forward<TupleType2>(t2));
}

/**
 * A utility for iterating over types in a list.
 * Example:
 * \code{.cpp}
 *     struct Functor {
 *         template <typename T>
 *         auto operator()(std::vector<Property*>& properties) {
 *             properties.push_pack(new OrdinalProperty<T>());
 *         }
 *     };
 *     std::vector<Property*>& properties;
 *     using Vec4s = std::tuple<vec4, dvec4, ivec4, size4_t>;
 *     util::for_each_type<Vec4s>{}(Functor{}, properties);
 * \endcode
 */
template <class... Types>
struct for_each_type;

template <typename T>
struct for_each_type<std::tuple<T>> {
    template <class F, class... Args>
    auto operator()(F&& f, Args&&... args) {
// Old versions of VS does not handle this correctly
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 191200000
        f.operator()<T>(std::forward<Args>(args)...);
#else
        f.template operator()<T>(std::forward<Args>(args)...);
#endif
        return std::forward<F>(f);
    }
};

template <class T, class... Types>
struct for_each_type<std::tuple<T, Types...>> {
    template <class F, class... Args>
    auto operator()(F&& f, Args&&... args) {
// Old versions of VS does not handle this correctly
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 191200000
        f.operator()<T>(std::forward<Args>(args)...);
#else
        f.template operator()<T>(std::forward<Args>(args)...);
#endif
        return for_each_type<std::tuple<Types...>>{}(std::forward<F>(f),
                                                     std::forward<Args>(args)...);
    }
};

/**
 * A utility for iterating over all permutations of pairs from two lists of types.
 * Example:
 * \code{.cpp}
 *     struct Functor {
 *         template <typename T, typename U>
 *         auto operator()(std::vector<Converter*>& converters) {
 *             properties.push_pack(new TypeConverter<T, U>());
 *         }
 *     };
 *     std::vector<Converter*>& converters;
 *     using Vec4s = std::tuple<vec4, dvec4, ivec4, size4_t>;
 *     util::for_each_typ_paire<Vec4s, Vec4s>{}(Functor{}, converters);
 * \endcode
 */
template <class ATypes, class BTypes>
struct for_each_type_pair;

template <class... ATypes, class... BTypes>
struct for_each_type_pair<std::tuple<ATypes...>, std::tuple<BTypes...>> {
private:
    template <typename AType>
    struct nestedhelper {
        template <typename BType, class F, class... Args>
        auto operator()(F&& f, Args&&... args) {
// Old versions of VS does not handle this correctly
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 191200000
            f.operator()<AType, BType>(std::forward<Args>(args)...);
#else
            f.template operator()<AType, BType>(std::forward<Args>(args)...);
#endif
        }
    };

    template <typename BTs>
    struct helper {
        template <typename AType, class F, class... Args>
        auto operator()(F&& f, Args&&... args) {
            for_each_type<BTs>{}(nestedhelper<AType>{}, std::forward<F>(f),
                                 std::forward<Args>(args)...);
        }
    };

public:
    template <class F, class... Args>
    auto operator()(F&& f, Args&&... args) {
        for_each_type<std::tuple<ATypes...>>{}(helper<std::tuple<BTypes...>>{}, std::forward<F>(f),
                                               std::forward<Args>(args)...);
    }
};

}  // namespace util

}  // namespace inviwo

#endif  // IVW_FOREACHARG_H
