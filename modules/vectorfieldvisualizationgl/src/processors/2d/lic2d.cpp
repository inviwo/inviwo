/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2026 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/processors/2d/lic2d.h>

#include <inviwo/core/util/formats.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>

#include <glm/matrix.hpp>  // for inverse

namespace inviwo {

namespace {

std::string_view toShaderString(LIC2D::Kernel kernel) {
    switch (kernel) {
        case LIC2D::Kernel::Gaussian:
            return "gaussian";
        case LIC2D::Kernel::Box:
        default:
            return "box";
    }
}

}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LIC2D::processorInfo_{
    "org.inviwo.LIC2D",            // Class identifier
    "LIC2D",                       // Display name
    "Vector Field Visualization",  // Category
    CodeState::Stable,             // Code state
    Tags::GL | Tag{"Layer"},       // Tags
    R"(Performs a line integral convolution (LIC) on a 2D vector field.)"_unindentHelp,
};
const ProcessorInfo& LIC2D::getProcessorInfo() const { return processorInfo_; }

LIC2D::LIC2D()
    : LayerGLProcessor{utilgl::findShaderResource("lic2d.frag")}
    , noiseTexture_{"noiseTexture",
                    "2D noise layer which will be convoluted along the vector field"_help}
    , direction_{"direction",
                 "Integration Direction",
                 "Specificies the direction for the line integral convolution."_help,
                 {{"bidirectional", "Forward & Backward", IntegrationDirection::Bidirectional},
                  {"forward", "Forward", IntegrationDirection::Forward},
                  {"backward", "Backward", IntegrationDirection::Backward}},
                 0,
                 InvalidationLevel::InvalidResources}
    , kernel_{"convolutionKernel",
              "Convolution Kernel",
              "Kernel used in the line integral convolution."_help,
              {{"box", "Box Filter", Kernel::Box}, {"gaussian", "Gaussian", Kernel::Gaussian}},
              0,
              InvalidationLevel::InvalidResources}
    , samples_{"samples", "Number of Steps",
               util::ordinalCount(20, 100).set(
                   "Number of integration steps in each direction."_help)}
    , stepLength_{"stepLength", "Step Length",
                  util::ordinalScale(0.003f, 0.01f)
                      .setInc(0.0001f)
                      .set("Length of each integration step."_help)}
    , normalizeVectors_{"normalizeVectors", "Normalize Vectors",
                        "If set, the vectors are normalized prior integration."_help, true,
                        InvalidationLevel::InvalidResources}
    , useRK4_{"useRK4", "Use Runge-Kutta4",
              "Use Runge-Kutta fourth order as integration scheme, otherwise Euler integration is used."_help,
              true, InvalidationLevel::InvalidResources}
    , postProcessing_{"postProcessing", "Post Processing",
                      "Apply some basic image operations to enhance the LIC"_help, true}
    , intensityMapping_{"intensityMapping", "Enable Intensity Remapping",
                        "Remap the resulting intensity values v with pow(v, 5 / pow(v + 1, 4)). "
                        "This will increase contrast."_help,
                        false}
    , brightness_{"brightness",
                  "Brightness",
                  "Adjusts the overall brightness."_help,
                  0.0f,
                  {-1.0f, ConstraintBehavior::Immutable},
                  {1.0f, ConstraintBehavior::Immutable}}
    , contrast_{"contrast",
                "Contrast",
                "Adjusts the overall contrast."_help,
                0.0f,
                {-1.0f, ConstraintBehavior::Immutable},
                {1.0f, ConstraintBehavior::Immutable}}
    , gamma_{"gamma",
             "Gamma Correction",
             "Gamma correction using pow(v, γ)."_help,
             1.0f,
             {0.0f, ConstraintBehavior::Immutable},
             {2.0f, ConstraintBehavior::Immutable}} {

    outport_.setHelp("Resulting layer with grayscale LIC (single channel)"_help);
    addPort(noiseTexture_);
    addProperties(direction_, kernel_, samples_, stepLength_, useRK4_, normalizeVectors_,
                  postProcessing_);
    postProcessing_.addProperties(intensityMapping_, brightness_, contrast_, gamma_);
}

void LIC2D::initializeShader(Shader& shader) {
    auto* fragShader = shader.getFragmentShaderObject();
    fragShader->addShaderDefine("KERNEL", toShaderString(kernel_));
    fragShader->addShaderDefine("INTEGRATION_DIRECTION",
                                fmt::format("{}", static_cast<int>(direction_.getSelectedValue())));
    fragShader->setShaderDefine("KERNEL_NORMALIZATION", kernel_ != Kernel::Gaussian);
    fragShader->setShaderDefine("NORMALIZATION", normalizeVectors_);
    fragShader->setShaderDefine("USE_RUNGEKUTTA", useRK4_);
}

void LIC2D::preProcess(TextureUnitContainer& cont, Shader& shader, const Layer& input, Layer&) {
    utilgl::bindAndSetUniforms(shader, cont, noiseTexture_);
    utilgl::setUniforms(shader, samples_, stepLength_);
    shader.setUniform("worldToTexture", glm::inverse(mat2(input.getBasis())));

    const auto wrapping = input.getWrapping();
    const vec2 clampMin{
        wrapping[0] == Wrapping::Clamp ? 0.0f : std::numeric_limits<float>::lowest(),
        wrapping[1] == Wrapping::Clamp ? 0.0f : std::numeric_limits<float>::lowest()};
    const vec2 clampMax{wrapping[0] == Wrapping::Clamp ? 1.0f : std::numeric_limits<float>::max(),
                        wrapping[1] == Wrapping::Clamp ? 1.0f : std::numeric_limits<float>::max()};
    shader.setUniform("clampMin", clampMin);
    shader.setUniform("clampMax", clampMax);

    shader.setUniform("postProcessing", postProcessing_.isChecked());
    if (postProcessing_) {
        utilgl::setUniforms(shader, intensityMapping_, brightness_, contrast_, gamma_);
    }
}

LayerConfig LIC2D::outputConfig(const Layer& input) const {
    const DataFormatBase* format = input.getDataFormat();

    return input.config().updateFrom(
        {.format = DataFormatBase::get(format->getNumericType(), 1, format->getPrecision()),
         .swizzleMask = swizzlemasks::defaultData(1),
         .dataRange = dvec2{0.0, 1.0},
         .valueRange = dvec2{0.0, 1.0}});
}

}  // namespace inviwo
