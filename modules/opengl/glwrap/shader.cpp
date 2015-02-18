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

Shader::Shader(std::string fragmentFilename, bool linkShader) {
    initialize();
    createAndAddShader(GL_VERTEX_SHADER, "img_identity.vert", linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(std::string vertexFilename, std::string fragmentFilename, bool linkShader) {
    initialize();
    createAndAddShader(GL_VERTEX_SHADER, vertexFilename, linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(std::string vertexFilename, std::string geometryFilename,
               std::string fragmentFilename, bool linkShader) {
    initialize();
    createAndAddShader(GL_VERTEX_SHADER, vertexFilename, linkShader);
    createAndAddShader(GL_GEOMETRY_SHADER, geometryFilename, linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(const char *fragmentFilename, bool linkShader) {
    initialize();
    createAndAddShader(GL_VERTEX_SHADER, "img_identity.vert", linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(const char *vertexFilename, const char *fragmentFilename, bool linkShader) {
    initialize();
    createAndAddShader(GL_VERTEX_SHADER, vertexFilename, linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(const char *vertexFilename, const char *geometryFilename,
               const char *fragmentFilename, bool linkShader) {
    initialize();
    createAndAddShader(GL_VERTEX_SHADER, vertexFilename, linkShader);
    createAndAddShader(GL_GEOMETRY_SHADER, geometryFilename, linkShader);
    createAndAddShader(GL_FRAGMENT_SHADER, fragmentFilename, linkShader);
    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::Shader(std::vector<ShaderObject *> &shaderObjects, bool linkShader) {
    initialize();

    for (size_t i = 0; i < shaderObjects.size(); ++i)
        (*shaderObjects_)[shaderObjects[i]->getShaderType()] = shaderObjects[i];

    attachAllShaderObjects();
    linkAndRegister(linkShader);
}

Shader::~Shader() { deinitialize(); }

void Shader::initialize() {
    id_ = glCreateProgram();
    LGL_ERROR;
    shaderObjects_ = new ShaderObjectMap();
}

void Shader::linkAndRegister(bool linkShader) {
    if (linkShader) link();

    ShaderManager::getPtr()->registerShader(this);
}

void Shader::deinitialize() {
    ShaderManager::getPtr()->unregisterShader(this);

    for (ShaderObjectMap::iterator it = shaderObjects_->begin(); it != shaderObjects_->end();
         it++) {
        detachShaderObject(it->second);
        delete it->second;
    }

    delete shaderObjects_;
    glDeleteProgram(id_);
    LGL_ERROR;
}

void Shader::createAndAddShader(GLenum shaderType, std::string fileName, bool linkShader) {
    (*shaderObjects_)[shaderType] = new ShaderObject(shaderType, fileName, linkShader);
}

void Shader::link() {
    ShaderManager::getPtr()->bindCommonAttributes(id_);

    glLinkProgram(id_);
    LGL_ERROR;
}

void Shader::build() {
    for (ShaderObjectMap::iterator it = shaderObjects_->begin(); it != shaderObjects_->end();
         it++) {
        it->second->build();
    }

    link();
}

void Shader::rebuild() {
    for (ShaderObjectMap::iterator it = shaderObjects_->begin(); it != shaderObjects_->end();
         it++) {
        it->second->rebuild();
    }

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
    for (ShaderObjectMap::iterator it = shaderObjects_->begin(); it != shaderObjects_->end();
         it++) {
        attachShaderObject(it->second);
    }
}

void Shader::detachAllShaderObject() {
    for (ShaderObjectMap::iterator it = shaderObjects_->begin(); it != shaderObjects_->end();
         it++) {
        detachShaderObject(it->second);
    }
}

//#define IVW_ELSE_WARN                                                                              \
//    else LogWarn(                                                                                  \
//        "Unable to set uniform " + name + " in shader " + getVertexShaderObject()->getFileName() + \
//        "/" +                                                                                      \
//        ((getGeometryShaderObject()) ? getGeometryShaderObject()->getFileName() + "/" : "") +      \
//        getFragmentShaderObject()->getFileName());
#define IVW_ELSE_WARN

void Shader::setUniform(const std::string &name, const GLint &value) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1) glUniform1i(uniformLocation, value);
    IVW_ELSE_WARN
}

void Shader::setUniform(const std::string &name, const GLfloat &value) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1) glUniform1f(uniformLocation, value);
    IVW_ELSE_WARN
}

void Shader::setUniform(const std::string &name, const vec2 &value) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1) glUniform2fv(uniformLocation, 1, glm::value_ptr(value));
    IVW_ELSE_WARN
}

void Shader::setUniform(const std::string &name, const vec3 &value) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1) glUniform3fv(uniformLocation, 1, glm::value_ptr(value));
    IVW_ELSE_WARN
}

void Shader::setUniform(const std::string &name, const vec4 &value) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1) glUniform4fv(uniformLocation, 1, glm::value_ptr(value));
    IVW_ELSE_WARN
}

void Shader::setUniform(const std::string &name, const ivec2 &value) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1) glUniform2iv(uniformLocation, 1, glm::value_ptr(value));
    IVW_ELSE_WARN
}

void Shader::setUniform(const std::string &name, const ivec3 &value) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1) glUniform3iv(uniformLocation, 1, glm::value_ptr(value));
    IVW_ELSE_WARN
}

void Shader::setUniform(const std::string &name, const ivec4 &value) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1) glUniform4iv(uniformLocation, 1, glm::value_ptr(value));
    IVW_ELSE_WARN
}

void Shader::setUniform(const std::string &name, const GLint *value, int count) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1) glUniform1iv(uniformLocation, count, value);
    IVW_ELSE_WARN
}

void Shader::setUniform(const std::string &name, const GLfloat *value, int count) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1) glUniform1fv(uniformLocation, count, value);
    IVW_ELSE_WARN
}

void Shader::setUniform(const std::string &name, const mat3 &value) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1)
        glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
    IVW_ELSE_WARN
}

void Shader::setUniform(const std::string &name, const mat4 &value) const {
    GLint uniformLocation = glGetUniformLocation(id_, name.c_str());

    if (uniformLocation != -1)
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
    IVW_ELSE_WARN
}

ShaderObject* Shader::getFragmentShaderObject() const {
    ShaderObjectMap::iterator it = shaderObjects_->find(GL_FRAGMENT_SHADER);
    if (it != shaderObjects_->end()) {
        return it->second;
    } else {
        return NULL;
    }
}

ShaderObject* Shader::getGeometryShaderObject() const {
    ShaderObjectMap::iterator it = shaderObjects_->find(GL_GEOMETRY_SHADER);
    if (it != shaderObjects_->end()) {
        return it->second;
    } else {
        return NULL;
    }
}

ShaderObject* Shader::getVertexShaderObject() const {
    ShaderObjectMap::iterator it = shaderObjects_->find(GL_VERTEX_SHADER);
    if (it != shaderObjects_->end()) {
        return it->second;
    } else {
        return NULL;
    }
}

} // namespace
