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
#include <inviwo/core/util/typetraits.h>

#include <string>
#include <string_view>
#include <algorithm>

#include <fmt/ranges.h>

namespace inviwo {

/**
 * LCString: lightweight wrapper that stores a lowercase-only string.
 * - Constructible/assignable from std::string and std::string_view
 * - Always stores lowercased characters (ASCII semantics)
 * - Convertible to std::string_view and std::string
 * - Provides const iterators, element access and C++20 three-way comparisons
 */
class IVW_CORE_API LCString {
public:
    using value_type = char;
    using size_type = std::string::size_type;
    using difference_type = std::string::difference_type;
    using const_iterator = std::string::const_iterator;
    using const_reverse_iterator = std::string::const_reverse_iterator;

    constexpr LCString() noexcept = default;
    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr LCString(const char* s) : data_{s} { makeLowerCase(); }
    constexpr explicit LCString(std::string_view s) : data_{s} { makeLowerCase(); }
    constexpr explicit LCString(const std::string& s) : data_{s} { makeLowerCase(); }
    constexpr explicit LCString(std::string&& s) : data_{std::move(s)} { makeLowerCase(); }

    constexpr LCString& operator=(std::string_view s) {
        data_.assign(s.begin(), s.end());
        makeLowerCase();
        return *this;
    }
    constexpr LCString& operator=(const std::string& s) {
        data_ = s;
        makeLowerCase();
        return *this;
    }
    constexpr LCString& operator=(std::string&& s) {
        data_ = std::move(s);
        makeLowerCase();
        return *this;
    }

    // Owning copy
    constexpr std::string str() const { return data_; }
    // Non-owning view
    constexpr std::string_view view() const noexcept { return std::string_view(data_); }

    // Implicit conversion to string_view
    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr operator std::string_view() const noexcept { return view(); }
    // For compatibility with APIs expecting const std::string&
    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr operator const std::string&() const noexcept { return data_; }

    constexpr bool empty() const noexcept { return data_.empty(); }
    constexpr const char* c_str() const noexcept { return data_.c_str(); }
    constexpr size_t size() const noexcept { return data_.size(); }

    // const iterators
    constexpr const_iterator begin() const noexcept { return data_.begin(); }
    constexpr const_iterator end() const noexcept { return data_.end(); }
    constexpr const_iterator cbegin() const noexcept { return data_.cbegin(); }
    constexpr const_iterator cend() const noexcept { return data_.cend(); }
    constexpr const_reverse_iterator rbegin() const noexcept { return data_.rbegin(); }
    constexpr const_reverse_iterator rend() const noexcept { return data_.rend(); }

    constexpr const char* data() const noexcept { return data_.data(); }

    // element access
    constexpr char operator[](size_type i) const { return data_[i]; }
    constexpr char at(size_type i) const { return data_.at(i); }

    constexpr int compare(std::string_view s) const noexcept { return view().compare(s); }

    // Defaulted three-way comparison and equality (member-wise on data_)
    constexpr auto operator<=>(const LCString& other) const noexcept = default;
    constexpr bool operator==(const LCString& other) const noexcept = default;

    constexpr auto operator<=>(std::string_view b) const noexcept { return view() <=> b; }
    constexpr bool operator==(std::string_view b) const noexcept { return view() == b; }
    constexpr auto operator<=>(const char* b) const noexcept {
        return view() <=> std::string_view{b};
    }
    constexpr bool operator==(const char* b) const noexcept {
        return view() == std::string_view{b};
    }

private:
    constexpr void makeLowerCase() noexcept {
        std::ranges::transform(data_, data_.begin(),
                               [](char c) { return static_cast<char>(std::tolower(c)); });
    }
    std::string data_;
};

constexpr std::string_view format_as(const LCString& s) { return s.view(); }

// Enable use of LCString in the serializers.
namespace detail {
inline void fromStr(std::string_view value, LCString& dest) { dest = value; }
}  // namespace detail
namespace util::detail {
template <>
struct is_string<LCString, void> : std::true_type {};
}  // namespace util::detail

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <typename Char>
struct fmt::is_range<::inviwo::LCString, Char> : std::false_type {};
#endif
