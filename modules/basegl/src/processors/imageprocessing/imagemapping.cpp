/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagemapping.h>

#include <inviwo/core/datastructures/image/layer.h>                      // for Layer
#include <inviwo/core/datastructures/representationconverter.h>          // for RepresentationCo...
#include <inviwo/core/datastructures/representationconverterfactory.h>   // for RepresentationCo...
#include <inviwo/core/datastructures/transferfunction.h>                 // for TransferFunction
#include <inviwo/core/ports/imageport.h>                                 // for ImageInport
#include <inviwo/core/processors/processorinfo.h>                        // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                       // for CodeState, CodeS...
#include <inviwo/core/processors/processortags.h>                        // for Tags, Tags::GL
#include <inviwo/core/properties/transferfunctionproperty.h>             // for TransferFunction...
#include <inviwo/core/util/formats.h>                                    // for DataFormatBase
#include <inviwo/core/util/logcentral.h>                                 // for LogCentral, LogWarn
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>  // for ImageGLProcessor
#include <modules/opengl/image/layergl.h>                                // for LayerGL
#include <modules/opengl/shader/shader.h>                                // for Shader
#include <modules/opengl/texture/textureunit.h>                          // for TextureUnit

#include <cstddef>        // for size_t
#include <memory>         // for shared_ptr, uniq...
#include <ostream>        // for operator<<, basi...
#include <string>         // for string
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set

namespace inviwo {

const ProcessorInfo ImageMapping::processorInfo_{
    "org.inviwo.ImageMapping",  // Class identifier
    "Image Mapping",            // Display name
    "Image Operation",          // Category
    CodeState::Stable,          // Code state
    Tags::GL,                   // Tags
};
const ProcessorInfo ImageMapping::getProcessorInfo() const { return processorInfo_; }

ImageMapping::ImageMapping()
    : ImageGLProcessor("img_mapping.frag")
    , transferFunction_("transferFunction", "Transfer Function") {
    addProperty(transferFunction_);
}

ImageMapping::~ImageMapping() {}

void ImageMapping::preProcess(TextureUnitContainer&) {
    TextureUnit transFuncUnit;
    const Layer* tfLayer = transferFunction_.get().getData();
    const LayerGL* transferFunctionGL = tfLayer->getRepresentation<LayerGL>();

    transferFunctionGL->bindTexture(transFuncUnit.getEnum());
    shader_.setUniform("transferFunc_", transFuncUnit.getUnitNumber());
}

void ImageMapping::afterInportChanged() {
    // Determine the precision of the output format based on the input,
    // but always output 4 component data representing RGBA
    const DataFormatBase* inputDataFormat = inport_.getData()->getDataFormat();
    size_t precision = inputDataFormat->getSize() / inputDataFormat->getComponents() * 8;
    const DataFormatBase* outputDataFormat =
        DataFormatBase::get(inputDataFormat->getNumericType(), 4, precision);
    if (dataFormat_ != outputDataFormat) {
        dataFormat_ = outputDataFormat;

        // The TF mapping currently only uses the first channel, print warning if we have more
        // channels
        if (inputDataFormat->getComponents() > 1)
            LogWarn("Input data has "
                    << inputDataFormat->getComponents()
                    << " components, only the first component will be used in the mapping");
    }
}

}  // namespace inviwo
