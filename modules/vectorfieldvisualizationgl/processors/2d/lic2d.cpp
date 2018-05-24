/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#include "lic2d.h"
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LIC2D::processorInfo_{
    "org.inviwo.LIC2D",            // Class identifier
    "LIC2D",                       // Display name
    "Vector Field Visualization",  // Category
    CodeState::Experimental,       // Code state
    Tags::GL,                      // Tags
};
const ProcessorInfo LIC2D::getProcessorInfo() const { return processorInfo_; }

LIC2D::LIC2D()
    : Processor()
    , vectorField_("vectorField", true)
    , noiseTexture_("noiseTexture", true)
    , LIC2D_("LIC2D")
    , samples_("samples", "Number of steps", 20, 3, 100)
    , stepLength_("stepLength", "Step Length", 0.003f, 0.0001f, 0.01f, 0.0001f)
    , normalizeVectors_("normalizeVectors", "Normalize vectors", true)
    , intensityMapping_("intensityMapping", "Enable intensity remapping", false)
    , useRK4_("useRK4", "Use Runge-Kutta4", true)
    , shader_("lic2d.frag") {
    addPort(vectorField_);
    addPort(noiseTexture_);
    addPort(LIC2D_);

    addProperty(samples_);
    addProperty(stepLength_);
    addProperty(normalizeVectors_);
    addProperty(intensityMapping_);
    addProperty(useRK4_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void LIC2D::process() {
    utilgl::activateAndClearTarget(LIC2D_);

    shader_.activate();
    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, vectorField_, ImageType::ColorOnly);
    utilgl::bindAndSetUniforms(shader_, units, noiseTexture_, ImageType::ColorOnly);

    utilgl::setUniforms(shader_, LIC2D_, samples_, stepLength_, normalizeVectors_,
                        intensityMapping_, useRK4_);

    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
