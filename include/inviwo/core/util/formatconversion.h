/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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
#include <string>
#include <fmt/format.h>
#include <array>

namespace inviwo {

namespace util {

static const size_t byte_swap = 1024;
static const size_t byte_size = 1;
static const size_t kilo_byte_size = byte_swap * byte_size;
static const size_t mega_byte_size = byte_swap * kilo_byte_size;
static const size_t giga_byte_size = byte_swap * mega_byte_size;
static const size_t tera_byte_size = byte_swap * giga_byte_size;
static const double byte_div = 1.0 / byte_swap;

IVW_CORE_API size_t bytes_to_kilobytes(size_t bytes);
IVW_CORE_API size_t bytes_to_megabytes(size_t bytes);
IVW_CORE_API size_t kilobytes_to_bytes(size_t bytes);
IVW_CORE_API size_t megabytes_to_bytes(size_t bytes);

IVW_CORE_API std::string formatBytesToString(size_t bytes);

}  // namespace util

template <typename T>
struct ByteSize {
    T size;
};

}  // namespace inviwo

template <typename T>
struct fmt::formatter<inviwo::ByteSize<T>, char> {
    fmt::formatter<std::string_view> strFormatter;
    char prefix = 'A';

    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        auto it = ctx.begin();
        if (it == ctx.end()) return it;

        if (*it == 'T' || *it == 'G' || *it == 'M' || *it == 'k' || *it == 'O') {
            prefix = *it;
            ++it;
        }
        if (*it == '}') {
            return it;
        }

        return strFormatter.parse(ctx);
    }

    template <class FmtContext>
    FmtContext::iterator format(inviwo::ByteSize<T> bs, FmtContext& ctx) const {
        std::array<char, 20> buff;

        auto end = buff.data();
        if (prefix == 'T' || (prefix == 'A' && bs.size >= 1'000'000'000'000)) {
            end = fmt::format_to_n(buff.data(), buff.size(), "{:.2f} TB",
                                   static_cast<double>(bs.size) / 1'000'000'000'000.0)
                      .out;
        } else if (prefix == 'G' || (prefix == 'A' && bs.size >= 1'000'000'000)) {
            end = fmt::format_to_n(buff.data(), buff.size(), "{:.2f} GB",
                                   static_cast<double>(bs.size) / 1'000'000'000.0)
                      .out;
        } else if (prefix == 'M' || (prefix == 'A' && bs.size >= 1'000'000)) {
            end = fmt::format_to_n(buff.data(), buff.size(), "{:.2f} MB",
                                   static_cast<double>(bs.size) / 1'000'000.0)
                      .out;
        } else if (prefix == 'k' || (prefix == 'A' && bs.size >= 1'000)) {
            end = fmt::format_to_n(buff.data(), buff.size(), "{:.2f} kB",
                                   static_cast<double>(bs.size) / 1'000.0)
                      .out;
        } else {
            end = fmt::format_to_n(buff.data(), buff.size(), "{:d} B", bs.size).out;
        }

        return strFormatter.format(
            std::string_view{buff.data(), static_cast<size_t>(std::distance(buff.data(), end))},
            ctx);
    }
};
