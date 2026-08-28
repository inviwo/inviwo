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

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/io/datareader.h>

#include <cstddef>
#include <optional>
#include <string_view>

namespace inviwo {

/**
 * @brief Option keys specific to the ffmpeg video readers, in addition to the ones in
 * inviwo::reader::option
 */
namespace videoreader::option {
/// Time in seconds of the frame to read, `double`. Mutually exclusive with reader::option::index,
/// whichever was set last is used.
inline constexpr std::string_view time = "time";
/// Index of the video stream to read, `int`. A negative value selects the best stream.
inline constexpr std::string_view stream = "stream";
}  // namespace videoreader::option

/**
 * @ingroup dataio
 * @brief Reads a single frame from a video file using ffmpeg
 *
 * Supported options
 *  - reader::option::index    frame to read, `std::ptrdiff_t`, negative values count from the end,
 *                             defaults to 0
 *  - videoreader::option::time   time in seconds of the frame to read, `double`
 *  - videoreader::option::stream video stream to read, `int`, defaults to -1
 */
class IVW_MODULE_FFMPEG_API FFmpegLayerReader : public DataReaderType<Layer> {
public:
    FFmpegLayerReader();
    FFmpegLayerReader(const FFmpegLayerReader& rhs) = default;
    virtual FFmpegLayerReader* clone() const override;
    virtual ~FFmpegLayerReader() = default;

    virtual std::shared_ptr<Layer> readData(const std::filesystem::path& filePath) override;
    using DataReaderType<Layer>::readData;

    virtual bool setOption(std::string_view key, std::any value) override;
    virtual std::any getOption(std::string_view key) override;

private:
    std::ptrdiff_t index_ = 0;
    std::optional<double> time_ = std::nullopt;
    int stream_ = -1;
};

/**
 * @ingroup dataio
 * @brief Reads a range of frames from a video file using ffmpeg
 *
 * Supported options
 *  - reader::option::index    first frame to read, `std::ptrdiff_t`, negative values count from the
 *                             end, defaults to 0
 *  - reader::option::count    number of frames to read, `std::ptrdiff_t`, a negative value reads
 *                             until the end of the video, defaults to
 *                             FFmpegLayerSequenceReader::defaultCount
 *  - reader::option::stride   step between the frames to read, `std::ptrdiff_t`, defaults to 1
 *  - videoreader::option::stream video stream to read, `int`, defaults to -1
 */
class IVW_MODULE_FFMPEG_API FFmpegLayerSequenceReader : public DataReaderType<LayerSequence> {
public:
    /// Videos can be arbitrarily long, only read this many frames unless told otherwise
    static constexpr std::ptrdiff_t defaultCount = 100;

    FFmpegLayerSequenceReader();
    FFmpegLayerSequenceReader(const FFmpegLayerSequenceReader& rhs) = default;
    virtual FFmpegLayerSequenceReader* clone() const override;
    virtual ~FFmpegLayerSequenceReader() = default;

    virtual std::shared_ptr<LayerSequence> readData(
        const std::filesystem::path& filePath) override;
    using DataReaderType<LayerSequence>::readData;

    virtual bool setOption(std::string_view key, std::any value) override;
    virtual std::any getOption(std::string_view key) override;

private:
    std::ptrdiff_t index_ = 0;
    std::ptrdiff_t count_ = defaultCount;
    std::ptrdiff_t stride_ = 1;
    int stream_ = -1;
};

}  // namespace inviwo
