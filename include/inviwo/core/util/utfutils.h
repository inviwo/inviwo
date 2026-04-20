/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <cwctype>
#include <cstdint>
#include <string_view>
#include <ranges>
#include <iterator>
#include <concepts>

#include <utf8cpp/utf8/checked.h>

namespace inviwo {

template <typename R>
concept CharRange = std::ranges::range<R> && std::same_as<std::ranges::range_value_t<R>, char>;

template <typename R>
concept WCharRange = std::ranges::range<R> && std::same_as<std::ranges::range_value_t<R>, wchar_t>;

template <typename R>
concept StringRange = CharRange<R> || WCharRange<R>;

namespace detail {

constexpr int utf8ByteCount(int32_t cp) noexcept {
    if (cp < 0x0) return 0;
    if (cp < 0x0080) return 1;
    if (cp < 0x0800) return 2;
    if (cp < 0x10000) return 3;
    return 4;
}

template <std::input_iterator Iter, std::sentinel_for<Iter> Sentinel>
class Utf8CodePointIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = int32_t;
    using difference_type = std::ptrdiff_t;
    using pointer = const int32_t*;
    using reference = int32_t;

    constexpr Utf8CodePointIterator() = default;
    constexpr Utf8CodePointIterator(Iter it, Sentinel end)
        : it_{it}
        , end_{end}
        , current_{it_ == end ? -1 : static_cast<int32_t>(utf8::peek_next(it_, end_))} {}

    constexpr int32_t operator*() const { return current_; }

    constexpr Utf8CodePointIterator& operator++() {
        std::advance(it_, utf8ByteCount(current_));
        if (it_ != end_) {
            current_ = static_cast<int32_t>(utf8::peek_next(it_, end_));
        } else {
            current_ = -1;
        }
        return *this;
    }
    constexpr Utf8CodePointIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    constexpr bool operator==(const Utf8CodePointIterator& other) const { return it_ == other.it_; }

private:
    Iter it_{};
    Sentinel end_{};
    int32_t current_{-1};
};

template <CharRange View>
class Utf8CodePointRange : public std::ranges::view_interface<Utf8CodePointRange<View>> {
public:
    using Iter = std::ranges::iterator_t<const View>;
    using Sentinel = std::ranges::sentinel_t<const View>;

    constexpr Utf8CodePointRange() = default;
    constexpr explicit Utf8CodePointRange(View view) : view_{std::move(view)} {}

    constexpr Utf8CodePointIterator<Iter, Sentinel> begin() const {
        return {std::ranges::begin(view_), std::ranges::end(view_)};
    }
    constexpr Utf8CodePointIterator<Iter, Sentinel> end() const {
        return {std::ranges::end(view_), std::ranges::end(view_)};
    }

private:
    View view_;
};

template <CharRange Range>
Utf8CodePointRange(Range&&) -> Utf8CodePointRange<std::views::all_t<Range>>;

/**
 * Range adaptor that transforms a range of char (UTF-8) or wchar_t into a range of int32_t
 * Unicode code points. Supports both function-call and pipe ('|') syntax via
 * std::ranges::range_adaptor_closure (C++23).
 *
 * @example
 * for (int32_t cp : myUtf8String | detail::codePoints) { ... }
 * for (int32_t cp : detail::codePoints(myWstring)) { ... }
 */
struct CodePointsAdaptor : std::ranges::range_adaptor_closure<CodePointsAdaptor> {
    constexpr auto operator()(CharRange auto&& r) const {
        return Utf8CodePointRange<std::views::all_t<decltype(r)>>{std::forward<decltype(r)>(r)};
    }
    constexpr auto operator()(WCharRange auto&& r) const {
        return std::forward<decltype(r)>(r) |
               std::views::transform([](wchar_t c) -> int32_t { return static_cast<int32_t>(c); });
    }
};

inline constexpr CodePointsAdaptor codePoints{};

inline constexpr int codePointToLower(int cp) noexcept {
    return cp > std::numeric_limits<unsigned char>::max() ? cp : std::tolower(cp);
}
inline constexpr int noTransform(int cp) noexcept { return cp; }

template <StringRange A, StringRange B, typename Transform>
constexpr auto stringCompare(A&& a, B&& b, Transform transform) {
    auto ra = codePoints(std::forward<A>(a));
    auto rb = codePoints(std::forward<B>(b));
    return std::lexicographical_compare_three_way(
        ra.begin(), ra.end(), rb.begin(), rb.end(),
        [&](int32_t x, int32_t y) { return transform(x) <=> transform(y); });
}

}  // namespace detail

