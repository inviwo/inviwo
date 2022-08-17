/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <modules/opengl/buffer/framebufferobject.h>
#include <array>
#include <string_view>

namespace inviwo {
/** \class VolumeNormalization
 *
 * GL implementation of volume normalization. The algorithm takes in a volume and normalized its
 * data in the selected channels to range [0,1].
 * Note that this algorithm normalizes channels independently, it does not normalize a multi-channel
 * volume in terms of vector norms!
 */
class IVW_MODULE_BASEGL_API VolumeNormalization {
public:
    template <typename Callback>
    VolumeNormalization(Callback C) : VolumeNormalization() {
        shader_.onReload(C);
    }
    VolumeNormalization();

    virtual ~VolumeNormalization() {}

    /**
     * Performs the normalization on the GPU.
     *
     * @param volume Input volume to be normalized.
     * @return A volume whose selected channels have been normalized.
     */
    std::shared_ptr<Volume> normalize(const Volume& volume);

    /**
     * Sets the channel that are to be normalized. In practice, this method in-/ejects shader
     * defines for every channel.
     *
     * @param channel Channel for which the normalization should be set to true or false.
     * @param normalize Boolean value indicating whether or not the selected channel should be
     * normalized.
     */
    void setNormalizeChannel(const size_t channel, const bool normalize);

    /**
     * Sets the channels that are to be normalized. In practice, this method in-/ejects shader
     * defines for every channel.
     *
     * @param normalize Set of boolean values indicating which channels to normalize.
     */
    void setNormalizeChannels(bvec4 normalize);

    /**
     * Resets the normalization settings. Channel 0 is set to true, rest to false.
     */
    void reset();

protected:
    Shader shader_;
    FrameBufferObject fbo_;

private:
    const std::array<std::string_view, 4> defines_{
        "NORMALIZE_CHANNEL_0", "NORMALIZE_CHANNEL_1", "NORMALIZE_CHANNEL_2",
        "NORMALIZE_CHANNEL_3"};  ///< Some string_view indirection.
    bool needsCompilation_;
};

}  // namespace inviwo
