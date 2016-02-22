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
#include <modules/opengl/shader/shaderresource.h>

namespace inviwo {

ShaderObject::ShaderObject(GLenum shaderType, std::string fileName, Compile compile, Error error)
    : shaderType_(shaderType), fileName_(fileName), id_(glCreateShader(shaderType)), error_(error) {
    initialize(compile);
}

ShaderObject::ShaderObject(GLenum shaderType, std::string fileName, bool compileShader)
    : ShaderObject(shaderType, fileName, compileShader ? Compile::Yes : Compile::No, Error::Warn) {}

ShaderObject::ShaderObject(const ShaderObject& rhs)
    : shaderType_(rhs.shaderType_)
    , fileName_(rhs.fileName_)
    , id_(glCreateShader(rhs.shaderType_))
    , outDeclarations_(rhs.outDeclarations_)
    , shaderDefines_(rhs.shaderDefines_)
    , shaderExtensions_(rhs.shaderExtensions_)
    , error_(rhs.error_) {
    
    initialize(rhs.isReady() ? Compile::Yes : Compile::No);
}

ShaderObject& ShaderObject::operator=(const ShaderObject& that) {
    if (this != &that) {
        glDeleteShader(id_);
        
        shaderType_ = that.shaderType_;
        fileName_ = that.fileName_;
        id_ = glCreateShader(shaderType_);
        outDeclarations_ = that.outDeclarations_;
        shaderDefines_ = that.shaderDefines_;
        shaderExtensions_ = that.shaderExtensions_;
        error_ = that.error_;
        
        initialize(that.isReady() ? Compile::Yes : Compile::No);
    }
    return *this;
}

ShaderObject::~ShaderObject() { glDeleteShader(id_); }

void ShaderObject::initialize(Compile shouldCompile) {
    // Help developer to spot errors
    std::string fileExtension = filesystem::getFileExtension(fileName_);
    if ((fileExtension == "vert" && shaderType_ != GL_VERTEX_SHADER) ||
        (fileExtension == "geom" && shaderType_ != GL_GEOMETRY_SHADER) ||
        (fileExtension == "frag" && shaderType_ != GL_FRAGMENT_SHADER)) {
        LogWarn("File extension does not match shader type: " << fileName_);
    }

    loadSource(fileName_);
    preprocess();
    upload();
    if (shouldCompile == Compile::Yes) compile();
}

void ShaderObject::build() {
    preprocess();
    upload();
    compile();
}

void ShaderObject::rebuild() {
    loadSource(fileName_);
    build();
}

void ShaderObject::preprocess() {
    lineNumberResolver_.clear();
    includeResources_.clear();
    sourceProcessed_ = getDefines() + getOutDeclarations() + getIncludes(resource_);
}

std::string ShaderObject::getDefines() {
    std::string globalGLSLHeader = ShaderManager::getPtr()->getGlobalGLSLHeader();

    std::string curLine;
    std::istringstream globalGLSLHeaderStream(globalGLSLHeader);
    while (std::getline(globalGLSLHeaderStream, curLine)) {
        lineNumberResolver_.emplace_back("GlobalGLSLSHeader", 0);
    }

    std::ostringstream extensions;
    for (const auto& se : shaderExtensions_) {
        extensions << "#extension " << se.first << " : " << (se.second ? "enable" : "disable")
                   << "\n";
        lineNumberResolver_.emplace_back("Extension", 0);
    }

    std::ostringstream defines;
    for (const auto& sd : shaderDefines_) {
        defines << "#define " << sd.first << " " << sd.second << "\n";
        lineNumberResolver_.emplace_back("Defines", 0);
    }

    std::string globalDefines;
    if (shaderType_ == GL_VERTEX_SHADER) {
        globalDefines += ShaderManager::getPtr()->getGlobalGLSLVertexDefines();
    } else if (shaderType_ == GL_FRAGMENT_SHADER) {
        globalDefines += ShaderManager::getPtr()->getGlobalGLSLFragmentDefines();
    }

    std::istringstream globalGLSLDefinesStream(globalDefines);
    while (std::getline(globalGLSLDefinesStream, curLine)) {
        lineNumberResolver_.emplace_back("GlobalGLSLSDefines", 0);
    }

    return globalGLSLHeader + extensions.str() + defines.str() + globalDefines;
}

std::string ShaderObject::getOutDeclarations() {
    std::ostringstream result;

    for (auto curDeclaration : outDeclarations_) {
        if (curDeclaration.second > -1) {
            result << "layout(location = " << curDeclaration.second << ") ";
        }
        result << "out vec4 " << curDeclaration.first << ";\n";
        lineNumberResolver_.emplace_back("Out Declaration", 0);
    }

    return result.str();
}

std::string ShaderObject::getIncludes(ShaderResource* resource) {
    std::ostringstream result;
    std::string curLine;
    includeResources_.push_back(resource);
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
                    "Include file " + incfile + " not found in shader search paths.",
                    IvwContext);
            }
            auto it = util::find(includeResources_, inc);
            if (it == includeResources_.end()) { // Only include files once.
                result << getIncludes(inc);
            }
            
        } else {
            result << curLine << "\n";
            lineNumberResolver_.emplace_back(resource->key(), localLineNumber);
        }
        localLineNumber++;
    }

    return result.str();
}

