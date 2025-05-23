/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/document.h>

#include <inviwo/core/util/detected.h>
#include <warn/push>
#include <warn/ignore/all>
#include <type_traits>
#include <string>
#include <string_view>
#include <warn/pop>

namespace inviwo::util {

template <typename T>
constexpr std::string_view classIdentifier() {
    if constexpr (requires {
                      { T::classIdentifier } -> std::convertible_to<std::string_view>;
                  }) {
        return T::classIdentifier;
    } else if constexpr (requires {
                             { T::CLASS_IDENTIFIER } -> std::convertible_to<std::string_view>;
                         }) {
        return T::CLASS_IDENTIFIER;
    } else {
        static_assert(util::alwaysFalse<T>(), "ClassIdentifier is missing for type");
        return "Unknown";
    }
}

template <typename T>
constexpr std::string_view dataName() {
    if constexpr (requires {
                      { T::dataName } -> std::convertible_to<std::string_view>;
                  }) {
        return T::dataName;
    } else {
        return classIdentifier<T>();
    }
}

template <typename T>
constexpr uvec3 colorCode() {
    if constexpr (requires {
                      { T::colorCode } -> std::convertible_to<glm::uvec3>;
                  }) {
        return T::colorCode;
    } else if constexpr (requires {
                             { T::COLOR_CODE } -> std::convertible_to<glm::uvec3>;
                         }) {
        return T::COLOR_CODE;
    } else {
        return uvec3(0);
    }
}

template <typename T>
Document info(const T& data) {
    if constexpr (requires(T t) {
                      { t.getInfo() } -> std::convertible_to<Document>;
                  }) {
        return data.getInfo();
    } else if constexpr (requires(T t) {
                             { t.getDataInfo() } -> std::convertible_to<std::string>;
                         }) {
        Document doc;
        doc.append("p", data.getDataInfo());
        return doc;
    } else {
        Document doc;
        doc.append("p", dataName<T>());
        return doc;
    }
}

}  // namespace inviwo::util
