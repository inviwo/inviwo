/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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

#include <inviwo/core/util/enumtraits.h>
#include <string>
#include <type_traits>

namespace inviwo {

namespace detail {
template <typename T>
concept pathlike = requires(T t, T b) {
    t.root_directory();
    t.lexically_proximate(b);
    t.generic_string();
    t / b;
};
}  // namespace detail

template <typename T>
struct OptionPropertyTraits {
    static std::string_view classIdentifier() {
        if constexpr (std::is_enum_v<T>) {
            static const std::string identifier =
                fmt::format("org.inviwo.OptionProperty{}", util::enumName<T>());
            return identifier;
        } else if constexpr (detail::pathlike<T>) {
            static constexpr std::string_view identifier = "org.inviwo.OptionPropertyPath";
            return identifier;
        } else {
            static const auto identifier =
                "org.inviwo.OptionProperty" + Defaultvalues<T>::getName();
            return identifier;
        }
    }
};

}  // namespace inviwo
