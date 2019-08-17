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

#include <stdio.h>
#include <fstream>
#include <string>
#include <modules/opengl/shader/shaderobject.h>

#include <inviwo/core/io/textfilereader.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/opengl/openglexception.h>
#include <modules/opengl/shader/shadermanager.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglcapabilities.h>

#include <fmt/format.h>

namespace inviwo {

std::string ShaderObject::InDeclaration::toString() const {
    return fmt::format(decl, fmt::arg("type", type), fmt::arg("name", name),
                       fmt::arg("location", location));
}

std::string ShaderObject::OutDeclaration::toString() const {
    return fmt::format(decl, fmt::arg("type", type), fmt::arg("name", name),
                       fmt::arg("location", location));
}

ShaderObject::ShaderObject(ShaderType shaderType, std::shared_ptr<const ShaderResource> resource)
    : shaderType_{shaderType}, id_{glCreateShader(shaderType)}, resource_{resource} {
    if (!shaderType_) {
        glDeleteShader(id_);
        throw OpenGLException("Invalid shader type", IVW_CONTEXT);
    }

    // Help developer to spot errors
    std::string fileExtension = filesystem::getFileExtension(resource_->key());
    if (fileExtension != shaderType_.extension()) {
        LogWarn("File extension does not match shader type: "
                << resource_->key() << "\n    expected extension: " << shaderType_.extension());
    }

    if (shaderType_ == ShaderType::Fragment) {
        addStandardFragmentOutDeclarations();
    } else if (shaderType_ == ShaderType::Vertex) {
        addStandardVertexInDeclarations();
    }
}

ShaderObject::ShaderObject(std::shared_ptr<const ShaderResource> resource)
    : ShaderObject(ShaderType::get(filesystem::getFileExtension(resource->key())), resource) {}

ShaderObject::ShaderObject(ShaderType shaderType, std::string fileName)
    : ShaderObject(shaderType, loadResource(fileName)) {}

ShaderObject::ShaderObject(std::string fileName)
    : ShaderObject(ShaderType::get(filesystem::getFileExtension(fileName)),
                   loadResource(fileName)) {}

ShaderObject::ShaderObject(GLenum shaderType, std::string fileName)
    : ShaderObject(ShaderType(shaderType), loadResource(fileName)) {}

ShaderObject::ShaderObject(const ShaderObject& rhs)
    : shaderType_(rhs.shaderType_)
    , id_(glCreateShader(rhs.shaderType_))
    , resource_(rhs.resource_)
    , inDeclarations_(rhs.inDeclarations_)
    , outDeclarations_(rhs.outDeclarations_)
    , shaderDefines_(rhs.shaderDefines_)
    , shaderExtensions_(rhs.shaderExtensions_)
    , sourceProcessed_{}
    , includeResources_{}
    , lnr_{}
    , callbacks_{}
    , resourceCallbacks_{} {}

ShaderObject::ShaderObject(ShaderObject&& rhs) noexcept
    : shaderType_(rhs.shaderType_)
    , id_(rhs.id_)
    , resource_(std::move(rhs.resource_))
    , inDeclarations_(std::move(rhs.inDeclarations_))
    , outDeclarations_(std::move(rhs.outDeclarations_))
    , shaderDefines_(std::move(rhs.shaderDefines_))
    , shaderExtensions_(std::move(rhs.shaderExtensions_))
    , sourceProcessed_{}
    , includeResources_{}
    , lnr_{}
    , callbacks_{}
    , resourceCallbacks_{} {

    rhs.id_ = 0;
    rhs.resourceCallbacks_.clear();
}

ShaderObject& ShaderObject::operator=(const ShaderObject& that) {
    ShaderObject copy(that);
    std::swap(shaderType_, copy.shaderType_);
    std::swap(id_, copy.id_);
    std::swap(resource_, copy.resource_);
    std::swap(inDeclarations_, copy.inDeclarations_);
    std::swap(outDeclarations_, copy.outDeclarations_);
    std::swap(shaderDefines_, copy.shaderDefines_);
    std::swap(shaderExtensions_, copy.shaderExtensions_);
    std::swap(sourceProcessed_, copy.sourceProcessed_);
    std::swap(includeResources_, copy.includeResources_);
    std::swap(lnr_, copy.lnr_);
    std::swap(callbacks_, copy.callbacks_);
    std::swap(resourceCallbacks_, copy.resourceCallbacks_);
    return *this;
}

ShaderObject& ShaderObject::operator=(ShaderObject&& that) noexcept {
    ShaderObject copy(std::move(that));
    std::swap(shaderType_, copy.shaderType_);
    std::swap(id_, copy.id_);
    std::swap(resource_, copy.resource_);
    std::swap(inDeclarations_, copy.inDeclarations_);
    std::swap(outDeclarations_, copy.outDeclarations_);
    std::swap(shaderDefines_, copy.shaderDefines_);
    std::swap(shaderExtensions_, copy.shaderExtensions_);
    std::swap(sourceProcessed_, copy.sourceProcessed_);
    std::swap(includeResources_, copy.includeResources_);
    std::swap(lnr_, copy.lnr_);
    std::swap(callbacks_, copy.callbacks_);
    std::swap(resourceCallbacks_, copy.resourceCallbacks_);
    return *this;
}

ShaderObject::~ShaderObject() { glDeleteShader(id_); }

GLuint ShaderObject::getID() const { return id_; }

std::string ShaderObject::getFileName() const { return resource_->key(); }

std::shared_ptr<const ShaderResource> ShaderObject::getResource() const { return resource_; }

const std::vector<std::shared_ptr<const ShaderResource>>& ShaderObject::getResources() const {
    return includeResources_;
}

ShaderType ShaderObject::getShaderType() const { return shaderType_; }

std::shared_ptr<const ShaderResource> ShaderObject::loadResource(std::string fileName) {
    return utilgl::findShaderResource(fileName);
}

void ShaderObject::build() {
    preprocess();
    upload();
    compile();
}

void ShaderObject::preprocess() {
    resourceCallbacks_.clear();
    lnr_.clear();
    auto holdOntoResources = includeResources_;  // Don't release until we have processed again.
    includeResources_.clear();

    std::ostringstream source;
    addDefines(source);
    addIncludes(source, resource_);
    sourceProcessed_ = source.str();
}

void ShaderObject::addDefines(std::ostringstream& source) {
    auto capa = ShaderManager::getPtr()->getOpenGLCapabilities();
    const auto current = capa->getCurrentShaderVersion();

    source << "#version " << current.getVersionAndProfileAsString() << "\n";
    lnr_.addLine("Version", 0);

    for (const auto& se : shaderExtensions_) {
        source << "#extension " << se.first << " : " << (se.second ? "enable" : "disable") << "\n";
        lnr_.addLine("Extensions", 0);
    }

    if (current.hasProfile()) {
        source << "#define GLSL_PROFILE_" + toUpper(current.getProfile()) + "\n";
        lnr_.addLine("Header", 0);
    }

    int lastVersion = -1;
    for (size_t i = capa->getCurrentShaderIndex(); i < capa->getNumberOfShaderVersions(); i++) {
        const auto version = capa->getShaderVersion(i);
        if (lastVersion != version.getVersion()) {
            source << "#define GLSL_VERSION_" << version.getVersionAsString() << "\n";
            lnr_.addLine("Header", 0);
            lastVersion = version.getVersion();
        }
    }

    if (capa->getMaxProgramLoopCount() > 0) {
        source << "#define MAX_PROGRAM_LOOP_COUNT " << capa->getMaxProgramLoopCount() << "\n";
        lnr_.addLine("Header", 0);
    }

    for (const auto& sd : shaderDefines_) {
        source << "#define " << sd.first << " " << sd.second << "\n";
        lnr_.addLine("Defines", 0);
    }

    for (auto decl : outDeclarations_) {
        source << decl.toString() << "\n";
        lnr_.addLine("Out Declaration", 0);
    }
    for (auto decl : inDeclarations_) {
        source << decl.toString() << "\n";
        lnr_.addLine("In Declaration", 0);
    }
}

void ShaderObject::addStandardFragmentOutDeclarations() {
    addOutDeclaration("FragData0", 0);
    addOutDeclaration("PickingData", 1);
}

void ShaderObject::addStandardVertexInDeclarations() {
    addInDeclaration(InDeclaration{"in_Vertex", 0});
    addInDeclaration(InDeclaration{"in_Normal", 1, "vec3"});
    addInDeclaration(InDeclaration{"in_Color", 2});
    addInDeclaration(InDeclaration{"in_TexCoord", 3, "vec3"});
}

void ShaderObject::addIncludes(std::ostringstream& source,
                               std::shared_ptr<const ShaderResource> resource) {
    std::ostringstream result;
    std::string curLine;
    includeResources_.push_back(resource);
    resourceCallbacks_.push_back(
        resource->onChange([this](const ShaderResource*) { callbacks_.invoke(this); }));
    std::istringstream shaderSource(resource->source());
    int localLineNumber = 1;
    bool isInsideBlockComment = false;
    while (std::getline(shaderSource, curLine)) {
        size_t curPos = 0;
        bool hasAddedInclude = false;
        while (curPos != std::string::npos) {
            if (isInsideBlockComment) {
                // If we currently are inside a block comment, we only need to look for where it
                // ends
                curPos = curLine.find("*/", curPos);
                isInsideBlockComment = curPos == std::string::npos;
                if (!isInsideBlockComment) {
                    curPos += 2;  // move curPos to the first character after the comment
                }

            } else {

                auto posInclude = curLine.find("#include", curPos);
                auto posLineComment = curLine.find("//", curPos);
                auto posBlockCommentStart = curLine.find("/*", curPos);

                // If we find two includes on the same line
                if (hasAddedInclude && posInclude != std::string::npos) {
                    std::ostringstream oss;
                    oss << "Found more than one include on line " << localLineNumber
                        << " in resource " << resource->key();
                    throw OpenGLException(oss.str(), IVW_CONTEXT);
                }

                // ignore everything after a line-comment "//" (unless it is inside a block comment)
                if (posLineComment != std::string::npos && posLineComment < posInclude &&
                    posLineComment < posBlockCommentStart) {
                    break;
                }

                // there is a block comment starting on this line, before a include: update curPos
                // and continue;
                // the include should be found in the next iteration
                if (posBlockCommentStart != std::string::npos &&
                    posBlockCommentStart < posInclude) {
                    isInsideBlockComment = true;
                    curPos = posBlockCommentStart;
                    continue;
                }

                // an include was found
                if (posInclude != std::string::npos) {
                    auto pathBegin = curLine.find("\"", posInclude + 1);
                    auto pathEnd = curLine.find("\"", pathBegin + 1);
                    std::string incfile(curLine, pathBegin + 1, pathEnd - pathBegin - 1);
                    auto inc = ShaderManager::getPtr()->getShaderResource(incfile);
                    if (!inc) {
                        throw OpenGLException(
                            "Include file " + incfile + " not found in shader search paths.",
                            IVW_CONTEXT);
                    }
                    auto it = util::find(includeResources_, inc);
                    if (it == includeResources_.end()) {  // Only include files once.
                        addIncludes(source, inc);
                        source << curLine.substr(pathEnd + 1);
                    }
                    hasAddedInclude = true;
                }

                // after the include it can still be comments, we need to detect if a block comment
                // starts

                // if the next thing is a line comment, we continue to next line
                if (posLineComment != std::string::npos && posLineComment < posBlockCommentStart) {
                    // there is a line-comment after the include and before any block comment, then
                    // go to next line
                    break;
                }

                // set curPos to either npos or the pos of the start of the next block comment
                curPos = posBlockCommentStart;  // will be npos of it has not been found
                // updated the flag to tell if we are in a block comment or not
                isInsideBlockComment = posBlockCommentStart != std::string::npos;
            }
        }

        if (!hasAddedInclude) {  // include the whole line
            source << curLine << "\n";
            lnr_.addLine(resource->key(), localLineNumber);
        }
        localLineNumber++;
    }
}

void ShaderObject::upload() {
    const char* source = sourceProcessed_.c_str();
    glShaderSource(id_, 1, &source, nullptr);
    LGL_ERROR;
}

bool ShaderObject::isReady() const {
    GLint res = GL_FALSE;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &res);
    return res == GL_TRUE;
}

