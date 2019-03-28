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

#include <modules/base/processors/imagecontourprocessor.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageContourProcessor::processorInfo_{
    "org.inviwo.ImageContourProcessor",  // Class identifier
    "Image Contour",                     // Display name
    "Image Processing",                  // Category
    CodeState::Experimental,             // Code state
    Tags::None,                          // Tags
};
const ProcessorInfo ImageContourProcessor::getProcessorInfo() const { return processorInfo_; }

ImageContourProcessor::ImageContourProcessor()
    : Processor()
    , image_("image", true)
    , mesh_("mesh")
    , channel_("channel", "Channel", 0, 0, 4)
    , isoValue_("iso", "ISO Value", 0.5, 0, 1)
    , color_("color", "Color", vec4(1.0)) {

    addPort(image_);
    addPort(mesh_);
    addProperty(channel_);
    addProperty(isoValue_);
    addProperty(color_);
    color_.setSemantics(PropertySemantics::Color);
    color_.setCurrentStateAsDefault();
}

void ImageContourProcessor::process() {
    if (image_.isChanged()) {
        auto max = image_.getData()->getDataFormat()->getComponents() - 1;
        channel_.setMaxValue(max);
    }
    mesh_.setData(
        ImageContour::apply(image_.getData()->getColorLayer()->getRepresentation<LayerRAM>(),
                            channel_, isoValue_, color_));
}

}  // namespace inviwo
