
/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <modules/opengl/shader/shader.h>

#include <inviwo/core/datastructures/camera/camera.h>  // for mat4
#include <inviwo/core/util/callback.h>                 // for CallBackList, BaseCallBack
#include <inviwo/core/util/glmmat.h>                   // for mat2, mat3
#include <inviwo/core/util/glmvec.h>                   // for ivec4, ivec3, ivec2, uvec4, vec4
#include <inviwo/core/util/iterrange.h>                // for as_range, iter_range
#include <inviwo/core/util/logcentral.h>               // for log, LogAudience, LogAudience::User
#include <inviwo/core/util/safecstr.h>                 // for SafeCStr
#include <inviwo/core/util/sourcecontext.h>            // for IVW_CONTEXT
#include <inviwo/core/util/stdextensions.h>            // for all_of
#include <inviwo/core/util/stringconversion.h>         // for toString, toLower
#include <inviwo/core/util/transformiterator.h>        // for TransformIterator, makeTransformIt...
#include <modules/opengl/inviwoopengl.h>               // for LGL_ERROR, LGL_ERROR_CLASS
#include <modules/opengl/openglexception.h>            // for OpenGLException
#include <modules/opengl/shader/shadermanager.h>       // for ShaderManager
#include <modules/opengl/shader/shaderobject.h>        // for ShaderObject, ShaderObject::InDecl...
#include <modules/opengl/shader/shadertype.h>          // for ShaderType, ShaderType::Fragment
#include <modules/opengl/shader/shaderutils.h>         // for findShaderResource, getProgramInfoLog
#include <modules/opengl/shader/standardshaders.h>     // for imgIdentityVert
#include <modules/opengl/texture/textureunit.h>        // for TextureUnit

#include <algorithm>  // for find_if
#include <cstddef>    // for size_t
#include <istream>    // for operator<<, basic_ostream, ostring...
#include <span>       // for span

#include <fmt/core.h>            // for format, basic_string_view
#include <fmt/format.h>          // for join
#include <glm/gtc/type_ptr.hpp>  // for value_ptr