void ShaderObject::compile() {
    glCompileShader(id_);
    if (!isReady()) {
        throw OpenGLException(
            resource_->key() + " " + lnr_.resolveLog(utilgl::getShaderInfoLog(id_)), IVW_CONTEXT);
    }

#ifdef IVW_DEBUG
    const auto log = utilgl::getShaderInfoLog(id_);
    if (!log.empty()) {
        util::log(IVW_CONTEXT, resource_->key() + " " + lnr_.resolveLog(log), LogLevel::Info,
                  LogAudience::User);
    }
#endif
}

void ShaderObject::addShaderDefine(const std::string& name, const std::string& value) {
    shaderDefines_[name] = value;
}
void ShaderObject::setShaderDefine(const std::string& name, bool exists, const std::string& value) {
    if (exists) {
        addShaderDefine(name, value);
    } else {
        removeShaderDefine(name);
    }
}

void ShaderObject::removeShaderDefine(const std::string& name) { shaderDefines_.erase(name); }

bool ShaderObject::hasShaderDefine(const std::string& name) const {
    return shaderDefines_.find(name) != shaderDefines_.end();
}

void ShaderObject::clearShaderDefines() { shaderDefines_.clear(); }

void ShaderObject::addShaderExtension(const std::string& extName, bool enabled) {
    shaderExtensions_[extName] = enabled;
}

