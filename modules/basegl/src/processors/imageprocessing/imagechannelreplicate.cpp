/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagechannelreplicate.h>

namespace inviwo {

const ProcessorInfo ImageChannelReplicate::processorInfo_{
    "org.inviwo.ImageChannelReplicate",  // Class identifier
    "Image Channel Replicate",           // Display name
    "Image Operation",                   // Category
    CodeState::Experimental,             // Code state
    Tags::GL,                            // Tags
};
const ProcessorInfo ImageChannelReplicate::getProcessorInfo() const { return processorInfo_; }

ImageChannelReplicate::ImageChannelReplicate()
    : ImageGLProcessor("img_channel_replicate.frag")
    , alpha_("alpha", "Alpha", 1.0f, 0.0f, 1.0f, 0.001f)
{
    addProperty(alpha_);
    dataFormat_ = DataVec4UInt8::get();
}

void ImageChannelReplicate::preProcess(TextureUnitContainer &)
{
    shader_.setUniform("alpha_", alpha_.get());
}

}  // namespace inviwo