namespace inviwo {
class ShaderResource;

const detail::Build detail::Build::Yes{};
const detail::Build detail::Build::No{nullptr};

// glCreateProgram This function returns 0 if an error occurs creating the program object.
Shader::Program::Program() : id{glCreateProgram()} {}
Shader::Program::Program(const Program&) : Program() {}
Shader::Program::Program(Program&& rhs) noexcept : id{rhs.id} { rhs.id = 0; }
Shader::Program& Shader::Program::operator=(const Program&) {
    if (id == 0) {
        id = glCreateProgram();
    }
    return *this;
}
Shader::Program& Shader::Program::operator=(Program&& that) noexcept {
    Program copy(std::move(that));
    std::swap(id, copy.id);
    return *this;
}

Shader::Program::~Program() {
    if (id != 0) {
        glDeleteProgram(id);
    }
}

Shader::Shader(const std::vector<std::pair<ShaderType, std::string>>& items, Build buildShader)
    : warningLevel_{UniformWarning::Ignore} {

    for (auto& item : items) {
        setShaderObject(item.first, utilgl::findShaderResource(item.second));
    }

    if (buildShader) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(
    const std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>>& items,
    Build buildShader)
    : warningLevel_{UniformWarning::Ignore} {

    for (auto& item : items) {
        setShaderObject(item.first, item.second);
    }
    if (buildShader) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(std::vector<std::unique_ptr<ShaderObject>>& shaderObjects, Build buildShader)
    : warningLevel_{UniformWarning::Ignore} {

    for (auto& obj : shaderObjects) {
        setShaderObject(std::move(*obj));
    }

    if (buildShader) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(std::string_view vertexFilename, std::string_view geometryFilename,
               std::string_view fragmentFilename, Build buildShader)
    : Shader({{ShaderType::Vertex, utilgl::findShaderResource(vertexFilename)},
              {ShaderType::Geometry, utilgl::findShaderResource(geometryFilename)},
              {ShaderType::Fragment, utilgl::findShaderResource(fragmentFilename)}},
             buildShader) {}

Shader::Shader(std::string_view vertexFilename, std::string_view fragmentFilename,
               Build buildShader)
    : Shader({{ShaderType::Vertex, utilgl::findShaderResource(vertexFilename)},
              {ShaderType::Fragment, utilgl::findShaderResource(fragmentFilename)}},
             buildShader) {}

Shader::Shader(std::string_view fragmentFilename, Build buildShader)
    : Shader({utilgl::imgIdentityVert(),
              {ShaderType::Fragment, utilgl::findShaderResource(fragmentFilename)}},
             buildShader) {}

Shader::Shader(Shader&& rhs)
    : program_{[&]() {
        ShaderManager::getPtr()->unregisterShader(&rhs);
        return std::move(rhs.program_);
    }()}
    , shaderObjects_{std::move(rhs.shaderObjects_)}
    , attached_{std::move(rhs.attached_)}
    , ready_(rhs.ready_)
    , warningLevel_{rhs.warningLevel_} {

    rhs.callbacks_.clear();
    rhs.ready_ = false;

    for (auto& elem : shaderObjects_) {
        callbacks_.emplace_back(elem.onChange([this](ShaderObject* o) { rebuildShader(o); }));
    }
    ShaderManager::getPtr()->registerShader(this);
}

Shader& Shader::operator=(Shader&& that) {
    if (this != &that) {
        if (ShaderManager::getPtr()->isRegistered(this)) {
            ShaderManager::getPtr()->unregisterShader(this);
        }
        detach();
        callbacks_.clear();
        attached_.clear();
        shaderObjects_.clear();

        ShaderManager::getPtr()->unregisterShader(&that);

        program_ = std::move(that.program_);
        ready_ = that.ready_;
        warningLevel_ = that.warningLevel_;
        shaderObjects_ = std::move(that.shaderObjects_);
        attached_ = std::move(that.attached_);

        that.ready_ = false;
        that.callbacks_.clear();

        for (auto& elem : shaderObjects_) {
            callbacks_.emplace_back(elem.onChange([this](ShaderObject* o) { rebuildShader(o); }));
        }

        ShaderManager::getPtr()->registerShader(this);
    }
    return *this;
}

Shader::~Shader() {
    detach();
    if (ShaderManager::isInitialized() && ShaderManager::getPtr()->isRegistered(this)) {
        ShaderManager::getPtr()->unregisterShader(this);
    }
}
void Shader::attach() {
    for (size_t i = 0; i < shaderObjects_.size(); ++i) {
        if (!attached_[i]) {
            glAttachShader(program_.id, shaderObjects_[i].getID());
            attached_[i] = true;
        }
    }
}
void Shader::detach() {
    for (size_t i = 0; i < shaderObjects_.size(); ++i) {
        if (attached_[i]) {
            glDetachShader(program_.id, shaderObjects_[i].getID());
            attached_[i] = false;
        }
    }
}

void Shader::build() {
    try {
        ready_ = false;
        for (auto& elem : shaderObjects_) elem.build();
        linkShader();
    } catch (OpenGLException& e) {
        handleError(e);
    }
}

void Shader::link() {
    try {
        ready_ = false;
        linkShader();
    } catch (OpenGLException& e) {
        handleError(e);
    }
}

void Shader::linkShader(bool notifyRebuild) {
    attach();

    uniformLookup_.clear();  // clear uniform location cache.
    bindAttributes();

    if (!util::all_of(shaderObjects_, [](const auto& elem) { return elem.isReady(); })) {
        util::log(IVW_CONTEXT, "Id: " + toString(program_.id) + " objects not ready when linking.",
                  LogLevel::Error, LogAudience::User);
        return;
    }

    glLinkProgram(program_.id);

    if (!checkLinkStatus()) {
        throw OpenGLException("Id: " + toString(program_.id) + " " +
                                  processLog(utilgl::getProgramInfoLog(program_.id)),
                              IVW_CONTEXT);
    }

    auto log = utilgl::getProgramInfoLog(program_.id);
    if (!log.empty()) {
        util::log(IVW_CONTEXT,
                  "Id: " + toString(program_.id) + " (" + shaderNames() + ") " + processLog(log),
                  LogLevel::Info, LogAudience::User);
    }

    LGL_ERROR_CLASS;
    ready_ = true;
    if (notifyRebuild) onReloadCallback_.invokeAll();
}

bool Shader::checkLinkStatus() const {
    GLint res;
    glGetProgramiv(program_.id, GL_LINK_STATUS, &res);
    return res == GL_TRUE;
}

void Shader::bindAttributes() {
    for (const auto& obj : getShaderObjects()) {
        for (const auto& item : obj.getInDeclarations()) {
            if (item.location >= 0) {
                glBindAttribLocation(program_.id, item.location, item.name.c_str());
            }
        }
        for (const auto& item : obj.getOutDeclarations()) {
            if (item.location >= 0) {
                glBindFragDataLocation(program_.id, item.location, item.name.c_str());
            }
        }
    }
}

void Shader::rebuildShader(ShaderObject* obj) {
    try {
        ready_ = false;
        obj->build();
        linkShader();

        onReloadCallback_.invokeAll();

        util::log(IVW_CONTEXT,
                  "Id: " + toString(program_.id) + ", resource: " + obj->getFileName() +
                      " successfully reloaded",
                  LogLevel::Info, LogAudience::User);

        auto log = utilgl::getProgramInfoLog(program_.id);
        if (!log.empty()) {
            util::log(IVW_CONTEXT, "Id: " + toString(program_.id) + " " + processLog(log),
                      LogLevel::Info, LogAudience::User);
        }

    } catch (OpenGLException& e) {
        util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
    }
}

void Shader::handleError(OpenGLException& e) {
    auto onError = ShaderManager::getPtr()->getOnShaderError();
    switch (onError) {
        case Shader::OnError::Warn:
            util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
            break;
        case Shader::OnError::Throw:
            throw;
    }
}

std::string Shader::processLog(std::string log) const {
    std::istringstream stream(log);
    std::ostringstream result;
    std::string line;
    ShaderType type(0);

    while (std::getline(stream, line)) {
        // This log matching needs more testing. Mostly guessing here.
        auto lline = toLower(line);
        if (lline.find("vertex") != std::string::npos)
            type = ShaderType::Vertex;
        else if (lline.find("geometry") != std::string::npos)
            type = ShaderType::Geometry;
        else if (lline.find("fragment") != std::string::npos)
            type = ShaderType::Fragment;
        else if (lline.find("tessellation control") != std::string::npos)
            type = ShaderType::TessellationControl;
        else if (lline.find("tessellation evaluation") != std::string::npos)
            type = ShaderType::TessellationEvaluation;
        else if (lline.find("compute") != std::string::npos)
            type = ShaderType::Compute;

        int origLineNumber = utilgl::getLogLineNumber(line);

        auto obj = getShaderObject(type);
        if (obj && origLineNumber > 0) {
            auto res = obj->resolveLine(origLineNumber - 1);
            auto lineNumber = res.second;
            auto fileName = res.first;
            result << "\n"
                   << fileName << " (" << lineNumber << "): " << line.substr(line.find(":") + 1);
        } else {
            result << "\n" << line;
        }
    }
    return result.str();
}

bool Shader::isReady() const { return ready_; }

void Shader::invalidate() { ready_ = false; }

void Shader::activate() {
    if (!ready_)
        throw OpenGLException(
            "Shader Id: " + toString(program_.id) + " not ready: " + shaderNames(), IVW_CONTEXT);
    glUseProgram(program_.id);
    LGL_ERROR;
}

void Shader::deactivate() {
    glUseProgram(0);
    LGL_ERROR;
}

void Shader::setUniformWarningLevel(UniformWarning level) { warningLevel_ = level; }

const BaseCallBack* Shader::onReload(std::function<void()> callback) {
    return onReloadCallback_.addLambdaCallback(callback);
}

std::shared_ptr<std::function<void()>> Shader::onReloadScoped(std::function<void()> callback) {
    return onReloadCallback_.addLambdaCallbackRaii(callback);
}

void Shader::removeOnReload(const BaseCallBack* callback) { onReloadCallback_.remove(callback); }

std::string Shader::shaderNames() const {
    const auto name = [](const ShaderObject& o) { return o.getFileName(); };
    return fmt::format("[{}]",
                       fmt::join(util::makeTransformIterator(name, shaderObjects_.begin()),
                                 util::makeTransformIterator(name, shaderObjects_.end()), ", "));
}

GLint Shader::findUniformLocation(std::string_view name) const {
    auto it = uniformLookup_.find(name);
    if (it != uniformLookup_.end()) {
        return it->second;
    } else {
        GLint location = glGetUniformLocation(program_.id, SafeCStr{name});
        uniformLookup_.try_emplace(std::string{name}, location);

        if (warningLevel_ == UniformWarning::Throw && location == -1) {
            throw OpenGLException(IVW_CONTEXT, "Unable to set uniform {} in shader id: {}, {}",
                                  name, program_.id, shaderNames());
        } else if (warningLevel_ == UniformWarning::Warn && location == -1) {
            util::log(IVW_CONTEXT,
                      fmt::format("Unable to set uniform {} in shader id {}, {}: ", name,
                                  program_.id, shaderNames()),
                      LogLevel::Warn, LogAudience::User);
        }

        return location;
    }
}

const ShaderObject* Shader::operator[](ShaderType type) const { return getShaderObject(type); }
ShaderObject* Shader::operator[](ShaderType type) { return getShaderObject(type); }

const ShaderObject* Shader::getShaderObject(ShaderType type) const {
    auto it = std::find_if(shaderObjects_.begin(), shaderObjects_.end(),
                           [&](const ShaderObject& obj) { return obj.getShaderType() == type; });
    if (it != shaderObjects_.end()) {
        return &(*it);
    } else {
        return nullptr;
    }
}

ShaderObject* Shader::getShaderObject(ShaderType type) {
    auto it = std::find_if(shaderObjects_.begin(), shaderObjects_.end(),
                           [&](const ShaderObject& obj) { return obj.getShaderType() == type; });
    if (it != shaderObjects_.end()) {
        return &(*it);
    } else {
        return nullptr;
    }
}

const ShaderObject* Shader::getFragmentShaderObject() const {
    return getShaderObject(ShaderType::Fragment);
}

const ShaderObject* Shader::getGeometryShaderObject() const {
    return getShaderObject(ShaderType::Geometry);
}

const ShaderObject* Shader::getVertexShaderObject() const {
    return getShaderObject(ShaderType::Vertex);
}

const ShaderObject* Shader::getComputeShaderObject() const {
    return getShaderObject(ShaderType::Compute);
}

const ShaderObject* Shader::getTessellationControlShaderObject() const {
    return getShaderObject(ShaderType::TessellationControl);
}

const ShaderObject* Shader::getTessellationEvaluationShaderObject() const {
    return getShaderObject(ShaderType::TessellationEvaluation);
}

ShaderObject* Shader::getFragmentShaderObject() { return getShaderObject(ShaderType::Fragment); }

ShaderObject* Shader::getGeometryShaderObject() { return getShaderObject(ShaderType::Geometry); }

ShaderObject* Shader::getVertexShaderObject() { return getShaderObject(ShaderType::Vertex); }

ShaderObject* Shader::getComputeShaderObject() { return getShaderObject(ShaderType::Compute); }

ShaderObject* Shader::getTessellationControlShaderObject() {
    return getShaderObject(ShaderType::TessellationControl);
}

ShaderObject* Shader::getTessellationEvaluationShaderObject() {
    return getShaderObject(ShaderType::TessellationEvaluation);
}

auto Shader::begin() -> iterator { return shaderObjects_.begin(); }
auto Shader::end() -> iterator { return shaderObjects_.end(); }
auto Shader::begin() const -> const_iterator { return shaderObjects_.begin(); }
auto Shader::end() const -> const_iterator { return shaderObjects_.end(); }
auto Shader::getShaderObjects() -> util::iter_range<iterator> {
    return util::as_range(begin(), end());
}
auto Shader::getShaderObjects() const -> util::iter_range<const_iterator> {
    return util::as_range(begin(), end());
}

void Shader::setShaderObject(ShaderType type, std::shared_ptr<const ShaderResource> resource) {
    setShaderObject(ShaderObject{type, resource});
}

void Shader::setShaderObject(ShaderObject object) {
    shaderObjects_.emplace_back(std::move(object));
    attached_.emplace_back(false);
    callbacks_.emplace_back(
        shaderObjects_.back().onChange([this](ShaderObject* o) { rebuildShader(o); }));
}

void Shader::setUniform(std::string_view name, bool value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1i(location, static_cast<int>(value));
}
void Shader::setUniform(std::string_view name, std::span<const bool> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1) {
        std::vector<int> tmp(values.begin(), values.end());
        glUniform1iv(location, static_cast<GLsizei>(tmp.size()), tmp.data());
    }
}
void Shader::setUniform(std::string_view name, int value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1i(location, value);
}
void Shader::setUniform(std::string_view name, std::span<const int> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1iv(location, static_cast<GLsizei>(values.size()), values.data());
}
void Shader::setUniform(std::string_view name, unsigned int value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1ui(location, value);
}
void Shader::setUniform(std::string_view name, std::span<const unsigned int> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1uiv(location, static_cast<GLsizei>(values.size()), values.data());
}
void Shader::setUniform(std::string_view name, float value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1f(location, value);
}
void Shader::setUniform(std::string_view name, std::span<const float> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1fv(location, static_cast<GLsizei>(values.size()), values.data());
}

void Shader::setUniform(std::string_view name, bvec2 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) {
        ivec2 tmp(value);
        glUniform2i(location, tmp[0], tmp[1]);
    }
}
void Shader::setUniform(std::string_view name, std::span<const bvec2> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1) {
        std::vector<ivec2> tmp(values.begin(), values.end());
        glUniform2iv(location, static_cast<GLsizei>(tmp.size()), glm::value_ptr(*tmp.data()));
    }
}
void Shader::setUniform(std::string_view name, bvec3 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) {
        ivec3 tmp(value);
        glUniform3i(location, tmp[0], tmp[1], tmp[2]);
    }
}
void Shader::setUniform(std::string_view name, std::span<const bvec3> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1) {
        std::vector<ivec3> tmp(values.begin(), values.end());
        glUniform3iv(location, static_cast<GLsizei>(tmp.size()), glm::value_ptr(*tmp.data()));
    }
}
void Shader::setUniform(std::string_view name, bvec4 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) {
        ivec4 tmp(value);
        glUniform4i(location, tmp[0], tmp[1], tmp[2], tmp[3]);
    }
}
void Shader::setUniform(std::string_view name, std::span<const bvec4> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1) {
        std::vector<ivec4> tmp(values.begin(), values.end());
        glUniform4iv(location, static_cast<GLsizei>(tmp.size()), glm::value_ptr(*tmp.data()));
    }
}

