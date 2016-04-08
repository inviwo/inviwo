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

#include <stdio.h>
#include <fstream>
#include <string>
#include "shaderobject.h"

#include <inviwo/core/io/textfilereader.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/opengl/openglexception.h>
#include <modules/opengl/shader/shadermanager.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

ShaderObject::ShaderObject(ShaderType shaderType, std::shared_ptr<const ShaderResource> resource)
    : shaderType_{shaderType}, resource_{resource}, id_{glCreateShader(shaderType)} {
    if (!shaderType_) throw OpenGLException("Invalid shader type", IvwContext);
 
    // Help developer to spot errors
    std::string fileExtension = filesystem::getFileExtension(resource_->key());
    if (fileExtension != shaderType_.extension()) {
        LogWarn("File extension does not match shader type: " << resource_->key());
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
    , resource_(rhs.resource_)
    , id_(glCreateShader(rhs.shaderType_))
    , outDeclarations_(rhs.outDeclarations_)
    , shaderDefines_(rhs.shaderDefines_)
    , shaderExtensions_(rhs.shaderExtensions_) {
}

ShaderObject& ShaderObject::operator=(const ShaderObject& that) {
    if (this != &that) {
        glDeleteShader(id_);
        
        shaderType_ = that.shaderType_;
        resource_ = that.resource_;
        id_ = glCreateShader(shaderType_);
        outDeclarations_ = that.outDeclarations_;
        shaderDefines_ = that.shaderDefines_;
        shaderExtensions_ = that.shaderExtensions_;
    }
    return *this;
}

ShaderObject::~ShaderObject() { glDeleteShader(id_); }

GLuint ShaderObject::getID() const {
    return id_;
}

std::string ShaderObject::getFileName() const {
    return resource_->key();
}

std::shared_ptr<const ShaderResource> ShaderObject::getResource() const {
    return resource_;
}

const std::vector<std::shared_ptr<const ShaderResource>>& ShaderObject::getResources() const {
    return includeResources_;
}

ShaderType ShaderObject::getShaderType() const {
    return shaderType_;
}

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
    lineNumberResolver_.clear();
    auto holdOntoResources = includeResources_; // Don't release until we have processed again.
    includeResources_.clear();

    std::ostringstream source;
    addDefines(source);
    addOutDeclarations(source);
    addIncludes(source, resource_);
    sourceProcessed_ = source.str();
}

void ShaderObject::addDefines(std::ostringstream& source) {
    {
        std::string globalGLSLHeader = ShaderManager::getPtr()->getGlobalGLSLHeader();
        std::string curLine;
        std::istringstream globalGLSLHeaderStream(globalGLSLHeader);
        while (std::getline(globalGLSLHeaderStream, curLine)) {
            lineNumberResolver_.emplace_back("GlobalGLSLSHeader", 0);
        }
        source << globalGLSLHeader;
    }

    {
        for (const auto& se : shaderExtensions_) {
            source << "#extension " << se.first << " : " << (se.second ? "enable" : "disable")
                   << "\n";
            lineNumberResolver_.emplace_back("Extension", 0);
        }
    }
    {
        for (const auto& sd : shaderDefines_) {
            source << "#define " << sd.first << " " << sd.second << "\n";
            lineNumberResolver_.emplace_back("Defines", 0);
        }
    }

    {
        std::string globalDefines;
        if (shaderType_ == ShaderType::Vertex) {
            globalDefines += ShaderManager::getPtr()->getGlobalGLSLVertexDefines();
        } else if (shaderType_ == ShaderType::Fragment) {
            globalDefines += ShaderManager::getPtr()->getGlobalGLSLFragmentDefines();
        }

        std::string curLine;
        std::istringstream globalGLSLDefinesStream(globalDefines);
        while (std::getline(globalGLSLDefinesStream, curLine)) {
            lineNumberResolver_.emplace_back("GlobalGLSLSDefines", 0);
        }
        source << globalDefines;
    }
}

void ShaderObject::addOutDeclarations(std::ostringstream& source) {
    for (auto curDeclaration : outDeclarations_) {
        if (curDeclaration.second > -1) {
            source << "layout(location = " << curDeclaration.second << ") ";
        }
        source << "out vec4 " << curDeclaration.first << ";\n";
        lineNumberResolver_.emplace_back("Out Declaration", 0);
    }
}

void ShaderObject::addIncludes(std::ostringstream& source, std::shared_ptr<const ShaderResource> resource) {
    std::ostringstream result;
    std::string curLine;
    includeResources_.push_back(resource);
    resourceCallbacks_.push_back(
        resource->onChange([this](const ShaderResource* res) { callbacks_.invoke(this); }));

    std::istringstream shaderSource(resource->source());
    int localLineNumber = 1;

    while (std::getline(shaderSource, curLine)) {
        auto posInclude = curLine.find("#include");
        auto posComment = curLine.find("//");

        if (posInclude != std::string::npos &&
            (posComment == std::string::npos || posComment > posInclude)) {
            auto pathBegin = curLine.find("\"", posInclude + 1);
            auto pathEnd = curLine.find("\"", pathBegin + 1);
            std::string incfile(curLine, pathBegin + 1, pathEnd - pathBegin - 1);

            auto inc = ShaderManager::getPtr()->getShaderResource(incfile);
            if (!inc) {
                throw OpenGLException(
                    "Include file " + incfile + " not found in shader search paths.", IvwContext);
            }
            auto it = util::find(includeResources_, inc);
            if (it == includeResources_.end()) {  // Only include files once.
                addIncludes(source, inc);
            }

        } else {
            source << curLine << "\n";
            lineNumberResolver_.emplace_back(resource->key(), localLineNumber);
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
            resource_->key() + " " +
                utilgl::reformatInfoLog(lineNumberResolver_, utilgl::getShaderInfoLog(id_)),
            IvwContext);
    }

#ifdef IVW_DEBUG
    auto log = utilgl::getShaderInfoLog(id_);
    if (!log.empty()) {
        util::log(IvwContext,
                  resource_->key() + " " + utilgl::reformatInfoLog(lineNumberResolver_, log),
                  LogLevel::Info, LogAudience::User);
    }
#endif
}

void ShaderObject::addShaderDefine(std::string name, std::string value) {
    shaderDefines_[name] = value;
}

void ShaderObject::removeShaderDefine(std::string name) { shaderDefines_.erase(name); }

bool ShaderObject::hasShaderDefine(const std::string& name) const {
    return shaderDefines_.find(name) != shaderDefines_.end();
}

void ShaderObject::clearShaderDefines() { shaderDefines_.clear(); }

void ShaderObject::addShaderExtension(std::string extName, bool enabled) {
    shaderExtensions_[extName] = enabled;
}

void ShaderObject::removeShaderExtension(std::string extName) { shaderExtensions_.erase(extName); }

bool ShaderObject::hasShaderExtension(const std::string& extName) const {
    return shaderExtensions_.find(extName) != shaderExtensions_.end();
}

void ShaderObject::clearShaderExtensions() { shaderExtensions_.clear(); }

void ShaderObject::addOutDeclaration(std::string name, int location) {
    auto it = util::find_if(outDeclarations_,
                            [&](std::pair<std::string, int>& elem) { return elem.first == name; });
    if (it != outDeclarations_.end()) {
        it->second = location;
    } else {
        outDeclarations_.push_back({name, location});
    }
}

void ShaderObject::clearOutDeclarations() { outDeclarations_.clear(); }

std::pair<std::string, unsigned int> ShaderObject::resolveLine(size_t line) const {
    if (line<lineNumberResolver_.size())
        return lineNumberResolver_[line];
    else 
        return {"",0};
}

std::string ShaderObject::print(bool showSource, bool preprocess) const {
    if (preprocess) {
        if (showSource) {
            std::string::size_type width = 0;
            for (auto l : lineNumberResolver_) {
                std::string file = splitString(l.first, '/').back();
                width = std::max(width, file.length());
            }

            size_t i = 0;
            std::string line;
            std::stringstream out;
            std::istringstream in(sourceProcessed_);
            while (std::getline(in, line)) {
                std::string file = i < lineNumberResolver_.size()
                                       ? splitString(lineNumberResolver_[i].first, '/').back()
                                       : "";
                unsigned int lineNumber =
                    i < lineNumberResolver_.size() ? lineNumberResolver_[i].second : 0;

                out << std::left << std::setw(width + 1) << file << std::right << std::setw(4)
                    << lineNumber << ": " << std::left << line << "\n";
                ++i;
            }
            return out.str();
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

                out << std::left << std::setw(file.length() + 1) << file << std::right
                    << std::setw(4) << lineNumber << ": " << std::left << line << "\n";
                ++lineNumber;
            }
            return out.str();
        } else {
            return resource_->source();
        }
    }
}

}  // namespace
