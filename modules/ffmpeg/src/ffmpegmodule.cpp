/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2024 Inviwo Foundation
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

#include <inviwo/ffmpeg/ffmpegmodule.h>
#include <inviwo/ffmpeg/processors/movieexport.h>
#include <inviwo/ffmpeg/ffmpeganimationrecorder.h>

#include <array>

extern "C" {
#include <libavutil/log.h>
}

namespace {

void ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list vl) {
    std::array<char, 1024> line;
    std::string dynamicLine;
    static int print_prefix = 1;
    std::string_view message;

    using namespace inviwo;

    if (auto needSize = av_log_format_line2(ptr, level, fmt, vl, line.data(),
                                            static_cast<int>(line.size()), &print_prefix);
        needSize < 0) {
        log::report(LogLevel::Error, SourceContext{"ffmpeg"_sl}, "Error formatting log message");
    } else if (needSize >= static_cast<int>(line.size())) {
        dynamicLine.resize(needSize + 1);
        if (av_log_format_line2(ptr, level, fmt, vl, dynamicLine.data(),
                                static_cast<int>(dynamicLine.size()), &print_prefix) > 0) {

            message = dynamicLine;
        }
    } else {
        message = line.data();
    }

    if (message.empty()) return;

    const auto logLevel = [&]() {
        if (level >= 32) {
            return LogLevel::Info;
        } else if (level >= 24) {
            return LogLevel::Warn;
        } else {
            return LogLevel::Error;
        }
    }();

    log::report(logLevel, SourceContext{"ffmpeg"_sl}, message);
}

}  // namespace

namespace inviwo {

FFmpegModule::FFmpegModule(InviwoApplication* app)
    : InviwoModule(app, "ffmpeg"), animation::AnimationSupplier(app) {

    // Processors
    registerProcessor<MovieExport>();

    av_log_set_callback(ffmpeg_log_callback);

    registerRecorderFactory(std::make_unique<FFmpegRecorderFactory>());
}

}  // namespace inviwo
