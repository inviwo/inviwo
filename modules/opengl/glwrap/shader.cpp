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

#include <modules/opengl/glwrap/shadermanager.h>

namespace inviwo {

Shader::Shader(std::string fragmentFilename, bool linkShader)
    : id_{glCreateProgram()}, warningLevel_{UniformWarning::Ignore} {
    createAndAddShader(GL_VERTEX_SHADER, "img_identity.vert", linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(std::string vertexFilename, std::string fragmentFilename, bool linkShader)
    : id_{glCreateProgram()}, warningLevel_{UniformWarning::Ignore} {
    createAndAddShader(GL_VERTEX_SHADER, vertexFilename, linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(std::string vertexFilename, std::string geometryFilename,
               std::string fragmentFilename, bool linkShader)
    : id_{glCreateProgram()}, warningLevel_{UniformWarning::Ignore} {
    createAndAddShader(GL_VERTEX_SHADER, vertexFilename, linkShader);
    createAndAddShader(GL_GEOMETRY_SHADER, geometryFilename, linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(const char *fragmentFilename, bool linkShader)
    : id_{glCreateProgram()}, warningLevel_{UniformWarning::Ignore} {
    createAndAddShader(GL_VERTEX_SHADER, "img_identity.vert", linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(const char *vertexFilename, const char *fragmentFilename, bool linkShader)
    : id_{glCreateProgram()}, warningLevel_{UniformWarning::Ignore} {
    createAndAddShader(GL_VERTEX_SHADER, vertexFilename, linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(const char *vertexFilename, const char *geometryFilename,
               const char *fragmentFilename, bool linkShader)
    : id_{glCreateProgram()}, warningLevel_{UniformWarning::Ignore} {
    createAndAddShader(GL_VERTEX_SHADER, vertexFilename, linkShader);
    createAndAddShader(GL_GEOMETRY_SHADER, geometryFilename, linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(std::vector<ShaderObject *> &shaderObjects, bool linkShader)
    : id_{glCreateProgram()}, warningLevel_{UniformWarning::Ignore} {
    for (auto &shaderObject : shaderObjects)
        shaderObjects_[shaderObject->getShaderType()].reset(shaderObject);

    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(const Shader &rhs, bool linkShader)
    : id_{glCreateProgram()}, warningLevel_{rhs.warningLevel_} {
    for (auto &it : rhs.shaderObjects_) {
        this->shaderObjects_[it.first].reset(it.second->clone(false));
    }
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader &Shader::operator=(const Shader &that) {
    if (this != &that) {
        for (auto &it : that.shaderObjects_) {
            this->shaderObjects_[it.first].reset(it.second->clone(false));
        }
        warningLevel_ = that.warningLevel_;
        attachAllShaderObjects();
        linkAndRegister(true);
    }
    return *this;
}

Shader::~Shader() {
    ShaderManager::getPtr()->unregisterShader(this);

    shaderObjects_.clear();

    glDeleteProgram(id_);
    LGL_ERROR;
}

Shader *Shader::clone(bool linkShader) { return new Shader(*this, linkShader); }

void Shader::linkAndRegister(bool linkShader) {
    if (linkShader) link();

    ShaderManager::getPtr()->registerShader(this);
}

void Shader::createAndAddShader(GLenum shaderType, std::string fileName, bool linkShader) {
    shaderObjects_[shaderType] = ShaderObjectPtr(new ShaderObject(shaderType, fileName, linkShader),
                                                 [this](ShaderObject *shaderObject) {
                                                     detachShaderObject(shaderObject);
                                                     delete shaderObject;
                                                 });
}

void Shader::link(bool notifyRebuild) {
    uniformLookup_.clear();  // clear uniform location cache.
    ShaderManager::getPtr()->bindCommonAttributes(id_);

    glLinkProgram(id_);
    LGL_ERROR;

    if (notifyRebuild) onReloadCallback_.invokeAll();
}

void Shader::build() {
    for (auto &elem : shaderObjects_) elem.second->build();
    link();
}

void Shader::rebuild() {
    for (auto &elem : shaderObjects_) elem.second->rebuild();
    link();
}

void Shader::activate() {
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

const BaseCallBack* Shader::onReload(std::function<void()> callback) {
    return onReloadCallback_.addLambdaCallback(callback); 
}

void Shader::removeOnReload(const BaseCallBack* callback) {
    onReloadCallback_.remove(callback);
}

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
            throw OpenGLException("Unable to set uniform " + name + " in shader " + shaderNames(),
                                  IvwContext);
        } else if (warningLevel_ == UniformWarning::Warn && location == -1) {
            LogWarn("Unable to set uniform " + name + " in shader " + shaderNames());
        }

        return location;
    }
}

void Shader::setUniform(const std::string &name, const GLint &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1i(location, value);
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

void Shader::setUniform(const std::string &name, const GLint *value, int count) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1iv(location, count, value);
}

void Shader::setUniform(const std::string &name, const GLfloat *value, int count) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniform1fv(location, count, value);
}

void Shader::setUniform(const std::string &name, const mat3 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const mat4 &value) const {
    GLint location = findUniformLocation(name);
    if (location != -1) glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

ShaderObject *Shader::getFragmentShaderObject() const {
    auto it = shaderObjects_.find(GL_FRAGMENT_SHADER);
    if (it != shaderObjects_.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

ShaderObject *Shader::getGeometryShaderObject() const {
    auto it = shaderObjects_.find(GL_GEOMETRY_SHADER);
    if (it != shaderObjects_.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

ShaderObject *Shader::getVertexShaderObject() const {
    auto it = shaderObjects_.find(GL_VERTEX_SHADER);
    if (it != shaderObjects_.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

}  // namespace
