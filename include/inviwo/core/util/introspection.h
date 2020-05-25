/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/document.h>

#include <inviwo/core/util/detected.h>

#include <warn/push>
#include <warn/ignore/all>
#include <type_traits>
#include <iostream>
#include <string>
#include <warn/pop>

namespace inviwo {

namespace util {

namespace detail {

template <typename T>
using upperClassIdentifierType = decltype(T::CLASS_IDENTIFIER);

template <typename T>
using lowerClassIdentifierType = decltype(T::classIdentifier);

template <typename T>
using dataNameType = decltype(T::dataName);

template <typename T>
using colorCodeUpperType = decltype(T::COLOR_CODE);

template <typename T>
using colorCodeLowerType = decltype(T::colorCode);

template <typename T>
using dataInfoType = decltype(std::declval<T>().getDataInfo());

template <typename T>
using infoType = decltype(std::declval<T>().getInfo());

}  // namespace detail

template <class T>
using HasClassIdentifierUpper =
    is_detected_exact<const std::string, detail::upperClassIdentifierType, T>;
template <class T>
using HasClassIdentifierLower =
    is_detected_exact<const std::string, detail::lowerClassIdentifierType, T>;

template <class T>
using HasClassIdentifier = std::disjunction<HasClassIdentifierUpper<T>, HasClassIdentifierLower<T>>;

template <typename T>
std::string classIdentifier() {
    if constexpr (HasClassIdentifierUpper<T>::value) {
        return T::CLASS_IDENTIFIER;
    } else if constexpr (HasClassIdentifierLower<T>::value) {
        return T::classIdentifier;
    } else {
        return {};
    }
}

template <class T>
using HasDataName = is_detected_exact<const std::string, detail::dataNameType, T>;

template <typename T>
std::string dataName() {
    if constexpr (HasDataName<T>::value) {
        return T::dataName;
    } else {
        return classIdentifier<T>();
    }
}

template <class T>
using HasColorCodeUpper = is_detected_exact<uvec3, detail::colorCodeUpperType, T>;
template <class T>
using HasColorCodeLower = is_detected_exact<uvec3, detail::colorCodeLowerType, T>;
template <class T>
using HasColorCode = std::disjunction<HasColorCodeUpper<T>, HasColorCodeLower<T>>;

template <typename T>
uvec3 colorCode() {
    if constexpr (HasColorCodeUpper<T>::value) {
        return T::COLOR_CODE;
    } else if constexpr (HasColorCodeLower<T>::value) {
        return T::colorCode;
    } else {
        return uvec3(0);
    }
}

template <typename T>
using HasDataInfo = is_detected_exact<std::string, detail::dataInfoType, T>;

template <typename T>
using HasInfo = is_detected_exact<Document, detail::infoType, T>;

template <typename T>
std::string data_info(const T* data) {
    if constexpr (HasDataInfo<T>::value) {
        return data->getDataInfo();
    } else {
        return {};
    }
}

template <typename T>
Document info(const T& data) {
    if constexpr (HasInfo<T>::value) {
        return data.getInfo();
    } else if constexpr (HasDataInfo<T>::value) {
        Document doc;
        doc.append("p", data.getDataInfo());
        return doc;
    } else {
        Document doc;
        doc.append("p", dataName<T>());
        return doc;
    }
}

}  // namespace util

}  // namespace inviwo
