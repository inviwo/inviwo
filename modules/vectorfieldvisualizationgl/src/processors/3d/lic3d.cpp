/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/processors/3d/lic3d.h>

#include <inviwo/core/datastructures/datamapper.h>                         // for DataMapper
#include <inviwo/core/datastructures/transferfunction.h>                   // for TransferFunction
#include <inviwo/core/ports/volumeport.h>                                  // for VolumeInport
#include <inviwo/core/processors/processorinfo.h>                          // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                         // for CodeState, Cod...
#include <inviwo/core/processors/processortags.h>                          // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>                           // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>                        // for FloatProperty
#include <inviwo/core/properties/transferfunctionproperty.h>               // for TransferFuncti...
#include <inviwo/core/util/formats.h>                                      // for DataFormat
#include <inviwo/core/util/glmvec.h>                                       // for dvec2, vec4
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>  // for VolumeGLProcessor
#include <modules/opengl/shader/shader.h>                                  // for Shader
#include <modules/opengl/shader/shaderutils.h>                             // for setUniforms
#include <modules/opengl/texture/textureutils.h>                           // for bindAndSetUnif...
#include <modules/opengl/volume/volumeutils.h>                             // for bindAndSetUnif...

#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

#include <glm/mat3x3.hpp>  // for mat
#include <glm/matrix.hpp>  // for inverse

#include <fmt/base.h>

namespace inviwo {
class TextureUnitContainer;

const ProcessorInfo LIC3D::processorInfo_{
    "org.inviwo.LIC3D",            // Class identifier
    "LIC3D",                       // Display name
    "Vector Field Visualization",  // Category
    CodeState::Stable,             // Code state
    Tags::GL,                      // Tags
    R"(Computes the line integral convolution (LIC) in three dimensions by advecting a noise field
along a vector field and an optional convolution kernel. The result is a density volume representing
the 3D LIC.)"_unindentHelp,
};

const ProcessorInfo& LIC3D::getProcessorInfo() const { return processorInfo_; }

LIC3D::LIC3D()
    : VolumeGLProcessor{"lic3d.frag", VolumeConfig{.format = DataFloat32::get(),
                                                   .swizzleMask = swizzlemasks::defaultData(1),
                                                   .dataRange = DataMapper::defaultDataRangeFor(
                                                       DataFloat32::get())}}
    , vectorField_{"vectorField", "Input vector field"_help}
    , direction_{"direction",
                 "Integration Direction",
                 "Specificies the direction for the line integral convolution"_help,
                 {{"bidirectional", "Forward & Backward", IntegrationDirection::Bidirectional},
                  {"forward", "Forward", IntegrationDirection::Forward},
                  {"backward", "Backward", IntegrationDirection::Backward}},
                 0,
                 InvalidationLevel::InvalidResources}
    , kernel_{"convolutionKernel",
              "Convolution Kernel",
              "Kernel used in the line integral convolution"_help,
              {{"box", "Box Filter", Kernel::Box}, {"gaussian", "Gaussian", Kernel::Gaussian}},
              0,
              InvalidationLevel::InvalidResources}
    , samples_{"samples", "Steps",
               util::ordinalCount(100, 500).set(
                   "Number of integration steps in each direction"_help)}
    , stepLength_{"stepLength", "Step Length",
                  util::ordinalScale(0.003f, 1.0f)
                      .setInc(0.0001f)
                      .set("Distance between each step along of the integration"_help)}
    , normalizeVectors_{"normalizeVectors", "Normalize vectors",
                        "If set, the vectors are normalized "_help, true,
                        InvalidationLevel::InvalidResources}
    , noiseRepeat_{"noiseRepeat", "Noise Repeat", util::ordinalScale(2.0f, 10.0f).setInc(0.001f)}
    , alphaScale_{"alphaScale", "Density Scaling", util::ordinalScale(10.0f, 100.0f).setInc(0.001f)}
    , outputDimensions_{"outputDimensions",
                        "Output Dimensions",
                        "Sets the dimensions of the resulting 3D LIC Volume"_help,
                        {{"noiseVolume", "Noise Volume", OutputDimensions::NoiseVolume},
                         {"vectorField", "Vector Field", OutputDimensions::VectorField},
                         {"custom", "Custom", OutputDimensions::Custom}}}
    , dims_{"dims", "Dimensions",
            OrdinalPropertyState<ivec3>{.value = ivec3{1},
                                        .min = ivec3{1},
                                        .minConstraint = ConstraintBehavior::Immutable,
                                        .max = ivec3{2048},
                                        .maxConstraint = ConstraintBehavior::Ignore,
                                        .semantics = PropertySemantics::Text}} {

    addPort(vectorField_);
    addProperties(outputDimensions_, dims_, direction_, kernel_, samples_, stepLength_,
                  normalizeVectors_, noiseRepeat_, alphaScale_);

    dims_.readonlyDependsOn(outputDimensions_, [](auto& p) {
        return p.getSelectedValue() != OutputDimensions::Custom;
    });

    IVW_ASSERT(inport_.has_value(), "Inport should be constructed");
    inport_->setHelp("Input noise volume"_help);
}

void LIC3D::initializeShader(Shader& shader) {
    auto* fragShader = shader.getFragmentShaderObject();
    if (kernel_ == Kernel::Gaussian) {
        fragShader->addShaderDefine("KERNEL", "gaussian");
    } else {
        fragShader->addShaderDefine("KERNEL", "box");
    }
    fragShader->addShaderDefine("INTEGRATION_DIRECTION",
                                fmt::format("{}", static_cast<int>(direction_.getSelectedValue())));
    if (normalizeVectors_) {
        fragShader->addShaderDefine("NORMALIZATION");
    } else {
        fragShader->removeShaderDefine("NORMALIZATION");
    }
}

void LIC3D::preProcess(TextureUnitContainer& cont, Shader& shader, VolumeConfig& config) {
    if (outputDimensions_.isModified()) {
        if (auto value = outputDimensions_.getSelectedValue();
            value == OutputDimensions::NoiseVolume) {
            dims_ = ivec3{(*inport_).getData()->getDimensions()};
        } else if (value == OutputDimensions::VectorField) {
            dims_ = ivec3{vectorField_.getData()->getDimensions()};
        }
    }

    utilgl::bindAndSetUniforms(shader, cont, *vectorField_.getData(), "vectorField");
    utilgl::setUniforms(shader, samples_, stepLength_, noiseRepeat_, alphaScale_);

    shader.setUniform("invBasis", glm::inverse(vectorField_.getData()->getBasis()));

    auto source = vectorField_.getData();
    config.dataRange = dvec2{0.0, 1.0};
    config.valueRange = dvec2{0.0, 1.0};
    config.xAxis = source->axes[0];
    config.yAxis = source->axes[1];
    config.zAxis = source->axes[2];
    config.model = source->getModelMatrix();
    config.world = source->getWorldMatrix();
    config.dimensions = size3_t{dims_.get()};
}

void LIC3D::postProcess(Volume& volume) {
    // since this processor uses VolumeGLProcessor::inport_ for the noise texture, we need to
    // copy all metadata including transformations from the vector field here
    volume.copyMetaDataFrom(*vectorField_.getData());
}

}  // namespace inviwo
