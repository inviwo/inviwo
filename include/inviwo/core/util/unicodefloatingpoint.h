/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <fmt/format.h>

#include <string>
#include <string_view>
#include <array>
#include <ranges>
#include <algorithm>

namespace inviwo {

struct IVW_CORE_API UnicodeFloat {
    double value;
};

namespace detail {

inline auto translateExponent(std::string_view s, auto it) {
    constexpr std::array<std::string_view, 10> superScriptNumbers = {
        "\u2070", "\u00B9", "\u00B2", "\u00B3", "\u2074",
        "\u2075", "\u2076", "\u2077", "\u2078", "\u2079"};

    constexpr std::string_view superScriptPlus = "\u207A";
    constexpr std::string_view superScriptMinus = "\u207B";

    size_t i = 0;
    if (i < s.size() && s[i] == '+') {
        it = std::ranges::copy(superScriptPlus, it).out;
        ++i;
    } else if (i < s.size() && s[i] == '-') {
        it = std::ranges::copy(superScriptMinus, it).out;
        ++i;
    }

    // Skip leading zeros
    while (i < s.size() && s[i] == '0') ++i;

    while (i < s.size() && s[i] >= '0' && s[i] <= '9') {
        it = std::ranges::copy(superScriptNumbers[s[i] - '0'], it).out;
        ++i;
    }

    return it;
}

// True if mantissa equals exactly: "1", "1.", "1.0", "1.000..." or the same
// with leading '+'. If mantissa has leading '-', we do NOT drop it (to avoid
// losing the sign).
inline bool hasUnitMantissa(std::string_view mantissa) {
    if (mantissa.empty()) return false;
    if (mantissa.front() == '-') return false;

    if (mantissa.front() == '+') {
        mantissa.remove_prefix(1);
    }

    if (mantissa == "1") return true;
    if (mantissa == "1.") return true;

    if (!mantissa.starts_with("1.")) return false;
    mantissa.remove_prefix(2);

    if (mantissa.empty()) return true;

    return std::ranges::all_of(mantissa, [](const char c) { return c == '0'; });
}

}  // namespace detail

}  // namespace inviwo

template <>
struct fmt::formatter<inviwo::UnicodeFloat> : fmt::formatter<double> {
    template <typename FormatContext>
    auto format(const inviwo::UnicodeFloat& v, FormatContext& ctx) const -> decltype(ctx.out()) {
        // Format using fmt's native float formatting (so we match 'g' exactly
        // for all specs).
        fmt::basic_memory_buffer<char, 64> buf;
        fmt::basic_format_context<fmt::appender, char> tmpCtx(fmt::appender(buf), ctx.args());
        fmt::formatter<double>::format(v.value, tmpCtx);
        std::string_view s(buf.data(), buf.size());

        // If not scientific form, return unchanged.
        const size_t epos = s.find_last_of("eE");
        if (epos == std::string_view::npos || epos + 1 >= s.size()) {
            return std::ranges::copy(s, ctx.out()).out;
        }

        const std::string_view mantissa = s.substr(0, epos);
        auto it = ctx.out();
        if (!inviwo::detail::hasUnitMantissa(mantissa)) {
            constexpr std::string_view times = "\u00D7";
            it = std::ranges::copy(mantissa, it).out;
            it = std::ranges::copy(times, it).out;
        }
        it = std::ranges::copy(std::string_view{"10"}, it).out;
        it = inviwo::detail::translateExponent(s.substr(epos + 1), it);

        return it;
    }
};
