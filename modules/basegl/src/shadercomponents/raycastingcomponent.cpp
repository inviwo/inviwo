/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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
#include <modules/opengl/shader/shader.h>                     // for Shader
#include <modules/opengl/shader/shaderobject.h>               // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>                // for setShaderDefines

#include <bitset>       // for bitset<>::reference
#include <iterator>     // for move_iterator, make_move_it...
#include <string_view>  // for string_view

#include <fmt/core.h>    // for format
#include <fmt/format.h>  // for operator""_a, udl_arg, lite...

namespace inviwo {
class Property;
class TextureUnitContainer;

namespace {

const std::vector<OptionPropertyIntOption> channelsList = {{"channel1", "Channel 1", 0},
                                                           {"channel2", "Channel 2", 1},
                                                           {"channel3", "Channel 3", 2},
                                                           {"channel4", "Channel 4", 3}};

}

RaycastingComponent::RaycastingComponent(std::string_view volume, IsoTFProperty& isotf)
    : ShaderComponent()
    , volume_{volume}
    , isotf_{isotf}
    , channel_("channel", "Render Channel", channelsList, 0)
    , raycasting_("raycaster", "Raycasting") {

    raycasting_.insertProperty(0, channel_);
    raycasting_.compositing_.setVisible(false);

    auto updateTFHistSel = [this]() {
        HistogramSelection selection{};
        selection[channel_] = true;
        isotf_.setHistogramSelection(selection);
    };
    updateTFHistSel();
    channel_.onChange(updateTFHistSel);
}

std::string_view RaycastingComponent::getName() const { return raycasting_.getIdentifier(); }

void RaycastingComponent::process(Shader& shader, TextureUnitContainer&) {
    shader.setUniform("samplingRate", raycasting_.samplingRate_.get());
    shader.setUniform("channel", static_cast<int>(channel_.getSelectedIndex()));
}
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

bool RaycastingComponent::setUsedChannels(size_t channels) {
    if (channels == channel_.size()) return false;

    channel_.replaceOptions(std::vector<OptionPropertyIntOption>{channelsList.begin(),
                                                                 channelsList.begin() + channels});
    channel_.setCurrentStateAsDefault();
    return true;
}

namespace {

constexpr std::string_view iso = util::trim(R"(
result = drawISO(result, {isoparams}, {volume}Voxel[{channel}], {volume}VoxelPrev[{channel}],
                 {gradient}, {gradientPrev}, {volume}Parameters.textureToWorld,
                 lighting, samplePosition, rayDirection,
                 cameraDir, rayPosition, rayStep, rayDepth);)");

constexpr std::string_view classify1 = util::trim(R"(
vec4 color{color} = texture({tf}, vec2({volume}Voxel[{channel}], 0.5));
)");

constexpr std::string_view classify = util::trim(R"(
color{color} = texture({tf}, vec2({volume}Voxel[{channel}], 0.5));
)");

constexpr std::string_view composite = util::trim(R"(
if (color{color}.a > 0) {{
    #if defined(SHADING_ENABLED) && defined(GRADIENTS_ENABLED)
    vec3 worldSpacePosition = ({volume}Parameters.textureToWorld * vec4(samplePosition, 1.0)).xyz;
    color{color}.rgb = APPLY_LIGHTING(lighting, color{color}.rgb, color{color}.rgb, vec3(1.0),
                                      worldSpacePosition, -{gradient}, cameraDir);
    #endif
    result = compositeDVR(result, color{color}, rayPosition, rayDepth, rayStep);
}}
)");

}  // namespace

auto RaycastingComponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;

    const auto isoparam = isotf_.isovalues_.getIdentifier();
    const auto tf = isotf_.tf_.getIdentifier();

    std::vector<Segment> segments{
        {std::string(R"(#include "raycasting/iso.glsl")"), placeholder::include, 1100},
        {std::string(R"(#include "utils/compositing.glsl")"), placeholder::include, 1100},
        {std::string(R"(uniform int channel = 0;)"), placeholder::uniform, 1100}};

    const auto gradient = fmt::format("{}Gradient", volume_);
    const auto gradientPrev = fmt::format("{}GradientPrev", volume_);

    std::vector<Segment> isoSegments{
        {fmt::format(iso, "volume"_a = volume_, "gradient"_a = gradient,
                     "gradientPrev"_a = gradientPrev, "isoparams"_a = isoparam,
                     "channel"_a = "channel"),
         placeholder::first, 1050},
        {fmt::format(iso, "volume"_a = volume_, "gradient"_a = gradient,
                     "gradientPrev"_a = gradientPrev, "isoparams"_a = isoparam,
                     "channel"_a = "channel"),
         placeholder::loop, 1050}};

