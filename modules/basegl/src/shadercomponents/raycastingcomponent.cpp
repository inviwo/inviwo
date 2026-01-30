/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/raycastingcomponent.h>

#include <inviwo/core/datastructures/histogram.h>             // for HistogramSelection
#include <inviwo/core/properties/isotfproperty.h>             // for IsoTFProperty
#include <inviwo/core/properties/isovalueproperty.h>          // for IsoValueProperty
#include <inviwo/core/properties/optionproperty.h>            // for OptionPropertyIntOption
#include <inviwo/core/properties/ordinalproperty.h>           // for FloatProperty
#include <inviwo/core/properties/raycastingproperty.h>        // for RaycastingProperty, Raycast...
#include <inviwo/core/properties/transferfunctionproperty.h>  // for TransferFunctionProperty
#include <inviwo/core/util/stringconversion.h>                // for trim
#include <inviwo/core/util/zip.h>                             // for enumerate, zipIterator, zipper
#include <modules/basegl/shadercomponents/shadercomponent.h>  // for ShaderComponent::Segment
#include <modules/basegl/shadercomponents/shadercomponentutil.h>
#include <modules/opengl/shader/shader.h>        // for Shader
#include <modules/opengl/shader/shaderobject.h>  // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>   // for setShaderDefines

#include <bitset>       // for bitset<>::reference
#include <iterator>     // for move_iterator, make_move_it...
#include <string_view>  // for string_view

#include <fmt/core.h>    // for format
#include <fmt/format.h>  // for operator""_a, udl_arg, lite...

