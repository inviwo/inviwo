/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2026 Inviwo Foundation
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

#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmutils.h>
#include <glm/detail/type_quat.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <optional>

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// Tell fmt to not treat glm types as ranges, otherwise the fmt range formatting will be used and
// will conflicts with the custom formatting below.

template <glm::length_t L, typename T, glm::qualifier Q, typename Char>
struct fmt::is_range<::glm::vec<L, T, Q>, Char> : std::false_type {};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q, typename Char>
struct fmt::is_range<::glm::mat<C, R, T, Q>, Char> : std::false_type {};

template <typename T, glm::qualifier Q, typename Char>
struct fmt::is_range<::glm::qua<T, Q>, Char> : std::false_type {};

template <glm::length_t L, typename T, glm::qualifier Q>
struct fmt::is_tuple_like<::glm::vec<L, T, Q>> : std::false_type {};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct fmt::is_tuple_like<::glm::mat<C, R, T, Q>> : std::false_type {};

template <typename T, glm::qualifier Q>
struct fmt::is_tuple_like<::glm::qua<T, Q>> : std::false_type {};

namespace inviwo {

namespace detail {

//  Advance past a single nested replacement field "{...}", respecting depth.
//  Returns iterator just past the closing '}'.
template <typename It>
constexpr It skipNestedField(It it, It end) {
    int depth = 0;
    while (it != end) {
        if (*it == '{') {
            ++depth;
            ++it;
        } else if (*it == '}') {
            --depth;
            ++it;
            if (depth == 0) return it;
        }
    }
    throw fmt::format_error("unterminated nested replacement field");
}

//  Locate the ':' that separates overallSpec from elementSpec, skipping over
//  any nested replacement fields { } in the process.
//  Returns end if there is no such separator.
template <typename It>
constexpr std::optional<It> findSpecSeparator(It it, It end) {
    while (it != end) {
        if (*it == '}') return std::nullopt;
        if (*it == ':') return it;
        if (*it == '{')
            it = skipNestedField(it, end);
        else
            ++it;
    }
    return std::nullopt;
}

struct StringFormatter {
private:
    fmt::detail::dynamic_format_specs<char> specs_;

public:
    constexpr auto parse(fmt::parse_context<char>& ctx,
                         std::optional<decltype(ctx.begin())> maybeEnd = std::nullopt) -> const
        char* {
        auto end = maybeEnd.value_or(ctx.end());
        if (ctx.begin() == end || *ctx.begin() == '}') return ctx.begin();
        end = parse_format_specs(ctx.begin(), end, specs_, ctx, fmt::detail::type::string_type);
        return end;
    }

    constexpr void set_debug_format(bool set = true) {
        specs_.set_type(set ? fmt::presentation_type::debug : fmt::presentation_type::none);
    }

    constexpr auto format(const std::string_view& val, fmt::format_context& ctx) const
        -> decltype(ctx.out()) {
        if (!specs_.dynamic()) return write<char>(ctx.out(), val, specs_, ctx.locale());
        auto specs = fmt::format_specs(specs_);
        handle_dynamic_spec(specs.dynamic_width(), specs.width, specs_.width_ref, ctx);
        handle_dynamic_spec(specs.dynamic_precision(), specs.precision, specs_.precision_ref, ctx);
        return fmt::detail::write<char>(ctx.out(), val, specs, ctx.locale());
    }
};

}  // namespace detail

template <typename GlmType, typename T = typename GlmType::value_type>
struct GlmFormatter {
    detail::StringFormatter overallFormatter;
    fmt::formatter<T> elementFormatter;

    // ── parse ─────────────────────────────────────────────────────────────────
    constexpr auto parse(fmt::format_parse_context& ctx) -> fmt::format_parse_context::iterator {
        auto it = ctx.begin();
        auto end = ctx.end();

        auto sep = detail::findSpecSeparator(it, end);
        it = overallFormatter.parse(ctx, sep);

        if (sep) {
            it = ++*sep;
            ctx.advance_to(it);
            it = elementFormatter.parse(ctx);
        }

        if (it != end && *it != '}') throw fmt::format_error("invalid glm formatter spec");

        return it;
    }

    auto format(const GlmType& obj, fmt::format_context& ctx) const
        -> fmt::format_context::iterator {
        fmt::memory_buffer buff;
        auto out = fmt::appender(buff);
        auto elemCtx = fmt::format_context(out, ctx.args(), ctx.locale());

        *out++ = '[';
        if constexpr (inviwo::util::rank_v<GlmType> == 2) {
            constexpr auto C = GlmType::length();
            constexpr auto R = GlmType::col_type::length();
            *out++ = '[';
            elementFormatter.format(obj[0][0], elemCtx);
            for (glm::length_t i = 1; i < R; ++i) {
                *out++ = ',';
                *out++ = ' ';
                elementFormatter.format(obj[0][i], elemCtx);
            }
            *out++ = ']';

            for (glm::length_t j = 1; j < C; ++j) {
                *out++ = ',';
                *out++ = '[';
                elementFormatter.format(obj[j][0], elemCtx);
                for (glm::length_t i = 1; i < R; ++i) {
                    *out++ = ',';
                    *out++ = ' ';
                    elementFormatter.format(obj[j][i], elemCtx);
                }
                *out++ = ']';
            }
        } else {
            elementFormatter.format(obj[0], elemCtx);
            for (glm::length_t i = 1; i < GlmType::length(); ++i) {
                *out++ = ',';
                *out++ = ' ';
                elementFormatter.format(obj[i], elemCtx);
            }
        }
        *out++ = ']';

        return overallFormatter.format(std::string_view{buff.data(), buff.size()}, ctx);
    }
};
}  // namespace inviwo

template <glm::length_t L, typename T, glm::qualifier Q>
struct fmt::formatter<glm::vec<L, T, Q>> : inviwo::GlmFormatter<glm::vec<L, T, Q>> {};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct fmt::formatter<glm::mat<C, R, T, Q>> : inviwo::GlmFormatter<glm::mat<C, R, T, Q>> {};

template <typename T, glm::qualifier Q>
struct fmt::formatter<glm::qua<T, Q>> : inviwo::GlmFormatter<glm::qua<T, Q>> {};

#endif
