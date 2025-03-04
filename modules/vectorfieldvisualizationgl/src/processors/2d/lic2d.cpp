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

#include <modules/vectorfieldvisualizationgl/processors/2d/lic2d.h>

#include <inviwo/core/util/formats.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

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
    , samples_{"samples", "Number of Steps",
               util::ordinalCount(20, 100).setMin(2).set(
                   "Total number of steps, both forward and backward."_help)}
    , stepLength_{"stepLength", "Step Length",
                  util::ordinalScale(0.003f, 0.01f).set("Length of each integration step."_help)}
    , normalizeVectors_{"normalizeVectors", "Normalize Vectors",
                        "Vectors are normalized prior integration."_help, true}
    , useRK4_{"useRK4", "Use Runge-Kutta4",
              "Use Runge-Kutta fourth order as integration scheme, otherwise Euler integration is used."_help,
              true}
    , postProcessing_{"postProcessing", "Post Processing",
                      "Apply some basic image operations to enhance the LIC"_help, true}
    , intensityMapping_{"intensityMapping", "Enable Intensity Remapping",
                        "Remap the resulting intensity values $v$ with $v^(5 / (v + 1)^4)$. "
                        "This will increase contrast."_help,
                        false}
    , brightness_{"brightness",
                  "Brightness",
                  "Adjusts the overall brightness"_help,
                  0.0f,
                  {-1.0f, ConstraintBehavior::Immutable},
                  {1.0f, ConstraintBehavior::Immutable}}
    , contrast_{"contrast",
                "Contrast",
                "Adjusts the overall contrast"_help,
                0.0f,
                {-1.0f, ConstraintBehavior::Immutable},
                {1.0f, ConstraintBehavior::Immutable}}
    , gamma_{"gamma",
             "Gamma Correction",
             "Gamma correction using $v^gamma$"_help,
             1.0f,
             {0.0f, ConstraintBehavior::Immutable},
             {2.0f, ConstraintBehavior::Immutable}} {

    outport_.setHelp("Resulting layer with grayscale LIC (single channel)"_help);
    addPort(noiseTexture_);
    addProperties(samples_, stepLength_, useRK4_, normalizeVectors_, postProcessing_);
    postProcessing_.addProperties(intensityMapping_, brightness_, contrast_, gamma_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void LIC2D::preProcess(TextureUnitContainer& cont, const Layer&, Layer&) {
    utilgl::bindAndSetUniforms(shader_, cont, noiseTexture_);
    utilgl::setUniforms(shader_, samples_, stepLength_, normalizeVectors_, useRK4_);
    shader_.setUniform("postProcessing", postProcessing_.isChecked());
    if (postProcessing_) {
        utilgl::setUniforms(shader_, intensityMapping_, brightness_, contrast_, gamma_);
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
