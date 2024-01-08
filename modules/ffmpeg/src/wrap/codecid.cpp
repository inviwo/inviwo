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

#include <inviwo/ffmpeg/wrap/codecid.h>

#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_id.h>
}

namespace inviwo::ffmpeg {

CodecID::CodecID() : id{AV_CODEC_ID_NONE} {}
CodecID::CodecID(AVCodecID id) : id{id} {}

std::string_view CodecID::name() const { return avcodec_get_name(id); }
std::optional<std::string_view> CodecID::longName() const {
    auto desc = avcodec_descriptor_get(id);
    if (desc && desc->long_name) {
        return desc->long_name;
    } else {
        return std::nullopt;
    }
}
std::string_view CodecID::typeString() const {
    if (auto* chr = av_get_media_type_string(type())) {
        return chr;
    } else {
        return "";
    }
}
std::string CodecID::desc() const { return fmt::format("{}", *this); }

AVMediaType CodecID::type() const { return avcodec_get_type(id); }

CodecID::operator bool() const { return id != AV_CODEC_ID_NONE; }

void CodecID::serialize(Serializer& s) const { s.serialize("codecId", id); }
void CodecID::deserialize(Deserializer& d) { d.deserialize("codecId", id); }

}  // namespace inviwo::ffmpeg