namespace inviwo {
class Property;
class TextureUnitContainer;

RaycastingComponent::RaycastingComponent(std::string_view volume, IsoTFProperty& isotf)
    : ShaderComponent()
    , volume_{volume}
    , isotf_{isotf}
    , channel_("channel", "Render Channel", util::enumeratedOptions("Channel", 4), 0)
    , raycasting_("raycaster", "Raycasting") {

    raycasting_.insertProperty(0, channel_);
    raycasting_.compositing_.setVisible(false);

    util::handleTFSelections(isotf_, channel_);
}

std::string_view RaycastingComponent::getName() const { return raycasting_.getIdentifier(); }

void RaycastingComponent::initializeResources(Shader& shader) {
    auto fso = shader.getFragmentShaderObject();

    {  // classification
        const std::string key = "APPLY_CLASSIFICATION(transferFunction, voxel, channel)";
        const std::string value = [&]() {
            switch (raycasting_.classification_.get()) {
                case RaycastingProperty::Classification::None:
                    return "vec4(voxel[channel])";
                case RaycastingProperty::Classification::TF:
                    return "texture(transferFunction, vec2(voxel[channel], 0.5));";
                case RaycastingProperty::Classification::Voxel:
                default:
                    return "voxel";
            }
        }();
        fso->addShaderDefine(key, value);
    }

    // gradients
    utilgl::setShaderDefines(
        shader, raycasting_.gradientComputation_,
        raycasting_.classification_.get() == RaycastingProperty::Classification::Voxel);
}

void RaycastingComponent::process(Shader& shader, TextureUnitContainer&) {
    shader.setUniform("samplingRate", raycasting_.samplingRate_.get());
    shader.setUniform("channel", static_cast<int>(channel_.getSelectedIndex()));

    const auto renderingType = raycasting_.renderingType_;
    if (renderingType == DvrIsosurface || renderingType == Dvr) {
        shader.setUniform("dvrReference", raycasting_.dvrReference_.get());
    }
}

namespace {

constexpr std::string_view iso = util::trim(R"(
result = drawISO(result, {isoparams}, {volume}Voxel[{channel}], {volume}VoxelPrev[{channel}],
                 {gradient}, {gradientPrev}, {volume}Parameters.textureToWorld,
                 lighting, samplePosition, rayDirection,
                 cameraDir, rayPosition, rayStep, rayDepth);)");

constexpr std::string_view init = util::trim(R"(
ShadingParameters shadingParams = defaultShadingParameters();
)");

constexpr std::string_view dvrReference = R"(uniform float dvrReference = 150.0;)";

constexpr std::string_view colorInit = util::trim(R"(
vec4 color{color} = vec4(0);
)");

constexpr std::string_view manualStepInit = util::trim(R"(
float {volume}worldStep = dvrReference * calcWorldStep(rayStep, rayDirection, 
                                                       mat3({volume}Parameters.dataToWorld));
)");

constexpr std::string_view automaticStepInit = util::trim(R"(
float {volume}worldStep = dvrReference * calcWorldStepScaled(rayStep, rayDirection, 
                                                             mat3({volume}Parameters.dataToWorld));
)");

constexpr std::string_view classify = util::trim(R"(
color{color} = texture({tf}, vec2({volume}Voxel[{channel}], 0.5));
)");

constexpr std::string_view shadeAndComposite = util::trim(R"(
if (color{color}.a > 0) {{
    shadingParams.colors = defaultMaterialColors(color{color}.rgb);
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
    color{color}.rgb = APPLY_LIGHTING_FUNC(lighting, shadingParams, cameraDir);

    if (rayDepth == -1.0 && color{color}.a > 0.0) rayDepth = rayPosition;
    result = DVRCompositing(result, color{color}, {volume}worldStep);
}}
)");

template <typename... Args>
auto makeFormatter(Args&&... args) {  // NOLINT(cppcoreguidelines-missing-std-forward)
    using FormatArgs = fmt::format_string<Args...>;
    return [fArgs = fmt::make_format_args(args...)](FormatArgs snippet) {
        return fmt::vformat(snippet, fArgs);
    };
}

}  // namespace

auto RaycastingComponent::getSegments() -> std::vector<Segment> {
    using fmt::literals::operator""_a;
    const auto renderingType = raycasting_.renderingType_;
    const auto& isoparam = isotf_.isovalues_.getIdentifier();
    const auto& tf = isotf_.tf_.getIdentifier();
    const auto gradient = fmt::format("{}Gradient", volume_);
    const auto gradientPrev = fmt::format("{}GradientPrev", volume_);

    auto format = makeFormatter("volume"_a = volume_, "gradient"_a = gradient,
                                "gradientPrev"_a = gradientPrev, "isoparams"_a = isoparam,
                                "channel"_a = "channel", "tf"_a = tf, "color"_a = "");

    const auto stepInit =
        raycasting_.dvrReferenceMode_.get() == RaycastingProperty::DVRReferenceMode::Automatic
            ? format(automaticStepInit)
            : format(manualStepInit);

    std::vector<Segment> segments{
        {.snippet = R"(#include "raycasting/iso.glsl")",
         .placeholder = placeholder::include,
         .priority = 1100},
        {.snippet = R"(#include "utils/compositing.glsl")",
         .placeholder = placeholder::include,
         .priority = 1100},
        {.snippet = R"(uniform int channel = 0;)",
         .placeholder = placeholder::uniform,
         .priority = 1100},
        {.snippet = std::string{init}, .placeholder = placeholder::first, .priority = 600},
    };

    if (renderingType == DvrIsosurface || renderingType == Isosurface) {
        auto isoSegments = std::to_array<Segment>(
            {{.snippet = format(iso), .placeholder = placeholder::first, .priority = 1000},
             {.snippet = format(iso), .placeholder = placeholder::loop, .priority = 1000}});

        segments.insert(segments.end(), std::make_move_iterator(isoSegments.begin()),
                        std::make_move_iterator(isoSegments.end()));
    }
    if (renderingType == DvrIsosurface || renderingType == Dvr) {
        auto dvrSegments = std::to_array<Segment>(
            {{.snippet = format(dvrReference),
              .placeholder = placeholder::uniform,
              .priority = 1110},
             {.snippet = stepInit, .placeholder = placeholder::first, .priority = 600},
             {.snippet = format(colorInit), .placeholder = placeholder::first, .priority = 600},
             {.snippet = format(classify), .placeholder = placeholder::first, .priority = 700},
             {.snippet = format(classify), .placeholder = placeholder::loop, .priority = 700},
             {.snippet = format(shadeAndComposite),
              .placeholder = placeholder::first,
              .priority = 1100},
             {.snippet = format(shadeAndComposite),
              .placeholder = placeholder::loop,
              .priority = 1100}});
        segments.insert(segments.end(), std::make_move_iterator(dvrSegments.begin()),
                        std::make_move_iterator(dvrSegments.end()));
    }

    return segments;
}

std::vector<Property*> RaycastingComponent::getProperties() { return {&raycasting_}; }

MultiRaycastingComponent::MultiRaycastingComponent(
    std::string_view volume, std::array<std::reference_wrapper<IsoTFProperty>, 4> isotfs)
    : ShaderComponent()
    , usedChannels_{4}
    , volume_{volume}
    , isotfs_{isotfs}
    , raycasting_("raycaster", "Raycasting") {

    raycasting_.compositing_.setVisible(false);
}

std::string_view MultiRaycastingComponent::getName() const { return raycasting_.getIdentifier(); }

bool MultiRaycastingComponent::setUsedChannels(size_t channels) {
    if (channels != usedChannels_) {
        usedChannels_ = channels;
        return true;
    } else {
        return false;
    }
}

void MultiRaycastingComponent::process(Shader& shader, TextureUnitContainer&) {
    shader.setUniform("samplingRate", raycasting_.samplingRate_.get());

    const auto renderingType = raycasting_.renderingType_;
    if (renderingType == DvrIsosurface || renderingType == Dvr) {
        shader.setUniform("dvrReference", raycasting_.dvrReference_.get());
    }
}
void MultiRaycastingComponent::initializeResources(Shader& shader) {
    auto fso = shader.getFragmentShaderObject();

    {  // classification
        const std::string key = "APPLY_CLASSIFICATION(transferFunction, voxel, channel)";
        const std::string value = [&]() {
            switch (raycasting_.classification_.get()) {
                case RaycastingProperty::Classification::None:
                    return "vec4(voxel[channel])";
                case RaycastingProperty::Classification::TF:
                    return "texture(transferFunction, vec2(voxel[channel], 0.5));";
                case RaycastingProperty::Classification::Voxel:
                default:
                    return "voxel";
            }
        }();
        fso->addShaderDefine(key, value);
    }

    // gradients
    utilgl::setShaderDefines(
        shader, raycasting_.gradientComputation_,
        raycasting_.classification_.get() == RaycastingProperty::Classification::Voxel);
}

auto MultiRaycastingComponent::getSegments() -> std::vector<Segment> {
    using fmt::literals::operator""_a;
    const auto renderingType = raycasting_.renderingType_;

    std::vector<Segment> segments{
        {.snippet = R"(#include "raycasting/iso.glsl")",
         .placeholder = placeholder::include,
         .priority = 1100},
        {.snippet = R"(#include "utils/compositing.glsl")",
         .placeholder = placeholder::include,
         .priority = 1100},
        {.snippet = std::string{init}, .placeholder = placeholder::first, .priority = 600}};

    if (renderingType == DvrIsosurface || renderingType == Dvr) {
        segments.emplace_back(std::string{dvrReference}, placeholder::uniform, 1110);

        const auto stepInit =
            raycasting_.dvrReferenceMode_.get() == RaycastingProperty::DVRReferenceMode::Automatic
                ? fmt::format(automaticStepInit, "volume"_a = volume_)
                : fmt::format(manualStepInit, "volume"_a = volume_);

        segments.emplace_back(stepInit, placeholder::first, 600);
    }

    for (auto&& [i, isotf] : util::enumerate(isotfs_)) {
        if (i >= usedChannels_) continue;
        const auto isoparam = isotf.get().isovalues_.getIdentifier();
        const auto tf = isotf.get().tf_.getIdentifier();
        const auto gradient = fmt::format("{}AllGradients[{}]", volume_, i);
        const auto gradientPrev = fmt::format("{}AllGradientsPrev[{}]", volume_, i);

        auto format = makeFormatter("volume"_a = volume_, "gradient"_a = gradient,
                                    "gradientPrev"_a = gradientPrev, "isoparams"_a = isoparam,
                                    "channel"_a = i, "tf"_a = tf, "color"_a = i);

        if (renderingType == DvrIsosurface || renderingType == Isosurface) {
            auto isoSegments = std::to_array<Segment>(
                {{.snippet = format(iso), .placeholder = placeholder::first, .priority = 700 + i},
                 {.snippet = format(iso), .placeholder = placeholder::loop, .priority = 700 + i}});

            segments.insert(segments.end(), std::make_move_iterator(isoSegments.begin()),
                            std::make_move_iterator(isoSegments.end()));
        }
        if (renderingType == DvrIsosurface || renderingType == Dvr) {
            auto dvrSegments = std::to_array<Segment>({{.snippet = format(colorInit),
                                                        .placeholder = placeholder::first,
                                                        .priority = 600 + i},
                                                       {.snippet = format(classify),
                                                        .placeholder = placeholder::first,
                                                        .priority = 700 + 1},
                                                       {.snippet = format(classify),
                                                        .placeholder = placeholder::loop,
                                                        .priority = 700 + 1},
                                                       {.snippet = format(shadeAndComposite),
                                                        .placeholder = placeholder::first,
                                                        .priority = 1100 + i},
                                                       {.snippet = format(shadeAndComposite),
                                                        .placeholder = placeholder::loop,
                                                        .priority = 1100 + i}});

            segments.insert(segments.end(), std::make_move_iterator(dvrSegments.begin()),
                            std::make_move_iterator(dvrSegments.end()));
        }
    }

    return segments;
}

std::vector<Property*> MultiRaycastingComponent::getProperties() { return {&raycasting_}; }

}  // namespace inviwo