    std::vector<Segment> dvrSegments{
        {fmt::format(classify1, "volume"_a = volume_, "tf"_a = tf, "channel"_a = "channel",
                     "color"_a = ""),
         placeholder::first, 600},
        {fmt::format(classify, "volume"_a = volume_, "tf"_a = tf, "channel"_a = "channel",
                     "color"_a = ""),
         placeholder::loop, 600},

        {fmt::format(composite, "volume"_a = volume_, "gradient"_a = gradient,
                     "channel"_a = "channel", "color"_a = ""),
         placeholder::first, 1100},
        {fmt::format(composite, "volume"_a = volume_, "gradient"_a = gradient,
                     "channel"_a = "channel", "color"_a = ""),
         placeholder::loop, 1100}};

    if (raycasting_.renderingType_ == RaycastingProperty::RenderingType::DvrIsosurface ||
        raycasting_.renderingType_ == RaycastingProperty::RenderingType::Isosurface) {
        segments.insert(segments.end(), std::make_move_iterator(isoSegments.begin()),
                        std::make_move_iterator(isoSegments.end()));
    }
    if (raycasting_.renderingType_ == RaycastingProperty::RenderingType::DvrIsosurface ||
        raycasting_.renderingType_ == RaycastingProperty::RenderingType::Dvr) {
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
    using namespace fmt::literals;

    std::vector<Segment> segments{
        {std::string(R"(#include "raycasting/iso.glsl")"), placeholder::include, 1100},
        {std::string(R"(#include "utils/compositing.glsl")"), placeholder::include, 1100},
    };

    for (auto&& [i, isotf] : util::enumerate(isotfs_)) {
        if (i >= usedChannels_) continue;
        const auto isoparam = isotf.get().isovalues_.getIdentifier();
        const auto tf = isotf.get().tf_.getIdentifier();
        const auto gradient = fmt::format("{}AllGradients[{}]", volume_, i);
        const auto gradientPrev = fmt::format("{}AllGradientsPrev[{}]", volume_, i);

        std::vector<Segment> isoSegments{
            {fmt::format(iso, "volume"_a = volume_, "gradient"_a = gradient,
                         "gradientPrev"_a = gradientPrev, "isoparams"_a = isoparam,
                         "channel"_a = i),
             placeholder::first, 1050 + i},
            {fmt::format(iso, "volume"_a = volume_, "gradient"_a = gradient,
                         "gradientPrev"_a = gradientPrev, "isoparams"_a = isoparam,
                         "channel"_a = i),
             placeholder::loop, 1050 + i}};

        std::vector<Segment> dvrSegments{
            {fmt::format(classify1, "volume"_a = volume_, "tf"_a = tf, "channel"_a = i,
                         "color"_a = i),
             placeholder::first, 600 + i},
            {fmt::format(classify, "volume"_a = volume_, "tf"_a = tf, "channel"_a = i,
                         "color"_a = i),
             placeholder::loop, 600 + 1},

            {fmt::format(composite, "volume"_a = volume_, "gradient"_a = gradient, "tf"_a = tf,
                         "channel"_a = i, "color"_a = i),
             placeholder::first, 1100 + i},
            {fmt::format(composite, "volume"_a = volume_, "gradient"_a = gradient, "tf"_a = tf,
                         "channel"_a = i, "color"_a = i),
             placeholder::loop, 1100 + i}};

        if (raycasting_.renderingType_ == RaycastingProperty::RenderingType::DvrIsosurface ||
            raycasting_.renderingType_ == RaycastingProperty::RenderingType::Isosurface) {
            segments.insert(segments.end(), std::make_move_iterator(isoSegments.begin()),
                            std::make_move_iterator(isoSegments.end()));
        }
        if (raycasting_.renderingType_ == RaycastingProperty::RenderingType::DvrIsosurface ||
            raycasting_.renderingType_ == RaycastingProperty::RenderingType::Dvr) {
            segments.insert(segments.end(), std::make_move_iterator(dvrSegments.begin()),
                            std::make_move_iterator(dvrSegments.end()));
        }
    }

    return segments;
}

std::vector<Property*> MultiRaycastingComponent::getProperties() { return {&raycasting_}; }

}  // namespace inviwo
