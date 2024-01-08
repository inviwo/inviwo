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
#pragma once

#include <inviwo/ffmpeg/ffmpegmoduledefine.h>

#include <inviwo/core/datastructures/image/layerram.h>

#include <inviwo/ffmpeg/outputstream.h>
#include <inviwo/ffmpeg/wrap/packet.h>
#include <inviwo/ffmpeg/wrap/format.h>

#include <thread>
#include <queue>
#include <vector>
#include <filesystem>
#include <mutex>
#include <condition_variable>

namespace inviwo::ffmpeg {

class IVW_MODULE_FFMPEG_API Recorder {
public:
    enum class Mode { Time, Evaluation };

    Recorder(const std::filesystem::path& filename, OutputFormat format, Mode aMode,
             OutputStream::Options opts);
    ~Recorder();

    const OutputStream& getStream();
    const Format& getFormat();

    /**
     * Copies the image data in layer into a ffmpeg frames and enques that for encoding
     * The layer will not be used after the return of the function.
     */
    void queueFrame(const LayerRAM& layer);

private:
    void run();

    Mode mode;
    Format out;
    OutputStream stream;
    Packet pkt;

    std::queue<Frame> queue_;
    std::vector<Frame> unused_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    std::exception_ptr eptr;
    int frameRate;

    std::thread worker;
};

}  // namespace inviwo::ffmpeg
