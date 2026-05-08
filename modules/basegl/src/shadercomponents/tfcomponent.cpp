/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/stringconversion.h>

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
         InvalidationLevel::InvalidResources}
    , volume_{volume} {}

std::string_view TFComponent::getName() const { return tf.getIdentifier(); }

void TFComponent::initializeResources(Shader& shader) {
    const bool absolute = tf.get().getType() == TFPrimitiveSetType::Absolute;
    const auto define = fmt::format("TF_ABSOLUTE_{}", toUpper(tf.getIdentifier()));
    shader.getFragmentShaderObject()->setShaderDefine(define, absolute);
}

void TFComponent::process(Shader& shader, TextureUnitContainer& cont) {
    utilgl::bindAndSetUniforms(shader, cont, tf);

    if (tf.get().getType() == TFPrimitiveSetType::Absolute) {
        const auto range = tf.get().getRange();
        const auto name = tf.getIdentifier();
        StrBuffer buff;
        shader.setUniform(buff.replace("{}Params.rangeMin", name),
                          static_cast<float>(range.x));
        shader.setUniform(buff.replace("{}Params.rangeMax", name),
                          static_cast<float>(range.y));
    }
}

std::vector<Property*> TFComponent::getProperties() { return {&tf}; }

auto TFComponent::getSegments() -> std::vector<Segment> {
    std::vector<Segment> segments;
    segments.push_back(
        Segment{fmt::format("uniform sampler2D {};", tf.getIdentifier()), placeholder::uniform,
                1050 + 1});
    segments.push_back(
        Segment{fmt::format("uniform TFParameters {}Params;", tf.getIdentifier()),
                placeholder::uniform, 1050 + 2});
    return segments;
}

}  // namespace inviwo
