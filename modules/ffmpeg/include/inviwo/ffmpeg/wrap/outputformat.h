/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2024 Inviwo Foundation
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
#include <inviwo/ffmpeg/wrap/codecid.h>

#include <optional>
#include <string_view>
#include <filesystem>

struct AVOutputFormat;

namespace inviwo::ffmpeg {

class IVW_MODULE_FFMPEG_API OutputFormat {
public:
    OutputFormat();
    OutputFormat(const AVOutputFormat* of);
    OutputFormat(const std::string& shortName);

    std::string_view name() const;
    std::string_view longName() const;
    std::optional<std::string_view> extensions() const;
    std::optional<std::string_view> mimeType() const;
    CodecID defaultAudioCodec() const;
    CodecID defaultVideoCodec() const;
    CodecID defaultSubtitleCodec() const;

    bool supportsCodec(CodecID codecId, std::optional<int> stdCompliance = std::nullopt) const;
    std::vector<CodecID> supportedCodecs(AVMediaType type) const;
    std::string desc() const;

    template <typename Callback>
    static void forEach(const Callback& callback) {
        void* opaque = nullptr;
        while (const AVOutputFormat* outputFormat = next(&opaque)) {
            std::invoke(callback, OutputFormat(outputFormat));
        }
    }

    CodecID guessVideoCodec(const std::filesystem::path& path) const;

    static OutputFormat guess(const std::filesystem::path& path);

    static std::string listFormats();

    operator bool() const;

    const AVOutputFormat* of;

private:
    static const AVOutputFormat* next(void** opaque);
};

}  // namespace inviwo::ffmpeg
