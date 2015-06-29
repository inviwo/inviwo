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

#include "shaderobject.h"
#include <stdio.h>
#include <fstream>
#include <string>

#include <inviwo/core/io/textfilereader.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/opengl/glwrap/shadermanager.h>
#include <modules/opengl/openglexception.h>

namespace inviwo {

ShaderObject::ShaderObject(GLenum shaderType, std::string fileName, bool compileShader)
    : shaderType_(shaderType)
    , fileName_(fileName)
    , id_(glCreateShader(shaderType)) {

    initialize(compileShader);
}

ShaderObject::ShaderObject(const ShaderObject& rhs, bool compileShader)
    : shaderType_(rhs.shaderType_), fileName_(rhs.fileName_), id_(glCreateShader(rhs.shaderType_)) {
    initialize(compileShader);
}

ShaderObject& ShaderObject::operator=(const ShaderObject& that) {
    if (this != &that) {
        glDeleteShader(id_);
        shaderType_ = that.shaderType_;
        fileName_ = that.fileName_;
        id_ = glCreateShader(shaderType_);
        initialize(true);
    }
    return *this;
}


ShaderObject::~ShaderObject() {
    glDeleteShader(id_);
}

ShaderObject* ShaderObject::clone(bool compileShader) {
    return new ShaderObject(*this, compileShader);
}

void ShaderObject::initialize(bool compileShader) {
    // Help developer to spot errors
    std::string fileExtension = filesystem::getFileExtension(fileName_);
    if ((fileExtension == "vert" && shaderType_ != GL_VERTEX_SHADER)
        || (fileExtension == "geom" && shaderType_ != GL_GEOMETRY_SHADER)
        || (fileExtension == "frag" && shaderType_ != GL_FRAGMENT_SHADER)) {
        LogWarn("File extension does not match shader type: " << fileName_);
    }
    
    loadSource(fileName_);
    preprocess();
    upload();

    if (compileShader) {
        compile();
    }
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
    includeFileNames_.clear();
    std::string shaderHeader = embeddDefines(source_);
    shaderHeader += embeddOutDeclarations(source_);
    sourceProcessed_ = shaderHeader + embeddIncludes(source_, fileName_);
}

std::string ShaderObject::embeddDefines(std::string source) {
    std::string globalGLSLHeader = ShaderManager::getPtr()->getGlobalGLSLHeader();
    
    std::string curLine;
    std::istringstream globalGLSLHeaderStream(globalGLSLHeader);
    while (std::getline(globalGLSLHeaderStream, curLine)) {
        lineNumberResolver_.emplace_back("GlobalGLSLSHeader", 0);
    }
    
    std::ostringstream extensions;
    for(const auto& se : shaderExtensions_) {
        extensions << "#extension " << se.first << " : " << (se.second ? "enable" : "disable") << "\n";
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

std::string ShaderObject::embeddOutDeclarations(std::string source) {
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

std::string ShaderObject::embeddIncludes(std::string source, std::string fileName) {
    std::ostringstream result;
    std::string curLine;
    std::istringstream shaderSource(source);
    int localLineNumber = 1;

    while (std::getline(shaderSource, curLine)) {
        std::string::size_type posInclude = curLine.find("#include");
        std::string::size_type posComment = curLine.find("//");

        if (posInclude != std::string::npos &&
            (posComment == std::string::npos || posComment > posInclude)) {
            std::string::size_type pathBegin = curLine.find("\"", posInclude + 1);
            std::string::size_type pathEnd = curLine.find("\"", pathBegin + 1);
            std::string includeFileName(curLine, pathBegin + 1, pathEnd - pathBegin - 1);
            bool includeFileFound = false;
            std::vector<std::string> shaderSearchPaths =
                ShaderManager::getPtr()->getShaderSearchPaths();

            for (auto& shaderSearchPath : shaderSearchPaths) {
                if (filesystem::fileExists(shaderSearchPath + "/" + includeFileName)) {
                    includeFileName = shaderSearchPath + "/" + includeFileName;
                    auto it = std::find(includeFileNames_.begin(), includeFileNames_.end(),
                                        includeFileName);
                    if (it == includeFileNames_.end()) {  // Only include files once.
                        includeFileNames_.push_back(includeFileName);
                        std::ifstream includeFileStream(includeFileName.c_str());
                        std::stringstream buffer;
                        buffer << includeFileStream.rdbuf();
                        std::string includeSource = buffer.str();

                        if (!includeSource.empty())
                            result << embeddIncludes(includeSource, includeFileName);
                    }
                    includeFileFound = true;
                    break;
                }
            }

            if (!includeFileFound) {
                std::string fileresourcekey = includeFileName;
                std::replace(fileresourcekey.begin(), fileresourcekey.end(), '/', '_');
                std::replace(fileresourcekey.begin(), fileresourcekey.end(), '.', '_');
                std::string includeSource =
                    ShaderManager::getPtr()->getShaderResource(fileresourcekey);
                if (!includeSource.empty()) {
                    result << embeddIncludes(includeSource, includeFileName);
                    includeFileFound = true;
                }
            }

            if (!includeFileFound) {
                throw OpenGLException("Include file " + includeFileName +
                                      " not found in shader search paths.", IvwContext);
            }

        } else {
            result << curLine << "\n";
            lineNumberResolver_.push_back(
                std::pair<std::string, unsigned int>(fileName, localLineNumber));
        }

        localLineNumber++;
    }

    return result.str();
}

void ShaderObject::loadSource(std::string fileName) {
    source_ = "";

    if (!fileName.empty()) {
        absoluteFileName_ = "";

        if (filesystem::fileExists(fileName)) {  // Absolute path was given
            absoluteFileName_ = fileName;
        } else {
            // Search in include directories added by modules
            std::vector<std::string> shaderSearchPaths =
                ShaderManager::getPtr()->getShaderSearchPaths();

            for (auto& shaderSearchPath : shaderSearchPaths) {
                if (filesystem::fileExists(shaderSearchPath + "/" + fileName)) {
                    absoluteFileName_ = shaderSearchPath + "/" + fileName;
                    break;
                }
            }
        }

        if (!absoluteFileName_.empty()) { // Load file
            try {
                TextFileReader fileReader(absoluteFileName_);
                source_ = fileReader.read();
            } catch (std::ifstream::failure&) {
                throw OpenGLException("Cound not read shader file: " + fileName, IvwContext);
            }
        } else { // try finding a Shader Resource
            std::string fileresourcekey = fileName;
            std::replace(fileresourcekey.begin(), fileresourcekey.end(), '/', '_');
            std::replace(fileresourcekey.begin(), fileresourcekey.end(), '.', '_');
            source_ = ShaderManager::getPtr()->getShaderResource(fileresourcekey);
        }
    } else {
        throw OpenGLException("Shader file: " + fileName + " not found in shader search paths.", IvwContext);
    }
}

void ShaderObject::upload() {
    const char* source = sourceProcessed_.c_str();
    glShaderSource(id_, 1, &source, nullptr);
    LGL_ERROR;
}

std::string ShaderObject::getShaderInfoLog() {
    GLint maxLogLength;
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &maxLogLength);
    LGL_ERROR;

    if (maxLogLength > 1) {
        GLchar* shaderInfoLog = new GLchar[maxLogLength];
        ivwAssert(shaderInfoLog != nullptr, "could not allocate memory for compiler log");
        GLsizei logLength;
        glGetShaderInfoLog(id_, maxLogLength, &logLength, shaderInfoLog);
        std::istringstream shaderInfoLogStr(shaderInfoLog);
        delete[] shaderInfoLog;
        return shaderInfoLogStr.str();
    } else return "";
}

int ShaderObject::getLogLineNumber(const std::string& compileLogLine) {
    int result = -1;
    std::istringstream input(compileLogLine);
    int num;

    if (input>>num) {
        char c;

        if (input>>c && c=='(') {
            if (input>>result) {
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

void ShaderObject::compile() {
    glCompileShader(id_);
    GLint compiledOk = 0;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &compiledOk);

    if (!compiledOk) {
        std::string compilerLog = getShaderInfoLog();
        compilerLog = reformatShaderInfoLog(compilerLog);
        throw OpenGLException(compilerLog, IvwContext);
    }
}

void ShaderObject::addShaderDefine(std::string name, std::string value) {
    shaderDefines_[name] = value;
}

void ShaderObject::removeShaderDefine(std::string name) {
    shaderDefines_.erase(name);
}

bool ShaderObject::hasShaderDefine(const std::string& name) const {
    return shaderDefines_.find(name) != shaderDefines_.end();
}

void ShaderObject::clearShaderDefines() {
    shaderDefines_.clear();
}

void ShaderObject::addShaderExtension(std::string extName, bool enabled) {
    shaderExtensions_[extName] = enabled;
}

void ShaderObject::removeShaderExtension(std::string extName) {
    shaderExtensions_.erase(extName);
}

bool ShaderObject::hasShaderExtension(const std::string& extName) const {
    return shaderExtensions_.find(extName) != shaderExtensions_.end();
}

void ShaderObject::clearShaderExtensions() {
    shaderExtensions_.clear();
}

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

void ShaderObject::clearOutDeclarations() {
    outDeclarations_.clear();
}

std::string ShaderObject::print(bool showSource) const {
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
}

} // namespace
