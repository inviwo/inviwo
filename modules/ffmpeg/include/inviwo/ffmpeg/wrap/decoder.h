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

#pragma once

#include <inviwo/ffmpeg/ffmpegmoduledefine.h>

#include <inviwo/ffmpeg/util.h>
#include <inviwo/ffmpeg/wrap/frame.h>
#include <inviwo/ffmpeg/wrap/packet.h>
#include <inviwo/ffmpeg/wrap/codecid.h>

extern "C" {
#include <libavutil/avutil.h>
}

struct AVCodecContext;
struct AVStream;

namespace inviwo::ffmpeg {

/**
 * @brief RAII wrapper around an AVCodecContext set up for decoding.
 * @see Encoder for the encoding counterpart
 */
class IVW_MODULE_FFMPEG_API Decoder : NoMoveCopy {
public:
    /**
     * @brief Create and open a decoder matching the codec parameters of @p stream
     * @throws Exception if no decoder is available for the codec or if it cannot be opened
     */
    explicit Decoder(const AVStream* stream);
    ~Decoder();

    void sendPacket(const Packet& pkt);

    /// Signal end of stream to the decoder, any remaining frames can then be collected with
    /// receiveFrame()
    void flush();

    /**
     * @brief Retrieve the next decoded frame
     * @return 0 on success, AVERROR(EAGAIN) if more packets are needed, AVERROR_EOF if the decoder
     * has been fully drained
     */
    int receiveFrame(Frame& frame);

    /// Discard any buffered state, must be called after seeking
    void reset();

    CodecID codecID() const;
    int width() const;
    int height() const;
    enum AVPixelFormat pixelFormat() const;

    AVCodecContext* ctx;
};

}  // namespace inviwo::ffmpeg
