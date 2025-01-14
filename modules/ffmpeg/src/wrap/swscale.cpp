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

#include <inviwo/ffmpeg/wrap/swscale.h>
#include <inviwo/core/util/exception.h>

extern "C" {
#include <libswscale/swscale.h>
}

namespace inviwo::ffmpeg {

SwScale::SwScale(int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH,
                 enum AVPixelFormat dstFormat, int flags, SwsFilter* srcFilter,
                 SwsFilter* dstFilter, const double* param)
    : ctx{sws_getContext(srcW, srcH, srcFormat, dstW, dstH, dstFormat, flags, srcFilter, dstFilter,
                         param)} {

    if (!ctx) {
        throw inviwo::Exception("Could not initialize the scale context");
    }
}

SwScale::~SwScale() { sws_freeContext(ctx); }

int SwScale::scale(const uint8_t* const srcSlice[], const int srcStride[], int srcSliceY,
                   int srcSliceH, uint8_t* const dst[], const int dstStride[]) {
    return sws_scale(ctx, srcSlice, srcStride, srcSliceY, srcSliceH, dst, dstStride);
}

}  // namespace inviwo::ffmpeg
