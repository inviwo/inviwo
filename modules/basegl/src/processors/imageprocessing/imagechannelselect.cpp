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

#include <modules/basegl/processors/imageprocessing/imagechannelselect.h>

namespace inviwo {

const ProcessorInfo ImageChannelSelect::processorInfo_{
    "org.inviwo.ImageChannelSelect",  // Class identifier
    "Image Channel Select",           // Display name
    "Image Operation",                // Category
    CodeState::Experimental,          // Code state
    Tags::GL,                         // Tags
};
const ProcessorInfo ImageChannelSelect::getProcessorInfo() const { return processorInfo_; }

static const std::string channelShaderDefine = "CHANNEL";

ImageChannelSelect::ImageChannelSelect()
    : ImageGLProcessor("img_channel_select.frag")
    , channelSelector_("channel", "Channel",
                       {{"r", "Red", 0}, {"g", "Green", 1}, {"b", "Blue", 2}, {"a", "Alpha", 3}}) {

    addProperty(channelSelector_);
}

void ImageChannelSelect::preProcess(TextureUnitContainer &) {
    auto inDF = inport_.getData()->getDataFormat();
    dataFormat_ = DataFormatBase::get(inDF->getNumericType(), 1, inDF->getPrecision());
    shader_.setUniform("channel", channelSelector_.get());
}
void ImageChannelSelect::postProcess() { swizzleMask_ = swizzlemasks::luminance; }

}  // namespace inviwo
