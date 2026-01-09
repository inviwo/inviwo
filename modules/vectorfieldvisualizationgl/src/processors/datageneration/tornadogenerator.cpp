/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/processors/datageneration/tornadogenerator.h>

#include <modules/opengl/shader/stringshaderresource.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

namespace {

constexpr std::string_view tornado = R"(

in vec4 dataposition_;

uniform float time = 0.0f;

void main() {

    float x = dataposition_.x;
    float y = dataposition_.y;
    float z = dataposition_.z;
    
    // For each z-slice, determine the spiral circle.
    float xc = 0.5 + 0.1 * sin(0.04 * time + 10.0 * z);

    // (xc,yc) determine the center of the circle.
    float yc = 0.5 + 0.1 * cos(0.03 * time + 3.0 * z);

    // The radius also changes at each z-slice.
    float r = 0.1 + 0.4 * z * z + 0.1 * z * sin(8.0 * z);

    // r is the center radius, r2 is for damping
    float r2 = 0.2 + 0.1 * z;

    float rdist = sqrt((y - yc) * (y - yc) + (x - xc) * (x - xc));
    float scale = abs(r - rdist);
    /*
     *  I do not like this next line. It produces a discontinuity
     *  in the magnitude. Fix it later.
     */
    if (scale > r2) {
        scale = 0.8 - scale;
    } else {
        scale = 1.0;
    }
    float z0 = max(0.1 * (0.1 - rdist * z), 0.0);
    rdist = sqrt(rdist * rdist + z0 * z0);
    scale = (r + r2 - rdist) * scale / (rdist + 0.00000000001);
    scale = scale / (1 + z);

    vec3 value = vec3(scale *  (y - yc) + 0.1 * (x - xc), 
                      scale * -(x - xc) + 0.1 * (y - yc),
                      scale * z0);

    FragData0 = vec4(value, 0.0);
}

)";

}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TornadoGenerator::processorInfo_{
    "org.inviwo.TornadoGenerator",  // Class identifier
    "Tornado Generator",            // Display name
    "Data Creation",                // Category
    CodeState::Stable,              // Code state
    Tags::GL,                       // Tags
    R"(Creates a tornado vector field)"_unindentHelp,
};

const ProcessorInfo& TornadoGenerator::getProcessorInfo() const { return processorInfo_; }

TornadoGenerator::TornadoGenerator()
    : VolumeGLProcessor{std::make_shared<StringShaderResource>("tornado.frag", tornado),
                        {.dimensions = size3_t{16},
                         .format = DataVec3Float32::get(),
                         .dataRange = dvec2(-1.0, 1.0),
                         .valueRange = dvec2(-1.0, 1.0)},
                        VolumeGLProcessor::UseInport::No}
    , size_("size", "Volume size", size3_t(16), {size3_t(3), ConstraintBehavior::Immutable},
            {size3_t(1024), ConstraintBehavior::Ignore})
    , time_{"time",
            "Time",
            0.0,
            {0.0f, ConstraintBehavior::Ignore},
            {100.f, ConstraintBehavior::Ignore}} {

    addProperties(size_, time_);
}

void TornadoGenerator::initializeShader(Shader&) {}

void TornadoGenerator::preProcess(TextureUnitContainer&, Shader& shader, VolumeConfig& config) {
    config.dimensions = size_.get();
    utilgl::setUniforms(shader, time_);
}
void TornadoGenerator::postProcess(Volume&) {}

}  // namespace inviwo
