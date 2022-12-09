/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/tfcomponent.h>

namespace inviwo {

TFComponent::TFComponent(std::string_view identifier, std::string_view name, Document help,
                         VolumeInport& volume)
    : ShaderComponent()
    , tf{identifier,
         name,
         std::move(help),
         TransferFunction(
             {{0.0, vec4(0.0f, 0.0f, 0.0f, 0.0f)}, {1.0, vec4(1.0f, 1.0f, 1.0f, 1.0f)}}),
         &volume,
         InvalidationLevel::InvalidResources} {}

std::string_view TFComponent::getName() const { return tf.getIdentifier(); }

void TFComponent::process(Shader& shader, TextureUnitContainer& cont) {
    utilgl::bindAndSetUniforms(shader, cont, tf);
}

std::vector<Property*> TFComponent::getProperties() { return {&tf}; }

auto TFComponent::getSegments() -> std::vector<Segment> {
    return {Segment{fmt::format("uniform sampler2D {};", tf.getIdentifier()), placeholder::uniform,
                    1050 + 1}};
}

}  // namespace inviwo
