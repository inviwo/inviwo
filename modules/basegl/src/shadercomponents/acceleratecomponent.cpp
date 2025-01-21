/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/acceleratecomponent.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

namespace {

constexpr std::string_view acceleratefrag = R"(
#include "utils/sampler3d.glsl"
#include "utils/classification.glsl"

uniform sampler3D volume;
uniform VolumeParameters volumeParameters;

uniform sampler2D transferFunction;

in vec4 texCoord_;

#define steps 8

void main() {
    vec3 delta = volumeParameters.reciprocalDimensions / vec3(steps, steps, steps);

    float alpha = 0.0;
    for(int i = 0; i < steps; ++i) {
        for(int j = 0; j < steps; ++j) {
            for(int k = 0; k < steps; ++k) {
                vec4 voxel = getVoxel(volume, volumeParameters, texCoord_.xyz + vec3(i,j,k) * delta);
                float a = applyTF(transferFunction, voxel).a;
                alpha = max(alpha, a);
            }
        }
    }
    
    FragData0 = alpha > 0.0 ? vec4(1.0, 1.0, 1.0, 1.0) : vec4(0.0, 0.0, 0.0, 0.0); 
}

)";

constexpr std::string_view uniforms = util::trim(R"(
uniform VolumeParameters {0}Parameters;
uniform sampler3D {0};
)");

constexpr std::string_view first = util::trim(R"(
float bigStep = calcStep(rayLength, rayDirection, samplingRate / 8.0, volumeParameters.dimensions);
float bigPosition = rayPosition + bigStep;
float {0}Value = getNormalizedVoxel({0}, {0}Parameters, samplePosition).x;
)");

constexpr std::string_view loop = util::trim(R"(
if(rayPosition < bigPosition && {0}Value == 0.0) continue;

if (rayPosition >= bigPosition) {{ 
    bigPosition += bigStep;
    {0}Value = getNormalizedVoxel({0}, {0}Parameters, samplePosition).x;
}}

)");

}  // namespace

AccelerateComponent::AccelerateComponent(VolumeInport& port, IsoTFProperty& isotf)
    : accVol{"acc"}
    , name_{fmt::format("Accelerate-{}", port.getIdentifier())}
    , port_{&port}
    , isotf_{&isotf}
    , shader_{{{ShaderType::Vertex, utilgl::findShaderResource("volume_gpu.vert")},
               {ShaderType::Geometry, utilgl::findShaderResource("volume_gpu.geom")},
               {ShaderType::Fragment,
                std::make_shared<StringShaderResource>("accelerate.frag", acceleratefrag)}},
              Shader::Build::No}
    , fbo_{} {}

std::string_view AccelerateComponent::getName() const { return name_; }

void AccelerateComponent::initializeResources(Shader&) { shader_.build(); }

void AccelerateComponent::process(Shader& shader, TextureUnitContainer& cont) {
    utilgl::bindAndSetUniforms(shader, cont, *accelerate_, "accelerate");
}

auto AccelerateComponent::getSegments() -> std::vector<Segment> {
    return {{fmt::format(uniforms, "accelerate"), placeholder::uniform, 200},
            {fmt::format(first, "accelerate"), placeholder::first, 200},
            {fmt::format(loop, "accelerate"), placeholder::loop, 200}};
}

void AccelerateComponent::preprocess() {
    if (!port_) return;
    if (!isotf_) return;

    bool dirty = port_->isChanged() || isotf_->isModified();

    auto data = port_->getData();
    if (!accelerate_ || data->getDimensions() / size3_t{8} != accelerate_->getDimensions()) {
        accelerate_ =
            std::make_shared<Volume>(VolumeConfig{.dimensions = data->getDimensions() / size3_t{8},
                                                  .format = DataFormat<float>::get(),
                                                  .dataRange = data->dataMap.dataRange,
                                                  .valueRange = data->dataMap.valueRange,
                                                  .model = data->getModelMatrix(),
                                                  .world = data->getWorldMatrix()});
        dirty = true;
    }
    if (!dirty) return;

    utilgl::Activate aShader{&shader_};

    TextureUnit unit;
    utilgl::bindTexture(*data, unit);
    shader_.setUniform("volume", unit.getUnitNumber());
    utilgl::setShaderUniforms(shader_, *accelerate_, "volumeParameters");

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, isotf_->tf_);

    const auto dim = static_cast<ivec3>(accelerate_->getDimensions());

    utilgl::Activate aFbo{&fbo_};
    utilgl::ViewportState vp{0, 0, dim.x, dim.y};

    VolumeGL* out = accelerate_->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(out->getTexture().get(), 0);
    utilgl::multiDrawImagePlaneRect(dim.z);

    accVol.setData(accelerate_);
}

}  // namespace inviwo
