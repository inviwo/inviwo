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

#ifndef IVW_ENUMTRAITS_H
#define IVW_ENUMTRAITS_H

#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/defaultvalues.h>

#include <type_traits>

namespace inviwo {

template <typename T>
struct EnumTraits {};

namespace util {
// type trait to check if T has an enum name
namespace detail {

template <typename T, class Enable = void>
struct HasEnumName : std::false_type {};

template <typename T>
struct HasEnumName<T, void_t<decltype(EnumTraits<T>::name())>> : std::true_type {};

}  // namespace detail

template <typename T>
struct HasEnumName : detail::HasEnumName<T> {};

template <typename T, typename std::enable_if<HasEnumName<T>::value, std::size_t>::type = 0>
std::string enumName() {
    return EnumTraits<T>::name();
}
template <typename T, typename std::enable_if<!HasEnumName<T>::value, std::size_t>::type = 0>
std::string enumName() {
    return "Enum" + Defaultvalues<std::underlying_type_t<T>>::getName();
}

}  // namespace util
}  // namespace inviwo

#endif  // IVW_ENUMTRAITS_H
