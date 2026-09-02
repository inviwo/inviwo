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
#include <inviwo/ffmpeg/wrap/packet.h>

#include <filesystem>

struct AVFormatContext;
struct AVStream;

namespace inviwo::ffmpeg {

/**
 * @brief RAII wrapper around an AVFormatContext used for demuxing, that is reading media files.
 * @see OutputContext for the muxing counterpart
 */
class IVW_MODULE_FFMPEG_API InputContext : NoMoveCopy {
public:
    explicit InputContext(std::filesystem::path aFilename);
    InputContext(const InputContext&) = delete;
    InputContext(InputContext&&) = delete;
    InputContext& operator=(const InputContext&) = delete;
    InputContext& operator=(InputContext&&) = delete;
    ~InputContext();

    /**
     * @brief Index of the video stream ffmpeg considers the most suitable one
     * @throws Exception if the file contains no video stream
     */
    int bestVideoStream() const;

    /**
     * @throws RangeException if @p index is not a valid stream index
     */
    AVStream* stream(int index) const;

    int nbStreams() const;

    /**
     * @brief Read the next packet from the file
     * @return false at end of file, true otherwise
     */
    bool readPacket(Packet& pkt);

    /**
     * @brief Seek to the keyframe at or before @p timestamp, given in the time base of stream
     * @p streamIndex
     */
    void seek(int streamIndex, int64_t timestamp);

    std::filesystem::path filename;
    AVFormatContext* ctx;
};

}  // namespace inviwo::ffmpeg
