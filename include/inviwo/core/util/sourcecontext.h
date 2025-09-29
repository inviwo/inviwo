/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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
#include <inviwo/core/util/hashcombine.h>

#include <string_view>
#include <iosfwd>
#include <source_location>
#include <iterator>
#include <fmt/format.h>

namespace inviwo {

class IVW_CORE_API Literal {
public:
    Literal() = delete;
    constexpr Literal(const Literal&) = default;
    constexpr Literal(Literal&&) noexcept = default;
    constexpr Literal& operator=(const Literal&) = default;
    constexpr Literal& operator=(Literal&&) noexcept = default;
    constexpr ~Literal() noexcept = default;

    constexpr std::string_view view() const { return view_; }

private:
    std::string_view view_;
    constexpr Literal(const char* str, size_t len) : view_{str, len} {}
    friend constexpr Literal operator""_sl(const char* str, size_t len);
};

constexpr Literal operator""_sl(const char* str, size_t len) { return Literal{str, len}; }

/**
 * Represents a location in source code
 */
class IVW_CORE_API SourceContext {
public:
    explicit constexpr SourceContext(Literal source, std::string_view file,
                                     std::string_view function, std::uint32_t line = 0,
                                     std::uint32_t column = 0)
        : SourceContext{source.view(), file, function, line, column} {}

    explicit constexpr SourceContext(
        Literal source, std::source_location location = std::source_location::current())
        : SourceContext{source.view(), location.file_name(), location.function_name(),
                        location.line(), location.column()} {}

    explicit(false) constexpr SourceContext(
        std::source_location location = std::source_location::current())
        : SourceContext{extractName(location.function_name()), location.file_name(),
                        location.function_name(), location.line(), location.column()} {}

    explicit constexpr SourceContext(const char*) = delete;
    explicit constexpr SourceContext(std::string_view) = delete;

    std::string_view source() const { return source_; };
    std::string_view file() const { return file_; };
    std::string_view function() const { return function_; };
    std::uint32_t line() const { return line_; };
    std::uint32_t column() const { return column_; };

private:
    static constexpr std::string_view extractName(std::string_view name) noexcept {
        static constexpr std::string_view ans = "::(anonymous namespace)::";

        if (const auto ansPos = name.find(ans); ansPos != std::string_view::npos) {
            const auto end = name.find('(', ansPos + ans.size());
            const auto start = name.rfind(' ', ansPos);
            name = name.substr(start + 1, end - start - 1);
        } else {
            const std::size_t end = name.find('(');
            name = name.substr(0, end);
            name = name.substr(name.rfind(' ') + 1);
        }
        return name;
    }

    explicit constexpr SourceContext(std::string_view source, std::string_view file,
                                     std::string_view function, std::uint32_t line,
                                     std::uint32_t column)
        : source_{source}, file_{file}, function_{function}, line_{line}, column_{column} {}

    std::string_view source_;
    std::string_view file_;
    std::string_view function_;
    std::uint32_t line_;
    std::uint32_t column_;
};

IVW_CORE_API std::ostream& operator<<(std::ostream& ss, const SourceContext& ec);

#define IVW_CONTEXT ::inviwo::SourceContext()
#define IVW_CONTEXT_CUSTOM(source)                   \
    []() {                                           \
        using namespace inviwo;                      \
        return ::inviwo::SourceContext(source##_sl); \
    }()

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::SourceContext> : fmt::formatter<fmt::string_view> {
    template <typename FormatContext>
    auto format(const inviwo::SourceContext& sc, FormatContext& ctx) const {
        fmt::memory_buffer buff;
        fmt::format_to(std::back_inserter(buff), "{} ({}:{})", sc.source(), sc.file(), sc.line());
        return formatter<fmt::string_view>::format(fmt::string_view(buff.data(), buff.size()), ctx);
    }
};

template <>
struct std::hash<::inviwo::SourceContext> {
    size_t operator()(const ::inviwo::SourceContext& context) const noexcept {
        size_t h = 0;
        ::inviwo::util::hash_combine(h, context.source());
        ::inviwo::util::hash_combine(h, context.file());
        ::inviwo::util::hash_combine(h, context.function());
        ::inviwo::util::hash_combine(h, context.line());
        return h;
    }
};
#endif
