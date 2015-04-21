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
#include <memory>
#include <string>
#include <algorithm>

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

template <typename T>
void erase_remove(T& cont, const typename T::value_type& elem) {
    cont.erase(std::remove(cont.begin(), cont.end(), elem), cont.end());
}

template <typename T>
void push_back_unique(T& cont, typename T::value_type elem) {
    if (std::find(cont.begin(), cont.end(), elem) == cont.end()) {
        cont.push_back(elem);
    }
}

template <typename T, typename UnaryPredicate>
bool all_of(T& cont, UnaryPredicate pred) {
    return std::all_of(cont.begin(), cont.end(), pred);
}
template <typename T, typename UnaryPredicate>
bool any_of(T& cont, UnaryPredicate pred) {
    return std::any_of(cont.begin(), cont.end(), pred);
}
template <typename T, typename UnaryPredicate>
bool none_of(T& cont, UnaryPredicate pred) {
    return std::none_of(cont.begin(), cont.end(), pred);
}


}  // namespace
}  // namespace

#endif  // IVW_STDEXTENSIONS_H
