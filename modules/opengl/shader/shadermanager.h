/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_SHADERMANAGER_H
#define IVW_SHADERMANAGER_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/inviwoopengl.h>

#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderobject.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/dispatcher.h>

#include <unordered_map>

namespace inviwo {

class OpenGLSettings;
class ShaderResource;
class OpenGLCapabilities;

template<typename T>
class TemplateOptionProperty;

class IVW_MODULE_OPENGL_API ShaderManager : public Singleton<ShaderManager> {

public:
    ShaderManager();
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;
    
    virtual ~ShaderManager() = default;

    void registerShader(Shader* shader);
    void unregisterShader(Shader* shader);

    std::string getGlobalGLSLHeader();
    std::string getGlobalGLSLVertexDefines();
    std::string getGlobalGLSLFragmentDefines();
    int getGlobalGLSLVersion();

    void bindCommonAttributes(unsigned int);

    void addShaderSearchPath(std::string);
    const std::vector<std::string>& getShaderSearchPaths();
    
    void addShaderResource(std::string key, std::string resource);
    void addShaderResource(std::string key, std::unique_ptr<ShaderResource> resource);
    std::shared_ptr<ShaderResource> getShaderResource(std::string key);
    
    const std::vector<Shader*>& getShaders() const;
    void rebuildAllShaders();

    void setOpenGLSettings(OpenGLSettings* settings);
    Shader::OnError getOnShaderError() const;
    
    using Callback = std::function<void(GLuint)>;
    template <typename T>
    std::shared_ptr<Callback> onDidAddShader(T&& callback);
    template <typename T>
    std::shared_ptr<Callback> onWillRemoveShader(T&& callback);

protected:
    OpenGLCapabilities* getOpenGLCapabilitiesObject();

private:
    bool addShaderSearchPathImpl(const std::string &);
    std::vector<Shader*> shaders_;
    OpenGLCapabilities* openGLInfoRef_;
    std::vector<std::string> shaderSearchPaths_;

    std::vector<std::shared_ptr<ShaderResource>> ownedResources_;
    std::unordered_map<std::string, std::weak_ptr<ShaderResource>> shaderResources_;
    
    TemplateOptionProperty<Shader::UniformWarning>* uniformWarnings_; // non-owning reference
    TemplateOptionProperty<Shader::OnError>* shaderObjectErrors_; // non-owning reference
    
    Dispatcher<void(GLuint)> shaderAddCallbacks_;
    Dispatcher<void(GLuint)> shaderRemoveCallbacks_;
};

template <typename T>
std::shared_ptr<ShaderManager::Callback> ShaderManager::onDidAddShader(T&& callback) {
    return shaderAddCallbacks_.add(std::forward<T>(callback));
}
template <typename T>
std::shared_ptr<ShaderManager::Callback> ShaderManager::onWillRemoveShader(T&& callback) {
    return shaderRemoveCallbacks_.add(std::forward<T>(callback));
}

} // namespace

#endif // IVW_SHADERMANAGER_H
