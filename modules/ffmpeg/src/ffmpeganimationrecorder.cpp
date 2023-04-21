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

#include <inviwo/ffmpeg/ffmpeganimationrecorder.h>

#include <inviwo/core/datastructures/image/layerram.h>

#include <fmt/format.h>
#include <fmt/std.h>
#include <fmt/chrono.h>

namespace inviwo {

class FFmpegRecorder : public animation::Recorder {
public:
    FFmpegRecorder(const std::filesystem::path& filename, ffmpeg::OutputFormat format,
                   ffmpeg::OutputStream::Options opts)
        : recorder{std::make_unique<ffmpeg::Recorder>(filename, format,
                                                      ffmpeg::Recorder::Mode::Evaluation, opts)} {

        util::logInfo(IVW_CONTEXT, "Recording to: {}", filename);
        util::logInfo(IVW_CONTEXT, "  - Format:   {}", recorder->getFormat().outputFormat().desc());
        util::logInfo(IVW_CONTEXT, "  - Codec:    {}", recorder->getStream().codec.codecID());
    }
    virtual ~FFmpegRecorder() = default;

    virtual void record(const LayerRAM& layer) override;

private:
    std::unique_ptr<ffmpeg::Recorder> recorder;
};

void FFmpegRecorder::record(const LayerRAM& layer) {
    if (recorder) {
        try {
            recorder->queueFrame(layer);
        } catch (const Exception& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error);
            recorder.reset();
        } catch (const std::exception& e) {
            util::log(IVW_CONTEXT, e.what(), LogLevel::Error);
            recorder.reset();
        } catch (...) {
            util::log(IVW_CONTEXT, "unknown error", LogLevel::Error);
            recorder.reset();
        }
    }
};

FFmpegRecorderFactory::FFmpegRecorderFactory()
    : name_{"ffmpeg"}
    , options_{"ffmpeg", "FFmpeg Options"}
    , file_{"file", "File", "File to export movie to"_help, {}, AcceptMode::Save, FileMode::AnyFile}
    , format_{"format", "Format",
              []() -> OptionPropertyState<std::string> {
                  std::vector<OptionPropertyOption<std::string>> opts;
                  opts.emplace_back("automatic", "Automatic", "automatic");

                  ffmpeg::OutputFormat::forEach([&](const ffmpeg::OutputFormat& format) {
                      if (format.defaultVideoCodec()) {
                          opts.push_back(OptionPropertyOption<std::string>{
                              std::string(format.name()),
                              fmt::format("{} ({})", format.name(), format.longName()),
                              std::string(format.name())});
                      }
                  });

                  return {.options = std::move(opts),
                          .selectedIndex = 0,
                          .help =
                              "Movie container format, 'automatic' will guess"
                              " a format from the filename"_help};
              }()}
    , activeFormat_{"activeFormat", "Selected Format"}
    , codec_{"codex",
             "Codec",
             "Video codec, automatic will derive the format from the filename"_help,
             {{"automatic", "Automatic", ffmpeg::CodecID{}}},
             0}
    , activeCodec_{"activeCodec", "Selected Codec"}
    , frameRate_{"frameRate", "Frame Rate",
                 util::ordinalCount<int>(25, 120).setMin(0).set(
                     "How many frames to export per second of wallclock"_help)}
    , bitRate_{"bitRate", "Bit Rate",
               util::ordinalCount<int>(8'000'000, 100'000'000)
                   .setMin(100'000)
                   .set("How many bit to spend per second"_help)} {

    options_.addProperties(file_, format_, activeFormat_, codec_, activeCodec_, frameRate_,
                           bitRate_);

    activeFormat_.setSerializationMode(PropertySerializationMode::None);
    activeFormat_.setReadOnly(true);
    activeCodec_.setSerializationMode(PropertySerializationMode::None);
    activeCodec_.setReadOnly(true);

    auto updateCodecs = [this]() {
        auto outputFormat = [&]() {
            if (format_.getSelectedIndex() == 0) {
                return ffmpeg::OutputFormat::guess(file_.get());
            } else {
                return ffmpeg::OutputFormat(format_.getSelectedValue());
            }
        }();

        std::vector<OptionPropertyOption<ffmpeg::CodecID>> opts;
        opts.emplace_back("automatic", "Automatic", ffmpeg::CodecID{});

        for (auto& codecId : outputFormat.supportedCodecs(AVMEDIA_TYPE_VIDEO)) {
            opts.push_back(OptionPropertyOption<ffmpeg::CodecID>{
                std::string(codecId.name()),
                fmt::format("{} ({})", codecId.name(), codecId.longName().value_or("-")), codecId});
        }

        codec_.replaceOptions(std::move(opts));
    };

    auto updateActive = [this]() {
        auto outputFormat = [&]() {
            if (format_.getSelectedIndex() == 0) {
                return ffmpeg::OutputFormat::guess(file_.get());
            } else {
                return ffmpeg::OutputFormat(format_.getSelectedValue());
            }
        }();

        activeFormat_.set(fmt::format("{} ({})", outputFormat.name(), outputFormat.longName()));

        auto codec = [&]() {
            if (codec_.getSelectedIndex() == 0) {
                return outputFormat.guessVideoCodec(file_.get());
            } else {
                return codec_.getSelectedValue();
            }
        }();

        activeCodec_.set(fmt::format("{} ({})", codec.name(), codec.longName().value_or("-")));
    };

    file_.onChange([updateCodecs, updateActive, this]() {
        if (auto outputFormat = ffmpeg::OutputFormat::guess(file_.get())) {
            updateCodecs();
            updateActive();
        }
    });

    format_.onChange([updateCodecs, updateActive]() {
        updateCodecs();
        updateActive();
    });

    codec_.onChange([updateActive]() { updateActive(); });
}

const std::string& FFmpegRecorderFactory::getClassIdentifier() const { return name_; }

CompositeProperty* FFmpegRecorderFactory::options() { return &options_; }

std::unique_ptr<animation::Recorder> FFmpegRecorderFactory::create(size2_t dimensions) {
    const auto format =
        (format_.getSelectedIndex() != 0 ? ffmpeg::OutputFormat(format_.getSelectedValue())
                                         : ffmpeg::OutputFormat{});

    return std::make_unique<FFmpegRecorder>(
        file_.get(), format,
        ffmpeg::OutputStream::Options{.codecId = codec_.getSelectedValue(),
                                      .width = static_cast<int>(dimensions.x),
                                      .height = static_cast<int>(dimensions.y),
                                      .frameRate = frameRate_,
                                      .bitRate = bitRate_});
}

}  // namespace inviwo
