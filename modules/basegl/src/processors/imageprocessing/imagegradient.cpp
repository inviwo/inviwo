/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagegradient.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageGradient::processorInfo_{
    "org.inviwo.ImageGradient",  // Class identifier
    "Image Gradient",            // Display name
    "Image Operation",           // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
};
const ProcessorInfo ImageGradient::getProcessorInfo() const { return processorInfo_; }

ImageGradient::ImageGradient()
    : ImageGLProcessor("imagegradient.frag")
    , channel_("channel", "Channel")
    , renormalization_("renormalization", "Renormalization", true) {
    dataFormat_ = DataVec2Float32::get();
    swizzleMask_ = {
        {ImageChannel::Red, ImageChannel::Green, ImageChannel::Zero, ImageChannel::One}};

    channel_.addOption("Channel 1", "Channel 1", 0);
    channel_.setCurrentStateAsDefault();

    inport_.onChange([&]() {
        if (inport_.hasData()) {
            int channels = static_cast<int>(inport_.getData()->getDataFormat()->getComponents());
            if (channels == static_cast<int>(channel_.size())) return;
            channel_.clearOptions();
            for (int i = 0; i < channels; i++) {
                std::stringstream ss;
                ss << "Channel " << i;
                channel_.addOption(ss.str(), ss.str(), i);
            }
            channel_.setCurrentStateAsDefault();
        }
    });

    addProperty(channel_);
    addProperty(renormalization_);
}

void ImageGradient::preProcess(TextureUnitContainer &) {
    shader_.setUniform("channel", channel_.getSelectedValue());
    shader_.setUniform("renormalization_", renormalization_.get() ? 1 : 0);
}

}  // namespace inviwo
