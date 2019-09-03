/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/basegl/datastructures/meshshadercache.h>
#include <modules/opengl/shader/shaderutils.h>

#include <inviwo/core/util/zip.h>
#include <inviwo/core/datastructures/geometry/mesh.h>

namespace inviwo {

MeshShaderCache::Requirement::Requirement(BufferType bufferType, RequireBuffer required,
                                          const std::string& glslType, const std::string& name)
    : getState{[bufferType, required](const Mesh& mesh, Mesh::MeshInfo) -> int {
        const auto res = mesh.findBuffer(bufferType);
        if (res.first) {
            return res.second;
        } else if (required == Mandatory) {
            throw Exception("Unsupported mesh type, a " + toString(bufferType) + " is needed",
                            IVW_CONTEXT_CUSTOM("MeshShaderCache"));
        } else {
            return -1;
        }
    }}
    , updateShader{[glslType, name = name.empty() ? toString(bufferType) : name](
                       int location, Shader& shader) -> void {
        if (location >= 0) {
            const auto& buffername = name;
            shader[ShaderType::Vertex]->addInDeclaration("in_" + buffername, location, glslType);
            for (auto& obj : shader.getShaderObjects()) {
                obj.addShaderDefine("HAS_" + toUpper(buffername));
            }
        }
    }} {}

MeshShaderCache::Requirement::Requirement(GetStateFunctor state, UpdateShaderFunctor update)
    : getState{std::move(state)}, updateShader{std::move(update)} {}

MeshShaderCache::MeshShaderCache(
    std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>> items,
    std::vector<Requirement> requirements, std::function<void(Shader&)> configureShader)
    : items_{std::move(items)}
    , config_{std::move(configureShader)}
    , stateFunctors_{std::move(requirements)} {}

MeshShaderCache::MeshShaderCache(std::vector<std::pair<ShaderType, std::string>> items,
                                 std::vector<Requirement> requirements,
                                 std::function<void(Shader&)> configureShader)
    : MeshShaderCache(utilgl::toShaderResources(items), std::move(requirements),
                      std::move(configureShader)) {}

Shader& MeshShaderCache::getShader(const Mesh& mesh, std::optional<Mesh::MeshInfo> meshInfo) {
    std::vector<int> state;
    for (auto& functor : stateFunctors_) {
        state.push_back(functor.getState(mesh, meshInfo ? *meshInfo : mesh.getDefaultMeshInfo()));
    }

    auto it = shaders_.find(state);
    if (it != shaders_.end()) {
        return it->second;
    } else {
        auto ins = shaders_.emplace(state, Shader(items_, Shader::Build::No));
        auto& shader = ins.first->second;
        shader[ShaderType::Vertex]->clearInDeclarations();

        for (auto&& item : util::zip(stateFunctors_, state)) {
            item.first().updateShader(item.second(), shader);
        }

        config_(shader);
        return shader;
    }
}

void MeshShaderCache::addState(GetStateFunctor getState, UpdateShaderFunctor updateShader) {
    shaders_.clear();
    stateFunctors_.emplace_back(std::move(getState), std::move(updateShader));
}

}  // namespace inviwo
