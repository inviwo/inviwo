/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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

#include <modules/basegl/processors/raycasting/texturedisosurfacerenderer.h>

#include <inviwo/core/algorithm/boundingbox.h>  // for boundingBox
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/ports/volumeport.h>                              // for VolumeInport
#include <inviwo/core/properties/isotfproperty.h>                      // for IsoTFProperty
#include <inviwo/core/util/stringconversion.h>                         // for trim
#include <modules/basegl/processors/raycasting/volumeraycasterbase.h>  // for VolumeRaycasterBase
#include <modules/basegl/shadercomponents/cameracomponent.h>           // for CameraComponent
#include <modules/basegl/shadercomponents/isotfcomponent.h>            // for IsoTFComponent
#include <modules/basegl/shadercomponents/raycastingcomponent.h>       // for RaycastingComponent
#include <modules/basegl/shadercomponents/volumecomponent.h>           // for VolumeComponent
#include <modules/basegl/shadercomponents/shadercomponent.h>  // for ShaderComponent::Segment
#include <modules/opengl/volume/volumeutils.h>                // for bindAndSetUniforms
#include <modules/opengl/texture/textureutils.h>
#include <modules/basegl/shadercomponents/shadercomponentutil.h>

#include <fmt/format.h>  // for compile_string_to_view, FMT...

