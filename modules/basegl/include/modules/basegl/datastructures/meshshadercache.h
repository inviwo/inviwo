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
#pragma once

#include <modules/basegl/baseglmoduledefine.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderresource.h>
#include <inviwo/core/datastructures/geometry/mesh.h>

#include <functional>
#include <map>
#include <optional>

namespace inviwo {

/**
 * \brief Keeps a set of shaders for various mesh configs
 */
class IVW_MODULE_BASEGL_API MeshShaderCache {
public:
    using GetStateFunctor = std::function<int(const Mesh&, Mesh::MeshInfo)>;
    using UpdateShaderFunctor = std::function<void(int, Shader&)>;

    enum RequireBuffer { Mandatory, Optional };

    struct IVW_MODULE_BASEGL_API Requirement {
        Requirement(BufferType bufferType, RequireBuffer required = Mandatory,
                    const std::string& glslType = "vec4", const std::string& name = "");

        Requirement(GetStateFunctor state, UpdateShaderFunctor update);

        GetStateFunctor getState;
        UpdateShaderFunctor updateShader;
    };

    MeshShaderCache(std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>> items,
                    std::vector<Requirement> requirements,
                    std::function<void(Shader&)> configureShader);

    MeshShaderCache(std::vector<std::pair<ShaderType, std::string>> items,
                    std::vector<Requirement> requirements,
                    std::function<void(Shader&)> configureShader);

    MeshShaderCache(const MeshShaderCache&) = delete;
    MeshShaderCache(MeshShaderCache&&) = delete;
    MeshShaderCache& operator=(const MeshShaderCache&) = delete;
    MeshShaderCache& operator=(MeshShaderCache&&) = delete;
    ~MeshShaderCache() = default;

    Shader& getShader(const Mesh& mesh, std::optional<Mesh::MeshInfo> meshInfo = std::nullopt);

    std::map<std::vector<int>, Shader>& getShaders() { return shaders_; }
    void addState(GetStateFunctor getState, UpdateShaderFunctor updateShader);

private:
    std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>> items_;
    std::function<void(Shader&)> config_;

    std::vector<Requirement> stateFunctors_;
    std::map<std::vector<int>, Shader> shaders_;
};

}  // namespace inviwo
