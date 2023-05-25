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

#include <inviwo/ffmpeg/processors/movieexport.h>

#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/util/glmconvert.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/threadutil.h>

#include <inviwo/ffmpeg/wrap/codec.h>
#include <inviwo/ffmpeg/wrap/codecid.h>
#include <inviwo/ffmpeg/wrap/format.h>
#include <inviwo/ffmpeg/wrap/frame.h>
#include <inviwo/ffmpeg/wrap/outputformat.h>
#include <inviwo/ffmpeg/wrap/packet.h>
#include <inviwo/ffmpeg/wrap/swscale.h>
#include <inviwo/ffmpeg/outputstream.h>
#include <inviwo/ffmpeg/util.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <chrono>
#include <vector>

#include <fmt/format.h>
#include <fmt/std.h>
#include <fmt/chrono.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MovieExport::processorInfo_{"org.inviwo.MovieExport",   // Class identifier
                                                "Movie Export",             // Display name
                                                "Data Output",              // Category
                                                CodeState::Stable,          // Code state
                                                Tags::CPU | Tag("Export"),  // Tags
                                                R"(Export image data to a movie)"_unindentHelp};

const ProcessorInfo MovieExport::getProcessorInfo() const { return processorInfo_; }

MovieExport::MovieExport()
    : Processor{}
    , inport_{"inport", "Image data to export"_help, OutportDeterminesSize::Yes}
    , file_{"file", "File", "File to export movie to"_help, {}, AcceptMode::Save, FileMode::AnyFile}
    , format_{"format", "Format", ffmpeg::formatOptionsState()}
    , activeFormat_{"activeFormat", "Selected Format"}
    , codec_{"codec",
             "Codec",
             "Video codec - automatic will derive the format from the filename"_help,
             {{"automatic", "Automatic", ffmpeg::CodecID{}}},
             0}
    , activeCodec_{"activeCodec", "Selected Codec"}
    , mode_{"mode",
            "Mode",
            {{"time", "Time-based recoding", ffmpeg::Recorder::Mode::Time},
             {"eval", "Evaluation-based recoding", ffmpeg::Recorder::Mode::Evaluation}},
            0}
    , frameRate_{"frameRate", "Frame Rate",
                 util::ordinalCount<int>(25, 120).setMin(0).set(
                     "How many frames to export per second of wallclock"_help)}
    , bitRate_{"bitRate", "Bit Rate",
               util::ordinalCount<int>(8'000'000, 100'000'000)
                   .setMin(100'000)
                   .set("How many bits to spend per second"_help)}
    , start_{"start", "Start"}
    , stop_{"stop", "Stop"} {

    addPorts(inport_);
    addProperties(file_, format_, activeFormat_, codec_, activeCodec_, mode_, frameRate_, bitRate_,
                  start_, stop_);

    activeFormat_.setSerializationMode(PropertySerializationMode::None);
    activeFormat_.setReadOnly(true);
    activeCodec_.setSerializationMode(PropertySerializationMode::None);
    activeCodec_.setReadOnly(true);

    auto updateCodecs = [this]() {
        auto outputFormat = getOutputFormat();
        codec_.replaceOptions(ffmpeg::codecsOptionsFor(outputFormat));
    };

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

MovieExport::~MovieExport() = default;

bool MovieExport::guessFormat() const { return codec_.getSelectedIndex() == 0; }

ffmpeg::OutputFormat MovieExport::getOutputFormat() const {
    if (guessFormat()) {
        return ffmpeg::OutputFormat::guess(file_.get());
    } else {
        return ffmpeg::OutputFormat(format_.getSelectedValue());
    }
}

bool MovieExport::guessCodec() const { return codec_.getSelectedIndex() == 0; }
ffmpeg::CodecID MovieExport::getCodec(const ffmpeg::OutputFormat& outputFormat) const {
    if (guessCodec()) {
        return outputFormat.guessVideoCodec(file_.get());
    } else {
        return codec_.getSelectedValue();
    }
}

void MovieExport::process() {
    auto img = inport_.getData();

    if (start_.isModified()) {

        auto format = (!guessFormat() ? ffmpeg::OutputFormat(format_.getSelectedValue())
                                      : ffmpeg::OutputFormat{});

        recorder = std::make_unique<ffmpeg::Recorder>(
            file_.get(), format, mode_,
            ffmpeg::OutputStream::Options{.codecId = codec_.getSelectedValue(),
                                          .width = static_cast<int>(img->getDimensions().x),
                                          .height = static_cast<int>(img->getDimensions().y),
                                          .frameRate = frameRate_,
                                          .bitRate = bitRate_});

        notifyObserversStartBackgroundWork(this, 1);

        util::logInfo(IVW_CONTEXT, "Recording to: {}", file_.get());
        util::logInfo(IVW_CONTEXT, "  - Format:   {}", recorder->getFormat().outputFormat().desc());
        util::logInfo(IVW_CONTEXT, "  - Codec:    {}", recorder->getStream().codec.codecID());
    }

    if (recorder) {
        try {
            recorder->queueFrame(*img->getColorLayer()->getRepresentation<LayerRAM>());
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

    if (stop_.isModified()) {
        recorder.reset();
        notifyObserversFinishBackgroundWork(this, 1);
    }
}

}  // namespace inviwo
