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
#pragma once

#include <inviwo/ffmpeg/ffmpegmoduledefine.h>

#include <inviwo/ffmpeg/util.h>

#include <inviwo/ffmpeg/wrap/format.h>
#include <inviwo/ffmpeg/wrap/frame.h>
#include <inviwo/ffmpeg/wrap/codec.h>
#include <inviwo/ffmpeg/wrap/codecid.h>
#include <inviwo/ffmpeg/wrap/swscale.h>

#include <functional>
#include <optional>

extern "C" {
#include <libavutil/avutil.h>
}

struct AVStream;

namespace inviwo::ffmpeg {

// a wrapper around a single output AVStream
struct IVW_MODULE_FFMPEG_API OutputStream : NoMoveCopy {
    struct Options {
        CodecID codecId = CodecID{};
        enum AVPixelFormat sourceFormat = AV_PIX_FMT_RGBA;
        int width = 1920;
        int height = 1080;
        int frameRate = 25;
        int64_t bitRate = 400000;
    };

    OutputStream(Format& format, Options opts);

    void openVideo(AVDictionary* opt_arg = nullptr);

    Frame* fillFrame(Frame& dst, std::function<void(AVFrame* pict, int width, int height)> filler);

    enum AVPixelFormat sourceFormat;
    Codec codec;
    AVStream* stream;

    std::optional<Frame> tmpFrame;
    std::optional<SwScale> scaler;
};

}  // namespace inviwo::ffmpeg
