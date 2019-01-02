/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

MeshShaderCache::MeshShaderCache(
    std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>> items,
    std::vector<Item> buffers, std::function<void(Shader&)> configureShader)
    : items_{std::move(items)}, config_{std::move(configureShader)}, buffers_{std::move(buffers)} {}

MeshShaderCache::MeshShaderCache(std::vector<std::pair<ShaderType, std::string>> items,
                                 std::vector<Item> buffers,
                                 std::function<void(Shader&)> configureShader)
    : MeshShaderCache(utilgl::toShaderResources(items), std::move(buffers),
                      std::move(configureShader)) {}

Shader& MeshShaderCache::getShader(const Mesh& mesh) {
    std::vector<int> locations(buffers_.size(), -1);
    for (auto&& item : util::zip(buffers_, locations)) {
        const auto res = mesh.findBuffer(item.first().bufferType);
        if (res.first) {
            item.second() = res.second;
        } else if (!item.first().optional) {
            throw Exception(
                "Unsupported mesh type, a " + toString(item.first().bufferType) + " is needed",
                IVW_CONTEXT);
        }
    }

    auto it = shaders_.find(locations);
    if (it != shaders_.end()) {
        return it->second;
    } else {
        auto ins = shaders_.emplace(locations, Shader(items_, Shader::Build::No));
        auto& shader = ins.first->second;
        shader[ShaderType::Vertex]->clearInDeclarations();
        for (auto&& item : util::zip(buffers_, locations)) {
            if (item.second() >= 0) {
                const auto& buffername = item.first().name;
                shader[ShaderType::Vertex]->addInDeclaration("in_" + buffername, item.second(),
                                                             item.first().glslType);
                for (auto& obj : shader.getShaderObjects()) {
                    obj.addShaderDefine("HAS_" + toUpper(buffername));
                }
            }
        }
        config_(shader);
        return shader;
    }
}

}  // namespace inviwo
