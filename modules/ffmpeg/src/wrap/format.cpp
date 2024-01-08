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

#include <inviwo/ffmpeg/wrap/format.h>
#include <inviwo/core/util/exception.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <fmt/std.h>

namespace inviwo::ffmpeg {

Format::Format(OutputFormat outputFormat, const std::filesystem::path& aFilename)
    : filename{aFilename} {
    avformat_alloc_output_context2(&ctx, outputFormat.of, nullptr, filename.string().c_str());
    if (!ctx) {
        throw inviwo::Exception(IVW_CONTEXT, "Could not deduce output format from file extension");
    }
}

Format::Format(const std::filesystem::path& aFilename) : filename{aFilename} {
    avformat_alloc_output_context2(&ctx, nullptr, nullptr, filename.string().c_str());
    if (!ctx) {
        throw inviwo::Exception(IVW_CONTEXT, "Could not deduce output format from file extension");
    }
}

Format::~Format() {
    if (!(ctx->oformat->flags & AVFMT_NOFILE)) {
        /* Close the output file. */
        avio_closep(&ctx->pb);
    }
    avformat_free_context(ctx);
}

void Format::open() {
    if (!(ctx->oformat->flags & AVFMT_NOFILE)) {
        int ret = avio_open(&ctx->pb, filename.string().c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            throw inviwo::Exception(IVW_CONTEXT, "Could not open '{}': {}", filename, Error{ret});
        }
    }
}

void Format::writeHeader(AVDictionary** options) {
    /* Write the stream header, if any. */
    if (auto ret = avformat_write_header(ctx, options); ret < 0) {
        throw inviwo::Exception(IVW_CONTEXT, "Error occurred when writing header, file: {}",
                                Error{ret});
    }
}

void Format::writeTrailer() {
    if (auto ret = av_write_trailer(ctx); ret < 0) {
        throw inviwo::Exception(IVW_CONTEXT, "Error occurred when writing trailer, file: {}",
                                Error{ret});
    }
}

void Format::writeFrame(const Packet& pkt) {
    /* pkt is now blank (av_interleaved_write_frame() takes ownership of
     * its contents and resets pkt), so that no unreferencing is necessary.
     * This would be different if one used av_write_frame(). */
    if (auto ret = av_interleaved_write_frame(ctx, pkt.pkt); ret < 0) {
        throw inviwo::Exception(IVW_CONTEXT, "Error while writing output packet: {}", Error(ret));
    }
}

AVStream* Format::newStream() {
    AVStream* stream = avformat_new_stream(ctx, nullptr);
    if (!stream) {
        throw inviwo::Exception(IVW_CONTEXT, "Could not allocate AVStream");
    }
    return stream;
}

int Format::queryCodec(CodecID codecId, std::optional<int> stdCompliance) {
    return avformat_query_codec(ctx->oformat, codecId.id,
                                stdCompliance.value_or(FF_COMPLIANCE_NORMAL));
}

OutputFormat Format::outputFormat() const { return ctx->oformat; }

}  // namespace inviwo::ffmpeg
