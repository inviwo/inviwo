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

#ifndef IVW_INTROSPECTION_H
#define IVW_INTROSPECTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glm.h>
#include <type_traits>

namespace inviwo {

namespace util {

template <class T>
class has_class_identifier {
    template <class U, class = typename std::enable_if<
                           !std::is_member_pointer<decltype(&U::CLASS_IDENTIFIER)>::value>::type>
    static std::true_type check(int);
    template <class>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<T>(0))::value;
};

template <typename T,
          typename std::enable_if<util::has_class_identifier<T>::value, std::size_t>::type = 0>
std::string class_identifier() {
    return T::CLASS_IDENTIFIER;
}
template <typename T,
          typename std::enable_if<!util::has_class_identifier<T>::value, std::size_t>::type = 0>
std::string class_identifier() {
    return std::string{};
}


template <class T>
class has_color_code {
    template <class U, class = typename std::enable_if<
                           !std::is_member_pointer<decltype(&U::COLOR_CODE)>::value>::type>
    static std::true_type check(int);
    template <class>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<T>(0))::value;
};

template <typename T,
          typename std::enable_if<util::has_color_code<T>::value, std::size_t>::type = 0>
uvec3 color_code() {
    return T::COLOR_CODE;
}
template <typename T,
          typename std::enable_if<!util::has_color_code<T>::value, std::size_t>::type = 0>
uvec3 color_code() {
    return uvec3(0);
}

template <typename C>
class has_data_info {
    template <typename T>
    static auto check(int) ->
        typename std::is_same<decltype(std::declval<T>().getDataInfo()), std::string>::type;

    template <typename T>
    static std::false_type check(...);
public:
    static const bool value = decltype(check<C>(0))::value;
};

template <typename T,
          typename std::enable_if<util::has_data_info<T>::value, std::size_t>::type = 0>
std::string data_info(const T* data) {
    return data->getDataInfo();
}
template <typename T,
          typename std::enable_if<!util::has_data_info<T>::value, std::size_t>::type = 0>
std::string data_info(const T*) {
    return "";
}


} // namespace

} // namespace

#endif // IVW_INTROSPECTION_H

