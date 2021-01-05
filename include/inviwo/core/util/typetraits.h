/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <type_traits>
#include <ostream>
#include <warn/pop>

namespace inviwo {

namespace util {

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
template <class, class = std::void_t<>>
struct is_dereferenceable : std::false_type {};
// specialization recognizes types that do support dereferencing:
template <class T>
struct is_dereferenceable<T, std::void_t<decltype(*std::declval<T>())>> : std::true_type {};

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
 * A type trait to determine if type "callback" can be called with certain arguments.
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

}  // namespace util
}  // namespace inviwo
