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

#include <inviwo/ffmpeg/outputstream.h>

#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/logcentral.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

namespace inviwo::ffmpeg {

OutputStream::OutputStream(Format& format, Options opts)
    : sourceFormat{opts.sourceFormat}
    , codec{opts.codecId ? opts.codecId : format.outputFormat().defaultVideoCodec()}
    , stream{format.newStream()}
    , tmpFrame{std::nullopt}
    , scaler{std::nullopt} {

    stream->id = format.ctx->nb_streams - 1;

    codec.ctx->bit_rate = opts.bitRate;

    codec.ctx->width = opts.width;
    codec.ctx->height = opts.height;

    /* timebase: This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/frameRate and timestamp increments should be
     * identical to 1. */
    stream->time_base = AVRational{1, opts.frameRate};
    codec.ctx->time_base = stream->time_base;

    codec.ctx->gop_size = 12; /* emit one intra frame every twelve frames at most */

    codec.ctx->pix_fmt = AV_PIX_FMT_YUV420P;  // seems many codecs handle this one?

    if (codec.ctx->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B-frames */
        codec.ctx->max_b_frames = 2;
    }
    if (codec.ctx->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        /* Needed to avoid using macroblocks in which
         * some coeffs overflow. This does not happen
         * with normal video, it just happens here as
         * the motion of the chroma plane does not
         * match the luma plane. */
        codec.ctx->mb_decision = 2;
    }

    /* Some formats want stream headers to be separate. */
    if (format.ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        codec.ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
}

void OutputStream::openVideo(AVDictionary* opt_arg) {
    codec.open(opt_arg);

    /* copy the stream parameters to the muxer */
    if (auto ret = avcodec_parameters_from_context(stream->codecpar, codec.ctx); ret < 0) {
        throw Exception(SourceContext{}, "Could not copy the stream parameters: {}", Error{ret});
    }
}

Frame* OutputStream::fillFrame(Frame& dst,
                               std::function<void(AVFrame* pict, int width, int height)> filler) {

    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally; make sure we do not overwrite it here */
    dst.makeWritable();

    if (codec.ctx->pix_fmt != sourceFormat) {
        if (!scaler) {
            scaler.emplace(codec.ctx->width, codec.ctx->height, sourceFormat, codec.ctx->width,
                           codec.ctx->height, codec.ctx->pix_fmt, SWS_BICUBIC, nullptr, nullptr,
                           nullptr);
        }
        if (!tmpFrame) {
            tmpFrame.emplace(sourceFormat, codec.ctx->width, codec.ctx->height);
        }

        filler(tmpFrame->frame, codec.ctx->width, codec.ctx->height);
        scaler->scale((const uint8_t* const*)tmpFrame->frame->data, tmpFrame->frame->linesize, 0,
                      codec.ctx->height, dst.frame->data, dst.frame->linesize);
    } else {
        filler(dst.frame, codec.ctx->width, codec.ctx->height);
    }

    return &dst;
}

}  // namespace inviwo::ffmpeg
