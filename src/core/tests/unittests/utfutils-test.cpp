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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/util/utfutils.h>

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <ranges>
#include <cwctype>

using namespace std::string_view_literals;

namespace inviwo {

// ---- CodePointsAdaptor — basic iteration ----

TEST(UtfUtils, CodePointsAdaptorUtf8FunctionCall) {
    // ASCII: each char is one code point
    const std::string_view ascii = "Hello";
    auto cps = detail::codePoints(ascii);
    const std::vector<int32_t> result(cps.begin(), cps.end());
    EXPECT_EQ(result, (std::vector<int32_t>{'H', 'e', 'l', 'l', 'o'}));
}

TEST(UtfUtils, CodePointsAdaptorUtf8PipeSyntax) {
    // Pipe syntax, multi-byte: "é" = U+00E9
    std::string_view u8str = "caf\xC3\xA9";  // "café"
    std::vector<int32_t> result;
    for (const int32_t cp : u8str | detail::codePoints) result.push_back(cp);
    EXPECT_EQ(result, (std::vector<int32_t>{'c', 'a', 'f', 0x00E9}));
}

TEST(UtfUtils, CodePointsAdaptorWChar) {
    std::wstring_view wstr = L"caf\u00E9";
    std::vector<int32_t> result;
    for (const int32_t cp : wstr | detail::codePoints) result.push_back(cp);
    EXPECT_EQ(result, (std::vector<int32_t>{'c', 'a', 'f', 0x00E9}));
}

// ---- CodePointsAdaptor — composed with std::ranges algorithms ----

TEST(UtfUtils, CodePointsWithRangesCountIf) {
    std::string_view u8str = "H\xC3\xA9llo W\xC3\xB6rld";  // Héllo Wörld
    auto nonAscii =
        std::ranges::count_if(u8str | detail::codePoints, [](int32_t cp) { return cp > 127; });
    EXPECT_EQ(nonAscii, 2);
}

TEST(UtfUtils, CodePointsWithRangesEqual) {
    std::string_view u8str = "caf\xC3\xA9";
    std::wstring_view wstr = L"caf\u00E9";
    EXPECT_TRUE(std::ranges::equal(u8str | detail::codePoints, wstr | detail::codePoints));
}

TEST(UtfUtils, CodePointsWithViewsTransform) {
    // All code points lowercased
    std::string_view u8str = "ABC";
    std::vector<int32_t> lower;
    for (auto cp : u8str | detail::codePoints | std::views::transform([](int32_t c) -> int32_t {
                       return detail::codePointToLower(c);
                   })) {
        lower.push_back(cp);
    }
    EXPECT_EQ(lower, (std::vector<int32_t>{'a', 'b', 'c'}));
}

// ---- CaseInsensitiveEqual — same type comparisons ----

TEST(UtfUtils, CaseInsensitiveEqualNarrowNarrow) {
    constexpr CaseInsensitiveEqual eq;
    EXPECT_TRUE(eq("hello"sv, "HELLO"sv));
    EXPECT_TRUE(eq("Hello"sv, "hello"sv));
    EXPECT_FALSE(eq("hello"sv, "world"sv));
    EXPECT_TRUE(eq(""sv, ""sv));
    EXPECT_FALSE(eq("a"sv, ""sv));
}

TEST(UtfUtils, CaseInsensitiveEqualWideWide) {
    constexpr CaseInsensitiveEqual eq;
    EXPECT_TRUE(eq(L"hello"sv, L"HELLO"sv));
    EXPECT_FALSE(eq(L"hello"sv, L"world"sv));
}

// ---- CaseInsensitiveEqual — mixed type comparisons ----

TEST(UtfUtils, CaseInsensitiveEqualMixedNarrowWide) {
    constexpr CaseInsensitiveEqual eq;
    EXPECT_TRUE(eq("hello", L"HELLO"));
    EXPECT_TRUE(eq(L"Hello", "hello"));
    EXPECT_FALSE(eq("hello", L"world"));
}

// ---- CaseInsensitiveLess ----

TEST(UtfUtils, CaseInsensitiveLessBasic) {
    constexpr CaseInsensitiveLess less;
    EXPECT_TRUE(less("apple"sv, "Banana"sv));
    EXPECT_FALSE(less("Banana"sv, "apple"sv));
    EXPECT_FALSE(less("hello"sv, "HELLO"sv));  // equal -> not less
    EXPECT_FALSE(less("HELLO"sv, "hello"sv));
}

TEST(UtfUtils, CaseInsensitiveLessAsMapComparator) {
    std::map<std::string, int, CaseInsensitiveLess> m;
    m["Alice"] = 1;
    m["alice"] = 2;  // overwrites "Alice" — same key under case-insensitive ordering
    EXPECT_EQ(m.size(), 1u);
    EXPECT_EQ(m["ALICE"], 2);
}

TEST(UtfUtils, CaseInsensitiveLessMixed) {
    constexpr CaseInsensitiveLess less;
    EXPECT_TRUE(less("apple", L"BANANA"));
    EXPECT_FALSE(less(L"BANANA", "apple"));
}

// ---- CaseSensitiveEqual ----

TEST(UtfUtils, CaseSensitiveEqualNarrowNarrow) {
    constexpr CaseSensitiveEqual eq;
    EXPECT_TRUE(eq("hello"sv, "hello"sv));
    EXPECT_FALSE(eq("hello"sv, "HELLO"sv));
    EXPECT_FALSE(eq("hello"sv, "world"sv));
}

TEST(UtfUtils, CaseSensitiveEqualMixed) {
    constexpr CaseSensitiveEqual eq;
    EXPECT_TRUE(eq("hello", L"hello"));
    EXPECT_FALSE(eq("hello", L"HELLO"));
}

// ---- CaseSensitiveLess ----

TEST(UtfUtils, CaseSensitiveLessBasic) {
    constexpr CaseSensitiveLess less;
    EXPECT_TRUE(less("apple"sv, "banana"sv));
    EXPECT_FALSE(less("banana"sv, "apple"sv));
    EXPECT_TRUE(less("HELLO"sv, "hello"sv));  // uppercase < lowercase in ASCII
}

// ---- Implicit conversions from std::string, const char*, std::wstring, const wchar_t* ----

TEST(UtfUtils, CaseInsensitiveEqualImplicitConversions) {
    constexpr CaseInsensitiveEqual eq;
    // All of these should compile and work via implicit conversion to string_view/wstring_view
    EXPECT_TRUE(eq("hello", "HELLO"));  // const char* vs const char*
    const auto hello = std::string_view{"hello"};
    EXPECT_TRUE(eq(hello, ("HELLO")));     // string vs const char*
    EXPECT_TRUE(eq("hello", (L"HELLO")));  // const char* vs const wchar_t*
    const auto whello = std::wstring{L"HELLO"};
    EXPECT_TRUE(eq(hello, whello));      // string vs wstring
    EXPECT_TRUE(eq(L"hello", "HELLO"));  // const wchar_t* vs const char*
}

}  // namespace inviwo
