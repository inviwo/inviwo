/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2024 Inviwo Foundation
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

#include <modules/basegl/util/meshbnlgl.h>

#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

MeshBnLGL::MeshBnLGL()
    : inport{"brushingAndLinking"}
    , highlight{"bnlHighlight", "Enable Highlight",
                "Parameters for color overlay of highlighted data"_help, true,
                vec3{0.35f, 0.75f, 0.93f}}
    , select{"bnlSelect", "Enable Selection", "Parameters for color overlay of a selection"_help,
             true, vec3{1.0f, 0.84f, 0.0f}}
    , filter{"bnlFilter", "Enable Filtering", "Parameters for color overlay of filtered data"_help,
             false, vec3{0.5f, 0.5f, 0.5f}}
    , buffer{32}
    , unitNumber{0} {}

void MeshBnLGL::update() {
    if (inport.isConnected()) {
        if (inport.isChanged() || filter.isModified() || select.isModified() ||
            highlight.isModified()) {
            const auto size = inport.getManager().getMax() + 1;
            if (size > buffer.getSize()) {
                buffer.setSize(bit_ceil(size));
            }

            auto bnlData = buffer.map(GL_WRITE_ONLY);
            std::fill(bnlData.begin(), bnlData.end(), 0);
            if (select) {
                for (auto i : inport.getSelectedIndices()) {
                    bnlData[i] = 1;
                }
            }
            if (highlight) {
                for (auto i : inport.getHighlightedIndices()) {
                    bnlData[i] = 2;
                }
            }
            if (filter) {
                for (auto i : inport.getFilteredIndices()) {
                    bnlData[i] = 3;
                }
            }
            buffer.unmap();
        }
    }
}

void MeshBnLGL::bind(TextureUnitContainer& cont) {
    auto& unit = cont.emplace_back();
    utilgl::bindTexture(buffer, unit);
    unitNumber = unit.getUnitNumber();
}

void MeshBnLGL::setUniforms(Shader& shader) const {
    utilgl::setUniforms(shader, filter, select, highlight);
    shader.setUniform("bnl", unitNumber);
}

MeshShaderCache::Requirement MeshBnLGL::getRequirement() const {
    return {[this](const Mesh&, Mesh::MeshInfo) -> int { return inport.isConnected() ? 1 : 0; },
            [](int mode, Shader& shader) {
                shader[ShaderType::Vertex]->setShaderDefine("ENABLE_BNL", mode == 1);
                if (auto* geometryShader = shader[ShaderType::Geometry]) {
                    geometryShader->setShaderDefine("ENABLE_BNL", mode == 1);
                }
            }};
}

}  // namespace inviwo
