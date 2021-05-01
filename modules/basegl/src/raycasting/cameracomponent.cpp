/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/basegl/raycasting/cameracomponent.h>
#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

CameraComponent::CameraComponent(std::string_view name,
                                 std::function<std::optional<mat4>()> boundingBox)
    : RaycasterComponent(), camera(std::string(name), "Camera", boundingBox) {}

std::string_view CameraComponent::getName() const { return camera.getIdentifier(); }

void CameraComponent::initializeResources(Shader& shader) { utilgl::addDefines(shader, camera); }

void CameraComponent::process(Shader& shader, TextureUnitContainer&) {
    utilgl::setUniforms(shader, camera);
}

std::vector<Property*> CameraComponent::getProperties() { return {&camera}; }

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform CameraParameters {0};
)");

}

auto CameraComponent::getSegments() -> std::vector<Segment> {
    return {Segment{fmt::format(uniforms, getName()), Segment::uniform, 500}};
}

}  // namespace inviwo
