/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/basegl/processors/layerprocessing/layergrayscale.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

const ProcessorInfo LayerGrayscale::processorInfo_{
    "org.inviwo.LayerGrayscale",  // Class identifier
    "Layer Grayscale",            // Display name
    "Layer Operation",            // Category
    CodeState::Stable,            // Code state
    Tags::GL,                     // Tags
    R"(Compute a gray-scale image from a color input layer. The alpha channel is not touched.
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

const ProcessorInfo& LayerGrayscale::getProcessorInfo() const { return processorInfo_; }

LayerGrayscale::LayerGrayscale()
    : LayerGLProcessor(utilgl::findShaderResource("img_graysc.frag"))
    , luminanceModel_{"luminanceModel", "Luminance Model", luminance::optionState()} {

    addProperty(luminanceModel_);
    outport_.setHelp("The grayscale output image."_help);
}

void LayerGrayscale::preProcess(TextureUnitContainer&, const Layer&, Layer&) {
    shader_.setUniform("weights", luminance::weights(luminanceModel_.get()));
}

}  // namespace inviwo
