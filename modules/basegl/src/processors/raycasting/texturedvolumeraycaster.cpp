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

#include <modules/basegl/processors/raycasting/texturedvolumeraycaster.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/basegl/shadercomponents/shadercomponentutil.h>

#include <fmt/base.h>

namespace inviwo {

TexturedVolumeComponent::TexturedVolumeComponent(std::string_view name, VolumeInport& volume)
    : ShaderComponent{}
    , secondaryVolumePort_{name,
                           "This secondary input volume is used to modify/replace color values of "
                           "the primary volume by applying a second transfer function."_help}
    , dependentLookup_{"dependentLookup", "Dependent Color Lookup"}
    , primaryChannel_{"channel", "Primary Channel", util::enumeratedOptions("Channel", 4)}
    , secondaryChannel_{"dependentVolumeChannel", "Channel", util::enumeratedOptions("Channel", 4)}
    , primaryTF_{"transferFunction", "Primary Transfer Function", &volume}
    , secondaryTF_{"secondaryTF", "Secondary Transfer Function", &secondaryVolumePort_}
    , operation_{"dependentOperation",
                 "Modify Primary Color",
                 {{"none", "Primary Color (no lookup)", Operation::None},
                  {"replaceColor", "Replace Color", Operation::ReplacePrimaryColor},
                  {"multipyColor", "Multiply Color", Operation::MultiplyPrimaryColor},
                  {"multiplyAll", "Multiply Color & Alpha", Operation::MultiplyPrimaryAll}},
                 1,
                 InvalidationLevel::InvalidResources}
    , raycasting_("raycaster", "Raycasting")
    , volume_{volume.getIdentifier()} {

    raycasting_.compositing_.setVisible(false);
    raycasting_.renderingType_.setVisible(false);
    raycasting_.classification_.setVisible(false);

    secondaryTF_.setHelp("Transfer function applied when sampling the secondary volume."_help);
    dependentLookup_.addProperties(secondaryChannel_, secondaryTF_, operation_);

    util::handleTFSelections(primaryTF_, primaryChannel_);
    util::handleTFSelections(secondaryTF_, secondaryChannel_);
}

std::string_view TexturedVolumeComponent::getName() const {
    return secondaryVolumePort_.getIdentifier();
}

void TexturedVolumeComponent::initializeResources(Shader& shader) {
    auto* fso = shader.getFragmentShaderObject();
    fso->addShaderDefine("DEPENDENT_LOOKUP_OP",
                         fmt::format("{}", static_cast<int>(operation_.getSelectedValue())));

    // gradients
    utilgl::setShaderDefines(
        shader, raycasting_.gradientComputation_,
        raycasting_.classification_.get() == RaycastingProperty::Classification::Voxel);
}

void TexturedVolumeComponent::process(Shader& shader, TextureUnitContainer& cont) {

    util::checkValidChannel(secondaryChannel_.getSelectedIndex(),
                            secondaryVolumePort_.getData()->getDataFormat()->getComponents(),
                            secondaryVolumePort_.getIdentifier());

    utilgl::bindAndSetUniforms(shader, cont, secondaryVolumePort_);

    TextureUnit tfUnit;
    utilgl::bindTexture(primaryTF_, tfUnit);
    shader.setUniform(primaryTF_.getIdentifier(), tfUnit);
    cont.push_back(std::move(tfUnit));

    TextureUnit secondaryTFunit;
    utilgl::bindTexture(secondaryTF_, secondaryTFunit);
    shader.setUniform(fmt::format("{}TF", getName()), secondaryTFunit);
    cont.push_back(std::move(secondaryTFunit));

    utilgl::setUniforms(shader, primaryChannel_, raycasting_.samplingRate_,
                        raycasting_.dvrReference_);

    shader.setUniform(fmt::format("{}Channel", getName()), secondaryChannel_);
}

std::vector<std::tuple<Inport*, std::string>> TexturedVolumeComponent::getInports() {
    return {{&secondaryVolumePort_, std::string{"dependentVolumes"}}};
}

std::vector<Property*> TexturedVolumeComponent::getProperties() {
    return {&primaryChannel_, &primaryTF_, &dependentLookup_, &raycasting_};
}

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform VolumeParameters {lookup}Parameters;
uniform sampler3D {lookup};
uniform sampler2D {tf};
uniform sampler2D {lookup}TF;
uniform int channel = 0;
uniform int {lookup}Channel = 0;
)");

constexpr std::string_view lookupFunction = util::trim(R"(
vec4 applyDependentLookup(in vec4 color, in vec3 samplePosition) {{
#if DEPENDENT_LOOKUP_OP == 0  // none
    return color;
#endif

    float {lookup}Sampled = getNormalizedVoxel({lookup}, {lookup}Parameters, samplePosition)[{lookup}Channel];
    vec4 dependentLookup = texture({lookup}TF, vec2({lookup}Sampled, 0.5));
    
#if DEPENDENT_LOOKUP_OP == 3 // multiply primary color and alpha
    color *= dependentLookup;
#elif DEPENDENT_LOOKUP_OP == 2 // multiply primary color
    color.rgb *= dependentLookup.rgb;
#else // replace primary color
    color.rgb = dependentLookup.rgb;
#endif
    return color;
}}
)");

constexpr std::string_view init = util::trim(R"(
ShadingParameters shadingParams = defaultShadingParameters();
vec4 color = vec4(0);
)");

