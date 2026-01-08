/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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
#include <inviwo/core/util/exception.h>

#include <charconv>
#include <string_view>
#include <string>
#include <numeric>
#include <expected>
#include <fmt/base.h>

#if !(defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L)
#include <fast_float/fast_float.h>
#endif

namespace inviwo::util {

template <typename T>
[[nodiscard]] std::expected<T, std::string> fromStr(std::string_view value) {
    const auto* const end = value.data() + value.size();

    T dest{};
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
    auto [p, ec] = std::from_chars(value.data(), end, dest);
#else
    auto [p, ec] = fast_float::from_chars(value.data(), end, dest);
#endif
    if (ec != std::errc() || p != end) {
        if constexpr (std::is_same_v<double, T> || std::is_same_v<float, T>) {
            if (value == "inf") {
                dest = std::numeric_limits<T>::infinity();
            } else if (value == "-inf") {
                dest = -std::numeric_limits<T>::infinity();
            } else if (value == "nan") {
                dest = std::numeric_limits<T>::quiet_NaN();
            } else if (value == "-nan" || value == "-nan(ind)") {
                dest = -std::numeric_limits<T>::quiet_NaN();
            } else {
                return std::unexpected(
                    fmt::format("Error parsing floating point number ({})", value));
            }
        } else {
            return std::unexpected(fmt::format("Error parsing number ({})", value));
        }
    }
    return dest;
}

template <typename T>
void fromStr(std::string_view value, T& dest) {
    dest = fromStr<T>(value)
               .or_else([](const std::string& error) -> std::expected<T, std::string> {
                   throw Exception(SourceContext{}, "{}", error);
               })
               .value();
}

}  // namespace inviwo::util
