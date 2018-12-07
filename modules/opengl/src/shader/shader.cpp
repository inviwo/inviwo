/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#include <modules/opengl/shader/shadermanager.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/opengl/shader/shaderresource.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

Shader::Shader(const std::vector<std::pair<ShaderType, std::string>> &items, Build buildShader)
    : warningLevel_{UniformWarning::Ignore} {
    for (auto &item : items) createAndAddShader(item.first, item.second);
    verify();
    if (buildShader == Build::Yes) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(
    const std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>> &items,
    Build buildShader)
    : warningLevel_{UniformWarning::Ignore} {
    for (auto &item : items) createAndAddShader(item.first, item.second);
    verify();
    if (buildShader == Build::Yes) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(std::vector<std::unique_ptr<ShaderObject>> &shaderObjects, bool buildShader)
    : warningLevel_{UniformWarning::Ignore} {
    for (auto &shaderObject : shaderObjects) createAndAddShader(std::move(shaderObject));
    verify();
    if (buildShader) build();
    ShaderManager::getPtr()->registerShader(this);
}

void Shader::verify() const {
    if (shaderObjects_.count(ShaderType::Vertex) == 0) {
        throw Exception("Vertex shader required, provide for example img_identity.vert");
    }
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

Shader::Shader(const Shader &rhs) : program_{rhs.program_}, warningLevel_{rhs.warningLevel_} {
    for (auto &elem : rhs.shaderObjects_) {
        createAndAddShader(util::make_unique<ShaderObject>(*(elem.second.get())));
    }

    if (rhs.isReady()) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(Shader &&rhs)
    : program_{std::move(rhs.program_)}, ready_(rhs.ready_), warningLevel_{rhs.warningLevel_} {

    ShaderManager::getPtr()->unregisterShader(&rhs);
    rhs.objectCallbacks_.clear();
    rhs.ready_ = false;

    shaderObjects_ = std::move(rhs.shaderObjects_);

    for (auto &elem : shaderObjects_) {
        objectCallbacks_.push_back(
            elem.second->onChange([this](ShaderObject *o) { rebuildShader(o); }));
    }

    ShaderManager::getPtr()->registerShader(this);
}

Shader &Shader::operator=(const Shader &that) {
    if (this != &that) {
        program_ = that.program_;

        shaderObjects_.clear();
        for (auto &elem : that.shaderObjects_) {
            createAndAddShader(util::make_unique<ShaderObject>(*(elem.second.get())));
        }
        warningLevel_ = that.warningLevel_;

        if (that.isReady()) build();
        if (!ShaderManager::getPtr()->isRegistered(this)) {
            // Need to re-register if we have been moved from
            ShaderManager::getPtr()->registerShader(this);
        }
    }
    return *this;
}

Shader &Shader::operator=(Shader &&that) {
    if (this != &that) {
        if (ShaderManager::getPtr()->isRegistered(this)) {
            ShaderManager::getPtr()->unregisterShader(this);
        }

        program_ = std::move(that.program_);
        ShaderManager::getPtr()->unregisterShader(&that);
        ready_ = that.ready_;
        warningLevel_ = that.warningLevel_;
        that.ready_ = false;
        that.objectCallbacks_.clear();

        shaderObjects_ = std::move(that.shaderObjects_);

        objectCallbacks_.clear();
        for (auto &elem : shaderObjects_) {
            objectCallbacks_.push_back(
                elem.second->onChange([this](ShaderObject *o) { rebuildShader(o); }));
        }
        ShaderManager::getPtr()->registerShader(this);
    }
    return *this;
}

Shader::~Shader() {
    if (ShaderManager::getPtr()->isRegistered(this)) {
        ShaderManager::getPtr()->unregisterShader(this);
    }
}

void Shader::createAndAddShader(ShaderType type, std::string fileName) {
    createAndAddShader(type, utilgl::findShaderResource(fileName));
}

void Shader::createAndAddShader(ShaderType type, std::shared_ptr<const ShaderResource> resource) {
    createAndAddShader(std::make_unique<ShaderObject>(type, resource));
}
void Shader::createAndAddShader(std::unique_ptr<ShaderObject> object) {
    auto ptr =
        ShaderObjectPtr(object.release(), [shaderId = program_.id](ShaderObject *shaderObject) {
            if (shaderObject != nullptr) {
                glDetachShader(shaderId, shaderObject->getID());
                LGL_ERROR;
                delete shaderObject;
            }
        });

    objectCallbacks_.push_back(ptr->onChange([this](ShaderObject *o) { rebuildShader(o); }));
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
    ShaderManager::getPtr()->bindCommonAttributes(program_.id);

    if (!util::all_of(shaderObjects_, [](const ShaderObjectMap::value_type &elem) {
            return elem.second->isReady();
        })) {
        util::log(IvwContext, "Id: " + toString(program_.id) + " objects not ready when linking.",
                  LogLevel::Error, LogAudience::User);
        return;
    }

    glLinkProgram(program_.id);

    if (!isReady()) {
        throw OpenGLException("Id: " + toString(program_.id) + " " +
                                  processLog(utilgl::getProgramInfoLog(program_.id)),
                              IvwContext);
    }

#ifdef IVW_DEBUG
    auto log = utilgl::getProgramInfoLog(program_.id);
    if (!log.empty()) {
        util::log(IvwContext, "Id: " + toString(program_.id) + " " + processLog(log),
                  LogLevel::Info, LogAudience::User);
    }
#endif

    LGL_ERROR;
    ready_ = true;
    if (notifyRebuild) onReloadCallback_.invokeAll();
}

void Shader::rebuildShader(ShaderObject *obj) {
    try {
        ready_ = false;
        obj->build();
        linkShader();

        onReloadCallback_.invokeAll();

        util::log(IvwContext,
                  "Id: " + toString(program_.id) + ", resource: " + obj->getFileName() +
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
            result << "\n"
                   << fileName << " (" << lineNumber << "): " << line.substr(line.find(":") + 1);
        } else {
            result << "\n" << line;
        }
    }
    return result.str();
}

bool Shader::isReady() const {
    GLint res;
    glGetProgramiv(program_.id, GL_LINK_STATUS, &res);
    return res == GL_TRUE;
}

void Shader::activate() {
    if (!ready_)
        throw OpenGLException(
            "Shader Id: " + toString(program_.id) + " not ready: " + shaderNames(), IvwContext);
    glUseProgram(program_.id);
    LGL_ERROR;
}

void Shader::deactivate() {
    glUseProgram(0);
    LGL_ERROR;
}

void Shader::attachShaderObject(ShaderObject *shaderObject) {
    glAttachShader(program_.id, shaderObject->getID());
    LGL_ERROR;
}

void Shader::detachShaderObject(ShaderObject *shaderObject) {
    glDetachShader(program_.id, shaderObject->getID());
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
        GLint location = glGetUniformLocation(program_.id, name.c_str());
        uniformLookup_[name] = location;

        if (warningLevel_ == UniformWarning::Throw && location == -1) {
            throw OpenGLException("Unable to set uniform " + name + " in shader id: " +
                                      toString(program_.id) + " " + shaderNames(),
                                  IvwContext);
        } else if (warningLevel_ == UniformWarning::Warn && location == -1) {
            util::log(IvwContext,
                      "Unable to set uniform " + name + " in shader " +
                          " in shader id: " + toString(program_.id) + " " + shaderNames(),
                      LogLevel::Warn, LogAudience::User);
        }

        return location;
    }
}

ShaderObject *Shader::operator[](ShaderType type) const { return getShaderObject(type); }

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

}  // namespace inviwo
