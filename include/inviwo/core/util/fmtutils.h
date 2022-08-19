/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <fmt/format.h>
#include <inviwo/core/util/detected.h>

namespace inviwo {

template <typename T>
struct FlagFormatter : fmt::formatter<fmt::string_view> {
    template <typename U>
    using HasEnumToStr = decltype(enumToStr(std::declval<U>()));

    template <typename FormatContext>
    auto format(T val, FormatContext& ctx) const {
        if constexpr (util::is_detected_exact_v<std::string_view, HasEnumToStr, T>) {
            return fmt::formatter<fmt::string_view>::format(enumToStr(val), ctx);
        } else {
            static_assert(util::alwaysFalse<T>(),
                          "Missing enumToStr(T val) overload for type T "
                          "FlagFormatter requires that a std::string_view enumToStr(T val) "
                          "overload exists in the namespace of T");
        }
    }
};

template <typename T>
struct FlagsFormatter : fmt::formatter<fmt::string_view> {
    template <typename FormatContext>
    auto format(T val, FormatContext& ctx) const {
        fmt::memory_buffer buff;
        fmt::format_to(std::back_inserter(buff), "{}", fmt::join(val, "+"));
        return formatter<fmt::string_view>::format(fmt::string_view(buff.data(), buff.size()), ctx);
    }
};

}  // namespace inviwo
