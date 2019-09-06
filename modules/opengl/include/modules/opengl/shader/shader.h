/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <modules/opengl/openglexception.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/shader/shadertype.h>
#include <modules/opengl/shader/uniformutils.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/callback.h>
#include <inviwo/core/util/transformiterator.h>
#include <inviwo/core/util/stdextensions.h>

#include <unordered_map>

namespace inviwo {

class IVW_MODULE_OPENGL_API Shader {
    struct Program {
        Program();
        Program(const Program &);
        Program(Program &&rhs) noexcept;
        Program &operator=(const Program &);
        Program &operator=(Program &&that) noexcept;
        ~Program();
        GLuint id = 0;
    };

    struct ShaderAttachment {
        ShaderAttachment();
        ShaderAttachment(Shader *shader, std::unique_ptr<ShaderObject> obj);
        ShaderAttachment(const ShaderAttachment &) = delete;
        ShaderAttachment(ShaderAttachment &&rhs) noexcept;
        ShaderAttachment &operator=(const ShaderAttachment &) = delete;
        ShaderAttachment &operator=(ShaderAttachment &&that) noexcept;
        ~ShaderAttachment();

        void attatch();
        void detatch();
        void setShader(Shader *shader);
        ShaderObject &obj() const { return *obj_; }

    private:
        Shader *shader_;
        std::unique_ptr<ShaderObject> obj_;
        std::shared_ptr<ShaderObject::Callback> callback_;
    };

    using ShaderMap = std::unordered_map<ShaderType, ShaderAttachment>;
    using transform_t = ShaderObject &(*)(typename ShaderMap::value_type &);
    using const_transform_t = const ShaderObject &(*)(const typename ShaderMap::value_type &);

public:
    enum class OnError { Warn, Throw };
    enum class Build { Yes, No };
    enum class UniformWarning { Ignore, Warn, Throw };

    using iterator = util::TransformIterator<transform_t, typename ShaderMap::iterator>;
    using const_iterator =
        util::TransformIterator<const_transform_t, typename ShaderMap::const_iterator>;

    Shader(const std::vector<std::pair<ShaderType, std::string>> &items,
           Build buildShader = Build::Yes);

    Shader(const std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>> &items,
           Build buildShader = Build::Yes);
    /*
     * Will add utilgl::imgIdentityVert() as vertex shader.
     */
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
    Shader(Shader &&rhs);
    Shader &operator=(const Shader &that);
    Shader &operator=(Shader &&that);

    virtual ~Shader();

    void link();
    void build();
    bool isReady() const;  // returns whether the shader has been built and linked successfully

    GLuint getID() const { return program_.id; }

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    util::iter_range<iterator> getShaderObjects();
    util::iter_range<const_iterator> getShaderObjects() const;

    ShaderObject *operator[](ShaderType type) const;
    ShaderObject *getShaderObject(ShaderType type) const;
    ShaderObject *getVertexShaderObject() const;
    ShaderObject *getGeometryShaderObject() const;
    ShaderObject *getFragmentShaderObject() const;

    void activate();
    void deactivate();

    template <typename T>
    void setUniform(const std::string &name, const T &value) const;

    template <typename T>
    void setUniform(const std::string &name, std::size_t len, const T *value) const;

    void setUniformWarningLevel(UniformWarning level);

    // Callback when the shader is reloaded. A reload can for example be triggered by a file change.
    const BaseCallBack *onReload(std::function<void()> callback);
    std::shared_ptr<std::function<void()>> onReloadScoped(std::function<void()> callback);
    void removeOnReload(const BaseCallBack *callback);

private:
    void bindAttributes();

    void handleError(OpenGLException &e);
    std::string processLog(std::string log) const;

    void rebuildShader(ShaderObject *obj);
    void linkShader(bool notifyRebuild = false);
    bool checkLinkStatus() const;

    static const transform_t transform;
    static const const_transform_t const_transform;

    std::string shaderNames() const;
    GLint findUniformLocation(const std::string &name) const;

    Program program_;

    // clear shader objects before the program is deleted
    ShaderMap shaderObjects_;

    bool ready_ = false;

    UniformWarning warningLevel_;
    // Uniform location cache. Clear after linking.
    mutable std::unordered_map<std::string, GLint> uniformLookup_;

    // Callback on reload.
    CallBackList onReloadCallback_;
};

template <typename T>
void Shader::setUniform(const std::string &name, const T &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) utilgl::UniformSetter<T>::set(location, value);
}

template <typename T>
void Shader::setUniform(const std::string &name, std::size_t len, const T *value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) utilgl::UniformSetter<T>::set(location, static_cast<GLsizei>(len), value);
}

}  // namespace inviwo

#endif  // IVW_SHADER_H