void Shader::setUniform(std::string_view name, ivec2 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform2i(location, value[0], value[1]);
}
void Shader::setUniform(std::string_view name, std::span<const ivec2> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniform2iv(location, static_cast<GLsizei>(values.size()), glm::value_ptr(*values.data()));
}
void Shader::setUniform(std::string_view name, ivec3 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform3i(location, value[0], value[1], value[2]);
}
void Shader::setUniform(std::string_view name, std::span<const ivec3> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniform3iv(location, static_cast<GLsizei>(values.size()), glm::value_ptr(*values.data()));
}
void Shader::setUniform(std::string_view name, ivec4 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform4i(location, value[0], value[1], value[2], value[3]);
}
void Shader::setUniform(std::string_view name, std::span<const ivec4> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniform4iv(location, static_cast<GLsizei>(values.size()), glm::value_ptr(*values.data()));
}

void Shader::setUniform(std::string_view name, uvec2 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform2ui(location, value[0], value[1]);
}
void Shader::setUniform(std::string_view name, std::span<const uvec2> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniform2uiv(location, static_cast<GLsizei>(values.size()),
                      glm::value_ptr(*values.data()));
}
void Shader::setUniform(std::string_view name, uvec3 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform3ui(location, value[0], value[1], value[2]);
}
void Shader::setUniform(std::string_view name, std::span<const uvec3> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniform3uiv(location, static_cast<GLsizei>(values.size()),
                      glm::value_ptr(*values.data()));
}
void Shader::setUniform(std::string_view name, uvec4 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform4ui(location, value[0], value[1], value[2], value[3]);
}
void Shader::setUniform(std::string_view name, std::span<const uvec4> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniform4uiv(location, static_cast<GLsizei>(values.size()),
                      glm::value_ptr(*values.data()));
}

