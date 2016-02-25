/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_SHADER_H
#define IVW_SHADER_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/shader/shadertype.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/callback.h>
#include <unordered_map>

namespace inviwo {

class IVW_MODULE_OPENGL_API Shader {
public:
    enum class OnError { Warn, Throw };
    enum class Build { Yes, No };

    using ShaderObjectPtr = std::unique_ptr<ShaderObject, std::function<void(ShaderObject *)>>;
    using ShaderObjectMap = std::unordered_map<ShaderType, ShaderObjectPtr>;

    enum class UniformWarning { Ignore, Warn, Throw };

    Shader(const std::vector<std::pair<ShaderType, std::string>> &items,
           Build buildShader = Build::Yes);

    Shader(const std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>> &items,
           Build buildShader = Build::Yes);

    Shader(std::string fragmentFilename, bool buildShader = true);
    Shader(std::string vertexFilename, std::string fragmentFilename, bool buildShader = true);
    Shader(std::string vertexFilename, std::string geometryFilename, std::string fragmentFilename,
           bool buildShader = true);
    
    // We need these to avoid strange implicit conversions...
    Shader(const char *fragmentFilename, bool buildShader = true);
    Shader(const char *vertexFilename, const char *fragmentFilename, bool buildShader = true);
    Shader(const char *vertexFilename, const char *geometryFilename, const char *fragmentFilename,
           bool buildShader = true);

    Shader(std::vector<std::unique_ptr<ShaderObject>> &shaderObjects, bool buildShader = true);

    
    Shader(const Shader &rhs);
    Shader &operator=(const Shader &that);

    virtual ~Shader();

    void link();
    void build();
    bool isReady() const; // returns whether the shader has been built and linked successfully

    GLuint getID() const { return id_; }
    const ShaderObjectMap& getShaderObjects() { return shaderObjects_; }

    ShaderObject* getShaderObject(ShaderType type) const;
    ShaderObject* getVertexShaderObject() const;
    ShaderObject* getGeometryShaderObject() const;
    ShaderObject* getFragmentShaderObject() const;

    void activate();
    void deactivate();

    void setUniform(const std::string &name, const GLint &value) const;
    void setUniform(const std::string &name, const GLint *value, int count) const;
    
    void setUniform(const std::string &name, const GLuint &value) const;
    void setUniform(const std::string &name, const GLuint *value, int count) const;

    void setUniform(const std::string &name, const GLfloat &value) const;
    void setUniform(const std::string &name, const GLfloat *value, int count) const;

    void setUniform(const std::string &name, const ivec2 &value) const;
    void setUniform(const std::string &name, const ivec3 &value) const;
    void setUniform(const std::string &name, const ivec4 &value) const;

    void setUniform(const std::string &name, const uvec2 &value) const;
    void setUniform(const std::string &name, const uvec3 &value) const;
    void setUniform(const std::string &name, const uvec4 &value) const;

    void setUniform(const std::string &name, const vec2 &value) const;
    void setUniform(const std::string &name, const vec3 &value) const;
    void setUniform(const std::string &name, const vec4 &value) const;

    void setUniform(const std::string &name, const mat2 &value) const;
    void setUniform(const std::string &name, const mat3 &value) const;
    void setUniform(const std::string &name, const mat4 &value) const;

    void setUniformWarningLevel(UniformWarning level);

    // Callback when the shader is reloaded. A reload can for example be triggered by a file change.
    const BaseCallBack* onReload(std::function<void()> callback);
    void removeOnReload(const BaseCallBack* callback);

private:
    void handleError(OpenGLException& e);
    std::string processLog(std::string log) const;

    void rebildShader(ShaderObject* obj);
    void linkShader(bool notifyRebuild = false);

    void createAndAddShader(ShaderType type, std::string fileName);
    void createAndAddShader(ShaderType type, std::shared_ptr<const ShaderResource> resource);
    void createAndAddShader(std::unique_ptr<ShaderObject> object);
    void createAndAddHelper(ShaderObject* object);

    void attachShaderObject(ShaderObject *shaderObject);
    void detachShaderObject(ShaderObject *shaderObject);

    void attachAllShaderObjects();
    void detachAllShaderObject();

    std::string shaderNames() const;
    GLint findUniformLocation(const std::string &name) const;

    GLuint id_;
    ShaderObjectMap shaderObjects_;

    bool ready_ = false;

    UniformWarning warningLevel_;
    // Uniform location cache. Clear after linking.
    mutable std::unordered_map<std::string, GLint> uniformLookup_;

    // Callback on reload.
    CallBackList onReloadCallback_;


    std::vector<std::shared_ptr<ShaderObject::Callback>> objectCallbacks_;
};

}  // namespace

#endif  // IVW_SHADER_H
