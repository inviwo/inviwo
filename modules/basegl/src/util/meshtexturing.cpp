/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <modules/basegl/util/meshtexturing.h>

#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

MeshTexturing::MeshTexturing(std::string_view identifier, Document help)
    : inport{identifier, std::move(help), OutportDeterminesSize::Yes}, unitNumber{0} {}

void MeshTexturing::bind(TextureUnitContainer& cont) {
    if (inport.hasData()) {
        auto& unit = cont.emplace_back();
        utilgl::bindColorTexture(*inport.getData(), unit);
        unitNumber = unit.getUnitNumber();
    } else {
        unitNumber = 0;
    }
}

void MeshTexturing::setUniforms(Shader& shader) const {
    shader.setUniform(inport.getIdentifier(), unitNumber);
}
MeshShaderCache::Requirement MeshTexturing::getRequirement() const {
    return {[this](const Mesh&, Mesh::MeshInfo) -> int { return inport.hasData() ? 1 : 0; },
            [](int mode, Shader& shader) {
                shader[ShaderType::Fragment]->setShaderDefine("ENABLE_TEXTURING", mode == 1);
            }};
}

}  // namespace inviwo
