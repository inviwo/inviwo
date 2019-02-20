/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/postprocessing/processors/imagehuesaturationluminance.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageHueSaturationLuminance::processorInfo_{
    "org.inviwo.ImageHueSaturationLuminance",  // Class identifier
    "Image Hue Saturation Luminance",          // Display name
    "Image Operation",                         // Category
    CodeState::Stable,                         // Code state
    Tags::None,                                // Tags
};
const ProcessorInfo ImageHueSaturationLuminance::getProcessorInfo() const { return processorInfo_; }

ImageHueSaturationLuminance::ImageHueSaturationLuminance()
    : ImageGLProcessor("huesaturationluminance.frag")
    , hue_("hue", "Hue", 0.f, 0.f, 1.f, .01f)
    , saturation_("sat", "Saturation", 0.f, -1.f, 1.f, .01f)
    , luminance_("lum", "Luminance", 0.f, -1.f, 1.f, .01f) {

    addProperty(hue_);
    addProperty(saturation_);
    addProperty(luminance_);
}

void ImageHueSaturationLuminance::preProcess(TextureUnitContainer &) {
    utilgl::setUniforms(shader_, hue_, saturation_, luminance_);
}

}  // namespace inviwo
