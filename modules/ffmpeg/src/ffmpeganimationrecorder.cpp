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

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>

#include <inviwo/core/util/stringconversion.h>

#include <fmt/format.h>
#include <fmt/std.h>
#include <fmt/chrono.h>

namespace inviwo {

namespace {

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

    virtual void record(const Layer& layer) override;

private:
    std::unique_ptr<ffmpeg::Recorder> recorder;
};

void FFmpegRecorder::record(const Layer& layer) {
    if (recorder) {
        try {
            recorder->queueFrame(*layer.getRepresentation<LayerRAM>());
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
}

}  // namespace

FFmpegRecorderFactory::FFmpegRecorderFactory()
    : name_{"ffmpeg"}
    , options_{"ffmpeg", "FFmpeg Options"}
    , file_{"file", "File", "File to export movie to"_help, {}, AcceptMode::Save, FileMode::AnyFile}
    , format_{"format", "Format", ffmpeg::formatOptionsState()}
    , activeFormat_{"activeFormat", "Selected Format"}
    , codec_{"codex",
             "Codec",
             "Video codec - automatic will derive the format from the filename"_help,
             {{"automatic", "Automatic", ffmpeg::CodecID{}}},
             0}
    , activeCodec_{"activeCodec", "Selected Codec"}
    , bitRate_{"bitRate", "Bit Rate",
               util::ordinalCount<int>(8'000'000, 100'000'000)
                   .setMin(100'000)
                   .set("How many bit to spend per second"_help)}
    , overwrite_{"overwrite", "Overwrite", false} {

    options_.addProperties(file_, format_, activeFormat_, codec_, activeCodec_, bitRate_,
                           overwrite_);

    activeFormat_.setSerializationMode(PropertySerializationMode::None);
    activeFormat_.setReadOnly(true);
    activeCodec_.setSerializationMode(PropertySerializationMode::None);
    activeCodec_.setReadOnly(true);

    auto updateCodecs = [this]() { codec_.replaceOptions(codecsOptionsFor(getOutputFormat())); };

    auto updateActive = [this]() {
        auto outputFormat = getOutputFormat();
        activeFormat_.set(fmt::format("{} ({})", outputFormat.name(), outputFormat.longName()));

        auto codec = getCodec(outputFormat);
        activeCodec_.set(fmt::format("{} ({})", codec.name(), codec.longName().value_or("-")));
    };

    file_.onChange([updateCodecs, updateActive, this]() {
        if (ffmpeg::OutputFormat::guess(file_.get())) {
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

bool FFmpegRecorderFactory::guessFormat() const { return codec_.getSelectedIndex() == 0; }

ffmpeg::OutputFormat FFmpegRecorderFactory::getOutputFormat() const {
    if (guessFormat()) {
        return ffmpeg::OutputFormat::guess(file_.get());
    } else {
        return ffmpeg::OutputFormat(format_.getSelectedValue());
    }
}

bool FFmpegRecorderFactory::guessCodec() const { return codec_.getSelectedIndex() == 0; }
ffmpeg::CodecID FFmpegRecorderFactory::getCodec(const ffmpeg::OutputFormat& outputFormat) const {
    if (guessCodec()) {
        return outputFormat.guessVideoCodec(file_.get());
    } else {
        return codec_.getSelectedValue();
    }
}

const std::string& FFmpegRecorderFactory::getClassIdentifier() const { return name_; }

BoolCompositeProperty* FFmpegRecorderFactory::options() { return &options_; }

std::unique_ptr<animation::Recorder> FFmpegRecorderFactory::create(
    const animation::RecorderOptions& opts) {
    const auto format = (!guessFormat() ? ffmpeg::OutputFormat(format_.getSelectedValue())
                                        : ffmpeg::OutputFormat{});

    auto file = file_.get();

    auto name = file.filename().string();
    replaceInString(name, "UPN", opts.sourceName);
    file.replace_filename(name);

    if (!overwrite_ && std::filesystem::is_regular_file(file)) {
        throw Exception(IVW_CONTEXT, "File already exists: {}", file);
    }

    return std::make_unique<FFmpegRecorder>(
        file_.get(), format,
        ffmpeg::OutputStream::Options{.codecId = codec_.getSelectedValue(),
                                      .width = static_cast<int>(opts.dimensions.x),
                                      .height = static_cast<int>(opts.dimensions.y),
                                      .frameRate = opts.frameRate,
                                      .bitRate = bitRate_});
}

}  // namespace inviwo
