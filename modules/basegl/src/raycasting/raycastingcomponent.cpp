/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include <modules/basegl/raycasting/raycastingcomponent.h>
#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/util/stringconversion.h>

#include <string_view>
#include <fmt/format.h>

namespace inviwo {
RaycastingComponent::RaycastingComponent(std::string_view volume)
    : RaycasterComponent(), volume_{volume}, raycasting_("raycaster", "Raycasting") {
    raycasting_.compositing_.setVisible(false);
}

std::string_view RaycastingComponent::getName() const { return raycasting_.getIdentifier(); }

void RaycastingComponent::process(Shader& shader, TextureUnitContainer&) {
    shader.setUniform("samplingRate", raycasting_.samplingRate_.get());
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

namespace {

constexpr std::string_view iso = util::trim(R"(
result = drawISO(result, isovalues, {0}Voxel[channel], {0}VoxelPrev[channel], 
                {0}Gradient, {0}GradientPrev, {0}Parameters.textureToWorld, 
                lighting, samplePosition, rayDirection, 
                cameraDir, rayPosition, rayStep, rayDepth);)");

constexpr std::string_view dvr2 = util::trim(R"(
result = drawDVR(result, transferFunction, samplePosition, {0}Voxel, channel, {0}Gradient,
                 {0}Parameters.textureToWorld, lighting, 
                 cameraDir, rayPosition, rayStep, rayDepth);)");

constexpr std::string_view dvr = util::trim(R"(
if ({0}Color.a > 0) {{
    #if defined(SHADING_ENABLED)
    vec3 worldSpacePosition = ({0}Parameters.textureToWorld * vec4(samplePosition, 1.0)).xyz;
    {0}Color.rgb = APPLY_LIGHTING(lighting, {0}Color.rgb, {0}Color.rgb, vec3(1.0), 
                                  worldSpacePosition, -{0}Gradient, cameraDir);
    #endif
    result = compositeDVR(result, {0}Color, rayPosition, rayDepth, rayStep);
}}
)");

}  // namespace

auto RaycastingComponent::getSegments() -> std::vector<Segment> {

    std::vector<Segment> segments;

    if (doISO()) {
        segments.push_back(
            Segment{std::string(R"(#include "raycasting/iso.glsl")"), Segment::include, 1100});
        segments.push_back(
            Segment{std::string("uniform IsovalueParameters isovalues;"), Segment::uniform, 1050});
        segments.push_back(Segment{fmt::format(FMT_STRING(iso), volume_), Segment::first, 1050});
        segments.push_back(Segment{fmt::format(FMT_STRING(iso), volume_), Segment::loop, 1050});
    }
    if (doDVR()) {
        segments.push_back(
            Segment{std::string(R"(#include "raycasting/dvr.glsl")"), Segment::include, 1100});
        segments.push_back(Segment{fmt::format(FMT_STRING(dvr), volume_), Segment::first, 1100});
        segments.push_back(Segment{fmt::format(FMT_STRING(dvr), volume_), Segment::loop, 1100});
    }

    return segments;
}
bool RaycastingComponent::doDVR() const {
    return raycasting_.renderingType_ == RaycastingProperty::RenderingType::DvrIsosurface ||
           raycasting_.renderingType_ == RaycastingProperty::RenderingType::Dvr;
}
bool RaycastingComponent::doISO() const {
    return raycasting_.renderingType_ == RaycastingProperty::RenderingType::DvrIsosurface ||
           raycasting_.renderingType_ == RaycastingProperty::RenderingType::Isosurface;
}
std::vector<Property*> RaycastingComponent::getProperties() { return {&raycasting_}; }

}  // namespace inviwo
