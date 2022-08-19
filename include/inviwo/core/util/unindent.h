/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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
#include <inviwo/core/util/staticstring.h>

#include <string>

namespace inviwo {

namespace indent {

/**
 * Go over a string from begin to end and call action for each char, skipping the leading
 * indentation of each line.
 * Example:
 * ```{.cpp}
 * R"(
 *     0123456789
 *     abcdefghij
 *         asdfasdfas
 *     )";
 * ```
 * becomes
 * ```
 * R"(0123456789
 * abcdefghij
 *     asdfasdfas
 * )"
 * ```
 * The first indented line determines the amount of indentation to remove. If any of the following
 * lines have less indentation a runtime_error will thrown.
 */
template <bool ThrowOnError, typename It, typename Func>
constexpr void withoutIndent(It begin, It end, Func&& action) {
    if (begin == end) return;

    auto p = begin;
    if (*p == '\n') {
        ++p;  // ignore empty first line
    } else {
        while (p != end && *p != '\n') {
            action(*p);
            ++p;
        }
        if (p == end) return;
        action(*p);  // consume first newline
        ++p;
    }
    // measure indent
    size_t indent = 0;
    while (p != end && *p == ' ') {
        ++indent;
        ++p;
    }
    while (p != end) {
        action(*p);
        if (*p++ == '\n') {
            for (size_t i = 0; i < indent; ++i) {
                if (p == end) return;
                if (*p != ' ') {
                    if constexpr (ThrowOnError) {
                        throw std::runtime_error("invalid indentation");
                    } else {
                        continue;
                    }
                }
                ++p;
            }
        }
    }
}

/**
 * The length of the string after removing the leading indentation of each line
 */
template <typename It>
constexpr size_t length(It begin, It end) {
    size_t len = 0;
    withoutIndent<false>(begin, end, [&](char) { ++len; });
    return len;
}
template <size_t N>
constexpr size_t length(const StaticString<N>& str) {
    return length(str.begin(), str.end());
}
template <size_t N>
constexpr size_t length(const char (&str)[N]) {
    return length(str, str + N - 1);
}

/**
 * Copy the chars of the string (being, end) to `out` skipping the leading indentation of each line
 */
template <typename It, typename Out>
constexpr size_t copy(It begin, It end, Out out) {
    size_t len = 0;
    withoutIndent<true>(begin, end, [&](char c) { *out++ = c; });
    return len;
}

template <size_t M, size_t N>
constexpr auto unindent(StaticString<N> str) {
    StaticString<M> res{};
    indent::copy(str.begin(), str.end(), res.begin());
    return res;
}
template <size_t M>
constexpr auto unindent(std::string_view str) {
    StaticString<M> res{};
    indent::copy(str.begin(), str.end(), res.begin());
    return res;
}

inline std::string unindent(std::string_view str) {
    const auto newlen = indent::length(str.begin(), str.end());
    std::string res(newlen, char{});
    indent::copy(str.begin(), str.end(), res.begin());
    return res;
}

}  // namespace indent

/**
 * Remove the leading indentation from each line of the string.
 * The first indented line determines the amount of indentation to remove. If any of the following
 * lines have less indentation a runtime_error will thrown.
 *
 * Example:
 * ```{.cpp}
 * auto str = R"(
 *     0123456789
 *     abcdefghij
 *         asdfasdfas
 *     )"_unindent;
 * ```
 * becomes
 * ```
 * 0123456789
 * abcdefghij
 *     asdfasdfas
 * ```
 * str will become an std::string
 */
inline std::string operator"" _unindent(const char* str, size_t len) {
    const auto newlen = indent::length(str, str + len);
    std::string res(newlen, char{});
    indent::copy(str, str + len, res.begin());
    return res;
}

/**
 * Remove leading indentation from each line of the string.
 * The first indented line determines the amount of indentation to remove, If any of the following
 * lines as less indentation a runtime_error will thrown
 *
 * Example:
 * ```{.cpp}
 * constexpr auto str = IVW_UNINDENT(R"(
 *     0123456789
 *     abcdefghij
 *     asdfasdfas
 *     )");
 * ```
 * becomes
 * ```
 * 0123456789
 * abcdefghij
 * asdfasdfas
 * ```
 * str will be come a `StaticString` at compile time
 */
#define IVW_UNINDENT(x) indent::unindent<indent::length(x)>(x)

}  // namespace inviwo