constexpr std::string_view manualStepInit = util::trim(R"(
float {volume}worldStep = dvrReference * calcWorldStepScaled(rayStep, rayDirection, 
                                                             mat3({volume}Parameters.dataToWorld));
)");

constexpr std::string_view automaticStepInit = util::trim(R"(
float {volume}worldStep = dvrReference * calcWorldStepScaled(rayStep, rayDirection, 
                                                             mat3({volume}Parameters.dataToWorld));
)");

constexpr std::string_view classification = util::trim(R"(
color = texture({tf}, vec2({volume}Voxel[channel], 0.5));
color = applyDependentLookup(color, samplePosition);
)");

constexpr std::string_view shadeAndComposite = util::trim(R"(
if (color.a > 0) {{
    shadingParams.colors = defaultMaterialColors(color.rgb);
    #if (defined(SHADING_ENABLED) && defined(GRADIENTS_ENABLED))
    shadingParams.normal = -{gradient};
    #if defined(SHADING_NORMAL) && (SHADING_NORMAL == 1)
    // backside shading only
    shadingParams.normal = -shadingParams.normal;
    #elif defined(SHADING_NORMAL) && (SHADING_NORMAL == 2)
    // two-sided shading
    if (dot(shadingParams.normal, rayDirection) > 0.0) {{
        shadingParams.normal = -shadingParams.normal;
    }}
    #endif
    shadingParams.worldPosition = ({volume}Parameters.textureToWorld * vec4(samplePosition, 1.0)).xyz;
    #endif
    color.rgb = APPLY_LIGHTING_FUNC(lighting, shadingParams, cameraDir);

    if (rayDepth == -1.0 && color.a > 0.0) rayDepth = rayPosition;
    result = DVRCompositing(result, color, {volume}worldStep);
}}
)");

template <typename... Args>
auto makeFormatter(Args&&... args) {
    using FormatArgs = fmt::format_string<Args...>;
    return [fArgs = fmt::make_format_args(args...)](FormatArgs snippet) {
        return fmt::vformat(snippet, fArgs);
    };
}

}  // namespace

auto TexturedVolumeComponent::getSegments() -> std::vector<Segment> {
    using fmt::literals::operator""_a;
    const auto& tf = primaryTF_.getIdentifier();
    const auto gradient = fmt::format("{}Gradient", volume_);

    auto format = makeFormatter("lookup"_a = getName(), "volume"_a = volume_,
                                "gradient"_a = gradient, "tf"_a = tf);

    const auto stepInit =
        raycasting_.dvrReferenceMode_.get() == RaycastingProperty::DVRReferenceMode::Automatic
            ? format(automaticStepInit)
            : format(manualStepInit);

    return {
        {.snippet = R"(#include "utils/compositing.glsl")",
         .placeholder = placeholder::include,
         .priority = 1100},
        {.snippet = R"(uniform float dvrReference = 150.0;)",
         .placeholder = placeholder::uniform,
         .priority = 1110},
        {.snippet = format(uniforms), .placeholder = placeholder::uniform, .priority = 1080},
        {.snippet = stepInit, .placeholder = placeholder::first, .priority = 600},
        {.snippet = format(lookupFunction), .placeholder = placeholder::uniform, .priority = 1500},
        {.snippet = format(init), .placeholder = placeholder::first, .priority = 600},
        {.snippet = format(classification), .placeholder = placeholder::first, .priority = 710},
        {.snippet = format(classification), .placeholder = placeholder::loop, .priority = 710},
        {.snippet = format(shadeAndComposite), .placeholder = placeholder::first, .priority = 1100},
        {.snippet = format(shadeAndComposite), .placeholder = placeholder::loop, .priority = 1100},
    };
}

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TexturedVolumeRaycaster::processorInfo_{
    "org.inviwo.TexturedVolumeRaycaster",         // Class identifier
    "Textured Volume Raycaster",                  // Display name
    "Volume Rendering",                           // Category
    CodeState::Experimental,                      // Code state
    Tags::GL | Tag{"Volume"} | Tag{"Raycaster"},  // Tags
    R"(Uses volume raycasting to render a Volumem with the color based on an additional volume
       and secondary transfer function for dependent color lookups. The primary volume is used
       to determine the opacity as well as the gradients for illumination whereas the color
       results from the secondary TF.

       Example Network:
       [core/dependent_volume_raycaster.inv](file:~modulePath~/data/workspaces/dependent_volume_raycaster.inv)
    )"_unindentHelp,
};

const ProcessorInfo& TexturedVolumeRaycaster::getProcessorInfo() const { return processorInfo_; }

TexturedVolumeRaycaster::TexturedVolumeRaycaster(std::string_view identifier,
                                                 std::string_view displayName)
    : VolumeRaycasterBase{identifier, displayName}
    , volume_{"volume", VolumeComponent::Gradients::Single,
              "input volume (Only one channel will be rendered)"_help}
    , textureComponent_{"colorLookup", volume_.volumePort}
    , entryExit_{}
    , background_{*this}
    , camera_{"camera", util::boundingBox(volume_.volumePort)}
    , light_{&camera_.camera}
    , positionIndicator_{}
    , sampleTransform_{} {

    registerComponents(volume_, textureComponent_, entryExit_, background_, camera_, light_,
                       positionIndicator_, sampleTransform_);
}

void TexturedVolumeRaycaster::process() {
    util::checkValidChannel(textureComponent_.selectedChannel(),
                            volume_.channelsForVolume().value_or(0));

    VolumeRaycasterBase::process();
}

}  // namespace inviwo