void Shader::setUniform(std::string_view name, vec2 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform2f(location, value[0], value[1]);
}
void Shader::setUniform(std::string_view name, std::span<const vec2> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniform2fv(location, static_cast<GLsizei>(values.size()), glm::value_ptr(*values.data()));
}
void Shader::setUniform(std::string_view name, vec3 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform3f(location, value[0], value[1], value[2]);
}
void Shader::setUniform(std::string_view name, std::span<const vec3> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniform3fv(location, static_cast<GLsizei>(values.size()), glm::value_ptr(*values.data()));
}
void Shader::setUniform(std::string_view name, vec4 value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform4f(location, value[0], value[1], value[2], value[3]);
}
void Shader::setUniform(std::string_view name, std::span<const vec4> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniform4fv(location, static_cast<GLsizei>(values.size()), glm::value_ptr(*values.data()));
}

void Shader::setUniform(std::string_view name, const mat2& value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setUniform(std::string_view name, std::span<const mat2> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniformMatrix2fv(location, static_cast<GLsizei>(values.size()), GL_FALSE,
                           glm::value_ptr(*values.data()));
}
void Shader::setUniform(std::string_view name, const mat3& value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setUniform(std::string_view name, std::span<const mat3> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniformMatrix3fv(location, static_cast<GLsizei>(values.size()), GL_FALSE,
                           glm::value_ptr(*values.data()));
}
void Shader::setUniform(std::string_view name, const mat4& value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setUniform(std::string_view name, std::span<const mat4> values) const {
    if (values.empty()) return;
    GLint location = findUniformLocation(name);
    if (location != -1)
        glUniformMatrix4fv(location, static_cast<GLsizei>(values.size()), GL_FALSE,
                           glm::value_ptr(*values.data()));
}

void Shader::setUniform(std::string_view name, const TextureUnit& texUnit) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1i(location, texUnit.getUnitNumber());
}

void Shader::setTransformFeedbackVaryings(std::span<const GLchar*> varyings, GLenum bufferMode) {
    glTransformFeedbackVaryings(program_.id, static_cast<GLsizei>(varyings.size()), varyings.data(),
                                bufferMode);
    ready_ = false;
}

}  // namespace inviwo
