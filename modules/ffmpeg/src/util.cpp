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

#include <inviwo/ffmpeg/util.h>
#include <inviwo/core/algorithm/markdown.h>

extern "C" {
#include <libavutil/error.h>
}

namespace inviwo::ffmpeg {
Error::Error(int errnum) {
    static_assert(bufferSize == AV_ERROR_MAX_STRING_SIZE);
    av_strerror(errnum, buff.data(), buff.size());
}

std::vector<OptionPropertyOption<ffmpeg::CodecID>> codecsOptionsFor(
    const OutputFormat& outputFormat) {

    std::vector<OptionPropertyOption<ffmpeg::CodecID>> opts;
    opts.emplace_back("automatic", "Automatic", ffmpeg::CodecID{});

    if (outputFormat) {
        for (auto& codecId : outputFormat.supportedCodecs(AVMEDIA_TYPE_VIDEO)) {
            opts.push_back(OptionPropertyOption<ffmpeg::CodecID>{
                std::string(codecId.name()),
                fmt::format("{} ({})", codecId.name(), codecId.longName().value_or("-")), codecId});
        }
    }

    return opts;
}

OptionPropertyState<std::string> formatOptionsState() {
    std::vector<OptionPropertyOption<std::string>> opts;
    opts.emplace_back("automatic", "Automatic", "automatic");

    ffmpeg::OutputFormat::forEach([&](const ffmpeg::OutputFormat& format) {
        if (format.defaultVideoCodec()) {
            opts.push_back(OptionPropertyOption<std::string>{
                std::string(format.name()),
                fmt::format("{} ({})", format.name(), format.longName()),
                std::string(format.name())});
        }
    });

    return {.options = std::move(opts),
            .selectedIndex = 0,
            .help =
                "Movie container format, 'automatic' will guess"
                " a format from the filename"_help};
}

}  // namespace inviwo::ffmpeg
