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

#include <modules/basegl/shadercomponents/lightvolumecomponent.h>

#include <inviwo/core/datastructures/light/lightingconfig.h>
#include <inviwo/core/datastructures/light/pointlight.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>

#include <fmt/format.h>

namespace inviwo {

LightVolumeComponent::LightVolumeComponent(Processor& processor, std::string_view volumeName,
                                           std::string_view gradientName)
    : ShaderComponent{}
    , lightVolume_{"lightVolume"}
    , lightSource_{"lightSource"}
    , scaling_{"lightIntensityScaling", "Light Intensity Scaling", util::ordinalScale(1.0f, 2.0f)}
    , shadingMode_("shadingMode", "Shading",
                   OptionPropertyState<ShadingMode>{
                       .options =
                           {
                               {"none", "No Shading", ShadingMode::None},
                               {"ambient", "Ambient", ShadingMode::Ambient},
                               {"diffuse", "Diffuse", ShadingMode::Diffuse},
                               {"specular", "Specular", ShadingMode::Specular},
                               {"blinnphong", "Blinn Phong", ShadingMode::BlinnPhong},
                               {"phong", "Phong", ShadingMode::Phong},
                           }}
                       .setSelectedValue(LightingConfig::defaultShadingMode)
                       .set(InvalidationLevel::InvalidResources))
    , material_{"material", "Material Properties"}
    , ambientColor_{"lightColorAmbient", "Ambient color",
                    util::ordinalColor(LightingConfig::defaultAmbient)}
    , diffuseColor_{"lightColorDiffuse", "Diffuse color",
                    util::ordinalColor(LightingConfig::defaultDiffuse)}
    , specularColor_{"lightColorSpecular", "Specular color",
                     util::ordinalColor(LightingConfig::defaultSpecular)}
    , specularExponent_{"materialShininess", "Shininess",
                        util::ordinalScale(LightingConfig::defaultSpecularExponent)}
    , volumeVarName_{std::move(volumeName)}
    , gradientVarName_{std::move(gradientName)}
    , usedChannels_{1} {

    material_.addProperties(ambientColor_, diffuseColor_, specularColor_, specularExponent_);

    lightVolume_.onConnect([&]() { processor.invalidate(InvalidationLevel::InvalidResources); });
    lightVolume_.onDisconnect([&]() { processor.invalidate(InvalidationLevel::InvalidResources); });
}

std::string_view LightVolumeComponent::getName() const { return lightVolume_.getIdentifier(); }

void LightVolumeComponent::process(Shader& shader, TextureUnitContainer& cont) {
    using namespace fmt::literals;

    if (auto type = lightSource_.getData()->getLightSourceType(); type != LightSourceType::Point) {
        throw Exception(SourceContext{}, "unsupported light source '{}', expected '{}'", type,
                        LightSourceType::Point);
    }

    utilgl::bindAndSetUniforms(shader, cont, lightVolume_);
    utilgl::setShaderUniforms(shader, scaling_);
    const auto* pointlight = dynamic_cast<const PointLight*>(lightSource_.getData().get());
    utilgl::setShaderUniforms(shader,
                              LightingState{
                                  .position = pointlight ? pointlight->getPosition() : vec3{1.0f},
                                  .ambient = ambientColor_,
                                  .diffuse = diffuseColor_,
                                  .specular = specularColor_,
                                  .exponent = specularExponent_,
                              },
                              "lighting");
}

void LightVolumeComponent::initializeResources(Shader& shader) {
    utilgl::addShaderDefines(shader, shadingMode_);
    shader.getFragmentShaderObject()->addShaderDefine("APPLY_LIGHTING_FUNC",
                                                      "applyLightVolumeLighting");
}

std::vector<std::tuple<Inport*, std::string>> LightVolumeComponent::getInports() {
    return {{&lightVolume_, std::string{"default"}}, {&lightSource_, std::string{"default"}}};
}

std::vector<Property*> LightVolumeComponent::getProperties() {
    return {&scaling_, &shadingMode_, &material_};
}

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform VolumeParameters {lightvolume}Parameters;
uniform sampler3D {lightvolume};
uniform float {scaling} = 1.0;
uniform LightParameters lighting;
)");

constexpr std::string_view lightVolFunc = util::trim(R"(
vec3 applyLightVolumeLighting(in LightParameters lightsource, in ShadingParameters shading,
                              in vec3 viewDir) {{
    vec3 color = applyLighting(lightsource, shading, viewDir);
    color *= shading.lightIntensity;
    return color;
}}
)");

constexpr std::string_view lightVolInit = util::trim(R"(
vec3 lightVoxel = vec3(0.0);
)");

constexpr std::string_view lightVolSampling = util::trim(R"(
if (color.a > 0) {{
    lightVoxel = getVoxel({lightvolume}, {lightvolume}Parameters, samplePosition){swizzle};
    shadingParams.lightIntensity = lightVoxel * {scaling};
}}
)");

}  // namespace

auto LightVolumeComponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;

    const bool hasLightColor =
        lightVolume_.hasData() && lightVolume_.getData()->getDataFormat()->getComponents() > 1;
    std::string_view swizzle = hasLightColor ? ".rgb" : ".rrr";

    return {
        {R"(#include "utils/shading.glsl")", placeholder::include, 500},
        {fmt::format(uniforms, "lightvolume"_a = getName(), "scaling"_a = scaling_.getIdentifier()),
         placeholder::uniform, 410},
        {fmt::format(lightVolFunc, "lightvolume"_a = getName(),
                     "scaling"_a = scaling_.getIdentifier(), "swizzle"_a = swizzle),
         placeholder::uniform, 1500},
        {fmt::format(lightVolInit, "lightvolume"_a = getName(),
                     "scaling"_a = scaling_.getIdentifier(), "swizzle"_a = swizzle),
         placeholder::first, 610},
        {fmt::format(lightVolSampling, "volume"_a = volumeVarName_, "gradient"_a = gradientVarName_,
                     "lightvolume"_a = getName(), "scaling"_a = scaling_.getIdentifier(),
                     "swizzle"_a = swizzle),
         placeholder::first, 1090},
        {fmt::format(lightVolSampling, "volume"_a = volumeVarName_, "gradient"_a = gradientVarName_,
                     "lightvolume"_a = getName(), "scaling"_a = scaling_.getIdentifier(),
                     "swizzle"_a = swizzle),
         placeholder::loop, 1090},
    };
}

bool LightVolumeComponent::setUsedChannels(size_t channels) {
    if (channels != usedChannels_) {
        usedChannels_ = channels;
        return true;
    } else {
        return false;
    }
}

}  // namespace inviwo