namespace views {
inline constexpr detail::CodePointsAdaptor codePoints{};
}  // namespace views

/**
 * @brief Case-insensitive equality comparison.
 * Supports all combinations of std::string_view (UTF-8) and std::wstring_view.
 * Uses utf8cpp to correctly decode multi-byte UTF-8 sequences.
 */
struct IVW_CORE_API CaseInsensitiveEqual {
    constexpr bool operator()(std::string_view a, std::string_view b) const {
        return detail::stringCompare(a, b, detail::codePointToLower) == 0;
    }
    constexpr bool operator()(std::string_view a, std::wstring_view b) const {
        return detail::stringCompare(a, b, detail::codePointToLower) == 0;
    }
    constexpr bool operator()(std::wstring_view a, std::string_view b) const {
        return detail::stringCompare(a, b, detail::codePointToLower) == 0;
    }
    constexpr bool operator()(std::wstring_view a, std::wstring_view b) const {
        return detail::stringCompare(a, b, detail::codePointToLower) == 0;
    }
    using is_transparent = int;
};

/**
 * @brief Case-insensitive less-than comparison.
 * Suitable for use as comparator in std::map, std::set etc.
 */
struct IVW_CORE_API CaseInsensitiveLess {
    constexpr bool operator()(std::string_view a, std::string_view b) const {
        return detail::stringCompare(a, b, detail::codePointToLower) < 0;
    }
    constexpr bool operator()(std::string_view a, std::wstring_view b) const {
        return detail::stringCompare(a, b, detail::codePointToLower) < 0;
    }
    constexpr bool operator()(std::wstring_view a, std::string_view b) const {
        return detail::stringCompare(a, b, detail::codePointToLower) < 0;
    }
    constexpr bool operator()(std::wstring_view a, std::wstring_view b) const {
        return detail::stringCompare(a, b, detail::codePointToLower) < 0;
    }
    using is_transparent = int;
};

/**
 * @brief Case-sensitive equality comparison.
 * Supports all combinations of std::string_view (UTF-8) and std::wstring_view.
 */
struct IVW_CORE_API CaseSensitiveEqual {
    constexpr bool operator()(std::string_view a, std::string_view b) const {
        return detail::stringCompare(a, b, detail::noTransform) == 0;
    }
    constexpr bool operator()(std::string_view a, std::wstring_view b) const {
        return detail::stringCompare(a, b, detail::noTransform) == 0;
    }
    constexpr bool operator()(std::wstring_view a, std::string_view b) const {
        return detail::stringCompare(a, b, detail::noTransform) == 0;
    }
    constexpr bool operator()(std::wstring_view a, std::wstring_view b) const {
        return detail::stringCompare(a, b, detail::noTransform) == 0;
    }
    using is_transparent = int;
};

/**
 * @brief Case-sensitive less-than comparison.
 * Suitable for use as comparator in ordered containers.
 */
struct IVW_CORE_API CaseSensitiveLess {
    constexpr bool operator()(std::string_view a, std::string_view b) const {
        return detail::stringCompare(a, b, detail::noTransform) < 0;
    }
    constexpr bool operator()(std::string_view a, std::wstring_view b) const {
        return detail::stringCompare(a, b, detail::noTransform) < 0;
    }
    constexpr bool operator()(std::wstring_view a, std::string_view b) const {
        return detail::stringCompare(a, b, detail::noTransform) < 0;
    }
    constexpr bool operator()(std::wstring_view a, std::wstring_view b) const {
        return detail::stringCompare(a, b, detail::noTransform) < 0;
    }
    using is_transparent = int;
};

}  // namespace inviwo
