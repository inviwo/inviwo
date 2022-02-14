/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <array>
#include <string_view>
#include <type_traits>
#include <string>

#include <fmt/format.h>

namespace inviwo {

template <size_t N>
struct StaticString;

namespace detail {

// Not constexpr in 17
template <class InputIt, class Size, class OutputIt>
constexpr OutputIt copy_n(InputIt first, Size count, OutputIt result) {
    if (count > 0) {
        *result++ = *first;
        for (Size i = 1; i < count; ++i) {
            *result++ = *++first;
        }
    }
    return result;
}

// Not constexpr in 17
template <class InputIt1, class InputIt2>
constexpr bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2) {
    for (; first1 != last1; ++first1, ++first2) {
        if (!(*first1 == *first2)) {
            return false;
        }
    }
    return true;
}

template <typename T>
struct static_size_t;

template <size_t N>
struct static_size_t<StaticString<N>> {
    static constexpr size_t size = N;
};

template <size_t N>
struct static_size_t<char[N]> {
    static_assert(N > 0);
    static constexpr size_t size = N - 1;
};

template <typename T>
constexpr size_t static_size = static_size_t<std::remove_cv_t<std::remove_reference_t<T>>>::size;

}  // namespace detail

/**
 * @brief A compile time string implementation.
 * Supports compile time string concatenation and conversion to std::string, std::string_view and
 * null terminated const char*. The stored string is null terminated.
 */
template <size_t N>
struct StaticString {
    constexpr StaticString() noexcept = default;
    constexpr StaticString(std::string_view sv) noexcept {
        ::inviwo::detail::copy_n(sv.begin(), std::min(N, sv.size()), str.begin());
        str[N] = 0;
    }
    template <typename... Ts>
    constexpr StaticString(Ts&&... strs) {
        size_t offset = 0;
        (
            [&]() {
                ::inviwo::detail::copy_n(std::begin(strs), ::inviwo::detail::static_size<Ts>,
                                         str.data() + offset);
                offset += ::inviwo::detail::static_size<Ts>;
            }(),
            ...);
        str[N] = 0;
    }

    constexpr size_t size() const noexcept { return N; }
    constexpr auto begin() const noexcept { return str.begin(); }
    constexpr auto begin() noexcept { return str.begin(); }
    constexpr auto end() const noexcept { return str.begin() + N; }
    constexpr auto end() noexcept { return str.begin() + N; }

    constexpr operator std::string_view() const noexcept { return {str.data(), N}; }
    constexpr operator fmt::string_view() const noexcept { return {str.data(), N}; }
    constexpr std::string_view view() const noexcept { return {str.data(), N}; }

    std::string string() const { return {str.data(), N}; }
    operator std::string() const noexcept {
        return {str.data(), N};
        ;
    }

    constexpr const char* c_str() const { return str.data(); }

    std::array<char, N + 1> str{0};
};

template <size_t N1, size_t N2>
constexpr bool operator==(const StaticString<N1>& a, const StaticString<N2>& b) noexcept {
    if constexpr (N1 == N2) {
        return detail::equal(a.begin(), a.end(), b.begin());
    } else {
        return false;
    }
}
template <size_t N1, size_t N2>
constexpr bool operator!=(const StaticString<N1>& a, const StaticString<N2>& b) noexcept {
    return a.str != b.str;
}

template <size_t N1, size_t N2>
constexpr bool operator==(const char (&a)[N1], const StaticString<N2>& b) {
    return StaticString{a} == b;
}
template <size_t N1, size_t N2>
constexpr bool operator==(const StaticString<N1>& a, const char (&b)[N2]) {
    return a == StaticString{b};
}

template <size_t N1, size_t N2>
constexpr bool operator!=(const char (&a)[N1], const StaticString<N2>& b) {
    return StaticString{a} != b;
}
template <size_t N1, size_t N2>
constexpr bool operator!=(const StaticString<N1>& a, const char (&b)[N2]) {
    return a != StaticString{b};
}

template <size_t N>
bool operator==(const std::string& a, const StaticString<N>& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end());
}
template <size_t N>
bool operator==(const StaticString<N>& a, const std::string& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end());
}

template <size_t N>
bool operator!=(const std::string& a, const StaticString<N>& b) {
    return !(a == b);
}
template <size_t N>
bool operator!=(const StaticString<N>& a, const std::string& b) {
    return !(a == b);
}

template <size_t N1, size_t N2>
constexpr auto operator+(const StaticString<N1>& a, const StaticString<N2>& b) {
    return StaticString{a, b};
}
template <size_t N1, size_t N2>
constexpr auto operator+(const char (&a)[N1], const StaticString<N2>& b) {
    return StaticString{a, b};
}
template <size_t N1, size_t N2>
constexpr auto operator+(const StaticString<N1>& a, const char (&b)[N2]) {
    return StaticString{a, b};
}

StaticString()->StaticString<0>;
template <typename... Ts>
StaticString(Ts&&... strs) -> StaticString<(::inviwo::detail::static_size<Ts> + ...)>;

// Enable the use of StaticStrings as formats strings in fmt
template <size_t N>
constexpr std::string_view to_string_view(const StaticString<N>& s) {
    return s.view();
}

}  // namespace inviwo

template <size_t N>
struct fmt::formatter<::inviwo::StaticString<N>> : ::fmt::formatter<string_view> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(const ::inviwo::StaticString<N>& str, FormatContext& ctx) {
        return ::fmt::formatter<string_view>::format(str.view(), ctx);
    }
};
