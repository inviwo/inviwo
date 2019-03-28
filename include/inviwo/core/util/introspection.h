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

#ifndef IVW_INTROSPECTION_H
#define IVW_INTROSPECTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/document.h>

#include <warn/push>
#include <warn/ignore/all>
#include <type_traits>
#include <iostream>
#include <warn/pop>

namespace inviwo {

namespace util {

template <class T>
class HasClassIdentifierUpper {
    template <class U, class = typename std::enable_if<
                           !std::is_member_pointer<decltype(&U::CLASS_IDENTIFIER)>::value>::type>
    static std::true_type check(int);
    template <class>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<T>(0))::value;
};

template <class T>
class HasClassIdentifierLower {
    template <class U, class = typename std::enable_if<
                           !std::is_member_pointer<decltype(&U::classIdentifier)>::value>::type>
    static std::true_type check(int);
    template <class>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<T>(0))::value;
};

template <class T>
class HasClassIdentifier {
public:
    static const bool value =
        HasClassIdentifierLower<T>::value || HasClassIdentifierUpper<T>::value;
};

template <typename T,
          typename std::enable_if<HasClassIdentifierUpper<T>::value, std::size_t>::type = 0>
std::string classIdentifier() {
    return T::CLASS_IDENTIFIER;
}
template <typename T,
          typename std::enable_if<HasClassIdentifierLower<T>::value, std::size_t>::type = 0>
std::string classIdentifier() {
    return T::classIdentifier;
}
template <typename T, typename std::enable_if<!HasClassIdentifier<T>::value, std::size_t>::type = 0>
std::string classIdentifier() {
    return {};
}

template <class T>
class HasDataName {
    template <class U, class = typename std::enable_if<
                           !std::is_member_pointer<decltype(&U::dataName)>::value>::type>
    static std::true_type check(int);
    template <class>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<T>(0))::value;
};

template <typename T, typename std::enable_if<HasDataName<T>::value, std::size_t>::type = 0>
std::string dataName() {
    return T::dataName;
}
template <typename T, typename std::enable_if<!HasDataName<T>::value, std::size_t>::type = 0>
std::string dataName() {
    return classIdentifier<T>();
}

template <class T>
class HasColorCodeUpper {
    template <class U, class = typename std::enable_if<
                           !std::is_member_pointer<decltype(&U::COLOR_CODE)>::value>::type>
    static std::true_type check(int);
    template <class>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<T>(0))::value;
};
template <class T>
class HasColorCodeLower {
    template <class U, class = typename std::enable_if<
                           !std::is_member_pointer<decltype(&U::colorCode)>::value>::type>
    static std::true_type check(int);
    template <class>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<T>(0))::value;
};
template <class T>
class HasColorCode {
public:
    static const bool value = HasColorCodeLower<T>::value || HasColorCodeUpper<T>::value;
};
template <typename T, typename std::enable_if<HasColorCodeUpper<T>::value, std::size_t>::type = 0>
uvec3 colorCode() {
    return T::COLOR_CODE;
}
template <typename T, typename std::enable_if<HasColorCodeLower<T>::value, std::size_t>::type = 0>
uvec3 colorCode() {
    return T::colorCode;
}
template <typename T, typename std::enable_if<!HasColorCode<T>::value, std::size_t>::type = 0>
uvec3 colorCode() {
    return uvec3(0);
}

template <typename C>
class HasDataInfo {
    template <typename T>
    static auto check(int) ->
        typename std::is_same<decltype(std::declval<T>().getDataInfo()), std::string>::type;

    template <typename T>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<C>(0))::value;
};

template <typename C>
class HasInfo {
    template <typename T>
    static auto check(int) ->
        typename std::is_same<decltype(std::declval<T>().getInfo()), Document>::type;

    template <typename T>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<C>(0))::value;
};

template <typename T, typename std::enable_if<HasDataInfo<T>::value, std::size_t>::type = 0>
std::string data_info(const T* data) {
    return data->getDataInfo();
}
template <typename T, typename std::enable_if<!HasDataInfo<T>::value, std::size_t>::type = 0>
std::string data_info(const T*) {
    return "";
}

template <typename T, typename std::enable_if<!HasInfo<T>::value && HasDataInfo<T>::value,
                                              std::size_t>::type = 0>
Document info(const T& data) {
    Document doc;
    doc.append("p", data.getDataInfo());
    return doc;
}
template <typename T, typename std::enable_if<HasInfo<T>::value && !HasDataInfo<T>::value,
                                              std::size_t>::type = 0>
Document info(const T& data) {
    return data.getInfo();
}
template <typename T, typename std::enable_if<!HasInfo<T>::value && !HasDataInfo<T>::value,
                                              std::size_t>::type = 0>
Document info(const T&) {
    Document doc;
    doc.append("p", dataName<T>());
    return doc;
}

}  // namespace util

}  // namespace inviwo

#endif  // IVW_INTROSPECTION_H
