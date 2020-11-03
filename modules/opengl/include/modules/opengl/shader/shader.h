/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <inviwo/core/util/callback.h>           // for BaseCallBack, CallBackList
#include <inviwo/core/util/glmmat.h>             // for mat2, mat3, mat4
#include <inviwo/core/util/glmvec.h>             // for bvec2, bvec3, bvec4, ivec2, ivec3, ivec4
#include <inviwo/core/util/iterrange.h>          // for iter_range
#include <modules/opengl/inviwoopengl.h>         // for GLint, GLuint
#include <modules/opengl/shader/shaderobject.h>  // for ShaderObject, ShaderObject::Callback

#include <cstddef>      // for nullptr_t
#include <functional>   // for function, less
#include <map>          // for map
#include <memory>       // for shared_ptr, unique_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for enable_if_t
#include <utility>      // for pair
#include <vector>       // for vector, vector<>::const_iterator, vector...
#include <span>         // for span

#include <inviwo/tracy/tracy.h>

namespace inviwo {

class OpenGLException;
class ShaderResource;
class ShaderType;
class TextureUnit;

namespace detail {
class IVW_MODULE_OPENGL_API Build {
    bool value_ = true;
    Build() = default;
    Build(std::nullptr_t) : value_{false} {}

public:
    // Allow explicit conversion from anything bool like
    template <
        typename T,
        std::enable_if_t<!std::is_same_v<T, bool> && std::is_constructible_v<bool, T>, int> = 0>
    explicit constexpr Build(T value) noexcept : value_{value} {}

    // Allow implicit construction only from bool
    // clang-format off
    template <typename T, std::enable_if_t<std::is_same_v<T, bool>, int> = 0>
    [[deprecated("Use Shader::Build::Yes or Shader::Build::No, instead of a true or false")]]
    constexpr Build(T value) noexcept : value_{value} {}
    // clang-format on

    explicit constexpr operator bool() { return value_; }

    static const Build Yes;
    static const Build No;

    friend constexpr bool operator==(Build a, Build b) { return b.value_ == a.value_; }
    friend constexpr bool operator!=(Build a, Build b) { return b.value_ != a.value_; }
};
}  // namespace detail

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
    enum class UniformWarning { Ignore, Warn, Throw };
    using Build = detail::Build;

    Shader(const std::vector<std::pair<ShaderType, std::string>>& items,
           Build buildShader = Build::Yes);
    Shader(const std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>>& items,
           Build buildShader = Build::Yes);
    Shader(std::vector<std::unique_ptr<ShaderObject>>& shaderObjects,
           Build buildShader = Build::Yes);

    /*
     * Will add utilgl::imgIdentityVert() as vertex shader.
     */
    Shader(std::string_view fragmentFilename, Build buildShader = Build::Yes);
    Shader(std::string_view vertexFilename, std::string_view fragmentFilename,
           Build buildShader = Build::Yes);
    Shader(std::string_view vertexFilename, std::string_view geometryFilename,
           std::string_view fragmentFilename, Build buildShader = Build::Yes);

    Shader(const Shader& rhs) = delete;
    Shader(Shader&& rhs);
    Shader& operator=(const Shader& that) = delete;
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
    const ShaderObject* getComputeShaderObject() const;
    const ShaderObject* getTesselationControlShaderObject() const;
    const ShaderObject* getTesselationEvaluationShaderObject() const;

    ShaderObject* getVertexShaderObject();
    ShaderObject* getGeometryShaderObject();
    ShaderObject* getFragmentShaderObject();
    ShaderObject* getComputeShaderObject();
    ShaderObject* getTesselationControlShaderObject();
    ShaderObject* getTesselationEvaluationShaderObject();

    void activate();
    void deactivate();

    void setUniform(std::string_view name, bool value) const;
    void setUniform(std::string_view name, std::span<const bool> values) const;
    void setUniform(std::string_view name, int value) const;
    void setUniform(std::string_view name, std::span<const int> values) const;
    void setUniform(std::string_view name, unsigned int value) const;
    void setUniform(std::string_view name, std::span<const unsigned int> values) const;
    void setUniform(std::string_view name, float value) const;
    void setUniform(std::string_view name, std::span<const float> values) const;

    void setUniform(std::string_view name, bvec2 value) const;
    void setUniform(std::string_view name, std::span<const bvec2> values) const;
    void setUniform(std::string_view name, bvec3 value) const;
    void setUniform(std::string_view name, std::span<const bvec3> values) const;
    void setUniform(std::string_view name, bvec4 value) const;
    void setUniform(std::string_view name, std::span<const bvec4> values) const;

    void setUniform(std::string_view name, ivec2 value) const;
    void setUniform(std::string_view name, std::span<const ivec2> values) const;
    void setUniform(std::string_view name, ivec3 value) const;
    void setUniform(std::string_view name, std::span<const ivec3> values) const;
    void setUniform(std::string_view name, ivec4 value) const;
    void setUniform(std::string_view name, std::span<const ivec4> values) const;

    void setUniform(std::string_view name, uvec2 value) const;
    void setUniform(std::string_view name, std::span<const uvec2> values) const;
    void setUniform(std::string_view name, uvec3 value) const;
    void setUniform(std::string_view name, std::span<const uvec3> values) const;
    void setUniform(std::string_view name, uvec4 value) const;
    void setUniform(std::string_view name, std::span<const uvec4> values) const;

    void setUniform(std::string_view name, vec2 value) const;
    void setUniform(std::string_view name, std::span<const vec2> values) const;
    void setUniform(std::string_view name, vec3 value) const;
    void setUniform(std::string_view name, std::span<const vec3> values) const;
    void setUniform(std::string_view name, vec4 value) const;
    void setUniform(std::string_view name, std::span<const vec4> values) const;

    void setUniform(std::string_view name, const mat2& value) const;
    void setUniform(std::string_view name, std::span<const mat2> values) const;
    void setUniform(std::string_view name, const mat3& value) const;
    void setUniform(std::string_view name, std::span<const mat3> values) const;
    void setUniform(std::string_view name, const mat4& value) const;
    void setUniform(std::string_view name, std::span<const mat4> values) const;

    void setUniform(std::string_view name, const TextureUnit& value) const;

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

    void attach();
    void detach();

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

}  // namespace inviwo