void ShaderObject::removeShaderExtension(const std::string& extName) {
    shaderExtensions_.erase(extName);
}

bool ShaderObject::hasShaderExtension(const std::string& extName) const {
    return shaderExtensions_.find(extName) != shaderExtensions_.end();
}

void ShaderObject::clearShaderExtensions() { shaderExtensions_.clear(); }

void ShaderObject::addOutDeclaration(const std::string& name, int location,
                                     const std::string& type) {
    addOutDeclaration(OutDeclaration{name, location, type});
}

void ShaderObject::addOutDeclaration(const OutDeclaration& decl) {
    auto it = util::find_if(outDeclarations_,
                            [&](const OutDeclaration& elem) { return elem.name == decl.name; });
    if (it != outDeclarations_.end()) {
        *it = decl;
    } else {
        outDeclarations_.push_back(decl);
    }
}

auto ShaderObject::getOutDeclarations() const -> const std::vector<OutDeclaration>& {
    return outDeclarations_;
}

void ShaderObject::clearOutDeclarations() { outDeclarations_.clear(); }

void ShaderObject::ShaderObject::addInDeclaration(const std::string& name, int location,
                                                  const std::string& type) {
    addInDeclaration(InDeclaration{name, location, type});
}
void ShaderObject::addInDeclaration(const InDeclaration& decl) {
    auto it = util::find_if(inDeclarations_,
                            [&](const InDeclaration& elem) { return elem.name == decl.name; });
    if (it != inDeclarations_.end()) {
        *it = decl;
    } else {
        inDeclarations_.push_back(decl);
    }
}
void ShaderObject::clearInDeclarations() { inDeclarations_.clear(); }
auto ShaderObject::getInDeclarations() const -> const std::vector<InDeclaration>& {
    return inDeclarations_;
}

