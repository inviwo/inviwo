/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#include <string_view>

namespace inviwo {
RaycastingComponent::RaycastingComponent()
    : RaycasterComponent(), raycasting_("raycaster", "Raycasting") {
    raycasting_.compositing_.setVisible(false);
}

std::string RaycastingComponent::getName() const { return raycasting_.getIdentifier(); }

void RaycastingComponent::setUniforms(Shader &shader, TextureUnitContainer &) const {
    shader.setUniform("samplingRate", raycasting_.samplingRate_.get());
}
void RaycastingComponent::setDefines(Shader &shader) const {
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
auto RaycastingComponent::getSegments() const -> std::vector<Segment> {
    static constexpr std::string_view iso{
        "result = drawISO(result, isovalues, voxel[channel], previousVoxel[channel], gradient,\n"
        "                 previousGradient, volumeParameters.textureToWorld, lighting, samplePos,\n"
        "                 rayDirection, toCameraDir, t, tIncr, tDepth);"};
    static constexpr std::string_view dvr{
        "result = drawDVR(result, transferFunction, samplePos, voxel, channel, gradient,\n"
        "                 volumeParameters.textureToWorld, lighting, toCameraDir, t, tIncr, "
        "tDepth);"};

    std::vector<Segment> segments;

    if (doISO()) {
        segments.push_back(
            Segment{std::string(R"(#include "raycasting/iso.glsl")"), Segment::include, 1100});
        segments.push_back(
            Segment{std::string("uniform IsovalueParameters isovalues;"), Segment::uniform, 1050});
        segments.push_back(Segment{std::string(iso), Segment::loop, 1050});
    }
    if (doDVR()) {
        segments.push_back(
            Segment{std::string(R"(#include "raycasting/dvr.glsl")"), Segment::include, 1100});
        segments.push_back(Segment{std::string(dvr), Segment::loop, 1100});
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
std::vector<Property *> RaycastingComponent::getProperties() { return {&raycasting_}; }

}  // namespace inviwo
