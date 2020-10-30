/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglexception.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/shader/shadertype.h>
#include <modules/opengl/shader/uniformutils.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/callback.h>
#include <inviwo/core/util/stdextensions.h>

#include <unordered_map>

namespace inviwo {

class IVW_MODULE_OPENGL_API Shader {
    struct Program {
        Program();
        Program(const Program&);
        Program(Program&& rhs) noexcept;
        Program& operator=(const Program&);
        Program& operator=(Program&& that) noexcept;
        ~Program();
        GLuint id = 0;
    };

public:
    enum class OnError { Warn, Throw };
    enum class Build { Yes, No };
    enum class UniformWarning { Ignore, Warn, Throw };

    Shader(const std::vector<std::pair<ShaderType, std::string>>& items,
           Build buildShader = Build::Yes);

    Shader(const std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>>& items,
           Build buildShader = Build::Yes);
    /*
     * Will add utilgl::imgIdentityVert() as vertex shader.
     */
    Shader(std::string fragmentFilename, bool buildShader = true);
    Shader(std::string vertexFilename, std::string fragmentFilename, bool buildShader = true);
    Shader(std::string vertexFilename, std::string geometryFilename, std::string fragmentFilename,
           bool buildShader = true);

    // We need these to avoid strange implicit conversions...
    Shader(const char* fragmentFilename, bool buildShader = true);
    Shader(const char* vertexFilename, const char* fragmentFilename, bool buildShader = true);
    Shader(const char* vertexFilename, const char* geometryFilename, const char* fragmentFilename,
           bool buildShader = true);

    Shader(std::vector<std::unique_ptr<ShaderObject>>& shaderObjects, bool buildShader = true);

    Shader(const Shader& rhs);
    Shader(Shader&& rhs);
    Shader& operator=(const Shader& that);
    Shader& operator=(Shader&& that);

    virtual ~Shader();

    void link();
    void build();
    bool isReady() const;  // returns whether the shader has been built and linked successfully

    GLuint getID() const { return program_.id; }

    using iterator = typename std::vector<ShaderObject>::iterator;
    using const_iterator = typename std::vector<ShaderObject>::const_iterator;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    util::iter_range<iterator> getShaderObjects();
    util::iter_range<const_iterator> getShaderObjects() const;

    const ShaderObject* getShaderObject(ShaderType type) const;
    ShaderObject* getShaderObject(ShaderType type);
    const ShaderObject* operator[](ShaderType type) const;
    ShaderObject* operator[](ShaderType type);
    const ShaderObject* getVertexShaderObject() const;
    const ShaderObject* getGeometryShaderObject() const;
    const ShaderObject* getFragmentShaderObject() const;

    ShaderObject* getVertexShaderObject();
    ShaderObject* getGeometryShaderObject();
    ShaderObject* getFragmentShaderObject();

    void activate();
    void deactivate();

    template <typename T>
    void setUniform(std::string_view name, const T& value) const;

    template <typename T>
    void setUniform(std::string_view name, std::size_t len, const T* value) const;

    void setUniformWarningLevel(UniformWarning level);

    // Callback when the shader is reloaded. A reload can for example be triggered by a file change.
    const BaseCallBack* onReload(std::function<void()> callback);
    std::shared_ptr<std::function<void()>> onReloadScoped(std::function<void()> callback);
    void removeOnReload(const BaseCallBack* callback);

private:
    void bindAttributes();

    void handleError(OpenGLException& e);
    std::string processLog(std::string log) const;

    void rebuildShader(ShaderObject* obj);
    void linkShader(bool notifyRebuild = false);
    bool checkLinkStatus() const;

    void attatch();
    void detatch();

    std::string shaderNames() const;
    GLint findUniformLocation(std::string_view name) const;

    Program program_;

    // clear shader objects before the program is deleted
    std::vector<ShaderObject> shaderObjects_;
    std::vector<bool> attached_;
    std::vector<std::shared_ptr<ShaderObject::Callback>> callbacks_;

    bool ready_ = false;

    UniformWarning warningLevel_;
    // Uniform location cache. Clear after linking.
    mutable std::map<std::string, GLint, std::less<>> uniformLookup_;

    // Callback on reload.
    CallBackList onReloadCallback_;
};

template <typename T>
void Shader::setUniform(std::string_view name, const T& value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) utilgl::UniformSetter<T>::set(location, value);
}

template <typename T>
void Shader::setUniform(std::string_view name, std::size_t len, const T* value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) utilgl::UniformSetter<T>::set(location, static_cast<GLsizei>(len), value);
}

}  // namespace inviwo
