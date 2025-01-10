/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <inviwo/ffmpeg/ffmpegmoduledefine.h>

#include <inviwo/core/properties/optionproperty.h>

#include <inviwo/ffmpeg/wrap/outputformat.h>
#include <inviwo/ffmpeg/wrap/codecid.h>

#include <vector>
#include <array>
#include <string_view>
#include <string>
#include <fmt/format.h>

namespace inviwo::ffmpeg {

struct IVW_MODULE_FFMPEG_API NoMoveCopy {
    NoMoveCopy() = default;
    ~NoMoveCopy() = default;
    NoMoveCopy(const NoMoveCopy&) = delete;
    NoMoveCopy(NoMoveCopy&&) = delete;
    NoMoveCopy& operator=(const NoMoveCopy&) = delete;
    NoMoveCopy& operator=(NoMoveCopy&&) = delete;
};

struct IVW_MODULE_FFMPEG_API Error {
    explicit Error(int errnum);
    static constexpr size_t bufferSize = 64;

    std::array<char, bufferSize> buff{0};

    operator std::string_view() const noexcept { return {buff.data()}; }

    std::string_view str() const noexcept { return {buff.data()}; }
};

IVW_MODULE_FFMPEG_API std::vector<OptionPropertyOption<ffmpeg::CodecID>> codecsOptionsFor(
    const OutputFormat& outputFormat);

IVW_MODULE_FFMPEG_API OptionPropertyState<std::string> formatOptionsState();

}  // namespace inviwo::ffmpeg

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::ffmpeg::Error> : fmt::formatter<fmt::string_view> {
    template <typename FormatContext>
    auto format(const inviwo::ffmpeg::Error& error, FormatContext& ctx) const {
        return formatter<fmt::string_view>::format(error.str(), ctx);
    }
};
#endif
