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
#include <inviwo/ffmpeg/wrap/decoder.h>
#include <inviwo/ffmpeg/wrap/frame.h>
#include <inviwo/ffmpeg/wrap/inputcontext.h>
#include <inviwo/ffmpeg/wrap/packet.h>
#include <inviwo/ffmpeg/wrap/swscale.h>

#include <inviwo/core/util/glmvec.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

extern "C" {
#include <libavutil/avutil.h>
}

struct AVStream;

namespace inviwo {

class Layer;
class DataFormatBase;

namespace ffmpeg {

/**
 * @brief Random access to the frames of a video file, decoded into Layers.
 *
 * Frames are converted to a Layer format matching the source as closely as possible, see
 * Info::format. Color videos are decoded into RGBA, grayscale videos into a single channel, and
 * sources with more than 8 bits per component are decoded into 16 bit Layers.
 *
 * Seeking is done by seeking to the closest preceding keyframe and then decoding forward. Reading
 * frames in order is therefore substantially faster than random access.
 */
class IVW_MODULE_FFMPEG_API Video : NoMoveCopy {
public:
    struct Info {
        size2_t dimensions{0, 0};
        /// Layer data format the frames are decoded into
        const DataFormatBase* format = nullptr;
        double frameRate = 0.0;
        /// Duration in seconds
        double duration = 0.0;
        /// Number of frames, might be an estimate based on duration and frame rate
        std::ptrdiff_t frames = 0;
        std::string codec;
    };

    /**
     * @brief Open @p filename and prepare the video stream @p streamIndex for decoding
     * @param filename video file to open
     * @param streamIndex index of the video stream to decode, -1 selects the best one
     * @throws Exception if the file cannot be opened, holds no video stream, or if there is no
     * decoder available for it
     */
    explicit Video(const std::filesystem::path& filename, int streamIndex = -1);
    ~Video();

    const Info& info() const;
    const std::filesystem::path& filename() const;
    int streamIndex() const;

    /// Index of the frame shown at @p time seconds
    std::ptrdiff_t frameAt(double time) const;

    /**
     * @brief Decode the frame at @p index
     * @param index frame to read, negative values are counted from the end, that is -1 is the last
     * frame
     * @param reuse optional Layer to decode into. It is only used if it is non-null and matches the
     * format, dimensions, and type of the decoded frames, otherwise it is ignored and a new Layer
     * is allocated.
     * @return the decoded frame, or nullptr if the stream ended before reaching @p index
     * @throws RangeException if @p index is out of bounds
     */
    std::shared_ptr<Layer> readFrame(std::ptrdiff_t index, std::shared_ptr<Layer> reuse = {});

    /**
     * @brief Decode the frame following the most recently read one
     * @param reuse optional Layer to decode into, @see readFrame
     * @return the decoded frame, or nullptr at the end of the stream
     */
    std::shared_ptr<Layer> readNextFrame(std::shared_ptr<Layer> reuse = {});

    /// Position the stream so that the next call to readNextFrame() returns frame @p index
    void seekToFrame(std::ptrdiff_t index);

private:
    /// Decode the next frame into frame_, returns false at the end of the stream
    bool decodeNextFrame();
    /// Frame index of the frame currently held by frame_
    std::ptrdiff_t indexOfCurrentFrame() const;
    /// Presentation timestamp, in stream time base, of frame @p index
    int64_t timestampOf(std::ptrdiff_t index) const;
    std::shared_ptr<Layer> toLayer(std::shared_ptr<Layer> reuse);

    InputContext input_;
    int streamIndex_;
    AVStream* stream_;
    Decoder decoder_;
    Packet packet_;
    Frame frame_;

    std::optional<SwScale> scaler_;
    enum AVPixelFormat scalerSourceFormat_;
    enum AVPixelFormat targetFormat_;

    Info info_;
    /// Index of the frame held by frame_, or -1 if nothing has been decoded since the last seek
    std::ptrdiff_t current_;
    bool draining_;
};

}  // namespace ffmpeg

}  // namespace inviwo
