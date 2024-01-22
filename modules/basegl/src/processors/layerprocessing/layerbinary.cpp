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

#include <modules/basegl/processors/layerprocessing/layerbinary.h>

#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerBinary::processorInfo_{
    "org.inviwo.LayerBinary",  // Class identifier
    "Layer Binary",            // Display name
    "Layer Operation",         // Category
    CodeState::Stable,         // Code state
    Tags::GL | Tag{"Layer"},   // Tags
    R"(Computes a binary image of the input layer using a threshold. The output will
       contain "0" for all values below the threshold and "1" otherwise.)"_unindentHelp};

const ProcessorInfo LayerBinary::getProcessorInfo() const { return processorInfo_; }

namespace {

constexpr std::string_view fragmentShader = util::trim(R"(
#include "utils/structs.glsl"

uniform sampler2D inport;
uniform ImageParameters inportParameters;
uniform ImageParameters outportParameters;
uniform int channel = 0;
uniform float threshold = 0.5;

void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;
    const bvec4 b = bvec4(texture(inport, texCoords)[channel] >= threshold);
    FragData0 = mix(vec4(0), vec4(1), b);
}
)");

const std::vector<OptionPropertyIntOption> channelsList = {{"channel1", "Channel 1", 0},
                                                           {"channel2", "Channel 2", 1},
                                                           {"channel3", "Channel 3", 2},
                                                           {"channel4", "Channel 4", 3}};
}  // namespace

LayerBinary::LayerBinary()
    : LayerGLProcessor{std::make_shared<StringShaderResource>("LayerBinary.frag", fragmentShader)}
    , channel_{"channel", "Channel", "Selected channel used for binarization"_help, channelsList}
    , threshold_{"threshold", "Threshold",
                 util::ordinalSymmetricVector(0.5f, 1.0f)
                     .set("Threshold used for binarization of the input layer"_help)} {

    addProperties(channel_, threshold_);
}

void LayerBinary::preProcess(TextureUnitContainer&) {
    utilgl::setUniforms(shader_, channel_, threshold_);
}

void LayerBinary::postProcess() {
    layer_->dataMap.initWithFormat(layer_->getDataFormat());
    layer_->dataMap.valueRange = dvec2(0.0, 1.0);
}

}  // namespace inviwo
