/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/ffmpeg/wrap/decoder.h>

#include <inviwo/core/util/exception.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace inviwo::ffmpeg {

Decoder::Decoder(const AVStream* stream) : ctx{nullptr} {
    const CodecID codecId{stream->codecpar->codec_id};
    const AVCodec* codec = avcodec_find_decoder(codecId.id);
    if (!codec) {
        throw Exception(SourceContext{}, "Could not find decoder for '{}'", codecId.name());
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        throw Exception(SourceContext{}, "Could not alloc a decoding context");
    }

    if (auto ret = avcodec_parameters_to_context(ctx, stream->codecpar); ret < 0) {
        avcodec_free_context(&ctx);
        throw Exception(SourceContext{}, "Could not copy the stream parameters: {}", Error{ret});
    }

    ctx->pkt_timebase = stream->time_base;
    ctx->thread_count = 0;  // let ffmpeg pick the thread count

    if (auto ret = avcodec_open2(ctx, codec, nullptr); ret < 0) {
        avcodec_free_context(&ctx);
        throw Exception(SourceContext{}, "Could not open video codec '{}': {}", codecId.name(),
                        Error{ret});
    }
}

Decoder::~Decoder() { avcodec_free_context(&ctx); }

void Decoder::sendPacket(const Packet& pkt) {
    if (auto ret = avcodec_send_packet(ctx, pkt.pkt); ret < 0) {
        throw Exception(SourceContext{}, "Error sending a packet to the decoder: {}", Error{ret});
    }
}

void Decoder::flush() {
    if (auto ret = avcodec_send_packet(ctx, nullptr); ret < 0) {
        throw Exception(SourceContext{}, "Error flushing the decoder: {}", Error{ret});
    }
}

int Decoder::receiveFrame(Frame& frame) {
    auto ret = avcodec_receive_frame(ctx, frame.frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return ret;

    if (ret < 0) {
        throw Exception(SourceContext{}, "Error decoding a frame: {}", Error{ret});
    }
    return ret;
}

void Decoder::reset() { avcodec_flush_buffers(ctx); }

CodecID Decoder::codecID() const { return ctx->codec_id; }
int Decoder::width() const { return ctx->width; }
int Decoder::height() const { return ctx->height; }
enum AVPixelFormat Decoder::pixelFormat() const { return ctx->pix_fmt; }

}  // namespace inviwo::ffmpeg
