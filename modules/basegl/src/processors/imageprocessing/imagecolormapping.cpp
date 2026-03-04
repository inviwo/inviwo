/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagecolormapping.h>

#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/formats.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>

#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>

namespace inviwo {

const ProcessorInfo ImageColorMapping::processorInfo_{
    "org.inviwo.ImageColorMapping",  // Class identifier
    "Image Color Mapping",           // Display name
    "Image Operation",               // Category
    CodeState::Stable,               // Code state
    Tags::GL,                        // Tags
    R"(
    Maps the input image to an output image with the help of a transfer function.
    )"_unindentHelp,
};
const ProcessorInfo& ImageColorMapping::getProcessorInfo() const { return processorInfo_; }

ImageColorMapping::ImageColorMapping()
    : ImageGLProcessor("img_mapping.frag")
    , channel_{"channel", "Channel", "Selected channel used for mapping"_help,
               util::enumeratedOptions("Channel", 4)}
    , transferFunction_(
          "transferFunction", "Transfer Function",
          "The transfer function used for mapping input to output values including the "
          "alpha channel."_help) {
    addProperties(channel_, transferFunction_);
}

void ImageColorMapping::preProcess(TextureUnitContainer& container, Shader& shader) {
    utilgl::bindAndSetUniforms(shader, container, transferFunction_);
    utilgl::setUniforms(shader, channel_);
}

void ImageColorMapping::afterInportChanged() {
    if (inport_.hasData()) {
        // Determine the precision of the output format based on the input,
        // but always output 4 component data representing RGBA
        const DataFormatBase* inputDataFormat = inport_.getData()->getDataFormat();
        size_t precision = inputDataFormat->getPrecision();
        dataFormat_ = DataFormatBase::get(inputDataFormat->getNumericType(), 4, precision);
    }
}

}  // namespace inviwo
