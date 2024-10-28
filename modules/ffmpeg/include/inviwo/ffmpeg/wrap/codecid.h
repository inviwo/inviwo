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

#include <inviwo/core/properties/optionproperty.h>

#include <string>
#include <string_view>
#include <optional>

#include <fmt/format.h>

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/codec_id.h>
}

namespace inviwo {

class Serializer;
class Deserializer;

}  // namespace inviwo

namespace inviwo::ffmpeg {

class IVW_MODULE_FFMPEG_API CodecID {
public:
    CodecID();
    CodecID(AVCodecID id);

    std::string_view name() const;
    std::optional<std::string_view> longName() const;

    std::string_view typeString() const;
    std::string desc() const;

    AVMediaType type() const;

    operator bool() const;

    auto operator<=>(const CodecID&) const = default;

    AVCodecID id;

    void serialize(Serializer& s) const;
    void deserialize(Deserializer& d);
};

}  // namespace inviwo::ffmpeg

template <>
struct inviwo::OptionPropertyTraits<inviwo::ffmpeg::CodecID> {
    static std::string_view classIdentifier() {
        static const std::string identifier = "org.inviwo.OptionProperty.ffmpeg.CodecID";
        return identifier;
    }
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::ffmpeg::CodecID> : fmt::formatter<fmt::string_view> {
    template <typename FormatContext>
    auto format(const inviwo::ffmpeg::CodecID& codecId, FormatContext& ctx) const {
        fmt::memory_buffer buff;
        if (codecId) {
            fmt::format_to(std::back_inserter(buff), "{} ({}) {}", codecId.name(),
                           codecId.longName().value_or("-"), codecId.typeString());
        } else {
            fmt::format_to(std::back_inserter(buff), "None");
        }
        return formatter<fmt::string_view>::format(fmt::string_view(buff.data(), buff.size()), ctx);
    }
};
#endif
