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

#include <modules/opengl/shader/shader.h>

#include <modules/opengl/shader/shadermanager.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/opengl/shader/shaderresource.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/shader/standardshaders.h>

namespace inviwo {

// glCreateProgram This function returns 0 if an error occurs creating the program object.
Shader::Program::Program() : id{glCreateProgram()} {}
Shader::Program::Program(const Program &) : Program() {}
Shader::Program::Program(Program &&rhs) noexcept : id{rhs.id} { rhs.id = 0; }
Shader::Program &Shader::Program::operator=(const Program &) {
    if (id == 0) {
        id = glCreateProgram();
    }
    return *this;
}
Shader::Program &Shader::Program::operator=(Program &&that) noexcept {
    Program copy(std::move(that));
    std::swap(id, copy.id);
    return *this;
}

Shader::Program::~Program() {
    if (id != 0) {
        glDeleteProgram(id);
    }
}

Shader::ShaderAttachment::ShaderAttachment() : shader_{nullptr}, obj_{}, callback_{} {}
Shader::ShaderAttachment::ShaderAttachment(Shader *shader, std::unique_ptr<ShaderObject> obj)
    : shader_{shader}
    , obj_{std::move(obj)}
    , callback_{obj_->onChange([s = shader_](ShaderObject *o) {
        if (s) s->rebuildShader(o);
    })} {
    attatch();
}
Shader::ShaderAttachment::ShaderAttachment(ShaderAttachment &&rhs) noexcept
    : shader_{rhs.shader_}, obj_{std::move(rhs.obj_)}, callback_{std::move(rhs.callback_)} {
    rhs.shader_ = nullptr;
}
Shader::ShaderAttachment &Shader::ShaderAttachment::operator=(ShaderAttachment &&that) noexcept {
    if (this != &that) {
        shader_ = that.shader_;
        obj_ = std::move(that.obj_);
        callback_ = std::move(that.callback_);
        that.shader_ = nullptr;
    }
    return *this;
}
Shader::ShaderAttachment::~ShaderAttachment() { detatch(); }

void Shader::ShaderAttachment::attatch() {
    if (obj_ && shader_) {
        glAttachShader(shader_->getID(), obj_->getID());
    }
}
void Shader::ShaderAttachment::detatch() {
    if (obj_ && shader_) {
        glDetachShader(shader_->getID(), obj_->getID());
    }
}

void Shader::ShaderAttachment::setShader(Shader *shader) {
    shader_ = shader;
    callback_ = obj_->onChange([s = shader_](ShaderObject *o) {
        if (s) s->rebuildShader(o);
    });
}

Shader::Shader(const std::vector<std::pair<ShaderType, std::string>> &items, Build buildShader)
    : warningLevel_{UniformWarning::Ignore} {

    for (auto &item : items) {
        shaderObjects_.emplace(
            item.first,
            ShaderAttachment(this, std::make_unique<ShaderObject>(
                                       item.first, utilgl::findShaderResource(item.second))));
    }

    if (buildShader == Build::Yes) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(
    const std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>> &items,
    Build buildShader)
    : warningLevel_{UniformWarning::Ignore} {

    for (auto &item : items) {
        shaderObjects_.emplace(item.first, ShaderAttachment(this, std::make_unique<ShaderObject>(
                                                                      item.first, item.second)));
    }
    if (buildShader == Build::Yes) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(std::vector<std::unique_ptr<ShaderObject>> &shaderObjects, bool buildShader)
    : warningLevel_{UniformWarning::Ignore} {

    for (auto &obj : shaderObjects) {
        auto type = obj->getShaderType();
        shaderObjects_.emplace(type, ShaderAttachment(this, std::move(obj)));
    }

    if (buildShader) build();
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
    : Shader({utilgl::imgIdentityVert(),
              {ShaderType::Fragment, utilgl::findShaderResource(fragmentFilename)}},
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
        shaderObjects_.emplace(
            elem.first, ShaderAttachment(this, std::make_unique<ShaderObject>(elem.second.obj())));
    }

    if (rhs.isReady()) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader::Shader(Shader &&rhs)
    : program_{std::move(rhs.program_)}
    , shaderObjects_{std::move(rhs.shaderObjects_)}
    , ready_(false)
    , warningLevel_{rhs.warningLevel_} {

    auto ready = rhs.ready_;

    ShaderManager::getPtr()->unregisterShader(&rhs);
    rhs.ready_ = false;

    for (auto &elem : shaderObjects_) elem.second.setShader(this);

    if (ready) build();
    ShaderManager::getPtr()->registerShader(this);
}

Shader &Shader::operator=(const Shader &that) {
    if (this != &that) {
        program_ = that.program_;

        shaderObjects_.clear();
        for (auto &elem : that.shaderObjects_) {
            shaderObjects_.emplace(
                elem.first,
                ShaderAttachment(this, std::make_unique<ShaderObject>(elem.second.obj())));
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
        auto ready = that.ready_;
        warningLevel_ = that.warningLevel_;
        that.ready_ = false;

        shaderObjects_ = std::move(that.shaderObjects_);
        for (auto &elem : shaderObjects_) elem.second.setShader(this);

        if (ready) build();
        ShaderManager::getPtr()->registerShader(this);
    }
    return *this;
}

Shader::~Shader() {
    if (ShaderManager::isInitialized() && ShaderManager::getPtr()->isRegistered(this)) {
        ShaderManager::getPtr()->unregisterShader(this);
    }
}

void Shader::build() {
    try {
        ready_ = false;
        for (auto &elem : shaderObjects_) elem.second.obj().build();
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

void Shader::linkShader(bool notifyRebuild) {
    uniformLookup_.clear();  // clear uniform location cache.
    bindAttributes();

    if (!util::all_of(shaderObjects_,
                      [](const auto &elem) { return elem.second.obj().isReady(); })) {
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

#ifdef IVW_DEBUG
    auto log = utilgl::getProgramInfoLog(program_.id);
    if (!log.empty()) {
        util::log(IVW_CONTEXT, "Id: " + toString(program_.id) + " " + processLog(log),
                  LogLevel::Info, LogAudience::User);
    }
#endif

    LGL_ERROR;
    ready_ = true;
    if (notifyRebuild) onReloadCallback_.invokeAll();
}

bool Shader::checkLinkStatus() const {
    GLint res;
    glGetProgramiv(program_.id, GL_LINK_STATUS, &res);
    return res == GL_TRUE;
}

void Shader::bindAttributes() {
    for (const auto &obj : getShaderObjects()) {
        for (const auto &item : obj.getInDeclarations()) {
            glBindAttribLocation(program_.id, item.location, item.name.c_str());
        }
        for (const auto &item : obj.getOutDeclarations()) {
            glBindFragDataLocation(program_.id, item.location, item.name.c_str());
        }
    }
}

void Shader::rebuildShader(ShaderObject *obj) {
    try {
        ready_ = false;
        obj->build();
        linkShader();

        onReloadCallback_.invokeAll();

        util::log(IVW_CONTEXT,
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

const BaseCallBack *Shader::onReload(std::function<void()> callback) {
    return onReloadCallback_.addLambdaCallback(callback);
}

std::shared_ptr<std::function<void()>> Shader::onReloadScoped(std::function<void()> callback) {
    return onReloadCallback_.addLambdaCallbackRaii(callback);
}

void Shader::removeOnReload(const BaseCallBack *callback) { onReloadCallback_.remove(callback); }

std::string Shader::shaderNames() const {
    std::vector<std::string> names;
    for (const auto &elem : shaderObjects_) {
        names.push_back(elem.second.obj().getFileName());
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
                                  IVW_CONTEXT);
        } else if (warningLevel_ == UniformWarning::Warn && location == -1) {
            util::log(IVW_CONTEXT,
                      "Unable to set uniform " + name + " in shader " +
                          " in shader id: " + toString(program_.id) + " " + shaderNames(),
                      LogLevel::Warn, LogAudience::User);
        }

        return location;
    }
}

ShaderObject *Shader::operator[](ShaderType type) const { return getShaderObject(type); }

ShaderObject *Shader::getShaderObject(ShaderType type) const {
    auto it = shaderObjects_.find(type);
    if (it != shaderObjects_.end()) {
        return &(it->second.obj());
    } else {
        return nullptr;
    }
}

ShaderObject *Shader::getFragmentShaderObject() const {
    return getShaderObject(ShaderType::Fragment);
}

ShaderObject *Shader::getGeometryShaderObject() const {
    return getShaderObject(ShaderType::Geometry);
}

ShaderObject *Shader::getVertexShaderObject() const { return getShaderObject(ShaderType::Vertex); }

const Shader::transform_t Shader::transform =
    [](std::unordered_map<ShaderType, ShaderAttachment>::value_type &item) -> ShaderObject & {
    return item.second.obj();
};

const Shader::const_transform_t Shader::const_transform =
    [](const std::unordered_map<ShaderType, ShaderAttachment>::value_type &item)
    -> const ShaderObject & { return item.second.obj(); };

auto Shader::begin() -> iterator { return iterator(transform, shaderObjects_.begin()); }
auto Shader::end() -> iterator { return iterator(transform, shaderObjects_.end()); }
auto Shader::begin() const -> const_iterator {
    return const_iterator(const_transform, shaderObjects_.begin());
}
auto Shader::end() const -> const_iterator {
    return const_iterator(const_transform, shaderObjects_.end());
}
auto Shader::getShaderObjects() -> util::iter_range<iterator> {
    return util::as_range(begin(), end());
}
auto Shader::getShaderObjects() const -> util::iter_range<const_iterator> {
    return util::as_range(begin(), end());
}

}  // namespace inviwo
