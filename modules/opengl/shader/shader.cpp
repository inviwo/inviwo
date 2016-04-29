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

#include "shader.h"

#include <modules/opengl/shader/shadermanager.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/opengl/shader/shaderresource.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

Shader::Shader(const std::vector<std::pair<ShaderType, std::string>> &items, Build buildShader)
    : id_{glCreateProgram()}, warningLevel_{UniformWarning::Ignore} {
    for (auto &item : items) createAndAddShader(item.first, item.second);

    if (buildShader == Build::Yes) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(
    const std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>> &items,
    Build buildShader)
    : id_{glCreateProgram()}, warningLevel_{UniformWarning::Ignore} {
    for (auto &item : items) createAndAddShader(item.first, item.second);

    if (buildShader == Build::Yes) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(std::string vertexFilename, std::string geometryFilename,
               std::string fragmentFilename, bool buildShader)
    : Shader({{ShaderType::Vertex, vertexFilename},
              {ShaderType::Geometry, geometryFilename},
              {ShaderType::Fragment, fragmentFilename}},
             buildShader ? Build::Yes : Build::No) {}

Shader::Shader(std::string vertexFilename, std::string fragmentFilename, bool buildShader)
    : Shader({{ShaderType::Vertex, vertexFilename}, {ShaderType::Fragment, fragmentFilename}},
             buildShader ? Build::Yes : Build::No) {}

Shader::Shader(std::string fragmentFilename, bool buildShader)
    : Shader({{ShaderType::Vertex, "img_identity.vert"}, {ShaderType::Fragment, fragmentFilename}},
             buildShader ? Build::Yes : Build::No) {}

Shader::Shader(const char *fragmentFilename, bool buildShader)
    : Shader(std::string(fragmentFilename), buildShader) {}

Shader::Shader(const char *vertexFilename, const char *fragmentFilename, bool buildShader)
    : Shader(std::string(vertexFilename), std::string(fragmentFilename), buildShader) {}

Shader::Shader(const char *vertexFilename, const char *geometryFilename,
               const char *fragmentFilename, bool buildShader)
    : Shader(std::string(vertexFilename), std::string(geometryFilename),
             std::string(fragmentFilename), buildShader) {}

Shader::Shader(std::vector<std::unique_ptr<ShaderObject>> &shaderObjects, bool buildShader)
    : id_{glCreateProgram()}, warningLevel_{UniformWarning::Ignore} {
    for (auto &shaderObject : shaderObjects) createAndAddShader(std::move(shaderObject));

    if (buildShader) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(const Shader &rhs) : id_{glCreateProgram()}, warningLevel_{rhs.warningLevel_} {
    for (auto &elem : rhs.shaderObjects_) {
        createAndAddShader(util::make_unique<ShaderObject>(*(elem.second.get())));
    }

    if (rhs.isReady()) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader &Shader::operator=(const Shader &that) {
    if (this != &that) {
        shaderObjects_.clear();
        for (auto &elem : that.shaderObjects_) {
            createAndAddShader(util::make_unique<ShaderObject>(*(elem.second.get())));
        }
        warningLevel_ = that.warningLevel_;

        if (that.isReady()) build();
    }
    return *this;
}

Shader::~Shader() {
    ShaderManager::getPtr()->unregisterShader(this);

    shaderObjects_.clear();

    glDeleteProgram(id_);
    LGL_ERROR;
}

void Shader::createAndAddShader(ShaderType type, std::string fileName) {
    createAndAddHelper(new ShaderObject(type, fileName));
}

void Shader::createAndAddShader(std::unique_ptr<ShaderObject> object) {
    createAndAddHelper(object.get());
    object.release();
}

void Shader::createAndAddShader(ShaderType type, std::shared_ptr<const ShaderResource> resource) {
    createAndAddHelper(new ShaderObject(type, resource));
}

void Shader::createAndAddHelper(ShaderObject *object) {
    auto ptr = ShaderObjectPtr(object, [this](ShaderObject *shaderObject) {
        detachShaderObject(shaderObject);
        delete shaderObject;
    });

    objectCallbacks_.push_back(ptr->onChange([this](ShaderObject *o) { rebildShader(o); }));
    attachShaderObject(ptr.get());

    shaderObjects_[object->getShaderType()] = std::move(ptr);
}

void Shader::build() {
    try {
        ready_ = false;
        for (auto &elem : shaderObjects_) elem.second->build();
        linkShader();
    } catch (OpenGLException &e) {
        handleError(e);
    }
}

void Shader::link() {
    try {
        ready_ = false;
        linkShader();
    } catch (OpenGLException &e) {
        handleError(e);
    }
}

void Shader::linkShader(bool notifyRebuild /*= false*/) {
    uniformLookup_.clear();  // clear uniform location cache.
    ShaderManager::getPtr()->bindCommonAttributes(id_);

    if (!util::all_of(shaderObjects_, [](const ShaderObjectMap::value_type &elem) {
            return elem.second->isReady();
        })) {
        util::log(IvwContext, "Id: " + toString(id_) + " objects not ready when linking.",
                  LogLevel::Error, LogAudience::User);
        return;
    }

    glLinkProgram(id_);

    if (!isReady()) {
        throw OpenGLException(
            "Id: " + toString(id_) + " " + processLog(utilgl::getProgramInfoLog(id_)), IvwContext);
    }

#ifdef IVW_DEBUG
    auto log = utilgl::getProgramInfoLog(id_);
    if (!log.empty()) {
        util::log(IvwContext, "Id: " + toString(id_) + " " + processLog(log), LogLevel::Info,
                  LogAudience::User);
    }
#endif

    LGL_ERROR;
    ready_ = true;
    if (notifyRebuild) onReloadCallback_.invokeAll();
}

void Shader::rebildShader(ShaderObject *obj) {
    try {
        ready_ = false;
        obj->build();
        linkShader();

        onReloadCallback_.invokeAll();

        util::log(IvwContext, "Id: " + toString(id_) + ", resource: " + obj->getFileName() +
                                  " successfully reloaded",
                  LogLevel::Info, LogAudience::User);
    } catch (OpenGLException &e) {
        util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
    }
}

void Shader::handleError(OpenGLException &e) {
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
        if (lline.find("vertex"))
            type = ShaderType::Vertex;
        else if (lline.find("geometry"))
            type = ShaderType::Geometry;
        else if (lline.find("fragment"))
            type = ShaderType::Fragment;
        else if (lline.find("tessellation control"))
            type = ShaderType::TessellationControl;
        else if (lline.find("tessellation evaluation"))
            type = ShaderType::TessellationEvaluation;
        else if (lline.find("compute"))
            type = ShaderType::Compute;

        int origLineNumber = utilgl::getLogLineNumber(line);

        auto obj = getShaderObject(type);
        if (obj && origLineNumber > 0) {
            auto res = obj->resolveLine(origLineNumber - 1);
            auto lineNumber = res.second;
            auto fileName = res.first;
            result << "\n" << fileName << " (" << lineNumber
                   << "): " << line.substr(line.find(":") + 1);
        } else {
            result << "\n" << line;
        }
    }
    return result.str();
}

bool Shader::isReady() const {
    GLint res;
    glGetProgramiv(id_, GL_LINK_STATUS, &res);
    return res == GL_TRUE;
}

void Shader::activate() {
    if (!ready_) throw OpenGLException("Shader Id: " + toString(id_) + " not ready: " + shaderNames(), IvwContext);
    glUseProgram(id_);
    LGL_ERROR;
}

void Shader::deactivate() {
    glUseProgram(0);
    LGL_ERROR;
}

void Shader::attachShaderObject(ShaderObject *shaderObject) {
    glAttachShader(id_, shaderObject->getID());
    LGL_ERROR;
}

void Shader::detachShaderObject(ShaderObject *shaderObject) {
    glDetachShader(id_, shaderObject->getID());
    LGL_ERROR;
}

void Shader::attachAllShaderObjects() {
    for (auto &elem : shaderObjects_) {
        attachShaderObject(elem.second.get());
    }
}

void Shader::detachAllShaderObject() {
    for (auto &elem : shaderObjects_) {
        detachShaderObject(elem.second.get());
    }
}

void Shader::setUniformWarningLevel(UniformWarning level) { warningLevel_ = level; }

const BaseCallBack *Shader::onReload(std::function<void()> callback) {
    return onReloadCallback_.addLambdaCallback(callback);
}

void Shader::removeOnReload(const BaseCallBack *callback) { onReloadCallback_.remove(callback); }

std::string Shader::shaderNames() const {
    std::vector<std::string> names;
    for (const auto &elem : shaderObjects_) {
        names.push_back(elem.second->getFileName());
    }
    return joinString(names, "/");
}

GLint Shader::findUniformLocation(const std::string &name) const {
    auto it = uniformLookup_.find(name);
    if (it != uniformLookup_.end()) {
        return it->second;
    } else {
        GLint location = glGetUniformLocation(id_, name.c_str());
        uniformLookup_[name] = location;

        if (warningLevel_ == UniformWarning::Throw && location == -1) {
            throw OpenGLException("Unable to set uniform " + name + " in shader id: " +
                                      toString(id_) + " " + shaderNames(),
                                  IvwContext);
        } else if (warningLevel_ == UniformWarning::Warn && location == -1) {
            util::log(IvwContext, "Unable to set uniform " + name + " in shader " +
                                      " in shader id: " + toString(id_) + " " + shaderNames(),
                      LogLevel::Warn, LogAudience::User);
        }

        return location;
    }
}

void Shader::setUniform(const std::string &name, const GLint &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1i(location, value);
}

void Shader::setUniform(const std::string &name, const GLint *value, int count) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1iv(location, count, value);
}

void Shader::setUniform(const std::string &name, const GLuint &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1ui(location, value);
}

void Shader::setUniform(const std::string &name, const GLuint *value, int count) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1uiv(location, count, value);
}

void Shader::setUniform(const std::string &name, const GLfloat *value, int count) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1fv(location, count, value);
}

void Shader::setUniform(const std::string &name, const ivec2 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform2iv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const ivec3 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform3iv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const ivec4 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform4iv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const uvec2 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform2uiv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const uvec3 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform3uiv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const uvec4 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform4uiv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const GLfloat &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1f(location, value);
}

void Shader::setUniform(const std::string &name, const vec2 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform2fv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const vec3 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform3fv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const vec4 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform4fv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const mat2 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const mat3 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const mat4 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

ShaderObject *Shader::getShaderObject(ShaderType type) const {
    return util::map_find_or_null(shaderObjects_, type,
                                  [](const ShaderObjectPtr &p) { return p.get(); });
}

ShaderObject *Shader::getFragmentShaderObject() const {
    return getShaderObject(ShaderType::Fragment);
}

ShaderObject *Shader::getGeometryShaderObject() const {
    return getShaderObject(ShaderType::Geometry);
}

ShaderObject *Shader::getVertexShaderObject() const { return getShaderObject(ShaderType::Vertex); }

}  // namespace
