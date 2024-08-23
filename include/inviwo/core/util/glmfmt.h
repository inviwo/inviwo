/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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
#include <glm/detail/type_quat.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>

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

template <typename T>
struct GlmFormatter {
    fmt::formatter<fmt::string_view> overallFormatter;
    fmt::formatter<T> elementFormatter;

    constexpr auto parse(fmt::format_parse_context& ctx) -> fmt::format_parse_context::iterator {
        const auto it = ctx.begin();
        const auto end = ctx.end();

        const auto range = std::string_view(it, end - it);
        const auto endPos = range.find_first_of(":}");
        const auto endIt =
            endPos != std::string_view::npos ? it + endPos + (range[endPos] == ':' ? 1 : 0) : end;
        const auto overallFormat = range.substr(0, endPos);

        fmt::format_parse_context overallCtx{overallFormat};
        overallFormatter.parse(overallCtx);

        ctx.advance_to(endIt);
        return elementFormatter.parse(ctx);
    }

    template <typename F>
    auto write(fmt::format_context& ctx, F writeElements) const {
        fmt::memory_buffer buff;
        auto out = fmt::appender(buff);
        fmt::format_context elementContext{out, ctx.args()};

        writeElements(out, elementFormatter, elementContext);

        return overallFormatter.format(std::string_view{buff.data(), buff.size()}, ctx);
    }
};

template <glm::length_t L, typename T, glm::qualifier Q>
struct fmt::formatter<glm::vec<L, T, Q>> : GlmFormatter<T> {
    auto format(const glm::vec<L, T, Q>& v, format_context& ctx) const -> format_context::iterator {
        return GlmFormatter<T>::write(ctx,
                                      [&v](auto it, auto& elementFormatter, auto& elementContext) {
                                          *it++ = '[';
                                          elementFormatter.format(v[0], elementContext);
                                          for (glm::length_t i = 1; i < L; ++i) {
                                              *it++ = ',';
                                              *it++ = ' ';
                                              elementFormatter.format(v[i], elementContext);
                                          }
                                          *it++ = ']';
                                      });
    }
};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct fmt::formatter<glm::mat<C, R, T, Q>> : GlmFormatter<T> {
    auto format(const glm::mat<C, R, T, Q>& m, format_context& ctx) const
        -> format_context::iterator {
        return GlmFormatter<T>::write(ctx,
                                      [&m](auto it, auto& elementFormatter, auto& elementContext) {
                                          *it++ = '[';
                                          *it++ = '[';
                                          elementFormatter.format(m[0][0], elementContext);
                                          for (glm::length_t i = 1; i < R; ++i) {
                                              *it++ = ',';
                                              *it++ = ' ';
                                              elementFormatter.format(m[0][i], elementContext);
                                          }
                                          *it++ = ']';

                                          for (glm::length_t j = 1; j < C; ++j) {
                                              *it++ = ',';
                                              *it++ = '[';
                                              elementFormatter.format(m[j][0], elementContext);
                                              for (glm::length_t i = 1; i < R; ++i) {
                                                  *it++ = ',';
                                                  *it++ = ' ';
                                                  elementFormatter.format(m[j][i], elementContext);
                                              }
                                              *it++ = ']';
                                          }
                                          *it++ = ']';
                                      });
    }
};

template <typename T, glm::qualifier Q>
struct fmt::formatter<glm::qua<T, Q>> : GlmFormatter<T> {
    auto format(const glm::qua<T, Q>& q, format_context& ctx) const -> format_context::iterator {
        return GlmFormatter<T>::write(ctx,
                                      [&q](auto it, auto& elementFormatter, auto& elementContext) {
                                          *it++ = '[';
                                          elementFormatter.format(q[0], elementContext);
                                          for (glm::length_t i = 1; i < 4; ++i) {
                                              *it++ = ',';
                                              *it++ = ' ';
                                              elementFormatter.format(q[i], elementContext);
                                          }
                                          *it++ = ']';
                                      });
    }
};
#endif
