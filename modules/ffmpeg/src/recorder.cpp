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

#include <inviwo/ffmpeg/recorder.h>

#include <inviwo/core/util/threadutil.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/indexmapper.h>

extern "C" {

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

namespace inviwo::ffmpeg {

namespace {

bool writeFrame(Format& format, Codec& codec, AVStream* st, const Frame& frame, Packet& pkt) {
    // send the frame to the encoder
    codec.sendFrame(frame);

    int ret = 0;
    while (ret >= 0) {
        ret = codec.receivePacket(pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;

        // rescale output packet timestamp values from codec to stream timebase
        av_packet_rescale_ts(pkt.pkt, codec.ctx->time_base, st->time_base);
        pkt.pkt->stream_index = st->index;

        // Write the compressed frame to the media file.
        format.writeFrame(pkt);
    }

    return ret != AVERROR_EOF;
}

}  // namespace

Recorder::Recorder(const std::filesystem::path& filename, OutputFormat format, Mode aMode,
                   OutputStream::Options opts)
    : mode{aMode}
    , out{format, filename}
    , stream{out, opts}
    , pkt{}
    , queue_{}
    , unused_{}
    , mutex_{}
    , condition_{}
    , stop_{false}
    , eptr{}
    , frameRate{opts.frameRate}
    , worker{} {

    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */
    if (out.ctx->oformat->video_codec == AV_CODEC_ID_NONE) {
        throw inviwo::Exception(IVW_CONTEXT, "No video codec");
    }

    worker = std::thread{[this]() {
        util::setThreadDescription("Inviwo FFmpeg Thread");
        run();
    }};
}

Recorder::~Recorder() {
    stop_ = true;
    condition_.notify_all();
    worker.join();
    if (eptr) {
        std::rethrow_exception(eptr);
    }
}

const OutputStream& Recorder::getStream() { return stream; }
const Format& Recorder::getFormat() { return out; }

void Recorder::queueFrame(const LayerRAM& layer) {
    std::optional<Frame> frame;
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (eptr) {
            std::rethrow_exception(std::exchange(eptr, nullptr));
        }

        if (!unused_.empty()) {
            frame.emplace(std::move(unused_.back()));
            unused_.pop_back();
        } else if (queue_.size() < 30) {
            frame.emplace(stream.codec.ctx->pix_fmt, stream.codec.ctx->width,
                          stream.codec.ctx->height);
        } else {
            util::log(IVW_CONTEXT, "Queue saturated");
        }
    }

    if (frame) {
        // this requires that .sourceFormat = AV_PIX_FMT_RGBA,
        stream.fillFrame(*frame, [&](AVFrame* pict, int width, int height) {
            if (static_cast<int>(layer.getDimensions().x) != width ||
                static_cast<int>(layer.getDimensions().y) != height) {
                throw inviwo::Exception(
                    IVW_CONTEXT, "Video dimensions do not match, expected: {}x{} got: {}x{}", width,
                    height, layer.getDimensions().x, layer.getDimensions().y);
            }

            if (layer.getDataFormat()->getId() == DataFormatId::Vec4UInt8) {
                auto* data = static_cast<const glm::tvec4<uint8_t>*>(layer.getData());
                util::IndexMapper2D im{layer.getDimensions()};

                for (int y = 0; y < height; y++) {
                    auto rowStart = glm::value_ptr(data[im(0, y)]);
                    std::copy(rowStart, rowStart + 4 * width,
                              &pict->data[0][(height - y - 1) * pict->linesize[0]]);
                }
            } else {
                layer.dispatch<void>([&](auto* rep) {
                    auto* data = rep->getDataTyped();
                    util::IndexMapper2D im{rep->getDimensions()};

                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width; x++) {
                            auto pix =
                                util::glm_convert_normalized<glm::tvec4<uint8_t>>(data[im(x, y)]);
                            std::copy(glm::value_ptr(pix), glm::value_ptr(pix) + 4,
                                      &pict->data[0][(height - y - 1) * pict->linesize[0] + x * 4]);
                        }
                    }
                });
            }
        });

        {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_.push(std::move(*frame));
        }
    }
    condition_.notify_one();
}

void Recorder::run() {
    using clock = std::chrono::high_resolution_clock;

    try {

        /* Now that all the parameters are set, we can
         * open the video codec and allocate the
         * necessary encode buffers. */
        stream.openVideo();

        /* open the output file, if needed */
        out.open();

        AVDictionary* opt = nullptr;
        out.writeHeader(&opt);

        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [&]() { return stop_ || !queue_.empty(); });
        }

        int64_t frameCount = 0;
        Frame frame{};

        const auto start = clock::now();

        if (mode == Mode::Time) {
            const std::chrono::microseconds frameTime{1'000'000 / frameRate};

            auto next = start;

            while (!stop_) {
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait_until(lock, next, [&]() {
                        return stop_ || !queue_.empty() || clock::now() > next;
                    });

                    if (!queue_.empty()) {
                        if (frame) unused_.emplace_back(std::move(frame));
                        frame = std::move(queue_.front());
                        queue_.pop();
                    }
                }

                frame.frame->pts = frameCount++;
                writeFrame(out, stream.codec, stream.stream, frame, pkt);
                next += frameTime;
            }

        } else if (mode == Mode::Evaluation) {
            while (!stop_) {
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait(lock, [&]() { return stop_ || !queue_.empty(); });

                    if (!queue_.empty()) {
                        if (frame) unused_.emplace_back(std::move(frame));
                        frame = std::move(queue_.front());
                        queue_.pop();
                    }
                }

                frame.frame->pts = frameCount++;
                writeFrame(out, stream.codec, stream.stream, frame, pkt);
            }
        }

        // Flush the encoder
        writeFrame(out, stream.codec, stream.stream, Frame{}, pkt);

        out.writeTrailer();

        const auto end = clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        LogInfo("Exported frames: " << frameCount << " "
                                    << "time:" << ms.count() << " ms, fps: "
                                    << static_cast<double>(frameCount) / ms.count() * 1000);

    } catch (...) {
        std::unique_lock<std::mutex> lock(mutex_);
        eptr = std::current_exception();
    }
}

}  // namespace inviwo::ffmpeg
