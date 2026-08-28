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

#include <inviwo/ffmpeg/ffmpegvideoreader.h>

#include <inviwo/ffmpeg/video.h>

#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/exception.h>

#include <algorithm>
#include <array>
#include <utility>

#include <fmt/format.h>
#include <fmt/std.h>

namespace inviwo {

namespace {

constexpr std::array<std::pair<std::string_view, std::string_view>, 17> videoExtensions{
    {{"mp4", "MPEG-4 Video"},
     {"m4v", "MPEG-4 Video"},
     {"mov", "QuickTime Movie"},
     {"mkv", "Matroska Video"},
     {"webm", "WebM Video"},
     {"avi", "Audio Video Interleave"},
     {"wmv", "Windows Media Video"},
     {"flv", "Flash Video"},
     {"mpg", "MPEG Video"},
     {"mpeg", "MPEG Video"},
     {"mts", "AVCHD Video"},
     {"m2ts", "Blu-ray BDAV Video"},
     {"ts", "MPEG Transport Stream"},
     {"ogv", "Ogg Video"},
     {"3gp", "3GPP Video"},
     {"y4m", "YUV4MPEG2 Video"},
     {"gif", "Graphics Interchange Format"}}};

// The DataReaderFactory is keyed on the full FileExtension, so the two readers need distinct
// descriptions to be able to register for the same file extensions
void addVideoExtensions(DataReader& reader, std::string_view suffix) {
    for (auto&& [extension, description] : videoExtensions) {
        reader.addExtension(FileExtension{.extension = LCString{extension},
                                          .description = fmt::format("{}{}", description, suffix)});
    }
}

std::optional<std::ptrdiff_t> toOffset(const std::any& value) {
    if (const auto* offset = std::any_cast<std::ptrdiff_t>(&value)) {
        return *offset;
    } else if (const auto* integer = std::any_cast<int>(&value)) {
        return static_cast<std::ptrdiff_t>(*integer);
    } else if (const auto* unsignedSize = std::any_cast<size_t>(&value)) {
        return static_cast<std::ptrdiff_t>(*unsignedSize);
    }
    return std::nullopt;
}

/// Accept both a chrono duration and a plain number of seconds
std::optional<FFmpegLayerReader::Seconds> toSeconds(const std::any& value) {
    if (const auto* seconds = std::any_cast<FFmpegLayerReader::Seconds>(&value)) {
        return *seconds;
    } else if (const auto* number = std::any_cast<double>(&value)) {
        return FFmpegLayerReader::Seconds{*number};
    }
    return std::nullopt;
}

}  // namespace

FFmpegLayerReader::FFmpegLayerReader() : DataReaderType<Layer>{} { addVideoExtensions(*this, ""); }

FFmpegLayerReader* FFmpegLayerReader::clone() const { return new FFmpegLayerReader{*this}; }

std::shared_ptr<Layer> FFmpegLayerReader::readData(const std::filesystem::path& filePath) {
    const auto path = downloadAndCacheIfUrl(filePath);
    checkExists(path);

    try {
        ffmpeg::Video video{path, stream_};
        const auto index = time_ ? video.frameAt(*time_) : index_;
        if (auto layer = video.readFrame(index)) {
            return layer;
        }
        throw DataReaderException(SourceContext{}, "Could not read frame {} of '{}'", index,
                                  filePath);
    } catch (const DataReaderException&) {
        throw;
    } catch (const Exception& e) {
        throw DataReaderException(SourceContext{}, "Error reading '{}': {}", filePath,
                                  e.getMessage());
    }
}

bool FFmpegLayerReader::setOption(std::string_view key, std::any value) {
    if (key == reader::option::index) {
        if (auto index = toOffset(value)) {
            index_ = *index;
            time_.reset();
            return true;
        }
    } else if (key == videoreader::option::time) {
        if (auto time = toSeconds(value)) {
            time_ = *time;
            return true;
        }
    } else if (key == videoreader::option::stream) {
        if (const auto* stream = std::any_cast<int>(&value)) {
            stream_ = *stream;
            return true;
        }
    }
    return false;
}

std::any FFmpegLayerReader::getOption(std::string_view key) {
    if (key == reader::option::index) {
        return index_;
    } else if (key == videoreader::option::time) {
        return time_ ? std::any{*time_} : std::any{};
    } else if (key == videoreader::option::stream) {
        return stream_;
    }
    return std::any{};
}

FFmpegLayerSequenceReader::FFmpegLayerSequenceReader() : DataReaderType<LayerSequence>{} {
    addVideoExtensions(*this, " Sequence");
}

FFmpegLayerSequenceReader* FFmpegLayerSequenceReader::clone() const {
    return new FFmpegLayerSequenceReader{*this};
}

std::shared_ptr<LayerSequence> FFmpegLayerSequenceReader::readData(
    const std::filesystem::path& filePath) {
    const auto path = downloadAndCacheIfUrl(filePath);
    checkExists(path);

    try {
        ffmpeg::Video video{path, stream_};
        const auto frames = video.info().frames;

        auto first = index_;
        if (first < 0) {
            if (frames <= 0) {
                throw DataReaderException(
                    SourceContext{},
                    "Cannot use a negative frame index, the number of frames in '{}' is unknown",
                    filePath);
            }
            first += frames;
        }
        if (first < 0) {
            throw DataReaderException(SourceContext{},
                                      "Frame index {} is out of range, '{}' has {} frames", index_,
                                      filePath, frames);
        }

        const auto stride = std::max<std::ptrdiff_t>(stride_, 1);

        auto sequence = std::make_shared<LayerSequence>();
        for (std::ptrdiff_t i = 0; count_ < 0 || i < count_; ++i) {
            const auto index = first + i * stride;
            if (frames > 0 && index >= frames) break;

            auto layer = video.readFrame(index);
            if (!layer) break;
            sequence->push_back(std::move(layer));
        }

        if (sequence->empty()) {
            throw DataReaderException(SourceContext{}, "Could not read any frames from '{}'",
                                      filePath);
        }
        return sequence;
    } catch (const DataReaderException&) {
        throw;
    } catch (const Exception& e) {
        throw DataReaderException(SourceContext{}, "Error reading '{}': {}", filePath,
                                  e.getMessage());
    }
}

bool FFmpegLayerSequenceReader::setOption(std::string_view key, std::any value) {
    if (key == reader::option::index) {
        if (auto index = toOffset(value)) {
            index_ = *index;
            return true;
        }
    } else if (key == reader::option::count) {
        if (auto count = toOffset(value)) {
            count_ = *count;
            return true;
        }
    } else if (key == reader::option::stride) {
        if (auto stride = toOffset(value)) {
            stride_ = *stride;
            return true;
        }
    } else if (key == videoreader::option::stream) {
        if (const auto* stream = std::any_cast<int>(&value)) {
            stream_ = *stream;
            return true;
        }
    }
    return false;
}

std::any FFmpegLayerSequenceReader::getOption(std::string_view key) {
    if (key == reader::option::index) {
        return index_;
    } else if (key == reader::option::count) {
        return count_;
    } else if (key == reader::option::stride) {
        return stride_;
    } else if (key == videoreader::option::stream) {
        return stream_;
    }
    return std::any{};
}

}  // namespace inviwo
