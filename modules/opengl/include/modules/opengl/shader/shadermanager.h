/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>  // for IVW_MODULE_OPENGL_API

#include <inviwo/core/util/dispatcher.h>   // for Dispatcher
#include <inviwo/core/util/singleton.h>    // for Singleton
#include <modules/opengl/inviwoopengl.h>   // for GLuint
#include <modules/opengl/shader/shader.h>  // for Shader, Shader::OnError, Shader::UniformW...

#include <functional>   // for less, function
#include <map>          // for map
#include <memory>       // for shared_ptr, weak_ptr, unique_ptr
#include <string>       // for string, operator<
#include <string_view>  // for string_view
#include <utility>      // for forward
#include <vector>       // for vector
#include <filesystem>

namespace inviwo {

class OpenGLSettings;
class ShaderResource;
template <typename T>
class OptionProperty;

class IVW_MODULE_OPENGL_API ShaderManager : public Singleton<ShaderManager> {

public:
    ShaderManager();
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    virtual ~ShaderManager() = default;

    void registerShader(Shader* shader);
    void unregisterShader(Shader* shader);
    bool isRegistered(Shader* shader) const;

    void addShaderSearchPath(const std::filesystem::path& path);
    const std::vector<std::filesystem::path>& getShaderSearchPaths();

    void addShaderResource(std::string key, std::string resource);
    void addShaderResource(std::unique_ptr<ShaderResource> resource);
    void addShaderResource(std::shared_ptr<ShaderResource> resource);
    std::shared_ptr<ShaderResource> getShaderResource(std::string_view key);

    const std::vector<Shader*>& getShaders() const;
    void rebuildAllShaders();

    void setOpenGLSettings(OpenGLSettings* settings);
    Shader::OnError getOnShaderError() const;

    using Callback = std::function<void(GLuint)>;
    template <typename T>
    std::shared_ptr<Callback> onDidAddShader(T&& callback);
    template <typename T>
    std::shared_ptr<Callback> onWillRemoveShader(T&& callback);

private:
    bool addShaderSearchPathImpl(const std::filesystem::path& path);
    std::vector<Shader*> shaders_;
    std::vector<std::filesystem::path> shaderSearchPaths_;

    std::vector<std::shared_ptr<ShaderResource>> ownedResources_;
    std::map<std::string, std::weak_ptr<ShaderResource>, std::less<>> shaderResources_;

    OptionProperty<Shader::UniformWarning>* uniformWarnings_;  // non-owning reference
    OptionProperty<Shader::OnError>* shaderObjectErrors_;      // non-owning reference

    Dispatcher<void(GLuint)> shaderAddCallbacks_;
    Dispatcher<void(GLuint)> shaderRemoveCallbacks_;

    friend Singleton<ShaderManager>;
    static ShaderManager* instance_;
};

template <typename T>
std::shared_ptr<ShaderManager::Callback> ShaderManager::onDidAddShader(T&& callback) {
    return shaderAddCallbacks_.add(std::forward<T>(callback));
}
template <typename T>
std::shared_ptr<ShaderManager::Callback> ShaderManager::onWillRemoveShader(T&& callback) {
    return shaderRemoveCallbacks_.add(std::forward<T>(callback));
}

}  // namespace inviwo
