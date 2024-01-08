/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/processors/datageneration/vectorfieldgenerator2d.h>

#include <inviwo/core/datastructures/image/image.h>       // for Image
#include <inviwo/core/datastructures/image/imagetypes.h>  // for ImageChannel, ImageChannel::Green
#include <inviwo/core/datastructures/image/layer.h>       // for Layer
#include <inviwo/core/ports/imageport.h>                  // for ImageOutport
#include <inviwo/core/processors/processor.h>             // for Processor
#include <inviwo/core/processors/processorinfo.h>         // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>        // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>         // for Tags, Tags::GL
#include <inviwo/core/properties/invalidationlevel.h>     // for InvalidationLevel, Invalidation...
#include <inviwo/core/properties/minmaxproperty.h>        // for FloatMinMaxProperty
#include <inviwo/core/properties/stringproperty.h>        // for StringProperty
#include <inviwo/core/util/formats.h>                     // for DataVec2Float32, DataFormat
#include <inviwo/core/util/glmvec.h>                      // for size2_t
#include <modules/opengl/shader/shader.h>                 // for Shader, Shader::Build
#include <modules/opengl/shader/shaderobject.h>           // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>            // for setUniforms
#include <modules/opengl/texture/textureutils.h>          // for activateAndClearTarget, deactiv...

#include <memory>       // for make_shared, shared_ptr
#include <string>       // for basic_string, string
#include <string_view>  // for string_view

namespace inviwo {

const ProcessorInfo VectorFieldGenerator2D::processorInfo_{
    "org.inviwo.VectorFieldGenerator2D",  // Class identifier
    "Vector Field Generator 2D",          // Display name
    "Data Creation",                      // Category
    CodeState::Stable,                    // Code state
    Tags::GL,                             // Tags
};
const ProcessorInfo VectorFieldGenerator2D::getProcessorInfo() const { return processorInfo_; }

VectorFieldGenerator2D::VectorFieldGenerator2D()
    : Processor()
    , outport_("outport", DataVec2Float32::get(), false)
    , size_("size", "Field size", size2_t(16), size2_t(1), size2_t(1024))
    , xRange_("xRange", "X Range", -1, 1, -10, 10)
    , yRange_("yRange", "Y Range", -1, 1, -10, 10)
    , xValue_("x", "X", "-x", InvalidationLevel::InvalidResources)
    , yValue_("y", "Y", "y", InvalidationLevel::InvalidResources)
    , shader_("vectorfieldgenerator2d.frag", Shader::Build::No) {
    addPort(outport_);

    addProperties(size_, xValue_, yValue_, xRange_, yRange_);
}

VectorFieldGenerator2D::~VectorFieldGenerator2D() = default;

void VectorFieldGenerator2D::initializeResources() {
    shader_.getFragmentShaderObject()->addShaderDefine("X_VALUE(x,y)", xValue_.get());
    shader_.getFragmentShaderObject()->addShaderDefine("Y_VALUE(x,y)", yValue_.get());

    shader_.build();
}

void VectorFieldGenerator2D::process() {

    auto layer = std::make_shared<Layer>(
        size_, DataVec2Float32::get(), LayerType::Color,
        SwizzleMask{ImageChannel::Red, ImageChannel::Green, ImageChannel::Zero, ImageChannel::One});
    auto image = std::make_shared<Image>(layer);

    utilgl::activateAndClearTarget(*image, ImageType::ColorOnly);

    shader_.activate();
    utilgl::setUniforms(shader_, xRange_, yRange_);

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();

    outport_.setData(image);
}

}  // namespace inviwo