std::pair<std::string, size_t> ShaderObject::LineNumberResolver::resolveLine(size_t line) const {
    if (line < lines_.size()) {
        return lines_[line];
    } else {
        return {"", 0};
    }
}

void ShaderObject::LineNumberResolver::addLine(const std::string& file, size_t line) {
    lines_.emplace_back(file, line);
}

void ShaderObject::LineNumberResolver::clear() { lines_.clear(); }

std::string ShaderObject::LineNumberResolver::resolveLog(const std::string& compileLog) const {
    std::ostringstream result;
    std::istringstream origShaderInfoLog(compileLog);

    std::string curLine;
    while (std::getline(origShaderInfoLog, curLine)) {
        if (!curLine.empty()) {
            const int origLineNumber = utilgl::getLogLineNumber(curLine);
            if (origLineNumber > 0) {
                const auto res = resolveLine(origLineNumber);
                result << "\n"
                       << res.first << " (" << res.second
                       << "): " << curLine.substr(curLine.find(":") + 1);
            } else {
                result << "\n" << curLine;
            }
        }
    }

    return std::move(result).str();
}

std::pair<std::string, size_t> ShaderObject::resolveLine(size_t line) const {
    return lnr_.resolveLine(line);
}

std::string ShaderObject::print(bool showSource, bool preprocess) const {
    if (preprocess) {
        if (showSource) {
            std::string::size_type width = 0;
            for (auto l : lnr_) {
                std::string file = splitString(l.first, '/').back();
                width = std::max(width, file.length());
            }

            size_t i = 0;
            std::stringstream out;
            std::istringstream in(sourceProcessed_);

            std::string line;
            while (std::getline(in, line)) {
                const auto res = lnr_.resolveLine(i);
                const std::string file =
                    res.first.empty() ? "" : splitString(res.first, '/').back();
                const size_t lineNumber = res.second;

                out << std::left << std::setw(width + 1u) << file << std::right << std::setw(4)
                    << lineNumber << ": " << std::left << line << "\n";
                ++i;
            }
            return std::move(out).str();
        } else {
            return sourceProcessed_;
        }
    } else {
        if (showSource) {
            size_t lineNumber = 1;
            std::string line;
            std::stringstream out;
            std::istringstream in(resource_->source());
            while (std::getline(in, line)) {
                std::string file = resource_->key();

                out << std::left << std::setw(file.length() + 1u) << file << std::right
                    << std::setw(4) << lineNumber << ": " << std::left << line << "\n";
                ++lineNumber;
            }
            return out.str();
        } else {
            return resource_->source();
        }
    }
}

}  // namespace inviwo
