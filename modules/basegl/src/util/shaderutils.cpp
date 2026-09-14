/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/basegl/util/shaderutils.h>

#include <inviwo/core/util/stringconversion.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderobject.h>

namespace inviwo::utilgl {

void addShaderDefines(Shader& shader, const StipplingProperty& property) {
    addShaderDefines(shader, property.mode.get());
}

void addShaderDefines(Shader& shader, StipplingData::Mode mode) {
    const auto value = [mode]() -> std::string_view {
        switch (mode) {
            using enum StipplingData::Mode;
            case ScreenSpace:
                return "1";
            case WorldSpace:
                return "2";
            case None:
            default:
                return {};
        }
    }();

    auto* fragShader = shader.getFragmentShaderObject();
    fragShader->setShaderDefine("ENABLE_STIPPLING", mode != StipplingData::Mode::None);
    fragShader->addShaderDefine("STIPPLE_MODE", value);
}

void setShaderUniforms(Shader& shader, const StipplingProperty& property, std::string_view name) {
    StrBuffer buff;
    shader.setUniform(buff.replace("{}.length", name), property.length.get());
    shader.setUniform(buff.replace("{}.spacing", name), property.spacing.get());
    shader.setUniform(buff.replace("{}.offset", name), property.offset.get());
    shader.setUniform(buff.replace("{}.worldScale", name), property.worldScale.get());
}

}  // namespace inviwo::utilgl
