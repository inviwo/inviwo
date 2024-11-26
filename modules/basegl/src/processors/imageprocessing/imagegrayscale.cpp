/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagegrayscale.h>

#include <inviwo/core/processors/processorinfo.h>                        // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                       // for CodeState, CodeS...
#include <inviwo/core/processors/processortags.h>                        // for Tags, Tags::GL
#include <inviwo/core/properties/optionproperty.h>                       // for OptionPropertyInt
#include <inviwo/core/util/glmvec.h>                                     // for vec3
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>  // for ImageGLProcessor
#include <modules/opengl/shader/shader.h>                                // for Shader

#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {
class TextureUnitContainer;

namespace luminance {
std::vector<OptionPropertyOption<Model>> options() {
    return {{"perceived", "Perceived", Model::PerceivedLum},
            {"relative", "Relative", Model::RelativeLum},
            {"average", "Average", Model::AverageLum},
            {"red", "Red only", Model::RedOnly},
            {"green", "Green only", Model::GreenOnly},
            {"blue", "Blue only", Model::BlueOnly}};
}

OptionPropertyState<Model> optionState() {
    return {.options = options(),
            .selectedIndex = 0,
            .help =
                "Model for converting the input to grayscale. Options are perceived (default), "
                "relative, average, red only, green only, and blue only"_help};
}
}  // namespace luminance

const ProcessorInfo ImageGrayscale::processorInfo_{
    "org.inviwo.ImageGrayscale",  // Class identifier
    "Image Grayscale",            // Display name
    "Image Operation",            // Category
    CodeState::Stable,            // Code state
    Tags::GL,                     // Tags
    R"(Compute a gray-scale image from a color input image. The alpha channel is not touched.
    The input image is converted to gray-scale as follows
    grayValue = l.r * in.r + l.g * in.g + l.b * in.b
    out.rgb = vec3(grayValue)
    out.a = in.a
    The color conversion factor _l_ depends on the chosen luminance model:
     * _perceived_ l.rgb = vec3(0.299, 0.587, 0.114)
     * _relative_ l.rgb = vec3(0.2126, 0.7152, 0.0722), XYZ color space
     * _average_ l.rgb = vec3(1/3, 1/3, 1/3)
     * _red_ l.rgb = vec3(1, 0, 0)
     * _green_ l.rgb = vec3(0, 1, 0)
     * _blue_ l.rgb = vec3(0, 0, 1))"_unindentHelp};

const ProcessorInfo& ImageGrayscale::getProcessorInfo() const { return processorInfo_; }

ImageGrayscale::ImageGrayscale()
    : ImageGLProcessor("img_graysc.frag")
    , luminanceModel_("luminanceModel", "Luminance Model", luminance::optionState()) {

    addProperty(luminanceModel_);
    outport_.setHelp("The grayscale output image."_help);
}

void ImageGrayscale::preProcess(TextureUnitContainer&) {
    shader_.setUniform("weights", luminance::weights(luminanceModel_.get()));
}

}  // namespace inviwo
