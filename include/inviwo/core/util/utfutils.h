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

namespace detail {

template <typename R>
concept CharRange = std::ranges::range<R> && std::same_as<std::ranges::range_value_t<R>, char>;

template <typename R>
concept WCharRange =
    std::ranges::range<R> && std::same_as<std::ranges::range_value_t<R>, wchar_t>;

template <typename R>
concept StringRange = CharRange<R> || WCharRange<R>;

template <std::input_iterator Iter, std::sentinel_for<Iter> Sentinel>
class Utf8CodePointIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = int32_t;
    using difference_type = std::ptrdiff_t;
    using pointer = const int32_t*;
    using reference = int32_t;

    Utf8CodePointIterator() = default;
    Utf8CodePointIterator(Iter it, Sentinel end) : it_{it}, end_{end} {
        if (it_ != end_) decode();
    }

    int32_t operator*() const { return current_; }

    Utf8CodePointIterator& operator++() {
        if (it_ != end_)
            decode();
        else
            current_ = -1;
        return *this;
    }
    Utf8CodePointIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const Utf8CodePointIterator& other) const { return it_ == other.it_; }
    bool operator!=(const Utf8CodePointIterator& other) const { return !(*this == other); }

private:
    void decode() { current_ = static_cast<int32_t>(utf8::next(it_, end_)); }
    Iter it_{};
    Sentinel end_{};
    int32_t current_{-1};
};

template <CharRange R>
class Utf8CodePointRange : public std::ranges::view_interface<Utf8CodePointRange<R>> {
public:
    using Iter = std::ranges::iterator_t<R>;
    using Sentinel = std::ranges::sentinel_t<R>;

    Utf8CodePointRange() = default;
    explicit Utf8CodePointRange(R&& r)
        : begin_{std::ranges::begin(r)}, end_{std::ranges::end(r)} {}
    explicit Utf8CodePointRange(R& r)
        : begin_{std::ranges::begin(r)}, end_{std::ranges::end(r)} {}

    auto begin() const { return Utf8CodePointIterator<Iter, Sentinel>{begin_, end_}; }
    auto end() const { return Utf8CodePointIterator<Iter, Sentinel>{end_, end_}; }

private:
    Iter begin_{};
    Sentinel end_{};
};

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
    auto operator()(CharRange auto&& r) const {
        return Utf8CodePointRange<decltype(r)>{std::forward<decltype(r)>(r)};
    }
    auto operator()(WCharRange auto&& r) const {
        return std::forward<decltype(r)>(r) |
               std::views::transform([](wchar_t c) -> int32_t { return static_cast<int32_t>(c); });
    }
};

inline constexpr CodePointsAdaptor codePoints{};

inline std::wint_t toLower(int32_t cp) {
    return std::towlower(static_cast<std::wint_t>(cp));
}
inline int32_t identity(int32_t cp) { return cp; }

template <StringRange A, StringRange B, typename Transform>
int compareImpl(A&& a, B&& b, Transform transform) {
    auto ra = codePoints(std::forward<A>(a));
    auto rb = codePoints(std::forward<B>(b));
    auto ait = std::ranges::begin(ra), aend = std::ranges::end(ra);
    auto bit = std::ranges::begin(rb), bend = std::ranges::end(rb);
    while (ait != aend && bit != bend) {
        auto cpA = transform(*ait);
        auto cpB = transform(*bit);
        if (cpA != cpB) return (cpA < cpB) ? -1 : 1;
        ++ait;
        ++bit;
    }
    if (ait != aend) return 1;
    if (bit != bend) return -1;
    return 0;
}

}  // namespace detail

/**
 * @brief Case-insensitive equality comparison.
 * Supports all combinations of std::string_view (UTF-8) and std::wstring_view.
 * Uses utf8cpp to correctly decode multi-byte UTF-8 sequences.
 */
struct IVW_CORE_API CaseInsensitiveEqual {
    bool operator()(std::string_view a, std::string_view b) const {
        return detail::compareImpl(a, b, detail::toLower) == 0;
    }
    bool operator()(std::string_view a, std::wstring_view b) const {
        return detail::compareImpl(a, b, detail::toLower) == 0;
    }
    bool operator()(std::wstring_view a, std::string_view b) const {
        return detail::compareImpl(a, b, detail::toLower) == 0;
    }
    bool operator()(std::wstring_view a, std::wstring_view b) const {
        return detail::compareImpl(a, b, detail::toLower) == 0;
    }
    using is_transparent = int;
};

/**
 * @brief Case-insensitive less-than comparison.
 * Suitable for use as comparator in std::map, std::set etc.
 */
struct IVW_CORE_API CaseInsensitiveLess {
    bool operator()(std::string_view a, std::string_view b) const {
        return detail::compareImpl(a, b, detail::toLower) < 0;
    }
    bool operator()(std::string_view a, std::wstring_view b) const {
        return detail::compareImpl(a, b, detail::toLower) < 0;
    }
    bool operator()(std::wstring_view a, std::string_view b) const {
        return detail::compareImpl(a, b, detail::toLower) < 0;
    }
    bool operator()(std::wstring_view a, std::wstring_view b) const {
        return detail::compareImpl(a, b, detail::toLower) < 0;
    }
    using is_transparent = int;
};

/**
 * @brief Case-sensitive equality comparison.
 * Supports all combinations of std::string_view (UTF-8) and std::wstring_view.
 */
struct IVW_CORE_API CaseSensitiveEqual {
    bool operator()(std::string_view a, std::string_view b) const {
        return detail::compareImpl(a, b, detail::identity) == 0;
    }
    bool operator()(std::string_view a, std::wstring_view b) const {
        return detail::compareImpl(a, b, detail::identity) == 0;
    }
    bool operator()(std::wstring_view a, std::string_view b) const {
        return detail::compareImpl(a, b, detail::identity) == 0;
    }
    bool operator()(std::wstring_view a, std::wstring_view b) const {
        return detail::compareImpl(a, b, detail::identity) == 0;
    }
    using is_transparent = int;
};

/**
 * @brief Case-sensitive less-than comparison.
 * Suitable for use as comparator in ordered containers.
 */
struct IVW_CORE_API CaseSensitiveLess {
    bool operator()(std::string_view a, std::string_view b) const {
        return detail::compareImpl(a, b, detail::identity) < 0;
    }
    bool operator()(std::string_view a, std::wstring_view b) const {
        return detail::compareImpl(a, b, detail::identity) < 0;
    }
    bool operator()(std::wstring_view a, std::string_view b) const {
        return detail::compareImpl(a, b, detail::identity) < 0;
    }
    bool operator()(std::wstring_view a, std::wstring_view b) const {
        return detail::compareImpl(a, b, detail::identity) < 0;
    }
    using is_transparent = int;
};

}  // namespace inviwo
