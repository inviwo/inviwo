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

#include <inviwo/ffmpeg/wrap/inputcontext.h>

#include <inviwo/core/util/exception.h>

extern "C" {
#include <libavformat/avformat.h>
}

#include <fmt/std.h>

#include <utility>

namespace inviwo::ffmpeg {

InputContext::InputContext(std::filesystem::path aFilename)
    : filename{std::move(aFilename)}, ctx{nullptr} {

    if (auto ret = avformat_open_input(&ctx, filename.string().c_str(), nullptr, nullptr);
        ret < 0) {
        throw Exception(SourceContext{}, "Could not open '{}': {}", filename, Error{ret});
    }

    if (auto ret = avformat_find_stream_info(ctx, nullptr); ret < 0) {
        avformat_close_input(&ctx);
        throw Exception(SourceContext{}, "Could not find stream information in '{}': {}", filename,
                        Error{ret});
    }
}

InputContext::~InputContext() { avformat_close_input(&ctx); }

int InputContext::bestVideoStream() const {
    const auto index = av_find_best_stream(ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (index < 0) {
        throw Exception(SourceContext{}, "Could not find any video stream in '{}': {}", filename,
                        Error{index});
    }
    return index;
}

AVStream* InputContext::stream(int index) const {
    if (index < 0 || index >= nbStreams()) {
        throw RangeException(SourceContext{}, "Invalid stream index {}, '{}' has {} streams", index,
                             filename, nbStreams());
    }
    return ctx->streams[index];
}

int InputContext::nbStreams() const { return static_cast<int>(ctx->nb_streams); }

bool InputContext::readPacket(Packet& pkt) {
    pkt.unref();
    const auto ret = av_read_frame(ctx, pkt.pkt);
    if (ret == AVERROR_EOF) return false;
    if (ret < 0) {
        throw Exception(SourceContext{}, "Error while reading from '{}': {}", filename, Error{ret});
    }
    return true;
}

void InputContext::seek(int streamIndex, int64_t timestamp) {
    if (auto ret = av_seek_frame(ctx, streamIndex, timestamp, AVSEEK_FLAG_BACKWARD); ret < 0) {
        throw Exception(SourceContext{}, "Error while seeking in '{}': {}", filename, Error{ret});
    }
}

}  // namespace inviwo::ffmpeg
