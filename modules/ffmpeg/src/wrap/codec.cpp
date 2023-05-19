/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <inviwo/ffmpeg/wrap/codec.h>

#include <inviwo/core/util/exception.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace inviwo::ffmpeg {

Codec::Codec(const AVCodec* codec) : ctx{avcodec_alloc_context3(codec)} {
    if (!ctx) {
        throw Exception(IVW_CONTEXT, "Could not alloc an encoding context");
    }
}

Codec::Codec(CodecID codecId) : Codec{findEncoder(codecId.id)} {}

Codec::~Codec() { avcodec_free_context(&ctx); }

void Codec::open(AVDictionary* opt_arg) {
    AVDictionary* opt = nullptr;
    av_dict_copy(&opt, opt_arg, 0);

    /* open the codec */
    // int ret = avcodec_open2(codec.ctx, codec.ctx->codec, &opt);
    int ret = avcodec_open2(ctx, ctx->codec, &opt);

    av_dict_free(&opt);
    if (ret < 0) {
        throw Exception(IVW_CONTEXT, "Could not open video codec: {}", Error{ret});
    }
}

void Codec::sendFrame(const Frame& frame) {
    if (auto ret = avcodec_send_frame(ctx, frame.frame); ret < 0) {
        throw Exception(IVW_CONTEXT, "Error sending a frame to the encoder: {}", Error(ret));
    }
}

int Codec::receivePacket(Packet& pkt) {
    auto ret = avcodec_receive_packet(ctx, pkt.pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return ret;

    if (ret < 0) {
        throw Exception(IVW_CONTEXT, "Error encoding a frame: {}", Error(ret));
    }
    return ret;
}
CodecID Codec::codecID() const { return ctx->codec_id; }

const AVCodec* Codec::findEncoder(CodecID codecId) {
    auto codec = avcodec_find_encoder(codecId.id);
    if (!codec) {
        throw Exception(IVW_CONTEXT_CUSTOM("Codec"), "Could not find encoder for '{}'",
                        codecId.name());
    }
    return codec;
}

}  // namespace inviwo::ffmpeg