void ShaderObject::loadSource(std::string fileName) {
    resource_ = ShaderManager::getPtr()->getShaderResource(fileName);
    if (!resource_) {
        throw OpenGLException(
            "Shader file: " + fileName + " not found in shader search paths or shader resources.",
            IvwContext);
    }
}

void ShaderObject::upload() {
    const char* source = sourceProcessed_.c_str();
    glShaderSource(id_, 1, &source, nullptr);
    LGL_ERROR;
}

std::string ShaderObject::getShaderInfoLog() const {
    GLint maxLogLength;
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &maxLogLength);
    LGL_ERROR;

    if (maxLogLength > 1) {
        auto shaderInfoLog = util::make_unique<GLchar[]>(maxLogLength);
        GLsizei logLength{0};
        glGetShaderInfoLog(id_, maxLogLength, &logLength, shaderInfoLog.get());
        return std::string(shaderInfoLog.get(), logLength);
    } else {
        return "";
    }
}

int ShaderObject::getLogLineNumber(const std::string& compileLogLine) {
    int result = -1;
    std::istringstream input(compileLogLine);
    int num;

    if (input >> num) {
        char c;

        if (input >> c && c == '(') {
            if (input >> result) {
                return result;
            }
        }
    }

    // ATI parsing:
    // ATI error: "ERROR: 0:145: Call to undeclared function 'texelFetch'\n"
    std::vector<std::string> elems;
    std::stringstream ss(compileLogLine);
    std::string item;
    while (std::getline(ss, item, ':')) {
        elems.push_back(item);
    }
    if (elems.size() >= 3) {
        std::stringstream number;
        number << elems[2];
        number >> result;
    }

    return result;
}

std::string ShaderObject::reformatShaderInfoLog(const std::string shaderInfoLog) {
    std::ostringstream result;
    std::string curLine;
    std::istringstream origShaderInfoLog(shaderInfoLog);

    while (std::getline(origShaderInfoLog, curLine)) {
        if (!curLine.empty()) {
            int origLineNumber = getLogLineNumber(curLine);
            if (origLineNumber > 0) {
                unsigned int lineNumber = lineNumberResolver_[origLineNumber - 1].second;
                std::string fileName = lineNumberResolver_[origLineNumber - 1].first;
                result << "\n" << fileName << " (" << lineNumber
                       << "): " << curLine.substr(curLine.find(":") + 1);
            }
        }
    }

    return result.str();
}

bool ShaderObject::isReady() const {
    GLint res;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &res);
    return res == GL_TRUE;
}

void ShaderObject::compile() {
    glCompileShader(id_);

    if (!isReady()) {
        std::string compilerLog = reformatShaderInfoLog(getShaderInfoLog());
        switch (error_) {
            case Error::Warn:
                LogError("ShaderObject compile error: " << compilerLog);
                break;
            case Error::Throw:
                throw OpenGLException(compilerLog, IvwContext);
        }
    }
}

void ShaderObject::setError(Error error) { error_ = error; }
ShaderObject::Error ShaderObject::getError() const { return error_; }

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
    bool outExists = false;

    for (auto& elem : outDeclarations_) {
        if (elem.first == name) {
            // updating location
            elem.second = location;
            outExists = true;
            break;
        }
    }
    if (!outExists) {
        outDeclarations_.push_back({name, location});
    }
}

void ShaderObject::clearOutDeclarations() { outDeclarations_.clear(); }

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
            size_t lineNumber = 0;
            std::string line;
            std::stringstream out;
            std::istringstream in(resource_->source());
            while (std::getline(in, line)) {
                std::string file = fileName_;

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
