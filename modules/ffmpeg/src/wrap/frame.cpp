/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <inviwo/ffmpeg/wrap/frame.h>

#include <inviwo/core/util/exception.h>
#include <inviwo/ffmpeg/util.h>

extern "C" {
#include <libavutil/frame.h>
}

namespace inviwo::ffmpeg {

Frame::Frame() : frame{nullptr} {}

Frame::Frame(enum AVPixelFormat pix_fmt, int width, int height) : frame{av_frame_alloc()} {

    if (!frame) {
        throw inviwo::Exception("Could not allocate frame data.");
    }

    frame->format = pix_fmt;
    frame->width = width;
    frame->height = height;

    /* allocate the buffers for the frame data */
    if (av_frame_get_buffer(frame, 0) < 0) {
        throw inviwo::Exception("Could not allocate frame data.");
    }
}

Frame::Frame(Frame&& rhs) : frame{std::exchange(rhs.frame, nullptr)} {}
Frame& Frame::operator=(Frame&& that) {
    std::swap(frame, that.frame);
    return *this;
}
Frame::~Frame() {
    if (frame) av_frame_free(&frame);
}

Frame::operator bool() const { return frame != nullptr; }

void Frame::makeWritable() {
    if (auto ret = av_frame_make_writable(frame); ret < 0) {
        throw inviwo::Exception(SourceContext{}, "Could not make video frame writable {}",
                                Error{ret});
    }
}

}  // namespace inviwo::ffmpeg
