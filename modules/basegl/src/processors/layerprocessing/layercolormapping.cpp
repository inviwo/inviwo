/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <modules/basegl/processors/layerprocessing/layercolormapping.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerColorMapping::processorInfo_{
    "org.inviwo.LayerColorMapping",  // Class identifier
    "Layer Color Mapping",           // Display name
    "Layer Operation",               // Category
    CodeState::Stable,               // Code state
    Tags::GL,                        // Tags
    R"(Applies a transferfunction to one channel of a Layer. The format of the RGBA output will
    match the input with a value range of [0, 1].)"_unindentHelp,
};

const ProcessorInfo& LayerColorMapping::getProcessorInfo() const { return processorInfo_; }

LayerColorMapping::LayerColorMapping()
    : LayerGLProcessor{utilgl::findShaderResource("img_mapping.frag")}
    , channel_{"channel", "Channel", "Selected channel used for color mapping"_help,
               util::enumeratedOptions("Channel", 4)}
    , transferFunction_(
          "transferFunction", "Transfer Function",
          "The transfer function used for mapping input to output values including the "
          "alpha channel."_help,
          TransferFunction{{
              {0.0, vec4(0.0f, 0.0f, 0.0f, 0.0f)},
              {1.0, vec4(1.0f, 1.0f, 1.0f, 1.0f)},
          }},
          &inport_) {
    addProperties(channel_, transferFunction_);
}

void LayerColorMapping::preProcess(TextureUnitContainer& container, const Layer&, Layer&) {
    utilgl::bindAndSetUniforms(shader_, container, transferFunction_);
    utilgl::setUniforms(shader_, channel_);
}

LayerConfig LayerColorMapping::outputConfig([[maybe_unused]] const Layer& input) const {
    const auto* inputFormat = input.getDataFormat();
    const auto* outputFormat =
        DataFormatBase::get(inputFormat->getNumericType(), 4, inputFormat->getPrecision());
    return input.config().updateFrom({.format = outputFormat,
                                      .swizzleMask = swizzlemasks::rgba,
                                      .dataRange = DataMapper::defaultDataRangeFor(inputFormat),
                                      .valueRange = dvec2{0.0, 1.0}});
}

}  // namespace inviwo
