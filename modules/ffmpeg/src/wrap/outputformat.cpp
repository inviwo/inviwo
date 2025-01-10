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

#include <inviwo/ffmpeg/wrap/outputformat.h>

#include <set>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

typedef struct AVCodecTag {
    enum AVCodecID id;
    unsigned int tag;
} AVCodecTag;

namespace inviwo::ffmpeg {

OutputFormat::OutputFormat() : of{nullptr} {}

OutputFormat::OutputFormat(const AVOutputFormat* of) : of{of} {}

OutputFormat::OutputFormat(const std::string& shortName)
    : OutputFormat{av_guess_format(shortName.c_str(), nullptr, nullptr)} {}

std::string_view OutputFormat::name() const { return of ? of->name : ""; }
std::string_view OutputFormat::longName() const { return of ? of->long_name : ""; }
std::optional<std::string_view> OutputFormat::extensions() const {
    if (of && of->extensions) {
        return std::string_view{of->extensions};
    } else {
        return std::nullopt;
    }
}
std::optional<std::string_view> OutputFormat::mimeType() const {
    if (of && of->mime_type) {
        return std::string_view{of->mime_type};
    } else {
        return std::nullopt;
    }
}

CodecID OutputFormat::defaultAudioCodec() const { return of ? of->audio_codec : CodecID{}; }
CodecID OutputFormat::defaultVideoCodec() const { return of ? of->video_codec : CodecID{}; }
CodecID OutputFormat::defaultSubtitleCodec() const { return of ? of->subtitle_codec : CodecID{}; }

bool OutputFormat::supportsCodec(CodecID codecId, std::optional<int> stdCompliance) const {
    if (!of) return false;
    return avformat_query_codec(of, codecId.id, stdCompliance.value_or(FF_COMPLIANCE_NORMAL)) == 1;
}

std::vector<CodecID> OutputFormat::supportedCodecs(AVMediaType type) const {
    if (!of) return {};

    auto* tags = of->codec_tag;

    std::set<CodecID> codecs;
    for (int i = 0; tags && tags[i]; i++) {
        for (const AVCodecTag* codec_tags = tags[i]; codec_tags->id != AV_CODEC_ID_NONE;
             codec_tags++) {
            auto codec = CodecID(codec_tags->id);
            if (codec.type() == type) {
                codecs.insert(codec);
            }
        }
    }

    std::vector<CodecID> res(codecs.begin(), codecs.end());
    std::sort(res.begin(), res.end(), [](CodecID a, CodecID b) { return a.name() < b.name(); });
    return res;
}

std::string OutputFormat::desc() const {
    return fmt::format("{} ({}) extensions: '{}' mime type: '{}'\n", name(), longName(),
                       extensions().value_or("_"), mimeType().value_or("-"));
}

std::string OutputFormat::listFormats() {
    std::string infos;
    forEach([&](const OutputFormat& format) {
        if (format.defaultVideoCodec()) {
            fmt::format_to(std::back_inserter(infos), "{}\n", format.desc());
        }
    });
    return infos;
}

OutputFormat::operator bool() const { return of != nullptr; }

const AVOutputFormat* OutputFormat::next(void** opaque) { return av_muxer_iterate(opaque); }

OutputFormat OutputFormat::guess(const std::filesystem::path& path) {
    return av_guess_format(nullptr, path.string().c_str(), nullptr);
}

CodecID OutputFormat::guessVideoCodec(const std::filesystem::path& path) const {
    if (of) {
        return av_guess_codec(of, nullptr, path.string().c_str(), nullptr, AVMEDIA_TYPE_VIDEO);
    } else {
        return CodecID{};
    }
}

}  // namespace inviwo::ffmpeg
