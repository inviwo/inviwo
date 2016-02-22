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

#ifndef IVW_SHADEROBJECT_H
#define IVW_SHADEROBJECT_H

#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglmoduledefine.h>

namespace inviwo {

class ShaderResource;

class IVW_MODULE_OPENGL_API ShaderObject {
public:
    enum class Compile { Yes, No };
    enum class Error { Warn, Throw };

    ShaderObject(GLenum shaderType, std::string fileName, Compile compile = Compile::Yes,
                 Error error = Error::Warn);
    ShaderObject(GLenum shaderType, std::string fileName, bool compileShader = true);
    ShaderObject(const ShaderObject& rhs);
    ShaderObject& operator=(const ShaderObject& that);

    ~ShaderObject();

    GLuint getID() const { return id_; }
    std::string getFileName() const { return fileName_; }
    ShaderResource* getResource() const {return resource_; }
    const std::vector<ShaderResource*>& getResources() const { return includeResources_; }
    GLenum getShaderType() const { return shaderType_; }
    void setError(Error error);
    Error getError() const;

    void loadSource(std::string fileName);
    void preprocess();
    void upload();
    void compile();

    bool isReady() const;

    std::string getShaderInfoLog() const;

    void build();
    void rebuild();

    void addShaderDefine(std::string name, std::string value = "");
    void removeShaderDefine(std::string name);
    bool hasShaderDefine(const std::string& name) const;
    void clearShaderDefines();

    void addShaderExtension(std::string extName, bool enabled);
    void removeShaderExtension(std::string extName);
    bool hasShaderExtension(const std::string& extName) const;
    void clearShaderExtensions();

    /**
     * \brief adds an additional output specifier to the fragment shader
     *  The given name will be added as
     *  \code{.cpp}
     *     out vec4 <name>;
     *  \encode
     *  If location index is positive, the output will be
     *  \code{.cpp}
     *     layout(location = <location>) out vec4 <name>;
     *  \encode
     *
     *  Location indices can be reused several times unless more than
     *  one output specifier is used.
     *
     * @param name      identifier of the output specifier
     * @param location  index location of the output (< MAX_RENDER_TARGETS)
     */
    void addOutDeclaration(std::string name, int location = -1);
    void clearOutDeclarations();

    std::string print(bool showSource = false, bool preprocess = true) const;

private:
    void initialize(Compile compile);
    std::string getDefines();
    std::string getOutDeclarations();
    std::string getIncludes(ShaderResource* resource);

    int getLogLineNumber(const std::string& compileLogLine);
    std::string reformatShaderInfoLog(const std::string compileLog);

    // state variables
    GLenum shaderType_;
    GLuint id_;
    std::string fileName_;
    std::vector<std::pair<std::string, int> > outDeclarations_;

    using ShaderDefines = std::map<std::string, std::string>;
    ShaderDefines shaderDefines_;

    using ShaderExtensions = std::map<std::string, bool>;  // extension name, enable flag
    ShaderExtensions shaderExtensions_;

    Error error_;

    // derived variables
    ShaderResource* resource_ = nullptr;
    std::string sourceProcessed_;
    std::vector<ShaderResource*> includeResources_;
    std::vector<std::pair<std::string, unsigned int> > lineNumberResolver_;
};

}  // namespace

#endif  // IVW_SHADEROBJECT_H