namespace inviwo {

TexturedIsoSurfaceComponent::TexturedIsoSurfaceComponent(std::string_view volume)
    : ShaderComponent()
    , volume{volume}
    , colorPort{"colorVolume", "Volume used to texture the iso surface"_help}
    , iso{nullptr}
    , tf{nullptr}
    , isoChannel("isoChannel", "Iso Channel", util::enumeratedOptions("Channel", 4), 0)
    , colorChannel("colorChannel", "Color Channel", util::enumeratedOptions("Channel", 4), 0)
    , samplingRate("samplingRate", "Sampling rate", 2.0f, 1.0f, 20.0f) {}

std::string_view TexturedIsoSurfaceComponent::getName() const { return "Texture"; }

std::vector<std::tuple<Inport*, std::string>> TexturedIsoSurfaceComponent::getInports() {
    return {{&colorPort, std::string{"volumes"}}};
}

void TexturedIsoSurfaceComponent::initializeResources(Shader& shader) {
    auto fso = shader.getFragmentShaderObject();
    fso->addShaderDefine(
        "COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParams, samplePos, channel)",
        "gradientCentralDiff(voxel, volume, volumeParams, samplePos, channel)");
    fso->addShaderDefine("GRADIENTS_ENABLED");
}

void TexturedIsoSurfaceComponent::process(Shader& shader, TextureUnitContainer& cont) {
    shader.setUniform("samplingRate", samplingRate.get());
    shader.setUniform("channel", static_cast<int>(isoChannel.getSelectedIndex()));
    shader.setUniform("colorChannel", static_cast<int>(colorChannel.getSelectedIndex()));
    utilgl::bindAndSetUniforms(shader, cont, colorPort);
}

std::vector<Property*> TexturedIsoSurfaceComponent::getProperties() {
    return {&isoChannel, &colorChannel, &samplingRate};
}

namespace {

constexpr std::string_view isoCalc = util::trim(R"(
vec4 drawISO(in vec4 result, in float isovalue, in float alpha, in float value,
             in float previousValue, in vec3 gradient, in vec3 previousGradient,
             in mat4 textureToWorld, in LightParameters lighting, in vec3 rayPosition,
             in vec3 rayDirection, in vec3 toCameraDir, in float t, in float tIncr,
             inout float tDepth) {{

    // check if the isovalue is lying in between current and previous sample
    // found isosurface if differences between current/prev sample and isovalue have different signs
    if ((isovalue - value) * (isovalue - previousValue) <= 0) {{
        // apply linear interpolation between current and previous sample to obtain location of
        // isosurface
        float x = (value - isovalue) / (value - previousValue);
        // if x == 1, isosurface was already computed for previous sampling position
        if (x >= 1.0) return result;

        vec3 isopos = rayPosition - tIncr * x * rayDirection;
        float voxel = getNormalizedVoxel({color}, {color}Parameters, isopos)[colorChannel];
        vec4 isocolor = texture({tf}, vec2(voxel, 0.5));
        isocolor.a *= alpha;
        
        #if defined(SHADING_ENABLED) && defined(GRADIENTS_ENABLED)
        vec3 isoGradient = mix(gradient, previousGradient, x);
        if (dot(isoGradient, rayDirection) < 0.0) {{  // two-sided lighting
            isoGradient = -isoGradient;
        }}

        vec3 isoposWorld = (textureToWorld * vec4(isopos, 1.0)).xyz;
        isocolor.rgb = APPLY_LIGHTING(lighting, isocolor.rgb, isocolor.rgb, vec3(1.0), isoposWorld,
                                      -isoGradient, toCameraDir);
        #endif

        if (tDepth < 0.0) {{  // blend isosurface color and adjust first-hit depth if necessary
            tDepth = t - x * tIncr;  // store depth of first hit, i.e. voxel with non-zero alpha
        }}
        isocolor.rgb *= isocolor.a;  // use pre-multiplied alpha
        
        // blend isosurface color with result accumulated so far
        result += (1.0 - result.a) * isocolor;
    }}

    return result;
}}
)");

constexpr std::string_view isoDraw = util::trim(R"(
vec4 drawISO(in vec4 result, in IsovalueParameters isoparams, in float value,
             in float previousValue, in vec3 gradient, in vec3 previousGradient,
             in mat4 textureToWorld, in LightParameters lighting, in vec3 rayPosition,
             in vec3 rayDirection, in vec3 toCameraDir, in float t, in float tIncr,
             inout float tDepth) {

    #if MAX_ISOVALUE_COUNT == 1
    if (isoparams.size > 0 ) {
        result = drawISO(result, isoparams.values[0], isoparams.colors[0].a, value, previousValue,
                         gradient, previousGradient, textureToWorld, lighting, rayPosition,
                         rayDirection, toCameraDir, t, tIncr, tDepth);
    }
    #else
    // multiple isosurfaces, need to determine order of traversal
    if (value - previousValue > 0) {
        for (int i = 0; i < isoparams.size; ++i) {
            result = drawISO(result, isoparams.values[i], isoparams.colors[i].a, value, previousValue,
                             gradient, previousGradient, textureToWorld, lighting, rayPosition,
                             rayDirection, toCameraDir, t, tIncr, tDepth);
        }
    } else {
        for (int i = isoparams.size; i > 0; --i) {
            result = drawISO(result, isoparams.values[i - 1], isoparams.colors[i - 1].a, value,
                             previousValue, gradient, previousGradient, textureToWorld, lighting,
                             rayPosition, rayDirection, toCameraDir, t, tIncr, tDepth);
        }
    }
    #endif

    return result;
}

)");

constexpr std::string_view classify = util::trim(R"(
result = drawISO(result, {iso}, {volume}Voxel[channel], {volume}VoxelPrev[channel],
                 {volume}Gradient, {volume}GradientPrev, {volume}Parameters.textureToWorld,
                 lighting, samplePosition, rayDirection,
                 cameraDir, rayPosition, rayStep, rayDepth);
)");

constexpr std::string_view uniforms = util::trim(R"(
uniform VolumeParameters {color}Parameters;
uniform sampler3D {color};
)");

template <typename... Args>
auto makeFormatter(Args&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    using FormatArgs = fmt::format_string<Args...>;
    return [fArgs = fmt::make_format_args(args...)](FormatArgs snippet) {
        return fmt::vformat(snippet, fArgs);
    };
}

}  // namespace

auto TexturedIsoSurfaceComponent::getSegments() -> std::vector<Segment> {
    using fmt::literals::operator""_a;

    auto format = makeFormatter("color"_a = colorPort.getIdentifier(), "volume"_a = volume,
                                "tf"_a = tf->getIdentifier(), "iso"_a = iso->getIdentifier());

    std::vector<Segment> segments{
        {.snippet = R"(#include "utils/compositing.glsl")",
         .placeholder = placeholder::include,
         .priority = 1100},
        {.snippet = R"(uniform int channel = 0;)",
         .placeholder = placeholder::uniform,
         .priority = 1100},
        {.snippet = R"(uniform int colorChannel = 0;)",
         .placeholder = placeholder::uniform,
         .priority = 1101},
        {.snippet = format(uniforms), .placeholder = placeholder::uniform, .priority = 400},
        {.snippet = format(isoCalc), .placeholder = placeholder::uniform, .priority = 3000},
        {.snippet = std::string{isoDraw}, .placeholder = placeholder::uniform, .priority = 3005},
        {.snippet = format(classify), .placeholder = placeholder::first, .priority = 600},
        {.snippet = format(classify), .placeholder = placeholder::loop, .priority = 600},
    };

    return segments;
}

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TexturedIsosurfaceRenderer::processorInfo_{
    "org.inviwo.TexturedIsosurfaceRenderer",  // Class identifier
    "Textured Isosurface Renderer",           // Display name
    "Volume Rendering",                       // Category
    CodeState::Stable,                        // Code state
    Tags::GL,                                 // Tags
    R"(Render an isosurface defined by the Volume data,
       but color it based on the values of a second volume.
       The isosurface retrieves its color using the second
       volume's value mapped through the transfer function.
       This makes it possible to visualize the other volume's
       value at the isosurface location.

       ![](file:~modulePath~/docs/images/textured_isosurface.png)

       Example Network:
       [core/textured_isosurface_renderer.inv](file:~modulePath~/data/workspaces/textured_isosurface_renderer.inv)
    )"_unindentHelp};

const ProcessorInfo& TexturedIsosurfaceRenderer::getProcessorInfo() const { return processorInfo_; }

TexturedIsosurfaceRenderer::TexturedIsosurfaceRenderer(std::string_view identifier,
                                                       std::string_view displayName)
    : VolumeRaycasterBase(identifier, displayName)
    , volume_{"volume", VolumeComponent::Gradients::Single,
              "Volume from which to extract iso surfaces"_help}
    , texturedComponent_{volume_.getName()}
    , iso_{"iso", "Iso Surfaces", "iso surfaces applied to the 'volume' data"_help,
           volume_.volumePort}
    , tf_{"tf", "Color TF",
          "TF used to texture the iso surface with the data in the 'colorVolume'"_help,
          texturedComponent_.colorPort}
    , entryExit_{}
    , background_{*this}
    , camera_{"camera", util::boundingBox(volume_.volumePort)}
    , light_{&camera_.camera}
    , positionIndicator_{}
    , sampleTransform_{} {

    texturedComponent_.iso = &iso_.iso;
    texturedComponent_.tf = &tf_.tf;

    registerComponents(volume_, iso_, tf_, texturedComponent_, entryExit_, background_, camera_,
                       light_, positionIndicator_, sampleTransform_);

    util::handleTFSelections(iso_.iso, texturedComponent_.isoChannel);
    util::handleTFSelections(tf_.tf, texturedComponent_.colorChannel);
}

void TexturedIsosurfaceRenderer::process() {
    util::checkValidChannel(texturedComponent_.isoChannel.getSelectedIndex(),
                            volume_.channelsForVolume().value_or(0));

    util::checkValidChannel(
        texturedComponent_.colorChannel.getSelectedIndex(),
        texturedComponent_.colorPort.getData()->getDataFormat()->getComponents(), "colorVolume");

    VolumeRaycasterBase::process();
}

}  // namespace inviwo
