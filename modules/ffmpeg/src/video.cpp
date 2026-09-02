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

#include <inviwo/ffmpeg/video.h>

#include <inviwo/ffmpeg/wrap/codecid.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/formats.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

#include <array>
#include <cmath>
#include <limits>

#include <fmt/std.h>

namespace inviwo::ffmpeg {

namespace {

struct Target {
    enum AVPixelFormat pixelFormat;
    const DataFormatBase* format;
};

/**
 * Pick the Layer format that matches @p source as closely as possible, and the pixel format
 * swscale should convert to in order to fill it.
 */
Target targetFor(enum AVPixelFormat source) {
    const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(source);
    const bool gray = desc && desc->nb_components == 1;
    const bool deep = desc && desc->comp[0].depth > 8;

    if (gray) {
        return deep ? Target{.pixelFormat = AV_PIX_FMT_GRAY16, .format = DataUInt16::get()}
                    : Target{.pixelFormat = AV_PIX_FMT_GRAY8, .format = DataUInt8::get()};
    } else {
        return deep ? Target{.pixelFormat = AV_PIX_FMT_RGBA64, .format = DataVec4UInt16::get()}
                    : Target{.pixelFormat = AV_PIX_FMT_RGBA, .format = DataVec4UInt8::get()};
    }
}

/// Number of frames we decode past the current position before we resort to seeking
constexpr std::ptrdiff_t maxForwardDecode = 60;

}  // namespace

Video::Video(const std::filesystem::path& filename, int streamIndex)
    : input_{filename}
    , streamIndex_{streamIndex >= 0 ? streamIndex : input_.bestVideoStream()}
    , stream_{input_.stream(streamIndex_)}
    , decoder_{stream_}
    , packet_{}
    , frame_{Frame::NoBuffers{}}
    , scaler_{std::nullopt}
    , scalerSourceFormat_{AV_PIX_FMT_NONE}
    , targetFormat_{AV_PIX_FMT_NONE}
    , info_{}
    , current_{-1}
    , pending_{std::numeric_limits<std::ptrdiff_t>::min()}
    , draining_{false} {

    const auto* codecpar = stream_->codecpar;
    if (codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
        throw Exception(SourceContext{}, "Stream {} of '{}' is not a video stream", streamIndex_,
                        input_.filename);
    }
    if (codecpar->width <= 0 || codecpar->height <= 0) {
        throw Exception(SourceContext{}, "Stream {} of '{}' has invalid dimensions {}x{}",
                        streamIndex_, input_.filename, codecpar->width, codecpar->height);
    }

    const auto source = codecpar->format != AV_PIX_FMT_NONE
                            ? static_cast<enum AVPixelFormat>(codecpar->format)
                            : decoder_.pixelFormat();
    const auto target = targetFor(source);
    targetFormat_ = target.pixelFormat;

    auto rate = stream_->avg_frame_rate;
    if (rate.num <= 0 || rate.den <= 0) {
        rate = av_guess_frame_rate(input_.ctx, stream_, nullptr);
    }
    info_.frameRate = av_q2d(rate);
    if (!(info_.frameRate > 0.0)) {
        info_.frameRate = 25.0;
    }

    if (stream_->duration != AV_NOPTS_VALUE && stream_->duration > 0) {
        info_.duration =
            Seconds{static_cast<double>(stream_->duration) * av_q2d(stream_->time_base)};
    } else if (input_.ctx->duration != AV_NOPTS_VALUE && input_.ctx->duration > 0) {
        info_.duration = Seconds{static_cast<double>(input_.ctx->duration) / AV_TIME_BASE};
    }

    if (stream_->nb_frames > 0) {
        info_.frames = static_cast<std::ptrdiff_t>(stream_->nb_frames);
    } else if (info_.duration.count() > 0.0) {
        info_.frames =
            static_cast<std::ptrdiff_t>(std::llround(info_.duration.count() * info_.frameRate));
    }

    info_.dimensions = size2_t{codecpar->width, codecpar->height};
    info_.format = target.format;
    info_.codec = CodecID{codecpar->codec_id}.name();
}

Video::~Video() = default;

const Video::Info& Video::info() const { return info_; }
const std::filesystem::path& Video::filename() const { return input_.filename; }
int Video::streamIndex() const { return streamIndex_; }

std::ptrdiff_t Video::frameAt(Seconds time) const {
    return static_cast<std::ptrdiff_t>(std::llround(time.count() * info_.frameRate));
}

Video::Seconds Video::timeOf(std::ptrdiff_t index) const {
    return Seconds{static_cast<double>(index) / info_.frameRate};
}

std::ptrdiff_t Video::currentFrame() const { return current_; }

Video::Seconds Video::currentTime() const { return timeOf(current_); }

int64_t Video::timestampOf(std::ptrdiff_t index) const {
    const int64_t start = stream_->start_time != AV_NOPTS_VALUE ? stream_->start_time : 0;
    const auto seconds = timeOf(index);
    return start + std::llround(seconds.count() / av_q2d(stream_->time_base));
}

std::ptrdiff_t Video::indexOfCurrentFrame() const {
    auto pts = frame_.frame->best_effort_timestamp;
    if (pts == AV_NOPTS_VALUE) {
        pts = frame_.frame->pts;
    }
    if (pts == AV_NOPTS_VALUE) {
        return current_ + 1;
    }
    const int64_t start = stream_->start_time != AV_NOPTS_VALUE ? stream_->start_time : 0;
    const double seconds = static_cast<double>(pts - start) * av_q2d(stream_->time_base);
    return static_cast<std::ptrdiff_t>(std::llround(seconds * info_.frameRate));
}

bool Video::decodeNextFrame() {
    while (true) {
        const auto ret = decoder_.receiveFrame(frame_);
        if (ret == 0) return true;
        if (ret == AVERROR_EOF) return false;

        // AVERROR(EAGAIN), the decoder needs more input
        if (draining_) return false;

        bool sent = false;
        while (input_.readPacket(packet_)) {
            if (packet_.pkt->stream_index == streamIndex_) {
                decoder_.sendPacket(packet_);
                sent = true;
                break;
            }
        }
        if (!sent) {
            decoder_.flush();
            draining_ = true;
        }
    }
}

void Video::seekToFrame(std::ptrdiff_t index) {
    input_.seek(streamIndex_, timestampOf(index));
    decoder_.reset();
    draining_ = false;
    current_ = index - 1;
    pending_ = index;
}

void Video::seekToTime(Seconds time) { seekToFrame(frameAt(time)); }

std::shared_ptr<Layer> Video::readNextFrame(std::shared_ptr<Layer> reuse) {
    while (decodeNextFrame()) {
        current_ = indexOfCurrentFrame();
        if (current_ >= pending_) {
            pending_ = std::numeric_limits<std::ptrdiff_t>::min();
            return toLayer(std::move(reuse));
        }
    }
    return nullptr;
}

std::shared_ptr<Layer> Video::readFrame(std::ptrdiff_t index, std::shared_ptr<Layer> reuse) {
    auto target = index;
    if (target < 0) {
        if (info_.frames <= 0) {
            throw RangeException(SourceContext{},
                                 "Cannot use a negative frame index, the number of frames in '{}' "
                                 "is unknown",
                                 input_.filename);
        }
        target += info_.frames;
    }
    if (target < 0 || (info_.frames > 0 && target >= info_.frames)) {
        throw RangeException(SourceContext{}, "Frame index {} is out of range, '{}' has {} frames",
                             index, input_.filename, info_.frames);
    }

    const auto next = current_ + 1;
    if (target < next || target > next + maxForwardDecode) {
        seekToFrame(target);
    }
    pending_ = target;

    return readNextFrame(std::move(reuse));
}

std::shared_ptr<Layer> Video::toLayer(std::shared_ptr<Layer> reuse) {
    const auto width = static_cast<int>(info_.dimensions.x);
    const auto height = static_cast<int>(info_.dimensions.y);
    const auto source = static_cast<enum AVPixelFormat>(frame_.frame->format);

    if (!scaler_ || scalerSourceFormat_ != source) {
        scaler_.emplace(frame_.frame->width, frame_.frame->height, source, width, height,
                        targetFormat_, SWS_BICUBIC, nullptr, nullptr, nullptr);
        scalerSourceFormat_ = source;
    }

    auto layer = [&]() {
        if (reuse && reuse->getDataFormat() == info_.format &&
            reuse->getDimensions() == info_.dimensions &&
            reuse->getLayerType() == LayerType::Color) {
            return reuse;
        }
        return std::make_shared<Layer>(info_.dimensions, info_.format, LayerType::Color,
                                       swizzlemasks::defaultColor(info_.format->getComponents()));
    }();

    // Preserve the aspect ratio of the video, normalizing the larger dimension to 1
    glm::dmat3 basis{1.0};
    if (info_.dimensions.x < info_.dimensions.y) {
        basis[0][0] = static_cast<double>(info_.dimensions.x) / static_cast<double>(info_.dimensions.y);
    } else {
        basis[1][1] = static_cast<double>(info_.dimensions.y) / static_cast<double>(info_.dimensions.x);
    }
    layer->setBasis(basis);

    auto* ram = layer->getEditableRepresentation<LayerRAM>();
    auto* data = static_cast<uint8_t*>(ram->getData());

    // All four target pixel formats, GRAY8, GRAY16, RGBA, and RGBA64, are single plane and packed,
    // so only plane 0 is used. getSizeInBytes() accounts for both the component size and the
    // number of channels.
    const auto stride = static_cast<int>(info_.dimensions.x * info_.format->getSizeInBytes());

    // Layers have their origin in the lower left corner, ffmpeg frames are top-down. Flip by
    // starting at the last row and using a negative stride.
    const std::array<uint8_t*, 4> dst{data + static_cast<std::ptrdiff_t>(height - 1) * stride,
                                      nullptr, nullptr, nullptr};
    const std::array<int, 4> dstStride{-stride, 0, 0, 0};

    scaler_->scale(frame_.frame->data, frame_.frame->linesize, 0, frame_.frame->height, dst.data(),
                   dstStride.data());

    const auto pts = frame_.frame->best_effort_timestamp;
    const int64_t start = stream_->start_time != AV_NOPTS_VALUE ? stream_->start_time : 0;
    const auto time = pts != AV_NOPTS_VALUE
                          ? Seconds{static_cast<double>(pts - start) * av_q2d(stream_->time_base)}
                          : timeOf(current_);

    layer->setMetaData<StringMetaData>("filename", input_.filename.generic_string());
    layer->setMetaData<IntMetaData>("frameIndex", static_cast<int>(current_));
    layer->setMetaData<IntMetaData>("frameCount", static_cast<int>(info_.frames));
    layer->setMetaData<DoubleMetaData>("frameRate", info_.frameRate);
    layer->setMetaData<DoubleMetaData>("time", time.count());

    return layer;
}

}  // namespace inviwo::ffmpeg
